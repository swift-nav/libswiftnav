/*
 * Copyright (C) 2015, 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_IONOSHPERE_H
#define LIBSWIFTNAV_IONOSHPERE_H

#include <swiftnav/common.h>
#include <swiftnav/gnss_time.h>

#ifdef __cplusplus
#include <swiftnav/linear_algebra.h>
extern "C" {
#endif

/** IS-GPS-200H Table 20-X: 2^-30 */
#define GPS_LNAV_IONO_SF_A0 C_1_2P30
/** IS-GPS-200H Table 20-X: 2^-27 */
#define GPS_LNAV_IONO_SF_A1 C_1_2P27
/** IS-GPS-200H Table 20-X: 2^-24 */
#define GPS_LNAV_IONO_SF_A2 C_1_2P24
/** IS-GPS-200H Table 20-X: 2^-24 */
#define GPS_LNAV_IONO_SF_A3 C_1_2P24
/** IS-GPS-200H Table 20-X: 2^11 */
#define GPS_LNAV_IONO_SF_B0 2048
/** IS-GPS-200H Table 20-X: 2^14 */
#define GPS_LNAV_IONO_SF_B1 16384
/** IS-GPS-200H Table 20-X: 2^16 */
#define GPS_LNAV_IONO_SF_B2 65536
/** IS-GPS-200H Table 20-X: 2^16 */
#define GPS_LNAV_IONO_SF_B3 65536

/** Structure holding Klobuchar ionospheric model parameters. */
typedef struct {
  gps_time_t toa; /**< Reference time of almanac. */
  double a0, a1, a2, a3;
  double b0, b1, b2, b3;
} ionosphere_t;

static const ionosphere_t DEFAULT_IONO_PARAMS = {
    .toa = {.tow = TOW_UNKNOWN, .wn = WN_UNKNOWN},
    .a0 = 0.0,
    .a1 = 0.0,
    .a2 = 0.0,
    .a3 = 0.0,
    .b0 = 0.0,
    .b1 = 0.0,
    .b2 = 0.0,
    .b3 = 0.0};

double calc_ionosphere(const gps_time_t *t_gps,
                       double lat_u,
                       double lon_u,
                       double a,
                       double e,
                       const ionosphere_t *i);

bool decode_iono_parameters(const u32 words[8], ionosphere_t *i);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
static inline bool operator==(const ionosphere_t &a, const ionosphere_t &b) {
  return double_approx_eq(gpsdifftime(&a.toa, &b.toa), 0.0) &&
         double_approx_eq(a.a0, b.a0) && double_approx_eq(a.a1, b.a1) &&
         double_approx_eq(a.a2, b.a2) && double_approx_eq(a.a3, b.a3) &&
         double_approx_eq(a.b0, b.b0) && double_approx_eq(a.b1, b.b1) &&
         double_approx_eq(a.b2, b.b2) && double_approx_eq(a.b3, b.b3);
}
#endif

#endif /* LIBSWIFTNAV_IONOSHPERE_H */
