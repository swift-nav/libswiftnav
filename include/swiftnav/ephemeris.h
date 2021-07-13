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

#include <stdbool.h>

#include <swiftnav/common.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_GPS_URA_INDEX (-1)
#define MAX_ALLOWED_GPS_URA_IDX 15
#define INVALID_URA_VALUE (-1.0f)
#define INVALID_GAL_SISA_INDEX 0xFF
#define URA_VALID(x) ((x) >= 0)

#define EPH_SOURCE_NUM_BITS 2

#define EPH_SOURCE_GPS_LNAV 0
#define EPH_SOURCE_GPS_CNAV 1
#define EPH_SOURCE_GPS_CNAV2 2

#define EPH_SOURCE_QZS_LNAV EPH_SOURCE_GPS_LNAV
#define EPH_SOURCE_QZS_CNAV EPH_SOURCE_GPS_CNAV
#define EPH_SOURCE_QZS_CNAV2 EPH_SOURCE_GPS_CNAV2

#define EPH_SOURCE_GAL_INAV 0
#define EPH_SOURCE_GAL_FNAV 1

#define EPH_SOURCE_BDS_D1_D2_NAV 0
#define EPH_SOURCE_BDS_BCNAV1 1
#define EPH_SOURCE_BDS_BCNAV2 2

#define EPH_SOURCE_GLO_FDMA 0
#define EPH_SOURCE_GLO_CDMA 1

#define BDS2_IODE_MAX 240
#define BDS2_IODC_MAX 240

#define BDS3_IODE_MAX 0xFF
#define BDS3_IODC_MAX 0x3FF

#ifndef BDS_FIT_INTERVAL_SECONDS
#define BDS_FIT_INTERVAL_SECONDS (3 * HOUR_SECS)
#endif

#define GAL_IOD_NAV_MAX 0x3FF

#define GLO_IOD_MAX 0x7F

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

#define GPS_IODE_MAX 0xFF
#define GPS_IODC_MAX 0x3FF

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
  EPH_INVALID_SID,
  EPH_INVALID_IOD,
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
  u16 iodc;        /**< Issue of data clock. Not suitable as a lookup key,
                        see get_ephemeris_key().
       
                        Source of value:
                        - GPS LNAV: IS-GPS-200K Table 20-I “IODC”
                        - GPS CNAV: IS-GPS-200K Table 30-III “t_oc”
                        - GPS CNAV-2: IS-GPS-800F Table 3.5-1 “t_oe”
                        - Galileo I/NAV: Galileo OS SIS ICD Issue 1.3 Tables 39 to 42
                      “IOD_nav”
                        - Galileo F/NAV: Galileo OS SIS ICD Issue 1.3 Tables 27 to 30
                      “IOD_nav”
                        - BDS D1 NAV: BDS-SIS-ICD-2.1 Table 5-7 “t_oc” where IODE = mod
                      (t_oe / 720, 240) per RTCM/CSNO recommendation
                        - BDS D2 NAV: BDS-SIS-ICD-2.1 Table 5-7 “t_oc” where IODC = mod
                      (t_oc / 720, 240) per RTCM/CSNO recommendation
                        - BDS B-CNAV1: BDS-SIS-ICD-B1C-1.0 Table 6-2 “IODC”
                        - BDS B-CNAV2: BDS-SIS-ICD-B2a-1.0 Table 6-1 “IODC”
                        - QZSS LNAV: IS-QZSS-PNT-003 Table 4.1.2-4 “IODC”
                        - QZSS CNAV: IS-QZSS-PNT-003 Table 4.3.2-8 “t_oc”
                        - QZSS CNAV-2: IS-QZSS-PNT-003 Table 4.2.2-4 “t_oe”
                        */
  u16 iode;        /**< Issue of data ephemeris. Not suitable as a lookup key,
                        see get_ephemeris_key().
       
                        Source of value:
                        - GPS LNAV: IS-GPS-200K Table 20-II “IODE”
                        - GPS CNAV: IS-GPS-750F Table 20-I “t_oe”
                        - GPS CNAV-2: IS-GPS-800F Table 3.5-1 “t_oe”
                        - Galileo I/NAV: Galileo OS SIS ICD Issue 1.3 Tables 39 to 42
                      “IOD_nav”
                        - Galileo F/NAV: Galileo OS SIS ICD Issue 1.3 Tables 27 to 30
                      “IOD_nav”
                        - BDS D1 NAV: BDS-SIS-ICD-2.1 Table 5-10 “t_oe” where IODE =
                      mod (t_oe / 720, 240) per RTCM/CSNO recommendation
                        - BDS D2 NAV: BDS-SIS-ICD-2.1 Table 5-10 “t_oe” where IODE =
                      mod (t_oe / 720, 240) per RTCM/CSNO recommendation
                        - BDS B-CNAV1: BDS-SIS-ICD-B1C-1.0 Table 6-2 “IODE”
                        - BDS B-CNAV2: BDS-SIS-ICD-B2a-1.0 Table 6-1 “IODE”
                        - QZSS LNAV: IS-QZSS-PNT-003 Tables 4.1.2-7 to 4.1.2-8 “IODE”
                        - QZSS CNAV: IS-QZSS-PNT-003 Table 4.3.2-4 “t_oe”
                        - QZSS CNAV-2: IS-QZSS-PNT-003 Table 4.2.2-4 “t_oe”
                        */
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
  u8 iod;        /**< Issue of data. Not suitable as a lookup key,
                      see get_ephemeris_key()
       
                      Source of value:
                      - GLONASS FDMA: GLONASS ICD L1 L2 Edition 5.1 Table 4.5 “t_b”
                      - GLONASS CDMA: GLONASS ICD CDMA L1 Edition 1.0 Table 5.2 “t_b”
                 */
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
  } data;
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

