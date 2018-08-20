/*
 * Copyright (C) 2010 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_ALMANAC_H
#define LIBSWIFTNAV_ALMANAC_H

#include <swiftnav/common.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \addtogroup almanac
 * \{ */

/** IS-GPS-200H Table 20.6: 602112 [s] */
#define GPS_LNAV_ALM_MAX_TOA 602112
/** Lowest SV PRN number [sv_id] */
#define GPS_LNAV_ALM_MIN_PRN 1
/** Highest SV PRN number [sv_id] */
#define GPS_LNAV_ALM_MAX_PRN 32
/** Almanac reference week subframe 5 page 25 [sv_id] */
#define GPS_LNAV_ALM_SVID_WEEK 51
/** SV health: subframe 5 page 25 [sv_id] */
#define GPS_LNAV_ALM_SVID_HEALTH_5 51
/** SV health: subframe 4 page 25 [sv_id] */
#define GPS_LNAV_ALM_SVID_HEALTH_4 63
/** SV capabilities: subframe 4 page 25 [sv_id] */
#define GPS_LNAV_ALM_SVID_CAPABILITIES 63
/** IONO parameters: subframe 4 page 18 [sv_id] */
#define GPS_LNAV_ALM_SVID_IONO 56
/** UTC parameters: subframe 4 page 18 [sv_id] */
#define GPS_LNAV_ALM_SVID_UTC 56
/** Data id number 2 described in IS-GPS-200H 20.3.3.5.1.1 [data_id] */
#define GPS_LNAV_ALM_DATA_ID_BLOCK_II 1

/** IS-GPS-200H Table 20.6: 2^12 */
#define GPS_LNAV_ALM_SF_TOA 4096
/** IS-GPS-200H Table 20.6: 2^-21 */
#define GPS_LNAV_ALM_SF_ECC C_1_2P21
/** IS-GPS-200H Table 20.6: 2^-19 */
#define GPS_LNAV_ALM_SF_INC C_1_2P19
/** IS-GPS-200H Table 20.6: .30 [semi-circle] */
#define GPS_LNAV_ALM_OFF_INC 0.30
/** IS-GPS-200H Table 20.6: 2^-38 */
#define GPS_LNAV_ALM_SF_OMEGADOT C_1_2P38
/** IS-GPS-200H Table 20.6: 2^-11 */
#define GPS_LNAV_ALM_SF_SQRTA C_1_2P11
/** IS-GPS-200H Table 20.6: 2^-23 */
#define GPS_LNAV_ALM_SF_OMEGA0 C_1_2P23
/** IS-GPS-200H Table 20.6: 2^-23 */
#define GPS_LNAV_ALM_SF_W C_1_2P23
/** IS-GPS-200H Table 20.6: 2^-23 */
#define GPS_LNAV_ALM_SF_M0 C_1_2P23
/** IS-GPS-200H Table 20.6: 2^-20 */
#define GPS_LNAV_ALM_SF_AF0 C_1_2P20
/** IS-GPS-200H Table 20.6: 2^-38 */
#define GPS_LNAV_ALM_SF_AF1 C_1_2P38

/** Structure containing the GPS almanac for one satellite. */
typedef struct {
  double m0;       /**< Mean anomaly at reference time [semi-circles] */
  double ecc;      /**< Eccentricity. */
  double sqrta;    /**< Square root of the semi-major axis [sqrt(m)] */
  double omega0;   /**< Longitude of ascending node
                        of orbit plane at weekly epoch [semi-circles] */
  double omegadot; /**< Rate of right ascension [semi-circles/s] */
  double w;        /**< Argument of perigee [semi-circles] */
  double inc;      /**< Inclindation angle at reference time [semi-circles]
                        This must include the 0.3 offset from NAV delta_i. */
  double af0;      /**< Time offset of the sat clock [s] **/
  double af1;      /**< Drift of the sat clock [s/s] **/
} almanac_kepler_t;

