/*
 * Copyright (C) 2014-2017 Swift Navigation Inc.
 * Contact: Fergus Noble <dev@swift-nav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef PIKSI_FIRMWARE_PRIVATE_GLONASS_PHASE_BIASES_H
#define PIKSI_FIRMWARE_PRIVATE_GLONASS_PHASE_BIASES_H
#include <math.h>

#include <swiftnav/common.h>
#include <swiftnav/constants.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_GLO_BIASES_MULTIPLIER 50.0

#define RTCM1230_MASK_L2P ((u8)1 << 0)
#define RTCM1230_MASK_L2OF ((u8)1 << 1)
#define RTCM1230_MASK_L1P ((u8)1 << 2)
#define RTCM1230_MASK_L1OF ((u8)1 << 3)

typedef struct {
  /* The following constants describes the biases that will be sent through
   * SBP_MSG_GLO_BIASES in the sbp stream. Biases are to be expressed in meters
   * and are not quantized
   */
  u8 mask;            /**< GLONASS FDMA signals mask [boolean] */
  double l1of_bias_m; /**< GLONASS L1 OF Code-Phase Bias [m] */
  double l1p_bias_m;  /**< GLONASS L1 P Code-Phase Bias [m] */
  double l2of_bias_m; /**< GLONASS L2 OF Code-Phase Bias [m] */
  double l2p_bias_m;  /**< GLONASS L2 P Code-Phase Bias [m] */
} glo_biases_t;

bool glonass_biases_are_equal(const glo_biases_t biases1,
                              const glo_biases_t biases2);

bool is_bias_mask_flag_set(const u8 msg_flags, const u8 flag);

// This constant structure contains the Piksi GLONASS phase bias
static const glo_biases_t piksi_glonass_biases = {.mask = 255,
                                                  .l1of_bias_m = 0.,
                                                  .l1p_bias_m = 0.,
                                                  .l2of_bias_m = 0.,
                                                  .l2p_bias_m = 0.};

double get_glonass_bias(const code_t code, const glo_biases_t biases);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // PIKSI_FIRMWARE_PRIVATE_GLONASS_PHASE_BIASES_H
