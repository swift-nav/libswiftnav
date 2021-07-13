/*
 * Copyright (C) 2020 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "swiftnav/decode_glo.h"

#include <assert.h>

#include <swiftnav/bits.h>
#include <swiftnav/ephemeris.h>
#include <swiftnav/logging.h>

/* GLO fit intervals do not seem to overlap.
   P1 field from string 1 defines a fit interval.
   tb field from string 2 changes at the beginning of the fit interval.
   tb field is TOE.
   The previous fit interval ends at the start of the frame, which delivers
   the new tb value. So there is a small time window, after the old ephemeris
   gets stale and the new ephemeris is not decoded yet.
   This define sets an overlap margin of #FIT_INTERVAL_MARGIN_S / 2 seconds
   to allow for the new ephemeris decoding. */
#define FIT_INTERVAL_MARGIN_S (10 * MINUTE_SECS)

enum glo_sv_model { SV_GLONASS, SV_GLONASS_M };

/* GLO parameter limits (ICD L1,L2 GLONASS edition 5.1 2008 Table 4.5) */
#define GLO_POS_MAX_M (2.7e4 * 1e3)         /* [m] */
#define GLO_VEL_MAX_M_S (4.3 * 1e3)         /* [m/s] */
#define GLO_ACC_MAX_M_S2 (6.2e-9 * 1e3)     /* [m/s^2] */
#define GLO_TK_MAX_HOURS 23                 /* [hours] */
#define GLO_TK_MAX_MINS 59                  /* [mins] */
#define GLO_TB_MIN_S (15 * MINUTE_SECS)     /* [s] */
#define GLO_TB_MAX_S (1425 * MINUTE_SECS)   /* [s] */
#define GLO_GAMMA_MAX 9.313225746154785e-10 /* 2^(-30)[unitless] */
#define GLO_TAU_MAX_S 0.001953125           /* 2^(-9)[s] */
#define GLO_D_TAU_MAX_S 13.97e-9            /* [s] */
#define GLO_NT_MAX_DAYS 1461                /* [days] */

/* ICD L1,L2 GLONASS edition 5.1 2008 Table 4.9 */
#define GLO_TAU_GPS_MAX_S 1.9e-3 /* [s] */

/* Word Ft (accuracy of measurements), refer to GLO ICD, Table 4.4 */
static const float f_t[] = {1.0f,
                            2.0f,
                            2.5f,
                            4.0f,
                            5.0f,
                            7.0f,
                            10.0f,
                            12.0f,
                            14.0f,
                            16.0f,
                            32.0f,
                            64.0f,
                            128.0f,
                            256.0f,
                            512.0f,
                            INVALID_URA_VALUE};

/* Word P1 (Time interval between adjacent values of tb).
   Refer to table 4.3 of GLO ICD */
static const u8 p1_lookup_min[] = {0, 30, 45, 60}; /* [min] */

/* These bit masks (for data bits 9..85) correspond to table 4.13 of GLO ICD
 * used in error correction algorithm */
static const u32 e_masks[7][3] = {
    {0xaaad5b00, 0x55555556, 0xaaaab},
    {0x33366d00, 0x9999999b, 0xccccd},
    {0xc3c78e00, 0xe1e1e1e3, 0x10f0f1},
    {0xfc07f000, 0xfe01fe03, 0xff01},
    {0xfff80000, 0xfffe0003, 0x1f0001},
    {0, 0xfffffffc, 1},
    {0, 0, 0x1ffffe},
};

/** Extract a word of n_bits length (n_bits <= 32) at position bit_index into
 * the subframe. Refer to bit index to Table 4.6 and 4.11 in GLO ICD 5.1 (pg.
 * 34)
 * \param n pointer to GLO navigation string to be parsed
 * \param bit_index number of bit the extract process start with. Range [1..85]
 * \param n_bits how many bits should be extracted [1..32]
 * \return word extracted from navigation string
 */
