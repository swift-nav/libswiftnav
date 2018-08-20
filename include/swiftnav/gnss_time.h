/*
 * Copyright (C) 2013 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_GNSS_TIME_H
#define LIBSWIFTNAV_GNSS_TIME_H

#include <stdbool.h>
#include <time.h>

#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of days in a common (non-leap) year. */
#define YEAR_DAYS (365)

/** Number of days in a leap year. */
#define LEAP_YEAR_DAYS (YEAR_DAYS + 1)

/** Number of days in a week. */
#define WEEK_DAYS 7

/** Day numbers start from 1 (Monday). */
#define WEEK_DAY_MIN 1

/** Number of months in a year. */
#define YEAR_MONTHS 12

/** Days in (leap) year 1980 since GPS epoch Jan 6th */
#define YEAR_1980_GPS_DAYS 361

/** Year of GPS epoch */
#define GPS_EPOCH_YEAR 1980

/** UTC (SU) offset (hours) */
#define UTC_SU_OFFSET 3

/** Number of seconds in a minute. */
#define MINUTE_SECS 60

/** Number of minutes in an hour. */
#define HOUR_MINUTES 60

/** Number of seconds in an hour. */
#define HOUR_SECS (MINUTE_SECS * HOUR_MINUTES)

/** Number of hours in a day. */
#define DAY_HOURS 24

/** Number of seconds in a day. */
#define DAY_SECS (DAY_HOURS * HOUR_MINUTES * MINUTE_SECS)

/** Number of seconds in a week. */
#define WEEK_SECS (WEEK_DAYS * DAY_SECS)

/** Number of microseconds in a second. */
#define SECS_US 1e6

/** Number of milliseconds in a second. */
#define SECS_MS 1000

/** Number of milliseconds in a week */
#define WEEK_MS (SECS_MS * WEEK_SECS)

/** Number of days in four years. */
#define FOUR_YEARS_DAYS (3 * YEAR_DAYS + LEAP_YEAR_DAYS)

/** Number of days in 100 years. */
#define HUNDRED_YEARS_DAYS (24 * FOUR_YEARS_DAYS + 4 * YEAR_DAYS)

/** Number of days in 400 years. */
#define FOUR_HUNDRED_YEARS_DAYS (3 * HUNDRED_YEARS_DAYS + 25 * FOUR_YEARS_DAYS)

/** Number of rollovers in the 10-bit broadcast GPS week number.
 * Update on next rollover on April 7, 2019.
 * \todo Detect and handle rollover more gracefully. */
#define GPS_WEEK_CYCLE 1

/** The GPS week reference number. The current GPS WN is always assumed to be
 * later than this reference number. It will keep the WN calculated from the
 * truncated 10-bit broadcast WN valid for ~20 years after this week.
 *
 * Current week number is set to 20 December 2015.
 *
 * TODO: update this from build date */
#define GPS_WEEK_REFERENCE 1876

/** The GPS week number at which we won't be able to figure out what
    time it is with the current reference. */
#define GPS_MAX_WEEK 2899

/** Unix timestamp of the GPS epoch 1980-01-06 00:00:00 UTC */
#define GPS_EPOCH 315964800

/** Modified Julian days of the GPS epoch 1980-01-06 00:00:00 UTC */
#define MJD_JAN_6_1980 44244

/** Modified Julian days of 1601-01-01 */
#define MJD_JAN_1_1601 -94187

#define WN_UNKNOWN -1
#define TOW_UNKNOWN -1

/** GLONASS minimum day number */
#define GLO_NT_0_FLOOR 1
/** GLONASS maximum days after the first year of the cycle*/
#define GLO_NT_0_CEILING LEAP_YEAR_DAYS /* 366 */
/** GLONASS maximum days after the second year of the cycle*/
#define GLO_NT_1_CEILING (GLO_NT_0_CEILING + YEAR_DAYS) /* 731 */
/** GLONASS maximum days after the third year of the cycle*/
#define GLO_NT_2_CEILING (GLO_NT_1_CEILING + YEAR_DAYS) /* 1096 */
/** GLONASS maximum days after the fourth year of the cycle*/
#define GLO_NT_3_CEILING (GLO_NT_2_CEILING + YEAR_DAYS) /* 1461 */

/** GLONASS number of the first 4-year cycle */
#define GLO_N4_MIN 1
/** GLONASS maximum number of 4-year cycles (rolls over on 1st Jan 2120) */
#define GLO_N4_MAX 31

/** Start of GLONASS time 1st Jan 1996 */
#define GLO_EPOCH_YEAR 1996
/** Start of GLONASS time in GPS time (31st Dec 1995 21:00:10) */
#define GLO_EPOCH_WN 834
#define GLO_EPOCH_TOW 75610.0

/** Constant difference of Beidou time from GPS time */
#define BDS_WEEK_TO_GPS_WEEK 1356
#define BDS_SECOND_TO_GPS_SECOND 14

/** Structure representing a GPS time. */
typedef struct __attribute__((packed)) {
  double tow; /**< Seconds since the GPS start of week. */
  s16 wn;     /**< GPS week number. */
} gps_time_t;

/** Structure representing a GLO epoch.
    Please refer to GLO ICD v5.1 2008 for details */
typedef struct {
  /** Day number within the four-year interval [1-1461].
      Comes from the field NT in the GLO string 4. */
  u16 nt;

  /** Four-year interval number starting from 1996 [1- ].
      Comes from the field N4 in the GLO string 5. */
  u8 n4;

  /** h/m/s come either from the field tb in the GLO string 2
      or the field tk in the GLO string 1 */
  u8 h;     /**< Hours [0-24] */
  u8 m;     /**< Minutes [0-59] */
  double s; /**< Seconds [0-60] */
} glo_time_t;

#define GPS_TIME_UNKNOWN ((gps_time_t){TOW_UNKNOWN, WN_UNKNOWN})

/** IS-GPS-200H Table 20-IX: 602112 [s] */
#define GPS_LNAV_UTC_MAX_TOT 602112
/** IS-GPS-200H Table 20-IX: 1 [days] */
#define GPS_LNAV_UTC_MIN_DN 1
/** IS-GPS-200H Table 20-IX: 7 [days] */
#define GPS_LNAV_UTC_MAX_DN 7

/** IS-GPS-200H Table 20-IX: 2^12 */
#define GPS_LNAV_UTC_SF_TOT 4096
/** IS-GPS-200H Table 20-IX: 2^-30 */
#define GPS_LNAV_UTC_SF_A0 C_1_2P30
/** IS-GPS-200H Table 20-iX: 2^-50 */
#define GPS_LNAV_UTC_SF_A1 C_1_2P50

/** IS-GPS-200H Table 30-IX: 2^4 */
#define GPS_CNAV_UTC_SF_TOT POW_TWO_4
/** IS-GPS-200H Table 30-IX: 2^-35 */
#define GPS_CNAV_UTC_SF_A0 C_1_2P35
/** IS-GPS-200H Table 30-IX: 2^-51 */
#define GPS_CNAV_UTC_SF_A1 C_1_2P51
/** IS-GPS-200H Table 30-IX: 2^-68 */
#define GPS_CNAV_UTC_SF_A2 C_1_2P68

/** Structure containing GPS UTC correction parameters. */
typedef struct {
  double a0;        /**< Modulo 1 sec offset from GPS to UTC [s] */
  double a1;        /**< Drift of time offset from GPS to UTC [s/s] */
  double a2;        /**< Drift rate correction from GPS to UTC [s/s] */
  gps_time_t tot;   /**< Reference time of UTC parameters. */
  gps_time_t t_lse; /**< Time of leap second event. */
  s8 dt_ls;         /**< Leap second delta from GPS to UTC before LS event [s]*/
  s8 dt_lsf;        /**< Leap second delta from GPS to UTC after LS event [s] */
} utc_params_t;

/** Structure representing UTC time. */
typedef struct {
  u16 year;      /**< Number of years AD. In four digit format. */
  u16 year_day;  /**< Day of the year (1 - 366). */
  u8 month;      /**< Month of the year (1 - 12). 1 = January, 12 = December. */
  u8 month_day;  /**< Day of the month (1 - 31). */
  u8 week_day;   /**< Day of the week (1 - 7). 1 = Monday, 7 = Sunday. */
  u8 hour;       /**< Minutes of the hour (0 - 59). */
  u8 minute;     /**< Minutes of the hour (0 - 59). */
  u8 second_int; /**< Integer part of seconds of the minute (0 - 60). */
  double second_frac; /**< Fractional part of seconds (0 - .99...). */
} utc_tm;

