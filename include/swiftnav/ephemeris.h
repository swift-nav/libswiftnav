/*
 * Copyright (C) 2010, 2016, 2020 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_EPHEMERIS_H
#define LIBSWIFTNAV_EPHEMERIS_H

#include <swiftnav/common.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_GPS_URA_INDEX -1
#define MAX_ALLOWED_GPS_URA_IDX 15
#define INVALID_URA_VALUE -1.0f
#define INVALID_GAL_SISA_INDEX 0xFF
#define URA_VALID(x) ((x) >= 0)

#define INVALID_IODE 0xFF
#define INVALID_IODC 0x3FF

#define EPH_SOURCE_GAL_INAV 0
#define EPH_SOURCE_GAL_FNAV 1

#ifndef BDS_FIT_INTERVAL_SECONDS
#define BDS_FIT_INTERVAL_SECONDS (3 * HOUR_SECS)
#endif

#ifndef GAL_WEEK_TO_GPS_WEEK
/** GST week offset to GPS */
#define GAL_WEEK_TO_GPS_WEEK 1024
#endif

#ifndef GAL_FIT_INTERVAL_SECONDS
/**
 * Galileo fit_interval definition
 * "Galileo Open Service: Service Definition Document"
 * Issue 1 Revision 0, 2016 December
 * Section 2.4.1
 * https://www.gsc-europa.eu/system/files/galileo_documents/Galileo-OS-SDD.pdf
 */
#define GAL_FIT_INTERVAL_SECONDS (4 * HOUR_SECS)
#endif

#ifndef GAL_INAV_CONTENT_BIT
/** Number of bits in one Galileo I/NAV page content */
#define GAL_INAV_CONTENT_BIT 128
#endif

#ifndef GAL_INAV_CONTENT_BYTE
/** Number of Bytes in one Galileo I/NAV page content */
#define GAL_INAV_CONTENT_BYTE ((GAL_INAV_CONTENT_BIT + CHAR_BIT - 1) / CHAR_BIT)
#endif

/** \addtogroup ephemeris
 * \{ */

/** IS-GPS-200H Table 20-I: maximum t_oc 604784 [s] */
#define GPS_LNAV_EPH_TOC_MAX 604784
/** IS-GPS-200H Table 20-III: maximum t_oe 604784 [s] */
#define GPS_LNAV_EPH_TOE_MAX 604784

/* Scale factors for ephemeris decoding and validation */
/** IS-GPS-200H Table 20-I: 2^-31 */
#define GPS_LNAV_EPH_SF_TGD C_1_2P31
/** IS-GPS-200H Table 20-I: 2^4 */
#define GPS_LNAV_EPH_SF_TOC 16
/** IS-GPS-200H Table 20-I: 2^-55 */
#define GPS_LNAV_EPH_SF_AF2 C_1_2P55
/** IS-GPS-200H Table 20-I: 2^-43 */
#define GPS_LNAV_EPH_SF_AF1 C_1_2P43
/** IS-GPS-200H Table 20-I: 2^-31 */
#define GPS_LNAV_EPH_SF_AF0 C_1_2P31
/** IS-GPS-200H Table 20-III: 2^-5 */
#define GPS_LNAV_EPH_SF_CRS C_1_2P5
/** IS-GPS-200H Table 20-III: 2^-43 */
#define GPS_LNAV_EPH_SF_DN C_1_2P43
/** IS-GPS-200H Table 20-III: 2^-31 */
#define GPS_LNAV_EPH_SF_M0 C_1_2P31
/** IS-GPS-200H Table 20-III: 2^-29 */
#define GPS_LNAV_EPH_SF_CUC C_1_2P29
/** IS-GPS-200H Table 20-III: 2^-33 */
#define GPS_LNAV_EPH_SF_ECC C_1_2P33
/** IS-GPS-200H Table 20-III: 2^-29 */
#define GPS_LNAV_EPH_SF_CUS C_1_2P29
/** IS-GPS-200H Table 20-III: 2^-19 */
#define GPS_LNAV_EPH_SF_SQRTA C_1_2P19
/** IS-GPS-200H Table 20-III: 2^4 */
#define GPS_LNAV_EPH_SF_TOE 16
/** IS-GPS-200H Table 20-III: 2^-29 */
#define GPS_LNAV_EPH_SF_CIC C_1_2P29
/** IS-GPS-200H Table 20-III: 2^-31 */
#define GPS_LNAV_EPH_SF_OMEGA0 C_1_2P31
/** IS-GPS-200H Table 20-III: 2^-29 */
#define GPS_LNAV_EPH_SF_CIS C_1_2P29
/** IS-GPS-200H Table 20-III: 2^-31 */
#define GPS_LNAV_EPH_SF_I0 C_1_2P31
/** IS-GPS-200H Table 20-III: 2^-5 */
#define GPS_LNAV_EPH_SF_CRC C_1_2P5
/** IS-GPS-200H Table 20-III: 2^-31 */
#define GPS_LNAV_EPH_SF_W C_1_2P31
/** IS-GPS-200H Table 20-III: 2^-43 */
#define GPS_LNAV_EPH_SF_OMEGADOT C_1_2P43
/** IS-GPS-200H Table 20-III: 2^-43 */
#define GPS_LNAV_EPH_SF_IDOT C_1_2P43

