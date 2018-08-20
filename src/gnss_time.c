/*
 * Copyright (C) 2013, 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <swiftnav/almanac.h>
#include <swiftnav/bits.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>

/** \defgroup time Time functions
 * Functions to handle GPS and UTC time values.
 * \{ */

/** Tell whether a `gps_time_t` struct is a valid GPS time.
 *
 * \param t GPS time struct.
 * \return true if the time is valid
 */
bool gps_time_valid(const gps_time_t *t) {
  return isfinite(t->tow) && t->tow >= 0 && t->tow < WEEK_SECS && t->wn >= 0;
}

/** Tell whether a `time_t` struct is a valid Unix time.
 *
 * \param t Unix time struct.
 * \return true if the time is valid
 */
bool unix_time_valid(const time_t *t) {
  return (isfinite((double)*t) && ((*t) >= 0) &&
          ((*t) > (GPS_WEEK_REFERENCE * WEEK_SECS) + GPS_EPOCH));
}

/** Tell whether a `gps_time_t` struct is valid and within current GPS week
 * cycle.
 *
 * \param t GPS time struct.
 * \return true if the time is valid
 */
bool gps_current_time_valid(const gps_time_t *t) {
  return gps_time_valid(t) && t->wn >= GPS_WEEK_REFERENCE &&
         t->wn < GPS_MAX_WEEK;
}

/** Normalize a `gps_time_t` GPS time struct in place.
 * Ensures that the time of week is greater than zero and less than one week by
 * wrapping and adjusting the week number accordingly.
 *
 * \param t GPS time struct.
 */
void normalize_gps_time(gps_time_t *t) {
  assert(t->wn >= WN_UNKNOWN);

  if (t->wn == WN_UNKNOWN) {
    /* don't touch week number if it is unknown */
    while (t->tow < 0) {
      t->tow += WEEK_SECS;
    }
    while (t->tow >= WEEK_SECS) {
      t->tow -= WEEK_SECS;
    }
    return;
  }

  unsafe_normalize_gps_time(t);
  assert(gps_time_valid(t));
}

/** Normalize a `gps_time_t` GPS time struct in place.
 * Ensures that the time of week is greater than zero and less than one week by
 * wrapping and adjusting the week number accordingly. Doesn't check for
 * positive week number (since some applications pass in times before the
 * start of GPS time in 1980)
 *
 * \param t GPS time struct.
 */
void unsafe_normalize_gps_time(gps_time_t *t) {
  while (t->tow < 0) {
    t->tow += WEEK_SECS;
    t->wn -= 1;
  }

  while (t->tow >= WEEK_SECS) {
    t->tow -= WEEK_SECS;
    t->wn += 1;
  }
}

/** Convert a GPS time stamp into UTC time structure.
 * Use the hard-coded leap second table.
 *
 * \param[in]  t gps_time_t structure
 * \param[out] u utc_tm structure
 * \param[in]  p pointer to UTC parameters (optional)
 */
void gps2utc(const gps_time_t *t, utc_tm *u, const utc_params_t *p) {
  assert(gps_time_valid(t));
  assert(u != NULL);

  /* Get the UTC offset at the time we're converting */
  double dt_utc = get_gps_utc_offset(t, p);
  /* Is it during a (positive) leap second event */
  bool is_lse = is_leap_second_event(t, p);

  double tow_utc = t->tow - dt_utc;

  if (is_lse) {
    /* positive leap second event ongoing, so we are at 23:59:60.xxxx
     * subtract one second from time for now to make the conversion
     * into yyyy/mm/dd HH:MM:SS.sssss format, and add it back later */
    tow_utc = tow_utc - 1;
  }

  gps_time_t t_u = {.wn = t->wn, .tow = tow_utc};
  normalize_gps_time(&t_u);

  /* break the time into components */
  make_utc_tm(&t_u, u);

  if (is_lse) {
    assert(u->hour == 23);
    assert(u->minute == 59);
    assert(u->second_int == 59);
    /* add the extra second back in*/
    u->second_int += 1;
  }
}

/** Break a (wn, tow) time stamp into components without considering leap
 * seconds.
 *
 * \param[in]  t gps_time_t structure
 * \param[out] u utc_tm structure
 *
 */
