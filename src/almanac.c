/*
 * Copyright (C) 2010,2016 Swift Navigation Inc.
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
#include <string.h>

#include <swiftnav/almanac.h>
#include <swiftnav/bits.h>
#include <swiftnav/constants.h>
#include <swiftnav/coord_system.h>
#include <swiftnav/ephemeris.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/linear_algebra.h>
#include <swiftnav/logging.h>
#include <swiftnav/shm.h>

/** \defgroup almanac Almanac
 * Functions and calculations related to the GPS almanac.
 *
 * \note All positions are referenced to the WGS84 coordinate system.
 * \see coord_system
 * \{ */

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
 * Helper to sign extend 11-bit value
 *
 * \param[in] arg Unsigned integer
 *
 * \return Sign-extended integer
 */
static inline s32 sign_extend11(u32 arg) {
  return BITS_SIGN_EXTEND_32(11, arg);
}

/** Calculate satellite position, velocity and clock offset from SBAS ephemeris.
 *
 * \param a Pointer to an almanac structure for the satellite of interest
 * \param t GPS time at which to calculate the satellite state
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 */
static s8 calc_sat_state_xyz_almanac(const almanac_t *a,
                                     const gps_time_t *t,
                                     double pos[3],
                                     double vel[3],
                                     double acc[3],
                                     double *clock_err,
                                     double *clock_rate_err) {
  ephemeris_t e;
  memset(&e, 0, sizeof(e));
  e.sid = a->sid;
  e.toe = a->toa;
  e.ura = a->ura;
  e.fit_interval = a->fit_interval;
  e.valid = a->valid;
  e.health_bits = a->health_bits;
  memcpy(e.xyz.pos, a->xyz.pos, sizeof(e.xyz.pos));
  memcpy(e.xyz.vel, a->xyz.vel, sizeof(e.xyz.vel));
  memcpy(e.xyz.acc, a->xyz.acc, sizeof(e.xyz.acc));
  u8 iode;
  u16 iodc;

  return calc_sat_state(
      &e, t, pos, vel, acc, clock_err, clock_rate_err, &iodc, &iode);
}

/** Calculate satellite position, velocity and clock offset from GPS ephemeris.
 *
 * References:
 *   -# IS-GPS-200D, Section 20.3.3.5.2.1 and Table 20-VI
 *
 * \param a Pointer to an almanac structure for the satellite of interest
 * \param t GPS time at which to calculate the satellite state
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 *
 * \return  0 on success,
 *         -1 if almanac is not valid or too old
 */
static s8 calc_sat_state_kepler_almanac(const almanac_t *a,
                                        const gps_time_t *t,
                                        double pos[3],
                                        double vel[3],
                                        double acc[3],
                                        double *clock_err,
                                        double *clock_rate_err) {
  ephemeris_t e;
  memset(&e, 0, sizeof(e));
  e.sid = a->sid;
  e.toe = a->toa;
  e.ura = a->ura;
  e.fit_interval = a->fit_interval;
  e.valid = a->valid;
  e.health_bits = a->health_bits;
  e.kepler.m0 = a->kepler.m0;
  e.kepler.ecc = a->kepler.ecc;
  e.kepler.sqrta = a->kepler.sqrta;
  e.kepler.omega0 = a->kepler.omega0;
  e.kepler.omegadot = a->kepler.omegadot;
  e.kepler.w = a->kepler.w;
  e.kepler.inc = a->kepler.inc;
  e.kepler.af0 = a->kepler.af0;
  e.kepler.af1 = a->kepler.af1;
  e.kepler.toc = a->toa;
  u16 iodc;
  u8 iode;

  return calc_sat_state(
      &e, t, pos, vel, acc, clock_err, clock_rate_err, &iodc, &iode);
}

/** Calculate satellite position, velocity and clock offset from almanac.
 *
 * Dispatch to internal function for Kepler/XYZ almanac depending on
 * constellation.
 *
 * \param a Pointer to an almanac structure for the satellite of interest
 * \param t GPS time at which to calculate the satellite state
 * \param pos Array into which to write calculated satellite position [m]
 * \param vel Array into which to write calculated satellite velocity [m/s]
 * \param clock_err Pointer to where to store the calculated satellite clock
 *                  error [s]
 * \param clock_rate_err Pointer to where to store the calculated satellite
 *                       clock error [s/s]
 *
 * \return  0 on success,
 *         -1 if almanac is not valid or too old
 */