/** Structure containing the SBAS almanac for one satellite. */
typedef struct {
  double pos[3]; /**< Position of the GEO at time toe [m] */
  double vel[3]; /**< velocity of the GEO at time toe [m/s] */
  double acc[3]; /**< velocity of the GEO at time toe [m/s^2] */
} almanac_xyz_t;

/** Structure containing the GLONASS almanac for one satellite. */

typedef struct {
  double lambda;   /**< Longitude of the first ascending node of the orbit
                        in PZ-90.02 coordinate system, [semi-circles] */
  double t_lambda; /**< Time of the first ascending node passage, [s]*/
  double i;        /**< Value of inclination at instant of t_lambda,
                        [semi-circles] */
  double t;        /**< Value of Draconian period at instant of t_lambda,
                        [s/orbital period] */
  double t_dot;    /**< Rate of change of the Draconian period,
                        [s/(orbital period^2)] */
  double epsilon;  /**< Eccentricity at instant of t_lambda_n_A,
                        [dimensionless] */
  double omega;    /**< Argument of perigee at instant of t_lambda,
                        [semi-circles] */
} almanac_glo_t;

/** Structure containing the almanac for one satellite. */
typedef struct {
  gnss_signal_t sid; /**< Signal ID. */
  gps_time_t toa;    /**< Reference time of almanac. */
  float ura;         /**< User range accuracy [m] */
  u32 fit_interval;  /**< Curve fit interval [s] */
  u8 valid;          /**< Almanac is valid. */
  u8 health_bits;    /**< Satellite health status:
                      * - MSB 3: NAV data health status. See IS-GPS-200H
                      *   Table 20-VII: NAV Data Health Indications;
                      * - LSB 5: Signal health status. See IS-GPS-200H
                      *   Table 20-VIII. Codes for Health of SV Signal
                      *   Components */
  union {
    almanac_kepler_t kepler; /**< Parameters specific to GPS. */
    almanac_xyz_t xyz;       /**< Parameters specific to SBAS. */
    almanac_glo_t glo;       /**< Parameters specific to GLONASS. */
  };
} almanac_t;

/**
 * Combined container for almanac time and health parameters.
 *
 * This container is populated from 25th page of GPS LNAV subframe 5.
 *
 * References:
 * - IS-GPS-200H, Figure 20-1, sheets 5 and 9 of 11.
 */
typedef struct {
  s16 wna; /**< Almanac week number (WN_a) */
  s32 toa; /**< Almanac time (t_oa) [s] */
} almanac_ref_week_t;

/**
 * Combined container for almanac health parameters.
 *
 * This container is populated from 25th page of GPS LNAV subframes 4 and 5.
 *
 * References:
 * - IS-GPS-200H, Figure 20-1, sheets 5 and 9 of 11.
 */
typedef struct {
  u32 health_bits_valid; /**< Health bits validity flags for SV 1 - 32.
                              Mask flag: (1u << (sv_id -1)) */
  u8 health_bits[32];    /**< 6 bit health bits values for SV 1 - 32.
                              See IS-GPS-200H 20.3.3.5.1.3.
                              Index: (sv_id - 1). */
} almanac_health_t;

/** \} */

s8 calc_sat_state_almanac(const almanac_t *a,
                          const gps_time_t *t,
                          double pos[3],
                          double vel[3],
                          double acc[3],
                          double *clock_err,
                          double *clock_rate_err);
s8 calc_sat_az_el_almanac(const almanac_t *a,
                          const gps_time_t *t,
                          const double ref[3],
                          double *az,
                          double *el);
s8 calc_sat_doppler_almanac(const almanac_t *a,
                            const gps_time_t *t,
                            const double ref[3],
                            double *doppler);

u8 almanac_valid(const almanac_t *a, const gps_time_t *t);
u8 almanac_healthy(const almanac_t *a);

bool almanac_equal(const almanac_t *a, const almanac_t *b);
bool almanac_decode_week(const u32 words[8], almanac_ref_week_t *ref_week);
bool almanac_decode_health(const u32 words[8], almanac_health_t *alm_health);
bool almanac_decode(const u32 words[8], almanac_t *a);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_ALMANAC_H */