u32 extract_word_glo(const glo_string_t *string, u16 bit_index, u8 n_bits) {
  assert(bit_index);
  assert(bit_index <= GLO_NAV_STR_BITS);

  assert(n_bits);
  assert(n_bits <= 32);

  bit_index--;
  u32 word = 0;
  u8 bix_hi = bit_index >> 5;
  u8 bix_lo = bit_index & 0x1F;
  if (bix_lo + n_bits <= 32) {
    word = string->word[bix_hi] >> bix_lo;
    word &= (0xffffffff << (32 - n_bits)) >> (32 - n_bits);
  } else {
    u8 s = 32 - bix_lo;
    word = extract_word_glo(string, bit_index + 1, s) |
           extract_word_glo(string, bit_index + 1 + s, n_bits - s) << s;
  }

  return word;
}

/** The function performs data verification and error detection
 * in received GLO navigation string. Refer to GLO ICD, section 4.7
 * \param string pointer to GLO navigation string
 * \return -1 -- received string is bad and should be dropped out,
 *          0 -- received string is good
 *          >0 -- number of bit in n->string_bits to be corrected (inverted)
 *                range[9..85]*/
s8 error_detection_glo(const glo_string_t *string) {
  u8 c = 0;
  u32 data1, data2, data3;
  bool p0, p1, p2, p3, beta, c_sum;
  u8 bit_set = 0;
  u8 k = 0;

  /* calculate C1..7 */
  for (u8 i = 0; i < 7; i++) {
    /* extract corresponding check bit of Hamming code */
    beta = extract_word_glo(string, i + 1, 1);
    /* extract data bits and apply mask */
    data1 = extract_word_glo(string, 1, 32) & e_masks[i][0];
    data2 = extract_word_glo(string, 33, 32) & e_masks[i][1];
    data3 = extract_word_glo(string, 65, 32) & e_masks[i][2];
    /* calculate parity for data[1..3] */
    p1 = parity(data1);
    p2 = parity(data2);
    p3 = parity(data3);
    bool p = beta ^ p1 ^ p2 ^ p3;
    /* calculate common parity and set according C bit */
    c |= (u8)p << i;
    if (p) {
      bit_set++; /* how many bits are set, used in error criteria */
      k = i + 1; /* store number of most significant checksum not equal to 0,
                    used in error criteria */
    }
  }

  /* calculate C sum */
  data1 = extract_word_glo(string, 1, 32) & 0xffffff00;
  data2 = extract_word_glo(string, 33, 32);
  data3 = extract_word_glo(string, 65, 32);
  p1 = parity(data1);
  p2 = parity(data2);
  p3 = parity(data3);
  p0 = parity(extract_word_glo(string, 1, 8));
  c_sum = p0 ^ p1 ^ p2 ^ p3;

  /* Now check C word to figure out is the string good, bad or
   * correction is needed */

  /* case a) from ICD */
  if ((!c_sum && !bit_set) || (1 == bit_set && c_sum)) {
    return 0; /* The string is good */
  }

  /* case b) from ICD */
  if (bit_set > 1 && c_sum) {
    u8 i_corr = (c & 0x7f) + 8 - k; /* define number of bit to be corrected */

    if (i_corr > GLO_NAV_STR_BITS) {
      return -1; /* odd number of multiple errors, bad string */
    }

    return i_corr; /* return the bit to be corrected */
  }

  /* case c) from ICD */
  if ((bit_set > 0 && !c_sum) || (0 == bit_set && c_sum)) {
    return -1; /* multiple errors, bad string */
  }

  /* should not be here */
  log_error("GLO error correction: unexpected case");
  return -1;
}

/** Decode position component of the ephemeris data (X/Y/Z)
 * \param string GLO nav string 1, 2, or 3
 * \return The decoded position component [m]
 */
static double decode_position_component(const glo_string_t *string) {
  double pos_m = extract_word_glo(string, 9, 26) * C_1_2P11 * 1000.0;
  u8 sign = extract_word_glo(string, 9 + 26, 1);
  if (sign) {
    pos_m *= -1;
  }
  return pos_m;
}