s8 calc_sat_state_almanac(const almanac_t *a,
                          const gps_time_t *t,
                          double pos[3],
                          double vel[3],
                          double acc[3],
                          double *clock_err,
                          double *clock_rate_err) {
  switch (sid_to_constellation(a->sid)) {
    case CONSTELLATION_GPS:
      return calc_sat_state_kepler_almanac(
          a, t, pos, vel, acc, clock_err, clock_rate_err);
    case CONSTELLATION_SBAS:
      return calc_sat_state_xyz_almanac(
          a, t, pos, vel, acc, clock_err, clock_rate_err);
      break;
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    case CONSTELLATION_GLO:
    case CONSTELLATION_BDS:
    case CONSTELLATION_GAL:
    case CONSTELLATION_QZS:
    default:
      assert(!"Unsupported constellation");
      return -1;
  }
}

/** Calculate the azimuth and elevation of a satellite from a reference
 * position given the satellite almanac.
 *
 * \param a  Pointer to an almanac structure for the satellite of interest.
 * \param t    GPS time at which to calculate the az/el.
 * \param ref  ECEF coordinates of the reference point from which the azimuth
 *             and elevation is to be determined, passed as [X, Y, Z], all in
 *             meters.
 * \param az   Pointer to where to store the calculated azimuth output [rad].
 * \param el   Pointer to where to store the calculated elevation output [rad].
 * \return  0 on success,
 *         -1 if almanac is not valid or too old
 */
s8 calc_sat_az_el_almanac(const almanac_t *a,
                          const gps_time_t *t,
                          const double ref[3],
                          double *az,
                          double *el) {
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err, clock_rate_err;
  s8 ret = calc_sat_state_almanac(
      a, t, sat_pos, sat_vel, sat_acc, &clock_err, &clock_rate_err);
  if (ret != 0) {
    return ret;
  }
  wgsecef2azel(sat_pos, ref, az, el);

  return 0;
}

/** Calculate the Doppler shift of a satellite as observed at a reference
 * position given the satellite almanac.
 *
 * \param a  Pointer to an almanac structure for the satellite of interest.
 * \param t    GPS time at which to calculate the az/el.
 * \param ref  ECEF coordinates of the reference point from which the
 *             Doppler is to be determined, passed as [X, Y, Z], all in
 *             meters.
 * \param doppler The Doppler shift [Hz].
 * \return  0 on success,
 *         -1 if almanac is not valid or too old
 */
s8 calc_sat_doppler_almanac(const almanac_t *a,
                            const gps_time_t *t,
                            const double ref[3],
                            double *doppler) {
  double sat_pos[3];
  double sat_vel[3];
  double sat_acc[3];
  double clock_err, clock_rate_err;
  double vec_ref_sat[3];

  s8 ret = calc_sat_state_almanac(
      a, t, sat_pos, sat_vel, sat_acc, &clock_err, &clock_rate_err);
  if (ret != 0) {
    return ret;
  }

  /* Find the vector from the reference position to the satellite. */
  vector_subtract(3, sat_pos, ref, vec_ref_sat);

  /* Find the satellite velocity projected on the line of sight vector from the
   * reference position to the satellite. */
  double radial_velocity =
      vector_dot(3, vec_ref_sat, sat_vel) / vector_norm(3, vec_ref_sat);

  /* Return the Doppler shift. */
  *doppler = sid_to_carr_freq(a->sid) * radial_velocity / GPS_C;

  return 0;
}

/** Is this almanac usable?
 *
 * \param a Almanac struct
 * \param t The current GPS time. This is used to determine the ephemeris age.
 * \return 1 if the ephemeris is valid and not too old.
 *         0 otherwise.
 */
u8 almanac_valid(const almanac_t *a, const gps_time_t *t) {
  assert(a != NULL);
  assert(t != NULL);

  if (!a->valid) {
    return 0;
  }

  if (!almanac_healthy(a)) {
    return 0;
  }

  if (a->fit_interval <= 0) {
    log_warn("almanac_valid used with 0 a->fit_interval");
    return 0;
  }

  /*
   * Almanac did not get time-stamped when it was received.
   */
  if (a->toa.wn == 0) {
    return 0;
  }

  /* Seconds from the time from almanac reference epoch (toe) */
  double dt = gpsdifftime(t, &a->toa);

  /* TODO: this doesn't exclude almanacs older than a week so could be made
   * better. */
  /* If dt is greater than fit_interval / 2 seconds our ephemeris isn't valid.
   */
  if (fabs(dt) > ((u32)a->fit_interval / 2)) {
    return 0;
  }

  return 1;
}

