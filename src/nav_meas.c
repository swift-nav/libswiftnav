/*
 * Copyright (C) 2017 Swift Navigation Inc.
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
#include <swiftnav/common.h>
#include <swiftnav/nav_meas.h>

static bool is_float_eq(const double a, const double b) {
  return fabs(a - b) < FLOAT_EQUALITY_EPS;
}

bool measurement_std_equal(const measurement_std_t *a,
                           const measurement_std_t *b) {
  if (sid_compare(a->sid, b->sid) != 0) {
    return false;
  }
  if (!is_float_eq(a->iono_std, b->iono_std)) {
    return false;
  }
  if (!is_float_eq(a->tropo_std, b->tropo_std)) {
    return false;
  }
  if (!is_float_eq(a->range_std, b->range_std)) {
    return false;
  }
  if (a->flags != b->flags) {
    return false;
  }
  return true;
}

bool nav_meas_equal(const navigation_measurement_t *a,
                    const navigation_measurement_t *b) {
  if (!is_float_eq(a->raw_pseudorange, b->raw_pseudorange)) {
    return false;
  }

  if (!is_float_eq(a->raw_carrier_phase, b->raw_carrier_phase)) {
    return false;
  }

  if (!is_float_eq(a->raw_measured_doppler, b->raw_measured_doppler)) {
    return false;
  }

  if (!is_float_eq(a->raw_computed_doppler, b->raw_computed_doppler)) {
    return false;
  }

  if (!is_float_eq(a->sat_pos[0], b->sat_pos[0]) ||
      !is_float_eq(a->sat_pos[1], b->sat_pos[1]) ||
      !is_float_eq(a->sat_pos[2], b->sat_pos[2])) {
    return false;
  }

  if (!is_float_eq(a->sat_vel[0], b->sat_vel[0]) ||
      !is_float_eq(a->sat_vel[1], b->sat_vel[1]) ||
      !is_float_eq(a->sat_vel[2], b->sat_vel[2])) {
    return false;
  }

  if (!is_float_eq(a->sat_acc[0], b->sat_acc[0]) ||
      !is_float_eq(a->sat_acc[1], b->sat_acc[1]) ||
      !is_float_eq(a->sat_acc[2], b->sat_acc[2])) {
    return false;
  }

  if (a->eph_key != b->eph_key) {
    return false;
  }

  if (!is_float_eq(a->sat_clock_err, b->sat_clock_err)) {
    return false;
  }

  if (!is_float_eq(a->sat_clock_err_rate, b->sat_clock_err_rate)) {
    return false;
  }

  if (!is_float_eq(a->cn0, b->cn0)) {
    return false;
  }

  if (!is_float_eq(a->lock_time, b->lock_time)) {
    return false;
  }

  if (!is_float_eq(a->elevation, b->elevation)) {
    return false;
  }

  if (!is_float_eq(gpsdifftime(&a->tot, &b->tot), 0)) {
    return false;
  }

  if (!sid_is_equal(a->sid, b->sid)) {
    return false;
  }

  if (a->flags != b->flags) {
    return false;
  }

  return true;
}

/** Compare navigation message by PRN.
 * This function is designed to be used together with qsort() etc.
 */
int nav_meas_cmp(const void *a, const void *b) {
  return sid_compare(((const navigation_measurement_t *)a)->sid,
                     ((const navigation_measurement_t *)b)->sid);
}

bool nav_meas_flags_valid(nav_meas_flags_t flags) {
  const nav_meas_flags_t all_valid =
      NAV_MEAS_FLAG_CODE_VALID & NAV_MEAS_FLAG_PHASE_VALID &
      NAV_MEAS_FLAG_MEAS_DOPPLER_VALID & NAV_MEAS_FLAG_COMP_DOPPLER_VALID &
      NAV_MEAS_FLAG_HALF_CYCLE_KNOWN & NAV_MEAS_FLAG_CN0_VALID;
  return ~all_valid & flags;
}

bool pseudorange_valid(const navigation_measurement_t *meas) {
  return (meas->flags & NAV_MEAS_FLAG_CODE_VALID) &&
         !(meas->flags & NAV_MEAS_FLAG_RAIM_EXCLUSION);
}

/** Convert navigation_measurement_t.lock_time into SBP lock time.
 *
 * Note: It is encoded according to DF402 from the RTCM 10403.2 Amendment 2
 * specification.  Valid values range from 0 to 15 and the most significant
 * nibble is reserved for future use.
 *
 * \param nm_lock_time Navigation measurement lock time [s]
 * \return SBP lock time
 */
u8 encode_lock_time(double nm_lock_time) {
  assert(nm_lock_time >= 0.0);

  /* Convert to milliseconds */
  u32 ms_lock_time;
  if (nm_lock_time < UINT32_MAX) {
    ms_lock_time = (u32)(nm_lock_time * SECS_MS);
  } else {
    ms_lock_time = UINT32_MAX;
  }

  if (ms_lock_time < 32) {
    return 0;
  }
  for (u8 i = 0; i < 16; i++) {
    if (ms_lock_time > (1u << (i + 5))) {
      continue;
    }
    return i;
  }
  return 15;
}

/** Convert SBP lock time into navigation_measurement_t.lock_time.
 *
 * Note: It is encoded according to DF402 from the RTCM 10403.2 Amendment 2
 * specification.  Valid values range from 0 to 15 and the most significant
 * nibble is reserved for future use.
 *
 * \param sbp_lock_time SBP lock time
 * \return Minimum possible lock time [s]
 */
double decode_lock_time(u8 sbp_lock_time) {
  /* MSB nibble is reserved */
  sbp_lock_time &= 0x0F;

  u32 ms_lock_time;
  if (sbp_lock_time == 0) {
    ms_lock_time = 0;
  } else {
    ms_lock_time = 1u << (sbp_lock_time + 4);
  }

  /* Convert to seconds */
  return (double)ms_lock_time / SECS_MS;
}

double nav_meas_cor_sat_clk_on_pseudorange(
    const navigation_measurement_t *nav_meas) {
  return (nav_meas->raw_pseudorange + GPS_C * nav_meas->sat_clock_err);
}

double nav_meas_cor_sat_clk_on_measured_doppler(
    const navigation_measurement_t *nav_meas) {
  double carrier_freq = sid_to_carr_freq(nav_meas->sid);
  return (nav_meas->raw_measured_doppler +
          nav_meas->sat_clock_err_rate * carrier_freq);
}

double nav_meas_cor_sat_clk_on_carrier_phase(
    const navigation_measurement_t *nav_meas) {
  double carrier_freq = sid_to_carr_freq(nav_meas->sid);
  return (nav_meas->raw_carrier_phase + nav_meas->sat_clock_err * carrier_freq);
}