/* Start times of UTC leap second events given in GPS time {wn, tow, gpst-utc}
 * The leap second event lasts for one second from the start time, and after
 * that the new offset is in effect. */
static const s32 utc_leaps[][3] = {
    {77, 259200, 1},    /* 01-07-1981 */
    {129, 345601, 2},   /* 01-07-1982 */
    {181, 432002, 3},   /* 01-07-1983 */
    {286, 86403, 4},    /* 01-07-1985 */
    {416, 432004, 5},   /* 01-01-1988 */
    {521, 86405, 6},    /* 01-01-1990 */
    {573, 172806, 7},   /* 01-01-1991 */
    {651, 259207, 8},   /* 01-07-1992 */
    {703, 345608, 9},   /* 01-07-1993 */
    {755, 432009, 10},  /* 01-07-1994 */
    {834, 86410, 11},   /* 01-01-1996 */
    {912, 172811, 12},  /* 01-07-1997 */
    {990, 432012, 13},  /* 01-01-1999 */
    {1356, 13, 14},     /* 01-01-2006 */
    {1512, 345614, 15}, /* 01-01-2009 */
    {1695, 15, 16},     /* 01-07-2012 */
    {1851, 259216, 17}, /* 01-07-2015 */
    {1930, 17, 18}      /* 01-01-2017 */
};

bool unix_time_valid(const time_t *t);
bool gps_time_valid(const gps_time_t *t);

bool gps_current_time_valid(const gps_time_t *t);

void normalize_gps_time(gps_time_t *t_gps);
void unsafe_normalize_gps_time(gps_time_t *t_gps);

time_t gps2time(const gps_time_t *t_gps);
gps_time_t time2gps_t(const time_t t_unix);
void gps2utc(const gps_time_t *t, utc_tm *u, const utc_params_t *p);
void make_utc_tm(const gps_time_t *t, utc_tm *u);

bool gpstime_in_range(const gps_time_t *begin,
                      const gps_time_t *end,
                      const gps_time_t *t);
double gpsdifftime(const gps_time_t *end, const gps_time_t *beginning);
void add_secs(gps_time_t *time, double secs);
void gps_time_match_weeks(gps_time_t *t, const gps_time_t *ref);
u16 gps_adjust_week_cycle(u16 wn_raw, u16 wn_ref);
u16 gps_adjust_week_cycle256(u16 wn_raw, u16 wn_ref);

static inline bool is_leap_year(s32 year) {
  return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}
u8 days_in_month(u16 year, u8 month);

gps_time_t glo2gps(const glo_time_t *glo_t, const utc_params_t *utc_params);
glo_time_t gps2glo(const gps_time_t *gps_t, const utc_params_t *utc_params);

double utc2gmst(utc_tm u, double ut1_utc);

u16 gps2doy(const gps_time_t *t);

bool decode_utc_parameters(const u32 words[8], utc_params_t *u);

double date2mjd(s32 year, s32 month, s32 day, s32 hour, s32 min, double sec);
void mjd2date(double mjd,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec);
utc_tm mjd2utc(double mjd);
double utc2mjd(const utc_tm *utc_time);
utc_tm date2utc(s32 year, s32 month, s32 day, s32 hour, s32 min, double sec);
void utc2date(const utc_tm *utc_time,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec);
gps_time_t mjd2gps(double mjd);
double gps2mjd(const gps_time_t *gps_time);
gps_time_t date2gps(
    s32 year, s32 month, s32 day, s32 hour, s32 min, double sec);
void gps2date(const gps_time_t *gps_time,
              s32 *year,
              s32 *month,
              s32 *day,
              s32 *hour,
              s32 *min,
              double *sec);

/* GPS-UTC time offset at given GPS time */
double get_gps_utc_offset(const gps_time_t *t, const utc_params_t *p);
/* UTC-GPS time offset at given UTC time */
double get_utc_gps_offset(const gps_time_t *utc_time, const utc_params_t *p);
/* is the GPS time within a leap second event (ie 23:59:60.dddd) */
bool is_leap_second_event(const gps_time_t *t, const utc_params_t *p);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_GNSS_TIME_H */
