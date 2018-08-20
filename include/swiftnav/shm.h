/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_SHM_H_
#define LIBSWIFTNAV_SHM_H_

#include <swiftnav/common.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Table 20-VII. NAV Data Health Indications
 * https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf
 */
typedef enum nav_data_health_indicator_e {
  NAV_DHI_OK = 0,
  NAV_DHI_PARITY_ERR = 1,
  NAV_DHI_TLM_HOW_ERR = 2,
  NAV_DHI_ZCOUNT_ERR = 3,
  NAV_DHI_SUB123_ERR = 4,
  NAV_DHI_SUB45_ERR = 5,
  NAV_DHI_UPDATA_ERR = 6,
  NAV_DHI_ALL_DATA_ERR = 7,
  NAV_DHI_COUNT = 8
} nav_dhi_t;

/* Possible SHM health states */
typedef enum {
  SHM_STATE_UNKNOWN, /* For signals without full support, e.g. GPS L5 */
  SHM_STATE_UNHEALTHY,
  SHM_STATE_HEALTHY,
} shm_state_t;

/* GPS Satellite Health Indicators
 * Reference: IS-GPS-200D
 */
typedef struct {
  bool shi_ephemeris_set; /* LNAV EPHEMERIS SV HEALTH (SHI1 in SHM doc)
                             6 bits, subframe 1, word 3 */
  u8 shi_ephemeris;

  bool shi_page25_set; /* LNAV PAGE#25 SV HEALTH (SHI3 in SHM doc)
                          6 bits, subframes 4 & 5, page 25 */
  u8 shi_page25;
  u32 shi_page25_timetag_s;

  bool shi_lnav_how_alert_set; /* LNAV alert flag (SHI4 in SHM doc)
                                  HOW, bit 18 */
  bool shi_lnav_how_alert;

  bool shi_cnav_alert_set; /* CNAV alert flag (SHI6 in SHM doc)
                              bit 38, each message */
  bool shi_cnav_alert;
} gps_sat_health_indicators_t;

/* GLO Satellite Health Indicator
 * Reference: GLO ICD 5.1.
 * This is (MSB of B || l).
 */
typedef struct {
  bool shi_set; /* SHI SV HEALTH */
  u8 shi;
} glo_sat_health_indicators_t;

/* BDS Satellite Health Indicator */
typedef struct {
  bool shi_set; /* SHI SV HEALTH */
  u8 shi;
} bds_sat_health_indicators_t;

/* GAL Satellite Health Indicator */
typedef struct {
  bool shi_set; /* SHI SV HEALTH */
  u8 shi;
} gal_sat_health_indicators_t;

void shm_gps_decode_shi_ephemeris(u32 sf1w3, u8* shi_ephemeris);

bool check_8bit_health_word(const u8 health_bits, const code_t code);
bool check_alma_page25_health_word(const u8 health_bits, const code_t code);
bool check_6bit_health_word(const u8 health_bits, const code_t code);
bool check_nav_dhi(const u8 health_8bits, const u8 disabled_errors);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_SHM_H_ */