/* BDS satellites can be either geostationary (GEO), geosynchronous (IGSO) or
 medium earth orbit (MEO) */
typedef enum { GEO, IGSO, MEO } satellite_orbit_type_t;

s8 calc_sat_state(const ephemeris_t *e,
                  const gps_time_t *t,
                  double pos[3],
                  double vel[3],
                  double acc[3],
                  double *clock_err,
                  double *clock_rate_err);
s8 calc_sat_state_orbit_type(const ephemeris_t *e,
                             const gps_time_t *t,
                             const satellite_orbit_type_t orbit_type,
                             double pos[3],
                             double vel[3],
                             double acc[3],
                             double *clock_err,
                             double *clock_rate_err);
s8 calc_sat_state_n(const ephemeris_t *e,
                    const gps_time_t *t,
                    const satellite_orbit_type_t orbit_type,
                    double pos[3],
                    double vel[3],
                    double acc[3],
                    double *clock_err,
                    double *clock_rate_err);
s8 calc_sat_az_el(const ephemeris_t *e,
                  const gps_time_t *t,
                  const double ref[3],
                  const satellite_orbit_type_t orbit_type,
                  double *az,
                  double *el,
                  bool check_e);
s8 calc_sat_doppler(const ephemeris_t *e,
                    const gps_time_t *t,
                    const double ref_pos[3],
                    const double ref_vel[3],
                    const satellite_orbit_type_t orbit_type,
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
bool decode_gal_ephemeris_safe(const u8 page[5][GAL_INAV_CONTENT_BYTE],
                               ephemeris_t *eph);

void decode_glo_ephemeris(const glo_string_t strings[5],
                          const gnss_signal_t sid,
                          const utc_params_t *utc_params,
                          ephemeris_t *eph);

bool ephemeris_equal(const ephemeris_t *a, const ephemeris_t *b);
bool ephemeris_healthy(const ephemeris_t *ephe, const code_t code);

u8 encode_ura(float ura);
float decode_ura_index(const u8 index);

u32 get_bds2_iod_crc(const ephemeris_t *eph);

s8 get_tgd_correction(const ephemeris_t *eph,
                      const gnss_signal_t *sid,
                      float *tgd);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_EPHEMERIS_H */
