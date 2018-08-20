/*
 * Copyright (c) 2014-2017 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <swiftnav/glonass_phase_biases.h>

bool glonass_biases_are_equal(const glo_biases_t biases1,
                              const glo_biases_t biases2) {
  if (biases1.mask != biases2.mask) {
    return false;
  }
  if (fabs(biases1.l1of_bias_m - biases2.l1of_bias_m) > FLOAT_EQUALITY_EPS) {
    return false;
  }
  if (fabs(biases1.l2of_bias_m - biases2.l2of_bias_m) > FLOAT_EQUALITY_EPS) {
    return false;
  }
  if (fabs(biases1.l1p_bias_m - biases2.l1p_bias_m) > FLOAT_EQUALITY_EPS) {
    return false;
  }
  if (fabs(biases1.l2p_bias_m - biases2.l2p_bias_m) > FLOAT_EQUALITY_EPS) {
    return false;
  }
  return true;
}

bool is_bias_mask_flag_set(const u8 msg_flags, const u8 flag) {
  return (msg_flags & flag) == flag;
}

double get_glonass_bias(const code_t code, const glo_biases_t biases) {
  /* For each frequency, we first check if the OF bias is present and return it.
   * If it is not present, we return the P-code bias. For instance Geo++ seems
   * to
   * be configured to broadcast L1OF and L2P biases probably to mimic GPS signal
   * table.
   * Finally, we return zero if none of the mask bit fore a given frequency is
   * set, as
   * the Septentrio does not seem to set its L1OF mask bit set to zero.
   * We do Piksi minus base biases as LSN filter sdiffs convention is rover
   * minus base
   * */
  if (code == CODE_GLO_L1OF) {
    if (is_bias_mask_flag_set(biases.mask, RTCM1230_MASK_L1OF)) {
      return piksi_glonass_biases.l1of_bias_m - biases.l1of_bias_m;
    }
    if (is_bias_mask_flag_set(biases.mask, RTCM1230_MASK_L1P)) {
      return piksi_glonass_biases.l1of_bias_m - biases.l1p_bias_m;
    }
  }
  if (code == CODE_GLO_L2OF) {
    if (is_bias_mask_flag_set(biases.mask, RTCM1230_MASK_L2OF)) {
      return piksi_glonass_biases.l2of_bias_m - biases.l2of_bias_m;
    }
    if (is_bias_mask_flag_set(biases.mask, RTCM1230_MASK_L2P)) {
      return piksi_glonass_biases.l2of_bias_m - biases.l2p_bias_m;
    }
  }
  return 0.0;
}
