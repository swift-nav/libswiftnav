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

#include <assert.h>

#include <swiftnav/constants.h>
#include <swiftnav/shm.h>
#include <swiftnav/signal.h>

/** Decode shi_ephemeris
 *  Refer to swiftnav/shm.h for details.
 *
 * \param sf1w3 word number 3 of subframe number 1
 * \param shi_ephemeris pointer to an shi_ephemeris variable
 */
void shm_gps_decode_shi_ephemeris(u32 sf1w3, u8* shi_ephemeris) {
  *shi_ephemeris = (sf1w3 >> 8) & 0x3F;
}

/** The six-bit health indication given by bits 17 through 22 of word three
 * refers to the transmitting SV. The MSB shall indicate a summary of the
 * health of the NAV data, where
 *   0 = all NAV data are OK,
 *   1 = some or all NAV data are bad.
 *
 * http://www.gps.gov/technical/icwg/IS-GPS-200H.pdf
 * 20.3.3.3.1.4 SV Health.
 *
 * \param health_bits SV health bits
 * \return true if the satellite health summary is OK.
 *         false otherwise.
 */
static bool gps_nav_data_health_summary(u8 health_bits) {
  u8 summary = (health_bits >> 5) & 0x1;

  return (0 == summary);
}

typedef enum gps_signal_component_health {
  ALL_SIGNALS_OK = 0,
  ALL_SIGNALS_WEAK,
  ALL_SIGNALS_DEAD,
  ALL_SIGNALS_NO_DATA,
  L1_P_SIGNAL_WEAK,
  L1_P_SIGNAL_DEAD,
  L1_P_SIGNAL_NO_DATA,
  L2_P_SIGNAL_WEAK,
  L2_P_SIGNAL_DEAD,
  L2_P_SIGNAL_NO_DATA,
  L1_C_SIGNAL_WEAK,
  L1_C_SIGNAL_DEAD,
  L1_C_SIGNAL_NO_DATA,
  L2_C_SIGNAL_WEAK,
  L2_C_SIGNAL_DEAD,
  L2_C_SIGNAL_NO_DATA,
  L1_L2_P_SIGNAL_WEAK,
  L1_L2_P_SIGNAL_DEAD,
  L1_L2_P_SIGNAL_NO_DATA,
  L1_L2_C_SIGNAL_WEAK,
  L1_L2_C_SIGNAL_DEAD,
  L1_L2_C_SIGNAL_NO_DATA,
  L1_SIGNAL_WEAK,
  L1_SIGNAL_DEAD,
  L1_SIGNAL_NO_DATA,
  L2_SIGNAL_WEAK,
  L2_SIGNAL_DEAD,
  L2_SIGNAL_NO_DATA,
  SV_TEMPORARILY_OUT,
  SV_WILL_BE_TEMPORARILY_OUT,
  ONLY_URA_VALID,
  MULTIPLE_PROBLEMS
} gps_signal_component_health_t;

/** Check SV health status from the almanac eight-bit word.
 *
 *  From IS-GPS-200H:
 *  The 3 MSBs of the eight-bit word provide NAV data health indication
 *  in accordance with the code given in Table 20-VII.
 *  The SV is considered healthy if ALL DATA OK is indicated.
 *
 *  For the 5 LSB the functionality of check_6bit_health_word() is re-used.
 *  NOTE: 6th bit is expected to be 0, as in ALL DATA OK, for this to work.
 *
 * \param health_bits SV health bits.
 *                    Refer to paragraph 20.3.3.5.1.3 for details.
 * \param code code for which health should be analyzed
 *
 * \return true if current signal is OK.
 *         false otherwise.
 */
bool check_8bit_health_word(const u8 health_bits, const code_t code) {
  if (!check_nav_dhi(health_bits, 0)) {
    return false;
  }
  return check_6bit_health_word(health_bits, code);
}

/** Check SV health status from the almanac six-bit word.
 *  These are the bits from almanac page 25, subframes 4 & 5.
 *
 *  From IS-GPS-200H paragraph 20.3.3.5.1.3:
 *  "6 ones" has a special meaning assigned.
 *  It indicates that the SV and related data is not available,
 *  and should be marked unusable.
 *
 *  For the 5 LSB the functionality of check_6bit_health_word() is re-used.
 *
 * \param health_bits SV health bits.
 *                    Refer to paragraph 20.3.3.5.1.3 for details.
 * \param code code for which health should be analyzed
 *
 * \return true if current signal is OK.
 *         false otherwise.
 */
bool check_alma_page25_health_word(const u8 health_bits, const code_t code) {
  u8 b = health_bits & 0x1f;
  if (gps_nav_data_health_summary(health_bits) && (MULTIPLE_PROBLEMS == b)) {
    return false;
  }
  return check_6bit_health_word(health_bits, code);
}

/** Check GPS NAV data health summary and signal component status.
 *
 *  From IS-GPS-200H:
 *  The six-bit words provide a one-bit summary of the NAV data's health status
 *  in the MSB position in accordance with paragraph 20.3.3.3.1.4. The five LSBs
 *  of both the eight-bit and the six-bit words provide the health status of the
 *  SV's signal components in accordance with the code given in Table 20-VIII.
 *
 * \param health_bits SV health bits. Refer to gps_nav_data_health_summary()
 *        documentation above for details.
 * \param code code for which health should be analyzed
 *
 * \return SHM_STATE_HEALTHY   if current signal is known to be healthy,
 *         SHM_STATE_UNHEALTHY if current signal is known to be unhealthy,
 *         SHM_STATE_UNKNOWN   otherwise
 */