/** Decode velocity component of the ephemeris data (Vx/Vy/Vz)
 * \param string GLO nav string 1, 2, or 3
 * \return The decoded velocity component [m/s]
 */
static double decode_velocity_component(const glo_string_t *string) {
  /* extract velocity (Vx or Vy or Vz) */
  double vel_mps = extract_word_glo(string, 41, 23) * C_1_2P20 * 1000.0;
  u8 sign = extract_word_glo(string, 41 + 23, 1);
  if (sign) {
    vel_mps *= -1;
  }
  return vel_mps;
}

/** Decode acceleration component of the ephemeris data (Ax/Ay/Az)
 * \param string GLO nav string 1, 2, or 3
 * \return The decoded acceleration component [m/s^2]
 */
static double decode_acceleration_component(const glo_string_t *string) {
  /* extract acceleration (Ax or Ay or Az) */
  double acc_mps2 = extract_word_glo(string, 36, 4) * C_1_2P30 * 1000.0;
  u8 sign = extract_word_glo(string, 36 + 4, 1);
  if (sign) {
    acc_mps2 *= -1;
  }
  return acc_mps2;
}

static u32 compute_ephe_fit_interval(const ephemeris_t *eph, u32 p1) {
  assert(eph);
  assert(p1 < ARRAY_SIZE(p1_lookup_min));

  u32 fit_interval_s = MINUTE_SECS * p1_lookup_min[p1];
  if (fit_interval_s != 0) {
    return fit_interval_s + FIT_INTERVAL_MARGIN_S;
  }

  if (0 == eph->fit_interval) {
    /* We have not decoded any fit interval yet,
       So let's default to the maximum fit interval possible + a margin.
       The maximum fit interval is defined by the maximum value of P1,
       which is 60 minutes. Once we have a real value from P1 we will
       start using the real value. */
    fit_interval_s = MINUTE_SECS * 60 + FIT_INTERVAL_MARGIN_S;
  } else {
    fit_interval_s = eph->fit_interval;
  }

  return fit_interval_s;
}

bool decode_glo_string_1(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk) {
  assert(string);
  assert(eph);
  assert(tk);

  if (0 != error_detection_glo(string)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: checksum mismatch");
    return false;
  }

  if (extract_word_glo(string, 81, 4) != 1) {
    return false;
  }

  double pos_m = decode_position_component(string); /* extract x */
  if ((pos_m < -GLO_POS_MAX_M) || (GLO_POS_MAX_M < pos_m)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: pos_x =%lf m", pos_m);
    return false;
  }
  eph->data.glo.pos[0] = pos_m;

  double vel_m_s = decode_velocity_component(string); /* extract Vx */
  if ((vel_m_s < -GLO_VEL_MAX_M_S) || (GLO_VEL_MAX_M_S < vel_m_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: vel_x=%lf m/s", vel_m_s);
    return false;
  }
  eph->data.glo.vel[0] = vel_m_s;

  double acc_m_s2 = decode_acceleration_component(string); /* extract Ax */
  if ((acc_m_s2 < -GLO_ACC_MAX_M_S2) || (GLO_ACC_MAX_M_S2 < acc_m_s2)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: acc_x=%lf m/s^2", acc_m_s2);
    return false;
  }
  eph->data.glo.acc[0] = acc_m_s2;

  /* extract tk */
  tk->h = (u8)extract_word_glo(string, 72, 5);
  if (tk->h > GLO_TK_MAX_HOURS) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: tk_h=%" PRIu8 " h", tk->h);
    return false;
  }
  tk->m = (u8)extract_word_glo(string, 66, 6);
  if (tk->m > GLO_TK_MAX_MINS) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: tk_m=%" PRIu8 " min", tk->m);
    return false;
  }
  tk->s = extract_word_glo(string, 65, 1) ? MINUTE_SECS / 2 : 0.0;

  /* extract P1 */
  u32 p1 = extract_word_glo(string, 77, 2);
  eph->fit_interval = compute_ephe_fit_interval(eph, p1);

  return true;
}