void make_utc_tm(const gps_time_t *t, utc_tm *u) {
  /* see http://www.ngs.noaa.gov/gps-toolbox/bwr-c.txt */

  /* seconds of the day */
  double t_utc = fmod(t->tow, DAY_SECS);

  /* Convert this into hours, minutes and seconds */
  u32 second_int = floor(t_utc);        /* The integer part of the seconds */
  u->second_frac = fmod(t_utc, 1.0);    /* The fractional part of the seconds */
  u->hour = second_int / HOUR_SECS;     /* The hours (1 - 23) */
  second_int -= u->hour * HOUR_SECS;    /* Remove the hours from seconds */
  u->minute = second_int / MINUTE_SECS; /* The minutes (1 - 59) */
  second_int -= u->minute * MINUTE_SECS; /* Remove the minutes from seconds */
  u->second_int = second_int;            /* The seconds (1 - 60) */

  /* Calculate the years */

  /* Days from 1 Jan 1980. GPS epoch is 6 Jan 1980 */
  u32 modified_julian_days =
      MJD_JAN_6_1980 + t->wn * 7 + floor(t->tow / DAY_SECS);
  u32 days_since_1601 = modified_julian_days - MJD_JAN_1_1601;

  /* Calculate the number of leap years */
  u16 num_400_years = days_since_1601 / FOUR_HUNDRED_YEARS_DAYS;
  u32 days_left = days_since_1601 - num_400_years * FOUR_HUNDRED_YEARS_DAYS;
  u16 num_100_years = days_left / HUNDRED_YEARS_DAYS -
                      days_left / (FOUR_HUNDRED_YEARS_DAYS - 1);
  days_left -= num_100_years * HUNDRED_YEARS_DAYS;
  u16 num_4_years = days_left / FOUR_YEARS_DAYS;
  days_left -= num_4_years * FOUR_YEARS_DAYS;
  u16 num_non_leap_years =
      days_left / YEAR_DAYS - days_left / (FOUR_YEARS_DAYS - 1);

  /* Calculate the total number of years from 1980 */
  u->year = 1601 + num_400_years * 400 + num_100_years * 100 + num_4_years * 4 +
            num_non_leap_years;

  /* Calculate the month of the year */

  /* Calculate the day of the current year */
  u->year_day = days_left - num_non_leap_years * YEAR_DAYS + 1;

  /* Work out if it is currently a leap year, 0 = no, 1 = yes` */
  u8 leap_year = (u8)is_leap_year(u->year);

  /* Roughly work out the month number */
  u8 month_guess = u->year_day * 0.032;

  /* Lookup table of the total number of days in the year after each month */
  /* First row is for a non-leap year, second row is for a leap year */
  static u16 days_after_month[2][13] = {
      {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}};

  /* Check if our guess was out, and what the correction is, */
  /* 0 = correct, 1 = wrong */
  u8 month_correction =
      (u->year_day - days_after_month[leap_year][month_guess + 1]) > 0;

  /* Calculate the corrected number of months */
  u->month = month_guess + month_correction + 1;

  /* Calculate the day of the month */
  u->month_day =
      u->year_day - days_after_month[leap_year][month_guess + month_correction];

  /* Calculate the day of the week. 1 Jan 1601 was a Monday */
  u->week_day = days_since_1601 % 7 + 1;
}

/** Convert a `gps_time_t` GPS time to a Unix `time_t`.
 * \note Adjusts for leap seconds using the hard-coded table.
 * \note Deprecated, use gps2utc instead
 *
 * \param t_gps GPS time struct.
 * \return Unix time.
 */
time_t gps2time(const gps_time_t *t_gps) {
  assert(gps_time_valid(t_gps));

  time_t t = GPS_EPOCH - get_gps_utc_offset(t_gps, NULL);

  t += WEEK_SECS * t_gps->wn;
  t += (s32)t_gps->tow;

  return t;
}

/** Convert a Unix `time_t` to a `gps_time_t` GPS time structure.
 *  Both input and output are assumed to be in GPS Time scale so no leap second
 * is applied
 * \param t_unix  Unix time.
 * \return GPS time struct.
 */
