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

#ifndef LIBSWIFTNAV_PVT_ENGINE_PVT_RESULT_H_
#define LIBSWIFTNAV_PVT_ENGINE_PVT_RESULT_H_

#include <swiftnav/common.h>
#include <swiftnav/gnss_time.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/* Define Time types - currently aligned to SBP messages */
#define NO_TIME 0
#define GNSS_TIME 1
#define TIME_SOURCE_MASK 0x07 /* Bits 0-2 */
#define DEFAULT_UTC 0
#define NVM_UTC 1
#define DECODED_UTC 2
#define UTC_SOURCE_MASK 0x18 /* Bits 3-4 */

/* Define Position types - currently aligned to SBP messages */
#define POSITION_MODE_NONE 0
#define POSITION_MODE_SPP 1
#define POSITION_MODE_DGNSS 2
#define POSITION_MODE_FLOAT 3
#define POSITION_MODE_FIXED 4
#define POSITION_MODE_DEAD_RECKONING 5
#define POSITION_MODE_SBAS 6

/* Define Velocity types - currently aligned to SBP messages */
#define VELOCITY_MODE_NONE 0
#define VELOCITY_MODE_MEASURED_DOPPLER 1
#define VELOCITY_MODE_COMPUTED_DOPPLER 2
#define VELOCITY_MODE_DEAD_RECKONING 3

/* These masks are used in the firmware. */
#define POSITION_MODE_MASK 0x07 /* Bits 0-2 */
#define VELOCITY_MODE_MASK 0x07 /* Bits 0-2 */
#define RAIM_REPAIR_FLAG 0x80   /* Bit 7 */

typedef struct pvt_engine_result_flags_t {
  u8 position_mode : 3;
  u8 velocity_mode : 3;
  u8 raim_repair_flag : 1;
} pvt_engine_result_flags_t;

typedef struct {
  gps_time_t time;
  bool valid;
  double baseline[3];
  double baseline_covariance[9];
  bool velocity_valid;
  double velocity[3];
  double velocity_covariance[9];
  u8 num_sats_used;
  u8 num_sigs_used;
  pvt_engine_result_flags_t flags;
  bool has_known_reference_pos;
  double known_reference_pos[3];
  double propagation_time;
} pvt_engine_result_t;

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif
