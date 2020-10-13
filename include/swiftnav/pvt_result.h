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
#define PROPAGATED_TIME 2
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

/* Define INS status types used in dr_runner */
#define INS_STATUS_AWAITING_INIT 0
#define INS_STATUS_ALIGNING 1
#define INS_STATUS_READY 2
#define INS_STATUS_OUTAGE_EXCEEDS_MAX 3
#define INS_STATUS_SEEDING 4
#define INS_STATUS_FASTSTARTING 5
#define INS_MODE_MASK 0x7                  /* Bit 0-2 */
#define INS_GNSS_FIX_AVAILABILITY_MASK 0x8 /* Bit 3 */
/* Bits 11-13 */
#define INS_STATUS_MOTION_STATE_UNKNOWN 0
#define INS_STATUS_MOTION_STATE_ARBITRARY_MOTION 1
#define INS_STATUS_MOTION_STATE_STRAIGHT_MOTION 2
#define INS_STATUS_MOTION_STATE_STATIONARY 3

/* Define GROUP_META flags used in fusion */
#define GROUP_META_TIME_REF_MASK 0x3        /* Bits 0-1 */
#define GROUP_META_SOLUTION_SOURCE_MASK 0xC /* Bits 2-3 */
#define GROUP_META_TIME_QUALITY_MASK 0x30   /* Bits 4-5 */
/* msg_group_meta.group_id */
#define GROUP_META_ID_UNKNOWN 0
#define GROUP_META_ID_FUSION_BESTPOS 1
#define GROUP_META_ID_GNSS 2
#define GROUP_META_N_GROUP_MSGS 14
/* max number of elements in msg_group_soln_meta.sol_in array */
#define SOLN_META_MAX_N_SOL_IN 20

/* Define SOLN_META flags used in fusion */
#define SOLN_META_ALIGNMENT_STATUS_UNKNOWN_OR_ALIGNED 0
#define SOLN_META_ALIGNMENT_STATUS_SEEDED_ALIGNING 1
#define SOLN_META_ALIGNMENT_STATUS_UNSEEDED_ALIGNING 2
#define SOLN_META_ALIGNMENT_STATUS_SEEDED_AWAITING 3
#define SOLN_META_ALIGNMENT_STATUS_UNSEEDED_AWAITING 4
/* soln_meta_msg.sensor_type[0:2] */
#define SOLN_META_SENSOR_TYPE_INVALID 0
#define SOLN_META_SENSOR_TYPE_GNSS_POS 1
#define SOLN_META_SENSOR_TYPE_GNSS_VEL_DELTA 2
#define SOLN_META_SENSOR_TYPE_GNSS_VEL_DOPPLER 3
#define SOLN_META_SENSOR_TYPE_ODO_TICKS 4
#define SOLN_META_SENSOR_TYPE_ODO_SPEED 5
#define SOLN_META_SENSOR_TYPE_IMU 6
/* sol_in[<GNSSInputType>].flags[0:1] */
#define SOLN_META_INPUT_TYPE_GNSS_POS 0
#define SOLN_META_INPUT_TYPE_GNSS_VEL_DOPPLER 1
#define SOLN_META_INPUT_TYPE_GNSS_VEL_DELTA 2
/* sol_in[<IMUInputType>].flags[0:1] */
#define SOLN_META_INPUT_TYPE_IMU_MEMS 0
#define SOLN_META_INPUT_TYPE_IMU_OTHER 1
/* sol_in[<IMUInputType>].flags[2:3] */
#define SOLN_META_INPUT_TYPE_IMU_GRADE_CONSUMER 0
#define SOLN_META_INPUT_TYPE_IMU_GRADE_TACTICAL 1
#define SOLN_META_INPUT_TYPE_IMU_GRADE_INTERM 2
#define SOLN_META_INPUT_TYPE_IMU_GRADE_SUP 3
/* sol_in[<IMUInputType>].flags[4:5] */
#define SOLN_META_INPUT_TYPE_IMU_TIME_REF_GPS 0
#define SOLN_META_INPUT_TYPE_IMU_TIME_REF_CPU 1
#define SOLN_META_INPUT_TYPE_IMU_TIME_REF_UNKNOWN 3
#define SOLN_META_INPUT_TYPE_IMU_TIME_REF_PPS 4
/* sol_in[<OdoInputType>].flags[0:1] */
#define SOLN_META_INPUT_TYPE_ODO_CLASS_SCALAR_TICKS 0
#define SOLN_META_INPUT_TYPE_ODO_CLASS_SCALAR_SPEED 1
#define SOLN_META_INPUT_TYPE_ODO_CLASS_MULTIDIM_TICKS 2
#define SOLN_META_INPUT_TYPE_ODO_CLASS_MULTIDIM_SPEED 3
/* sol_in[<OdoInputType>].flags[2:3] */
#define SOLN_META_INPUT_TYPE_ODO_GRADE_LOW 0
#define SOLN_META_INPUT_TYPE_ODO_GRADE_MED 1
#define SOLN_META_INPUT_TYPE_ODO_GRADE_SUP 2

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