gps_time_t time2gps_t(const time_t t_unix) {
  assert(unix_time_valid(&t_unix));
  gps_time_t t_gps;
  t_gps.wn = (s16)((t_unix - GPS_EPOCH) / WEEK_SECS);
  t_gps.tow = t_unix - GPS_EPOCH - WEEK_SECS * t_gps.wn;
  assert(gps_time_valid(&t_gps));
  return t_gps;
}

/** Checks if GPS time t is within begin and end */
bool gpstime_in_range(const gps_time_t *bgn,
                      const gps_time_t *end,
                      const gps_time_t *t) {
  assert(bgn);
  assert(bgn->tow != TOW_UNKNOWN);
  assert(bgn->wn != WN_UNKNOWN);
  assert(end);
  assert(end->tow != TOW_UNKNOWN);
  assert(end->wn != WN_UNKNOWN);
  assert(t);
  assert(t->tow != TOW_UNKNOWN);
  assert(t->tow != WN_UNKNOWN);

  double since_bgn_s = gpsdifftime(t, bgn);
  if (since_bgn_s < 0) {
    return false;
  }
  double range_s = gpsdifftime(end, bgn);
  return (since_bgn_s <= range_s);
}

/** Time difference in seconds between two GPS times.
 * If the week number field of either time is unspecified, the result
 * will be as if the week numbers had been chosen for the times to be
 * as close as possible.
 * \param end Higher bound of the time interval whose length is calculated.
 * \param beginning Lower bound of the time interval whose length is
 *                  calculated. If this describes a time point later than end,
 *                  the result is negative.
 * \return The time difference in seconds between `beginning` and `end`.
 */
double gpsdifftime(const gps_time_t *end, const gps_time_t *beginning) {
  double dt = end->tow - beginning->tow;
  if (end->wn == WN_UNKNOWN || beginning->wn == WN_UNKNOWN) {
    /* One or both of the week numbers is unspecified.  Assume times
       are within +/- 0.5 weeks of each other. */
    if (dt > WEEK_SECS / 2) dt -= WEEK_SECS;
    if (dt < -WEEK_SECS / 2) dt += WEEK_SECS;
  } else {
    /* Week numbers were provided - use them. */
    dt += (end->wn - beginning->wn) * WEEK_SECS;
  }
  return dt;
}

/** Add secs seconds to the GPS time.
 * \param time The time to be modified.
 * \param secs Number of seconds to add to the time
 */
void add_secs(gps_time_t *time, double secs) {
  time->tow += secs;
  unsafe_normalize_gps_time(time);
  return;
}

/** Given an unknown week number in t, fill in the week number from ref
 * under the assumption the two times are separated by less than a week.
 *
 * \param t Pointer to GPS time whose week number will be set
 * \param ref Reference GPS time
 */
void gps_time_match_weeks(gps_time_t *t, const gps_time_t *ref) {
  if (ref->wn == WN_UNKNOWN) {
    return;
  }
  t->wn = ref->wn;
  double dt = t->tow - ref->tow;
  if (dt > WEEK_SECS / 2)
    t->wn--;
  else if (dt < -WEEK_SECS / 2)
    t->wn++;

  assert(gps_time_valid(t));
}

/** Adjust the week number of wn_raw to correctly reflect the current week
 * cycle.
 *
 * Assumes the current week number cannot be earlier than the reference WN. So
 * will return the correct WN for at most 20 years after the reference WN.
 *
 * \param wn_raw Raw week number from NAV data stream that is modulo 1024
 * \param wn_ref Reference week number that is from some point in the past
 *
 * \return The absolute week number counted from 1980
 *
 * \sa gps_adjust_week_cycle256
 */
u16 gps_adjust_week_cycle(u16 wn_raw, u16 wn_ref) {
  /* note the week numbers are unsigned so they cannot be WN_UNKNOWN */
  if (wn_raw >= wn_ref) {
    return wn_raw;
  }

  return wn_raw + 1024 * ((wn_ref + 1023 - wn_raw) / 1024);
}

/** Adjust the week number of wn_raw to correctly reflect the current week
 * cycle.
 *
 * Assumes the current week number cannot be earlier than the reference WN. So
 * will return the correct WN for at most 4.83 years after the reference WN.
 *
 * \param wn_raw Raw week number from NAV data stream that is modulo 256
 * \param wn_ref Reference week number that is from some point in the past
 *
 * \return The absolute week number counted from 1980
 *
 * \sa gps_adjust_week_cycle
 */
u16 gps_adjust_week_cycle256(u16 wn_raw, u16 wn_ref) {
  /* note the week numbers are unsigned so they cannot be WN_UNKNOWN */
  if (wn_raw >= wn_ref) {
    return wn_raw;
  }

  return wn_raw + 256 * ((wn_ref + 255 - wn_raw) / 256);
}

/** Transformation of GLONASS-M current data information into gps_time_t.
 *
 *  Reference: GLONASS ICD Edition 5.1 2008
 *
 * \param glo_t pointer to GLO time structure
 * \param utc_params pointer to UTC parameters structure (optional)
 * \return converted gps time
 */

gps_time_t glo2gps(const glo_time_t *glo_t, const utc_params_t *utc_params) {
  u8 year_of_cycle = 0;
  u16 day_of_year = 0;
  u32 glo_year = 0;
  gps_time_t gps_t = GPS_TIME_UNKNOWN;

  if (glo_t->n4 < GLO_N4_MIN || glo_t->n4 > GLO_N4_MAX) {
    return GPS_TIME_UNKNOWN;
  }

  if (glo_t->nt < GLO_NT_0_FLOOR) {
    return GPS_TIME_UNKNOWN;
  } else if (glo_t->nt <= GLO_NT_0_CEILING) {
    year_of_cycle = 1;
    day_of_year = glo_t->nt;
  } else if (glo_t->nt <= GLO_NT_1_CEILING) {
    year_of_cycle = 2;
    day_of_year = glo_t->nt - LEAP_YEAR_DAYS;
  } else if (glo_t->nt <= GLO_NT_2_CEILING) {
    year_of_cycle = 3;
    day_of_year = glo_t->nt - LEAP_YEAR_DAYS - YEAR_DAYS;
  } else if (glo_t->nt <= GLO_NT_3_CEILING) {
    year_of_cycle = 4;
    day_of_year = glo_t->nt - LEAP_YEAR_DAYS - YEAR_DAYS * 2;
  } else {
    return GPS_TIME_UNKNOWN;
  }

  glo_year = GLO_EPOCH_YEAR + 4 * (glo_t->n4 - 1) + (year_of_cycle - 1);

  /* Calculate days since GPS epoch */
  s64 days_gps_epoch = YEAR_1980_GPS_DAYS + day_of_year - 1;
  u32 y = GPS_EPOCH_YEAR;
  while (++y < glo_year) {
    days_gps_epoch += is_leap_year(y) ? LEAP_YEAR_DAYS : YEAR_DAYS;
  }

  double sec = glo_t->s;
  bool is_leap_second = (sec >= 60);
  /* if leap second is ongoing, subtract the extra second and add it back
   * after the conversion */
  if (is_leap_second) {
    sec -= 1.0;
  }

  /* First produce the time stamp in UTC in wn/tow form */
  gps_t.wn = days_gps_epoch / WEEK_DAYS;
  gps_t.tow = (days_gps_epoch % WEEK_DAYS) * DAY_SECS +
              (glo_t->h - UTC_SU_OFFSET) * HOUR_SECS + glo_t->m * MINUTE_SECS +
              sec;

  normalize_gps_time(&gps_t);

  /* offset from UTC time to GPS time */
  double d_utc = get_utc_gps_offset(&gps_t, utc_params);

  /* check if there was a leap second during this week */
  gps_time_t week_start = {.wn = gps_t.wn, .tow = 0.0};

  /* there was a leap second during this week, add that in */
  if (get_utc_gps_offset(&week_start, utc_params) < d_utc - 1) {
    gps_t.tow += 1;
  }

  /* add back the ongoing leap second subtracted earlier */
  if (is_leap_second) {
    gps_t.tow += 1;
  }

  /* convert to GPS time */
  gps_t.tow -= d_utc;
  normalize_gps_time(&gps_t);
  return gps_t;
}

/* Function taken from RTKLib implementation
 * (utc2gmst at line 1435 of rtkcmn.c,
 * see https://github.com/swift-nav/RTKLIB/blob/master/src/rtkcmn.c)
 */
double utc2gmst(utc_tm u, double ut1_utc) {
  double tut0_mjd, tut_2000_mjd;
  double ut, t1, t2, t3, gmst0, gmst;

  tut0_mjd = utc2mjd(&u);
  tut0_mjd += ut1_utc / (double)DAY_SECS;
  tut_2000_mjd = date2mjd(2000, 1, 1, 12, 0, 0.0);
  s32 year, month, day, hour, min;
  double sec;
  mjd2date(tut0_mjd, &year, &month, &day, &hour, &min, &sec);
  tut0_mjd = date2mjd(year, month, day, 0, 0, 0.0);
  ut = hour * HOUR_SECS + min * MINUTE_SECS + sec;
  t1 = (tut0_mjd - tut_2000_mjd) / 36525.0;
  t2 = t1 * t1;
  t3 = t2 * t1;
  gmst0 = 24110.54841 + 8640184.812866 * t1 + 0.093104 * t2 - 6.2E-6 * t3;
  gmst = gmst0 + 1.002737909350795 * ut;

  return fmod(gmst, (double)DAY_SECS) * M_PI /
         (double)(DAY_SECS / 2); /* 0 <= gmst <= 2*PI */
}

/** Transform GPS time into GLONASS time
 *
 * \param gps_t pointer to gps_time_t structure
 * \param utc_params pointer to UTC parameters structure (optional)
 * \return converted GLO time
 */

glo_time_t gps2glo(const gps_time_t *gps_t, const utc_params_t *utc_params) {
  glo_time_t glo_t;
  gps_time_t glo_epoch = {.wn = GLO_EPOCH_WN, .tow = GLO_EPOCH_TOW};

  /* GLO time is not defined before the GLO epoch */
  assert(gpsdifftime(gps_t, &glo_epoch) >= 0);

  /* break the time into an UTC time structure and apply GPS-UTC offset */
  utc_tm u;
  gps2utc(gps_t, &u, utc_params);

  u.hour += UTC_SU_OFFSET;

  /* adding the offset may cause day, month or year to roll over */
  if (u.hour >= DAY_HOURS) {
    u.month_day += 1;
    u.week_day += 1;
    u.year_day += 1;
    u.hour -= DAY_HOURS;
    if (u.week_day > WEEK_DAYS) {
      /* roll the weekday over independent of month or year */
      u.week_day = WEEK_DAY_MIN;
    }
    if (u.month_day > days_in_month(u.year, u.month)) {
      u.month += 1;
      u.month_day = 1;
      if (u.month > YEAR_MONTHS) {
        u.year += 1;
        u.month = 1;
        u.year_day = 1;
      }
    }
  }

  /* number of the 4-year cycle */
  glo_t.n4 = floor((u.year - GLO_EPOCH_YEAR) / 4) + 1;
  /* day number within the cycle */
  glo_t.nt = u.year_day;
  /* add the days from the previous years in this 4-year cycle */
  for (u16 y = GLO_EPOCH_YEAR + (glo_t.n4 - 1) * 4; y < u.year; y++) {
    glo_t.nt += is_leap_year(y) ? LEAP_YEAR_DAYS : YEAR_DAYS;
  }

  glo_t.h = u.hour;
  glo_t.m = u.minute;
  glo_t.s = (double)u.second_int + u.second_frac;

  return glo_t;
}

/** GPS time to day of year.
 * \note Adjusts for leap seconds using the hard-coded table.
 *
 * \param t GPS time
 * \return The day of year (days since Jan 1st)
 */
u16 gps2doy(const gps_time_t *t) {
  utc_tm u;
  gps2utc(t, &u, NULL);
  return u.year_day - 1;
}

/**
 * Helper to sign extend 24-bit value
 *
 * \param[in] arg Unsigned integer
 *
 * \return Sign-extended integer
 */
static inline s32 sign_extend24(u32 arg) {
  return BITS_SIGN_EXTEND_32(24, arg);
}

/**
 * Decodes UTC parameters from GLS LNAV message subframe 4.
 *
 * The method decodes UTC data from GPS LNAV subframe 4 words 6-10.
 *
 * \note Fills out the full time of week from current gps week cycle. Also
 * sets t_lse to the exact GPS time at the start of the leap second event.
 *
 * References:
 * -# IS-GPS-200H, Section 20.3.3.5.1.6
 *
 * \param[in]  words    Subframe 4 page 18.
 * \param[out] u        Destination object.
 *
 * \retval true  UTC parameters have been decoded.
 * \retval false Decoding error.
 */
bool decode_utc_parameters(const u32 words[8], utc_params_t *u) {
  bool retval = false;

  assert(NULL != words);
  assert(NULL != u);

  memset(u, 0, sizeof(*u));

  /* Word 3 bits 1-2: data ID */
  u8 data_id = words[3 - 3] >> (30 - 2) & 0x3;
  /* Word 3 bits 3-8: SV ID */
  u8 sv_id = words[3 - 3] >> (30 - 8) & 0x3F;

  if (GPS_LNAV_ALM_DATA_ID_BLOCK_II == data_id &&
      GPS_LNAV_ALM_SVID_UTC == sv_id) {
    /* Word 6 bits 1-24 */
    u->a1 = sign_extend24(words[6 - 3] >> (30 - 24) & 0xFFFFFF) *
            GPS_LNAV_UTC_SF_A1;
    /* Word 7 bits 1-24 and word 8 bits 1-8 */
    u->a0 = (s32)(((words[7 - 3] >> (30 - 24) & 0xFFFFFF) << 8) |
                  (words[8 - 3] >> (30 - 8) & 0xFF)) *
            GPS_LNAV_UTC_SF_A0;
    /* Word 8 bits 9-16 */
    u8 tot = words[8 - 3] >> (30 - 16) & 0xFF;
    u->tot.tow = tot * GPS_LNAV_UTC_SF_TOT;
    /* Word 8 bits 17-24 */
    u8 wn_t = words[8 - 3] >> (30 - 24) & 0xFF;
    u->tot.wn = gps_adjust_week_cycle256(wn_t, GPS_WEEK_REFERENCE);
    /* Word 9 bits 1-8 */
    u->dt_ls = (s8)(words[9 - 3] >> (30 - 8) & 0xFF);
    /* Word 9 bits 9-16 */
    u8 wn_lsf = words[9 - 3] >> (30 - 16) & 0xFF;
    u->t_lse.wn = gps_adjust_week_cycle256(wn_lsf, GPS_WEEK_REFERENCE);
    /* Word 9 bits 17-24 */
    u8 dn = words[9 - 3] >> (30 - 24) & 0xFF;
    if ((dn < GPS_LNAV_UTC_MIN_DN) || (dn > GPS_LNAV_UTC_MAX_DN)) {
      return false;
    }
    u->t_lse.tow = dn * DAY_SECS;
    normalize_gps_time(&u->t_lse);
    /* Word 10 bits 1-8 */
    u->dt_lsf = (s8)(words[10 - 3] >> (30 - 8) & 0xFF);

    /* t_lse now points to the midnight near the leap second event. Add
     * the current leap second value and polynomial UTC correction to set t_lse
     * to the beginning of the leap second event */
    u->t_lse.tow += u->dt_ls + u->a0 + u->a1 * gpsdifftime(&u->t_lse, &u->tot);
    normalize_gps_time(&u->t_lse);

    retval = true;
  }

  return retval;
}

/* Taken with permission from http://www.leapsecond.com/tools/gpsdate.c */
/*
 * Return Modified Julian Day given calendar year,
 * month (1-12), and day (1-31).
 * - Valid for Gregorian dates from 17-Nov-1858.
 * - Adapted from sci.astro FAQ.
 */
/* NOTE: This function will be inaccurate by up to a second on the day of a leap
 * second. */
double date2mjd(s32 year, s32 month, s32 day, s32 hour, s32 min, double sec) {
  s32 full_days = 367 * year - 7 * (year + (month + 9) / 12) / 4 -
                  3 * ((year + (month - 9) / 7) / 100 + 1) / 4 +
                  275 * month / 9 + day + 1721028 - 2400000;
  double frac_days = (double)hour / (double)DAY_HOURS +
                     (double)min / (double)(DAY_HOURS * HOUR_MINUTES) +
                     sec / (double)DAY_SECS;
  return (double)full_days + frac_days;
}

/* Taken with permission from http://www.leapsecond.com/tools/gpsdate.c */
/*
 * Convert Modified Julian Day to calendar date.
 * - Assumes Gregorian calendar.
 * - Adapted from Fliegel/van Flandern ACM 11/#10 p 657 Oct 1968.
 */