static shm_state_t check_6bit_health(const u8 health_bits, const code_t code) {
  /* Check NAV data health summary */
  if (((CODE_GPS_L1CA == code) || (CODE_AUX_GPS == code) ||
       (CODE_GPS_L1P == code)) &&
      !gps_nav_data_health_summary(health_bits)) {
    return SHM_STATE_UNHEALTHY;
  }

  const u8 b = health_bits & 0x1f;

  /* Check general issues */
  if (b == ALL_SIGNALS_WEAK || b == ALL_SIGNALS_DEAD ||
      b == ALL_SIGNALS_NO_DATA || b == SV_TEMPORARILY_OUT ||
      b == SV_WILL_BE_TEMPORARILY_OUT || b == ONLY_URA_VALID ||
      b == MULTIPLE_PROBLEMS) {
    return SHM_STATE_UNHEALTHY;
  }

  /* Check code specific issues */
  switch ((s8)code) {
    case CODE_GPS_L1CA:
    case CODE_AUX_GPS:
      if (b == L1_C_SIGNAL_WEAK || b == L1_C_SIGNAL_DEAD ||
          b == L1_C_SIGNAL_NO_DATA || b == L1_L2_C_SIGNAL_WEAK ||
          b == L1_L2_C_SIGNAL_DEAD || b == L1_L2_C_SIGNAL_NO_DATA ||
          b == L1_SIGNAL_WEAK || b == L1_SIGNAL_DEAD ||
          b == L1_SIGNAL_NO_DATA) {
        return SHM_STATE_UNHEALTHY;
      }
      return SHM_STATE_HEALTHY;

    case CODE_GPS_L2CM:
    case CODE_GPS_L2CL:
    case CODE_GPS_L2CX:
      if (b == L2_C_SIGNAL_WEAK || b == L2_C_SIGNAL_DEAD ||
          b == L2_C_SIGNAL_NO_DATA || b == L1_L2_C_SIGNAL_WEAK ||
          b == L1_L2_C_SIGNAL_DEAD || b == L1_L2_C_SIGNAL_NO_DATA ||
          b == L2_SIGNAL_WEAK || b == L2_SIGNAL_DEAD ||
          b == L2_SIGNAL_NO_DATA) {
        return SHM_STATE_UNHEALTHY;
      }
      return SHM_STATE_HEALTHY;

    case CODE_GPS_L1P:
      if (b == L1_P_SIGNAL_WEAK || b == L1_P_SIGNAL_DEAD ||
          b == L1_P_SIGNAL_NO_DATA || b == L1_L2_P_SIGNAL_WEAK ||
          b == L1_L2_P_SIGNAL_DEAD || b == L1_L2_P_SIGNAL_NO_DATA ||
          b == L1_SIGNAL_WEAK || b == L1_SIGNAL_DEAD ||
          b == L1_SIGNAL_NO_DATA) {
        return SHM_STATE_UNHEALTHY;
      }
      return SHM_STATE_HEALTHY;

    case CODE_GPS_L2P:
      if (b == L2_P_SIGNAL_WEAK || b == L2_P_SIGNAL_DEAD ||
          b == L2_P_SIGNAL_NO_DATA || b == L1_L2_P_SIGNAL_WEAK ||
          b == L1_L2_P_SIGNAL_DEAD || b == L1_L2_P_SIGNAL_NO_DATA ||
          b == L2_SIGNAL_WEAK || b == L2_SIGNAL_DEAD ||
          b == L2_SIGNAL_NO_DATA) {
        return SHM_STATE_UNHEALTHY;
      }
      return SHM_STATE_HEALTHY;

    case CODE_GPS_L5I:
    case CODE_GPS_L5Q:
    case CODE_GPS_L5X:
      return SHM_STATE_HEALTHY;

    default:
      assert(!"Unsupported code");
      return SHM_STATE_UNKNOWN;
  }
}

/** Check 6-bit health word.
 *
 * \param health_bits SV health bits. Refer to gps_nav_data_health_summary()
 *        documentation above for details.
 * \param code code for which health should be analyzed
 *
 * \returns false if signal health of specified signal is SHM_STATE_UNHEALTHY,
 *          true otherwise
 */
bool check_6bit_health_word(const u8 health_bits, const code_t code) {
  /* Currently check_6bit_health() will return SHM_STATE_UNKNOWN for GPS L5
   * signals only, until proper health status support is implemented. */
  return (SHM_STATE_UNHEALTHY != check_6bit_health(health_bits, code));
}

/** Check NAV data health indications
 * \param[in] health_8bits 8-bit health word
 * \param[in] disabled_errors Mask for disabling unrelevant errors
 *
 * \return true if no errors detected
 *         false if errors detected
 */
bool check_nav_dhi(const u8 health_8bits, const u8 disabled_errors) {
  u8 nav_dhi = (health_8bits >> 5) & 0x7;

  /* All clear */
  if (NAV_DHI_OK == nav_dhi) {
    return true;
  }

  /* Error is masked */
  if (disabled_errors & (1 << nav_dhi)) {
    return true;
  }

  /* Error is indicated and not masked */
  return false;
}
