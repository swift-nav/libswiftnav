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