/* NOTE: This function will be inaccurate by up to a second on the day of a leap
 * second. */
void mjd2date(double mjd,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec) {
  s32 J, C, Y, M;

  J = mjd + 2400001 + 68569;
  C = 4 * J / 146097;
  J = J - (146097 * C + 3) / 4;
  Y = 4000 * (J + 1) / 1461001;
  J = J - 1461 * Y / 4 + 31;
  M = 80 * J / 2447;
  *day = J - 2447 * M / 80;
  J = M / 11;
  *month = M + 2 - (12 * J);
  *year = 100 * (C - 49) + Y + J;
  double int_part;
  double frac_part = modf(mjd, &int_part);
  *hour = frac_part * (double)DAY_HOURS;
  *min = (frac_part - (double)(*hour) / (double)(DAY_HOURS)) * DAY_HOURS *
         HOUR_MINUTES;
  *sec = (frac_part - (double)(*hour) / (double)(DAY_HOURS) -
          (double)(*min) / (double)(DAY_HOURS) / (double)(HOUR_MINUTES)) *
         (double)DAY_SECS;
}

/* NOTE: This function will be inaccurate by up to a second on the week of a
 * leap second. */
utc_tm mjd2utc(double mjd) {
  utc_tm ret;
  double utc_days = mjd - MJD_JAN_6_1980;
  gps_time_t utc_time;
  utc_time.wn = utc_days / WEEK_DAYS;
  utc_time.tow = (utc_days - utc_time.wn * WEEK_DAYS) * (double)DAY_SECS;
  make_utc_tm(&utc_time, &ret);
  return ret;
}

/* NOTE: This function will be inaccurate by up to a second on the day of a leap
 * second. */
double utc2mjd(const utc_tm *utc_time) {
  double secs = (double)utc_time->second_int + utc_time->second_frac;
  return date2mjd(utc_time->year,
                  utc_time->month,
                  utc_time->month_day,
                  utc_time->hour,
                  utc_time->minute,
                  secs);
}

/* NOTE: This function will be inaccurate by up to a second on the week of a
 * leap second. */
utc_tm date2utc(s32 year, s32 month, s32 day, s32 hour, s32 min, double sec) {
  double mjd = date2mjd(year, month, day, hour, min, sec);
  return mjd2utc(mjd);
}

void utc2date(const utc_tm *utc_time,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec) {
  *year = utc_time->year;
  *month = utc_time->month;
  *day = utc_time->month_day;
  *hour = utc_time->hour;
  *min = utc_time->minute;
  *sec = (double)utc_time->second_int + utc_time->second_frac;
}

/* NOTE: This function will be inaccurate by up to a second on the week of a
 * leap second. */
gps_time_t mjd2gps(double mjd) {
  double utc_days = mjd - MJD_JAN_6_1980;
  gps_time_t utc_time;
  utc_time.wn = utc_days / WEEK_DAYS;
  utc_time.tow = (utc_days - utc_time.wn * WEEK_DAYS) * (double)DAY_SECS;
  double leap_secs = get_utc_gps_offset(&utc_time, NULL);
  gps_time_t gps_time = utc_time;
  add_secs(&gps_time, -leap_secs);
  return gps_time;
}

/* NOTE: This function will be inaccurate by up to a second on the day of a leap
 * second. */
double gps2mjd(const gps_time_t *gps_time) {
  utc_tm utc_time;
  gps2utc(gps_time, &utc_time, NULL);
  return utc2mjd(&utc_time);
}

/* NOTE: This function will be inaccurate by up to a second on the week of a
 * leap second. */
gps_time_t date2gps(
    s32 year, s32 month, s32 day, s32 hour, s32 min, double sec) {
  return mjd2gps(date2mjd(year, month, day, hour, min, sec));
}

void gps2date(const gps_time_t *gps_time,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec) {
  utc_tm utc_time;
  gps2utc(gps_time, &utc_time, NULL);
  return utc2date(&utc_time, year, month, day, hour, min, sec);
}

/** Return the number of days in given month */
u8 days_in_month(u16 year, u8 month) {
  static u8 days_in_month_lookup[13] = {
      0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && is_leap_year(year)) {
    return 29;
  } else {
    return days_in_month_lookup[month];
  }
}