/** Check almanac health based on almanac 8-bit health word.
 *
 * \param[in] alm almanac struct
 *
 * \return 1 if almanac is healthy
 *         0 otherwise
 */
u8 almanac_healthy(const almanac_t *alm) {
  /* Check 3 MSB; TLM/HOW, ZCOUNT, SF123 errors don't effect almanac content */
  u8 ignore = (1 << NAV_DHI_TLM_HOW_ERR) | (1 << NAV_DHI_SUB123_ERR) |
              (1 << NAV_DHI_ZCOUNT_ERR);
  if (!check_nav_dhi(alm->health_bits, ignore)) {
    return 0;
  }

  /* Check 5 LSBs */
  return check_6bit_health_word(alm->health_bits & 0x1F, alm->sid.code) ? 1 : 0;
}

static bool almanac_xyz_equal(const almanac_xyz_t *a, const almanac_xyz_t *b) {
  return (memcmp(a->pos, b->pos, sizeof(a->pos)) == 0) &&
         (memcmp(a->vel, b->vel, sizeof(a->vel)) == 0) &&
         (memcmp(a->acc, b->acc, sizeof(a->acc)) == 0);
}

static bool almanac_kepler_equal(const almanac_kepler_t *a,
                                 const almanac_kepler_t *b) {
  return (a->m0 == b->m0) && (a->ecc == b->ecc) && (a->sqrta == b->sqrta) &&
         (a->omega0 == b->omega0) && (a->omegadot == b->omegadot) &&
         (a->w == b->w) && (a->inc == b->inc) && (a->af0 == b->af0) &&
         (a->af1 == b->af1);
}

static bool almanac_glo_equal(const almanac_glo_t *a, const almanac_glo_t *b) {
  return (a->lambda == b->lambda) && (a->t_lambda == b->t_lambda) &&
         (a->i == b->i) && (a->t == b->t) && (a->t_dot == b->t_dot) &&
         (a->epsilon == b->epsilon) && (a->omega == b->omega);
}

/** Are the two almanacs the same?
 *
 * \param a First almanac
 * \param b Second almanac
 * \return true if they are equal
 */
bool almanac_equal(const almanac_t *a, const almanac_t *b) {
  if (!sid_is_equal(a->sid, b->sid) || (a->ura != b->ura) ||
      (a->fit_interval != b->fit_interval) || (a->valid != b->valid) ||
      (a->health_bits != b->health_bits) || (a->toa.wn != b->toa.wn) ||
      (a->toa.tow != b->toa.tow))
    return false;

  switch (sid_to_constellation(a->sid)) {
    case CONSTELLATION_GPS:
      return almanac_kepler_equal(&a->kepler, &b->kepler);
    case CONSTELLATION_SBAS:
      return almanac_xyz_equal(&a->xyz, &b->xyz);
    case CONSTELLATION_GLO:
      return almanac_glo_equal(&a->glo, &b->glo);
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    case CONSTELLATION_BDS:
    case CONSTELLATION_GAL:
    case CONSTELLATION_QZS:
    default:
      assert(!"Unsupported constellation");
      return false;
  }
}

/**
 * Decode almanac's week number and SV health flags
 *
 * References:
 * -# IS-GPS-200D, Section 20.3.3.5
 *
 * \param[in]  words    Words 3-10 of subframe 4 or 5.
 * \param[out] ref_week Reference time to update on success.
 *
 * \retval true  Some data has been decoded
 * \retval false No data has been decoded
 */
bool almanac_decode_week(const u32 words[8], almanac_ref_week_t *ref_week) {
  bool retval = false;

  assert(NULL != words);
  assert(NULL != ref_week);

  memset(ref_week, 0, sizeof(*ref_week));

  /* Word 3 bits 1-2: data ID */
  u8 data_id = words[3 - 3] >> (30 - 2) & 0x3;
  /* Word 3 bits 3-8: SV ID */
  u8 sv_id = words[3 - 3] >> (30 - 8) & 0x3F;

  if (GPS_LNAV_ALM_DATA_ID_BLOCK_II == data_id &&
      GPS_LNAV_ALM_SVID_WEEK == sv_id) {
    /* IS-GPS-200D, 20.3.3.5.1.5 Almanac Reference Week.
     * Subframe 5, page 25:
     *   Word 3 bits 17-24: number of the week (WN_a), modulo-256
     *   Word 3 bits  9-16: t_oa, referenced by this WN_a
     */
    u8 wn = words[3 - 3] >> (30 - 24) & 0xFF;
    u8 toa = words[3 - 3] >> (30 - 16) & 0xFF;

    ref_week->toa = toa * GPS_LNAV_ALM_SF_TOA;
    ref_week->wna = gps_adjust_week_cycle256(wn, GPS_WEEK_REFERENCE);

    retval = true;
  }

  return retval;
}