typedef enum {
  EPH_NULL,
  EPH_INVALID,
  EPH_WN_EQ_0,
  EPH_FIT_INTERVAL_EQ_0,
  EPH_UNHEALTHY,
  EPH_TOO_OLD,
  EPH_VALID
} ephemeris_status_t;

/** Structure containing the GPS ephemeris for one satellite. */
typedef struct {
  union {
    float gps_s[2];  /**< GPS TGD  */
    float qzss_s[2]; /**< QZSS TGD */
    float bds_s[2];  /**< tgd_bds_s[0] = BDS TGD1,
                          tgd_bds_s[1] = BDS TGD2 */
    float gal_s[2];  /**< tgd_gal_s[0] = GAL E5a/E1 BGD,
                          tgd_gal_s[1] = GAL E5b/E1 BGD*/
  } tgd;
  double crc;      /**< Amplitude of the cosine harmonic correction term
                        to the orbit radius [m] */
  double crs;      /**< Amplitude of the sine harmonic correction term
                        to the orbit radius [m] */
  double cuc;      /**< Amplitude of the cosine harmonic correction term
                        to the argument of latitude [rad] */
  double cus;      /**< Amplitude of the sine harmonic correction term
                        to the argument of latitude [rad] */
  double cic;      /**< Amplitude of the cosine harmonic correction term
                        to the angle of inclination [rad] */
  double cis;      /**< Amplitude of the sine harmonic correction term
                        to the angle of inclination [rad] */
  double dn;       /**< Mean motion difference from computed value
                        [rad/s] */
  double m0;       /**< Mean anomaly at reference time [rad] */
  double ecc;      /**< Eccentricity, dimensionless */
  double sqrta;    /**< Square root of the semi-major axis [sqrt(m)] */
  double omega0;   /**< Longitude of ascending node
                        of orbit plane at weekly epoch [rad] */
  double omegadot; /**< Rate of right ascension [rad/s] */
  double w;        /**< Argument of perigee [rad] */
  double inc;      /**< Inclindation angle at reference time [rad] */
  double inc_dot;  /**< Rate of inclination angle [rad/s] */
  double af0;      /**< Time offset of the sat clock [s] **/
  double af1;      /**< Drift of the sat clock [s/s] **/
  double af2;      /**< Acceleration of the sat clock [s/s^2] **/
  gps_time_t toc;  /**< Reference time of clock. */
  u16 iodc;        /**< Issue of data clock. */
  u16 iode;        /**< Issue of data ephemeris. */
} ephemeris_kepler_t;

/** Structure containing the SBAS ephemeris for one satellite. */
typedef struct {
  double pos[3]; /**< Position of the GEO at time toe [m] */
  double vel[3]; /**< velocity of the GEO at time toe [m/s] */
  double acc[3]; /**< velocity of the GEO at time toe [m/s^2] */
  double a_gf0;  /**< Time offset of the GEO clock w.r.t. SNT [s] */
  double a_gf1;  /**< Drift of the GEO clock w.r.t. SNT [s/s] */
} ephemeris_xyz_t;

/** Structure containing the GLONASS ephemeris for one satellite. */
typedef struct {
  double gamma;  /**< Relative deviation of predicted carrier frequency
                      from nominal value, dimensionless */
  double tau;    /**< Correction to the SV time [s]*/
  double d_tau;  /**< Equipment delay between L1 and L2 [s] */
  double pos[3]; /**< Position of the SV at tb in PZ-90.02 coordinates
                      system [m] */
  double vel[3]; /**< Velocity vector of the SV at tb in PZ-90.02
                      coordinates system [m/s] */
  double acc[3]; /**< Acceleration vector of the SV at tb in PZ-90.02
                      coordinates system [m/s^2] */
  u16 fcn;       /**< Frequency slot associated with the GLO SV */
  u8 iod;        /**< Issue of ephemeris data */
} ephemeris_glo_t;

/** Structure containing the ephemeris for one satellite. */
typedef struct {
  gnss_signal_t sid; /**< Signal ID. */
  gps_time_t toe;    /**< Reference time of ephemeris. */
  float ura;         /**< User range accuracy [m] */
  u32 fit_interval;  /**< Curve fit interval [s] */
  u8 valid;          /**< Ephemeris is valid. */
  u8 health_bits;    /**< Satellite health status. */
  u8 source;         /**< Nav data source of ephemeris */
  union {
    ephemeris_kepler_t kepler; /**< Parameters specific to GPS. */
    ephemeris_xyz_t xyz;       /**< Parameters specific to SBAS. */
    ephemeris_glo_t glo;       /**< Parameters specific to GLONASS. */
  };
} ephemeris_t;

#define GLO_NAV_STR_BITS 85 /**< Length of GLO navigation string */
#define GLO_NAV_STR_WORDS 3 /**< Number of u32 words for nav string buffer */

typedef struct {
  u32 word[GLO_NAV_STR_WORDS];
} glo_string_t;

/* Callback type for converting GLO time to GPS time using stored UTC parameters
 */
typedef gps_time_t (*glo2gps_with_utc_params_t)(const glo_time_t *glo_t);

/** \} */

extern const float g_bds_ura_table[16];

s8 calc_sat_state(const ephemeris_t *e,
                  const gps_time_t *t,
                  double pos[3],
                  double vel[3],
                  double acc[3],
                  double *clock_err,
                  double *clock_rate_err,
                  u16 *iodc,
                  u8 *iode);
s8 calc_sat_state_n(const ephemeris_t *e,
                    const gps_time_t *t,
                    double pos[3],
                    double vel[3],
                    double acc[3],
                    double *clock_err,
                    double *clock_rate_err,
                    u16 *iodc,
                    u8 *iode);
s8 calc_sat_az_el(const ephemeris_t *e,
                  const gps_time_t *t,
                  const double ref[3],
                  double *az,
                  double *el,
                  bool check_e);
s8 calc_sat_doppler(const ephemeris_t *e,
                    const gps_time_t *t,
                    const double ref_pos[3],
                    const double ref_vel[3],
                    double *doppler);
ephemeris_status_t get_ephemeris_status_t(const ephemeris_t *e);
ephemeris_status_t ephemeris_valid_detailed(const ephemeris_t *e,
                                            const gps_time_t *t);
u8 ephemeris_valid(const ephemeris_t *e, const gps_time_t *t);
u8 ephemeris_params_valid(const gps_time_t *bgn,
                          const gps_time_t *end,
                          const gps_time_t *toc,
                          const gps_time_t *t);
void decode_ephemeris(const u32 frame_words[3][8],
                      ephemeris_t *e,
                      double tot_tow);

void decode_bds_d1_ephemeris(const u32 words[3][10],
                             gnss_signal_t sid,
                             ephemeris_t *ephe);

void decode_gal_ephemeris(const u8 page[5][GAL_INAV_CONTENT_BYTE],
                          ephemeris_t *eph);

void decode_glo_ephemeris(const glo_string_t strings[5],
                          const gnss_signal_t sid,
                          const utc_params_t *utc_params,
                          ephemeris_t *eph);

bool ephemeris_equal(const ephemeris_t *a, const ephemeris_t *b);
bool ephemeris_healthy(const ephemeris_t *ephe, const code_t code);

u8 encode_ura(float ura);
float decode_ura_index(const u8 index);

u32 get_ephemeris_iod_or_iodcrc(const ephemeris_t *eph);
s8 get_tgd_correction(const ephemeris_t *eph,
                      const gnss_signal_t *sid,
                      float *tgd);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_EPHEMERIS_H */