/** Difference between GPS and UTC time. Use UTC params struct if given,
 * otherwise use the hard-coded table.
 *
 * \param t GPS time
 * \param p UTC parameters structure (optional)
 * \return Difference in seconds
 *
 */
double get_gps_utc_offset(const gps_time_t *t, const utc_params_t *p) {
  /* use utc_params if it is given and valid */
  if (p != NULL && p->t_lse.wn > 0) {
    double dt = gpsdifftime(t, &p->tot);

    /* The polynomial UTC to GPS correction */
    double dt_utc = p->a0 + p->a1 * dt + p->a2 * dt * dt;

    /* the new UTC offset takes effect exactly 1 second after the start of
     * (a positive) leap second event */
    if (gpsdifftime(t, &p->t_lse) >= 1.0) {
      dt_utc += (double)p->dt_lsf;
    } else {
      dt_utc += (double)p->dt_ls;
    }

    return dt_utc;
  }

  /* utc_params not given, so iterate through the leap second table,
   * starting from latest */
  for (s16 i = ARRAY_SIZE(utc_leaps) - 1; i >= 0; i--) {
    gps_time_t t_leap = {.wn = utc_leaps[i][0], .tow = (double)utc_leaps[i][1]};
    /* the UTC offset takes effect exactly 1 second after the start of
     * (a positive) leap second event */
    if (gpsdifftime(t, &t_leap) >= 1.0) {
      return utc_leaps[i][2];
    }
  }

  /* time is before the first known leap second event */
  return 0.0;
}

/** Difference between UTC and GPS time. Use UTC params struct if given,
 * otherwise use the hard-coded table.
 *
 * \param utc_time UTC time in (wn, tow) format
 * \param p UTC parameters structure (optional)
 * \return Difference in seconds
 */
double get_utc_gps_offset(const gps_time_t *utc_time, const utc_params_t *p) {
  /* use utc_params if it is given and valid */
  if (p != NULL && p->t_lse.wn > 0) {
    double dt = gpsdifftime(utc_time, &p->tot) + p->dt_ls;

    /* The polynomial UTC to GPS correction */
    double dt_utc = p->a0 + p->a1 * dt + p->a2 * dt * dt;

    /* the new UTC offset takes effect after the leap second event */
    if (gpsdifftime(utc_time, &p->t_lse) >= -p->dt_ls - dt_utc) {
      dt_utc += (double)p->dt_lsf;
    } else {
      dt_utc += (double)p->dt_ls;
    }

    return -dt_utc;
  }

  for (s16 i = ARRAY_SIZE(utc_leaps) - 1; i >= 0; i--) {
    gps_time_t t_leap = {.wn = utc_leaps[i][0], .tow = (double)utc_leaps[i][1]};
    /* the new UTC offset takes effect after the leap second event */
    if (gpsdifftime(utc_time, &t_leap) >= -utc_leaps[i][2] + 1) {
      return -utc_leaps[i][2];
    }
  }

  /* time is before the first known leap second event */
  return 0.0;
}

/** Find if a UTC leap second event is ongoing. Use the UTC params struct if
 * given, otherwise use the hard-coded table.
 *
 * \param t GPS time
 * \param p UTC parameters structure (optional)
 * \return True if time is within a leap second event
 */
bool is_leap_second_event(const gps_time_t *t, const utc_params_t *p) {
  /* use utc_params if it is given and valid */
  if (p != NULL && p->t_lse.wn > 0) {
    /* the UTC offset takes effect exactly 1 second after the start of
     * the (positive) leap second event */
    double dt = gpsdifftime(t, &p->t_lse);

    if (dt >= 0.0 && dt < 1.0) {
      /* time is during the leap second event */
      return true;
    } else {
      return false;
    }
  }

  /* iterate through the leap second table, starting from latest */
  for (s16 i = ARRAY_SIZE(utc_leaps) - 1; i >= 0; i--) {
    gps_time_t t_leap = {.wn = utc_leaps[i][0], .tow = utc_leaps[i][1]};

    double dt = gpsdifftime(t, &t_leap);

    if (dt > 1.0) {
      /* time is past the last known leap second event */
      return false;
    } else if (dt >= 0.0 && dt < 1.0) {
      /* time is during the leap second event */
      return true;
    }
  }
  /* time is before the first known leap second event */
  return false;
}

/** \} */