/**
 * Decode SV health flags
 *
 * SV health flags are lower 5 bits from individual almanac's health + 1 extra
 * bit.
 *
 * References:
 * -# IS-GPS-200D, Section 20.3.3.5
 *
 * \param[in]  words      Words 3-10 of subframe 4 page 25 or subframe 5 page
 * 25.
 * \param[out] alm_health Container for decoded data.
 *
 * \retval true  Some data has been decoded
 * \retval false No data has been decoded
 */
bool almanac_decode_health(const u32 words[8], almanac_health_t *alm_health) {
  bool retval = false;

  assert(NULL != words);
  assert(NULL != alm_health);

  memset(alm_health, 0, sizeof(*alm_health));

  /* Word 3 bits 1-2: data ID */
  u8 data_id = words[3 - 3] >> (30 - 2) & 0x3;
  /* Word 3 bits 3-8: SV ID */
  u8 sv_id = words[3 - 3] >> (30 - 8) & 0x3F;

  if (GPS_LNAV_ALM_DATA_ID_BLOCK_II == data_id &&
      GPS_LNAV_ALM_SVID_HEALTH_4 == sv_id) {
    /* IS-GPS-200D, 20.3.3.5.1.3 SV Health
     * Subframe 4, page 25:
     *   Word  8 bits 19-24: 6-bit SV 25 health
     *   Word  9 bits 1-6,7-12,13-18,19-24: 4x6-bit SV 26-29 health
     *   Word 10 bits 1-6,7-12,13-18: 3x6-bit SV 30-32 health
     */
    alm_health->health_bits[25 - 1] = words[8 - 3] >> (30 - 24) & 0x3F;
    for (u8 shift = 30 - 6, sv_idx = (26 - 1);
         shift >= 30 - 24 || (assert(sv_idx == 30 - 1), false);
         shift -= 6, sv_idx++) {
      assert(sv_idx >= (26 - 1) && sv_idx <= (29 - 1));
      alm_health->health_bits[sv_idx] = words[9 - 3] >> shift & 0x3F;
    }
    for (u8 shift = 30 - 6, sv_idx = (30 - 1);
         shift >= 30 - 18 || (assert(sv_idx == 33 - 1), false);
         shift -= 6, sv_idx++) {
      assert(sv_idx >= (30 - 1) && sv_idx <= (32 - 1));
      alm_health->health_bits[sv_idx] = words[10 - 3] >> shift & 0x3F;
    }
    /* Set bits 24-31 */
    alm_health->health_bits_valid |= 0xFF000000;

    retval = true;
  } else if (GPS_LNAV_ALM_DATA_ID_BLOCK_II == data_id &&
             GPS_LNAV_ALM_SVID_HEALTH_5 == sv_id) {
    /* IS-GPS-200D, 20.3.3.5.1.3 SV Health
     * Subframe 5, page 25:
     *   Words 4-9 bits 1-6,7-12,13-18,19-24: 4x6-bit SV health for SV 1-24
     */
    for (u8 word = (4 - 3), sv_idx = (1 - 1);
         word <= (9 - 3) || (assert(sv_idx == 25 - 1), false);
         ++word) {
      for (u8 shift = (30 - 6); shift >= 30 - 24; shift -= 6, sv_idx++) {
        assert(/* sv_idx >= (1 - 1) && */ sv_idx <= (24 - 1));
        alm_health->health_bits[sv_idx] = words[word] >> shift & 0x3F;
      }
    }
    /* Set bits 0-23 */
    alm_health->health_bits_valid |= 0xFFFFFF;
    retval = true;
  }

  return retval;
}