bool decode_glo_string_2(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *toe) {
  assert(string);
  assert(eph);
  assert(toe);
  if (0 != error_detection_glo(string)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: checksum mismatch");
    return false;
  }

  if (extract_word_glo(string, 81, 4) != 2) {
    return false;
  }

  double pos_m = decode_position_component(string); /* extract y */
  if ((pos_m < -GLO_POS_MAX_M) || (GLO_POS_MAX_M < pos_m)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: pos_y =%lf m", pos_m);
    return false;
  }
  eph->data.glo.pos[1] = pos_m;

  double vel_m_s = decode_velocity_component(string); /* extract Vy */
  if ((vel_m_s < -GLO_VEL_MAX_M_S) || (GLO_VEL_MAX_M_S < vel_m_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: vel_y=%lf m/s", vel_m_s);
    return false;
  }
  eph->data.glo.vel[1] = vel_m_s;

  double acc_m_s2 = decode_acceleration_component(string); /* extract Ay */
  if ((acc_m_s2 < -GLO_ACC_MAX_M_S2) || (GLO_ACC_MAX_M_S2 < acc_m_s2)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: acc_y=%lf m/s^2", acc_m_s2);
    return false;
  }
  eph->data.glo.acc[1] = acc_m_s2;

  /* extract MSB of B (if the bit is 0 the SV is OK ) */
  eph->health_bits |= extract_word_glo(string, 80, 1);

  u32 tb_s = extract_word_glo(string, 70, 7) * 15 * MINUTE_SECS;
  if ((tb_s < GLO_TB_MIN_S) || (GLO_TB_MAX_S < tb_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: tb_s=%" PRIu32 " s", tb_s);
    return false;
  }
  toe->h = tb_s / HOUR_SECS;
  toe->m = (tb_s - toe->h * HOUR_SECS) / MINUTE_SECS;
  toe->s = tb_s - (toe->h * HOUR_SECS) - (toe->m * MINUTE_SECS);
  eph->data.glo.iod = tb_s & 0x7f; /* 7 LSB of Tb as IOD */

  return true;
}

bool decode_glo_string_3(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *toe) {
  assert(string);
  assert(eph);
  assert(toe);
  if (0 != error_detection_glo(string)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: checksum mismatch");
    return false;
  }

  if (extract_word_glo(string, 81, 4) != 3) {
    return false;
  }

  double pos_m = decode_position_component(string); /* extract z */
  if ((pos_m < -GLO_POS_MAX_M) || (GLO_POS_MAX_M < pos_m)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: pos_z =%lf m", pos_m);
    return false;
  }
  eph->data.glo.pos[2] = pos_m;

  double vel_m_s = decode_velocity_component(string); /* extract Vz */
  if ((vel_m_s < -GLO_VEL_MAX_M_S) || (GLO_VEL_MAX_M_S < vel_m_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: vel_z=%lf m/s", vel_m_s);
    return false;
  }
  eph->data.glo.vel[2] = vel_m_s;

  double acc_m_s2 = decode_acceleration_component(string); /* extract Az */
  if ((acc_m_s2 < -GLO_ACC_MAX_M_S2) || (GLO_ACC_MAX_M_S2 < acc_m_s2)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: acc_z=%lf m/s^2", acc_m_s2);
    return false;
  }
  eph->data.glo.acc[2] = acc_m_s2;

  /* extract gamma */
  double gamma = extract_word_glo(string, 69, 10) * C_1_2P40;
  ;
  u8 sign = extract_word_glo(string, 69 + 10, 1);
  if (sign) {
    gamma *= -1;
  }
  if ((gamma < -GLO_GAMMA_MAX) || (GLO_GAMMA_MAX < gamma)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: gamma=%lf", gamma);
    return false;
  }
  eph->data.glo.gamma = gamma;
  /* extract l, if it is 0 the SV is OK, so OR it with B */
  eph->health_bits |= extract_word_glo(string, 65, 1);

  return true;
}

bool decode_glo_string_4(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk,
                         glo_time_t *toe,
                         u8 *age_of_data_days) {
  assert(string);
  assert(eph);
  assert(tk);
  assert(toe);
  assert(age_of_data_days);
  if (0 != error_detection_glo(string)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: checksum mismatch");
    return false;
  }

  if (extract_word_glo(string, 81, 4) != 4) {
    return false;
  }

  /* extract tau */
  double tau_s = (s32)extract_word_glo(string, 59, 21) * C_1_2P30;
  u8 sign = extract_word_glo(string, 59 + 21, 1);
  if (sign) {
    tau_s *= -1;
  }
  if ((tau_s < -GLO_TAU_MAX_S) || (GLO_TAU_MAX_S < tau_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: tau=%lf s", tau_s);
    return false;
  }
  eph->data.glo.tau = tau_s;

  /* extract d_tau */
  double d_tau_s = extract_word_glo(string, 54, 4) * C_1_2P30;
  sign = extract_word_glo(string, 54 + 4, 1);
  if (sign) {
    d_tau_s *= -1;
  }
  if ((d_tau_s < -GLO_D_TAU_MAX_S) || (GLO_D_TAU_MAX_S < d_tau_s)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: d_tau=%lf s", d_tau_s);
    return false;
  }
  eph->data.glo.d_tau = d_tau_s;

  /* extract E_n age of data */
  *age_of_data_days = extract_word_glo(string, 49, 5);

  /* extract n */
  u16 glo_slot_id = extract_word_glo(string, 11, 5);
  if (!glo_slot_id_is_valid(glo_slot_id)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: glo_slot_id=%" PRIu16, glo_slot_id);
    return false;
  }
  eph->sid.sat = glo_slot_id;

  /* extract Ft (URA) */
  eph->ura = f_t[extract_word_glo(string, 30, 4)];

  /*extract Nt*/
  u16 nt_days = (u16)extract_word_glo(string, 16, 11);
  if (GLO_NT_MAX_DAYS < nt_days) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: nt=%" PRIu16 " days", nt_days);
    return false;
  }
  tk->nt = nt_days;
  toe->nt = nt_days;

  u32 M = extract_word_glo(string, 9, 2);
  if (SV_GLONASS == M) {
    /* this breaks the assumption that all visible GLO satellites should be
       at least of "Glonass M" model*/
    log_warn_sid(eph->sid, "Non GLONASS M SV detected");
  }
  return true;
}

bool decode_glo_string_5(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk,
                         glo_time_t *toe,
                         float *tau_gps_s) {
  assert(string);
  assert(eph);
  assert(tk);
  assert(toe);
  assert(tau_gps_s);
  if (0 != error_detection_glo(string)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: checksum mismatch");
    return false;
  }

  if (extract_word_glo(string, 81, 4) != 5) {
    return false;
  }

  /* extract N4 */
  u8 n4 = (u8)extract_word_glo(string, 32, 5);
  /* ICD L1,L2 GLONASS edition 5.1 2008 Table 4.5 Table 4.9 */
  if (0 == n4) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: n4=0");
    return false;
  }
  tk->n4 = n4;
  toe->n4 = n4;

  /* extract tau GPS [s] */
  double tau_gps = extract_word_glo(string, 10, 21) * C_1_2P30;
  u8 sign = extract_word_glo(string, 31, 1);
  if (sign) {
    tau_gps *= -1;
  }
  if (GLO_TAU_GPS_MAX_S < fabs(tau_gps)) {
    log_debug_sid(eph->sid, "GLO-NAV-ERR: tau_gps=%lf", tau_gps);
    return false;
  }

  *tau_gps_s = (float)tau_gps;

  return true;
}