/**
 * Decodes almanac from GLS LNAV message subframe.
 *
 * The method decodes almanac data from GPS LNAV subframe words 3-10.
 *
 * References:
 * -# IS-GPS-200D, Section 20.3.3.5
 *
 * \param[in]  words    Subframe 4 pages 2-5, 7-10 or subframe 5 pages 1-24.
 * \param[out] a        Destination object.
 *
 * \retval true  Almanac has been decoded.
 * \retval false Decoding error.
 */
bool almanac_decode(const u32 words[8], almanac_t *a) {
  bool retval = false;

  assert(NULL != words);
  assert(NULL != a);

  memset(a, 0, sizeof(*a));

  /* Word 3 bits 1-2: data ID */
  u8 data_id = words[3 - 3] >> (30 - 2) & 0x3;
  /* Word 3 bits 3-8: SV ID */
  u8 sv_id = words[3 - 3] >> (30 - 8) & 0x3F;

  if (GPS_LNAV_ALM_DATA_ID_BLOCK_II == data_id &&
      sv_id >= GPS_LNAV_ALM_MIN_PRN && sv_id <= GPS_LNAV_ALM_MAX_PRN) {
    a->valid = true;
    a->sid = construct_sid(CODE_GPS_L1CA, sv_id);

    /* Fit interval for almanacs is at least 140 hours */
    a->fit_interval = HOUR_SECS * 70 * 2;

    /* See IS-GPS-200H 20.3.3.5.2.1 "Almanac":
     * Operational Interval | Almanac Ephemeris URE 1 sigma (meters)
     * Normal               | 900 *,**
     * Short-term Extended  | 900 - 3600*
     * Long-term Extended   | 3600 - 300000*
     * Notes:
     * - URE values generally tend to degrade quadratically over time. Larger
     *   errors may be encountered during eclipse seasons and whenever a
     *   propulsive event has occurred.
     * - After the CS is unable to upload the SVs, URE values for the SVs
     *   operating in the Autonav mode
     */
    a->ura = 900;

    a->toa.wn = WN_UNKNOWN;

    /* Word 4 bits 1-8: t_oa */
    u8 toa = words[4 - 3] >> (30 - 8) & 0xFF;
    a->toa.tow = toa * GPS_LNAV_ALM_SF_TOA;

    /* Word 5 bits 17-24: SV health */
    a->health_bits = words[5 - 3] >> (30 - 24) & 0xFF;

    almanac_kepler_t *k = &a->kepler;

    /* Word 3 bits 9-24 */
    k->ecc = (words[3 - 3] >> (30 - 24) & 0xFFFF) * GPS_LNAV_ALM_SF_ECC;
    /* Word 4 bits 9-24 */
    k->inc = (s16)(words[4 - 3] >> (30 - 24) & 0xFFFF) *
                 (GPS_LNAV_ALM_SF_INC * GPS_PI) +
             (GPS_LNAV_ALM_OFF_INC * GPS_PI);
    /* Word 5 bits 1-16 */
    k->omegadot = (s16)(words[5 - 3] >> (30 - 16) & 0xFFFF) *
                  (GPS_LNAV_ALM_SF_OMEGADOT * GPS_PI);
    /* Word 6 bits 1-24 */
    k->sqrta = (words[6 - 3] >> (30 - 24) & 0xFFFFFF) * GPS_LNAV_ALM_SF_SQRTA;
    /* Word 7 bits 1-24 */
    k->omega0 = sign_extend24(words[7 - 3] >> (30 - 24) & 0xFFFFFF) *
                (GPS_LNAV_ALM_SF_OMEGA0 * GPS_PI);
    /* Word 8 bits 1-24 */
    k->w = sign_extend24(words[8 - 3] >> (30 - 24) & 0xFFFFFF) *
           (GPS_LNAV_ALM_SF_W * GPS_PI);
    /* Word 9 bits 1-24 */
    k->m0 = sign_extend24(words[9 - 3] >> (30 - 24) & 0xFFFFFF) *
            (GPS_LNAV_ALM_SF_M0 * GPS_PI);
    /* Word 10 bits 1-8 MSB bits 20-22 LSB */
    k->af0 = sign_extend11((words[10 - 3] >> (30 - 8 - 3) & 0x7F8) |
                           (words[10 - 3] >> (30 - 22) & 0x7)) *
             GPS_LNAV_ALM_SF_AF0;
    /* Word 10 bits 9-19 */
    k->af1 =
        sign_extend11(words[10 - 3] >> (30 - 19) & 0x7FF) * GPS_LNAV_ALM_SF_AF1;
    retval = true;
  }

  return retval;
}

/** \} */
