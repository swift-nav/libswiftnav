/*
 * Copyright (c) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *          Pasi Miettinen <pasi.miettinen@exafore.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_SIGNAL_H
#define LIBSWIFTNAV_SIGNAL_H

#include <stdbool.h>

#include <swiftnav/common.h>
#include <swiftnav/logging.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \addtogroup signal
 * \{ */

/* Number of satellites in each constellation. */
#define NUM_SATS_GPS 32
#define NUM_SATS_SBAS 19
/* Number of GLO SV.
 * refer to https://igscb.jpl.nasa.gov/pipermail/igsmail/2012/007771.html and
 * https://igscb.jpl.nasa.gov/pipermail/igsmail/2015/008391.html */
#define NUM_SATS_GLO 28
#define NUM_SATS_BDS 37
#define NUM_SATS_GAL 50
#define NUM_SATS_QZS 10

#define NUM_SATS                                                               \
  (NUM_SATS_GPS + NUM_SATS_SBAS + NUM_SATS_GLO + NUM_SATS_BDS + NUM_SATS_QZS + \
   NUM_SATS_GAL)

#define MAX_NUM_SATS         \
  (MAX(NUM_SATS_GPS,         \
       MAX(NUM_SATS_SBAS,    \
           MAX(NUM_SATS_GLO, \
               MAX(NUM_SATS_BDS, MAX(NUM_SATS_QZS, NUM_SATS_GAL))))))

/* Number of codes in each constellation. */
#define NUM_CODES_GPS 13
#define NUM_CODES_SBAS 5
#define NUM_CODES_GLO 4
#define NUM_CODES_BDS 15
#define NUM_CODES_QZS 11
#define NUM_CODES_GAL 16

#define NUM_CODES                                                   \
  (NUM_CODES_GPS + NUM_CODES_SBAS + NUM_CODES_GLO + NUM_CODES_BDS + \
   NUM_CODES_GAL + NUM_CODES_QZS)

/* Max number of GLO frequency slot, correspond to frequency slot 6 */
#define GLO_MAX_FCN 14

/* Min number of GLO frequency slot, correspond to frequency slot -7 */
#define GLO_MIN_FCN 1

/* Frequency of GLO channel is unknown */
#define GLO_FCN_UNKNOWN 0

/* Used to produce an unshifted GLO frequency slot out of GLO slots in
   GLO_MIN_FCN .. GLO_MAX_FCN range */
#define GLO_FCN_OFFSET 8

/* GLO Orbital slot is unknown */
#define GLO_ORBIT_SLOT_UNKNOWN 0

/* Number of signals in each code. */
#define NUM_SIGNALS_GPS_L1CA (NUM_SATS_GPS)
#define NUM_SIGNALS_GPS_L2C (NUM_SATS_GPS)
#define NUM_SIGNALS_GPS_L5 (NUM_SATS_GPS)
#define NUM_SIGNALS_GPS_L1P (NUM_SATS_GPS)
#define NUM_SIGNALS_GPS_L2P (NUM_SATS_GPS)
#define NUM_SIGNALS_GPS_L1C (NUM_SATS_GPS)

#define NUM_SIGNALS_SBAS_L1CA (NUM_SATS_SBAS)
#define NUM_SIGNALS_SBAS_L5 (NUM_SATS_SBAS)

#define NUM_SIGNALS_GLO_L1OF (NUM_SATS_GLO)
#define NUM_SIGNALS_GLO_L2OF (NUM_SATS_GLO)
#define NUM_SIGNALS_GLO_L1P (NUM_SATS_GLO)
#define NUM_SIGNALS_GLO_L2P (NUM_SATS_GLO)

#define NUM_SIGNALS_BDS2_B1 (NUM_SATS_BDS)
#define NUM_SIGNALS_BDS2_B2 (NUM_SATS_BDS)
#define NUM_SIGNALS_BDS3_B1C (NUM_SATS_BDS)
#define NUM_SIGNALS_BDS3_B5 (NUM_SATS_BDS)
#define NUM_SIGNALS_BDS3_B7 (NUM_SATS_BDS)
#define NUM_SIGNALS_BDS3_B3 (NUM_SATS_BDS)

#define NUM_SIGNALS_GAL_E1 (NUM_SATS_GAL)
#define NUM_SIGNALS_GAL_E6 (NUM_SATS_GAL)
#define NUM_SIGNALS_GAL_E7 (NUM_SATS_GAL)
#define NUM_SIGNALS_GAL_E8 (NUM_SATS_GAL)
#define NUM_SIGNALS_GAL_E5 (NUM_SATS_GAL)

#define NUM_SIGNALS_QZS_L1 (NUM_SATS_QZS)
#define NUM_SIGNALS_QZS_L1C (NUM_SATS_QZS)
#define NUM_SIGNALS_QZS_L2C (NUM_SATS_QZS)
#define NUM_SIGNALS_QZS_L5 (NUM_SATS_QZS)

/* Number of frequencies in GLO. */
#define NUM_FREQ_GLO_L1OF (GLO_MAX_FCN)
#define NUM_FREQ_GLO_L2OF (GLO_MAX_FCN)

/* Number of signals in each constellation. */
#define NUM_SIGNALS_GPS                                                       \
  (2 * NUM_SIGNALS_GPS_L1CA + 3 * NUM_SIGNALS_GPS_L2C + NUM_SIGNALS_GPS_L1P + \
   NUM_SIGNALS_GPS_L2P + 3 * NUM_SIGNALS_GPS_L5 + 3 * NUM_SIGNALS_GPS_L1C)

#define NUM_SIGNALS_SBAS (2 * NUM_SIGNALS_SBAS_L1CA + 3 * NUM_SIGNALS_SBAS_L5)

#define NUM_SIGNALS_GLO                                                \
  (NUM_SIGNALS_GLO_L1OF + NUM_SIGNALS_GLO_L2OF + NUM_SIGNALS_GLO_L1P + \
   NUM_SIGNALS_GLO_L2P)

#define NUM_SIGNALS_BDS                                                       \
  (2 * NUM_SIGNALS_BDS2_B1 + NUM_SIGNALS_BDS2_B2 + 3 * NUM_SIGNALS_BDS3_B1C + \
   3 * NUM_SIGNALS_BDS3_B5 + 3 * NUM_SIGNALS_BDS3_B7 +                        \
   3 * NUM_SIGNALS_BDS3_B3)
#define NUM_SIGNALS_GAL                                                       \
  (4 * NUM_SIGNALS_GAL_E1 + 3 * NUM_SIGNALS_GAL_E6 + 3 * NUM_SIGNALS_GAL_E7 + \
   3 * NUM_SIGNALS_GAL_E8 + 3 * NUM_SIGNALS_GAL_E5)

#define NUM_SIGNALS_QZS                               \
  (2 * NUM_SIGNALS_QZS_L1 + 3 * NUM_SIGNALS_QZS_L1C + \
   3 * NUM_SIGNALS_QZS_L2C + 3 * NUM_SIGNALS_QZS_L5)

#define NUM_SIGNALS                                                         \
  (NUM_SIGNALS_GPS + NUM_SIGNALS_SBAS + NUM_SIGNALS_GLO + NUM_SIGNALS_BDS + \
   NUM_SIGNALS_GAL + NUM_SIGNALS_QZS)

#define GPS_FIRST_PRN 1
#define SBAS_FIRST_PRN 120
#define GLO_FIRST_PRN 1
#define BDS_FIRST_PRN 1
#define GAL_FIRST_PRN 1
#define QZS_FIRST_PRN 193

#define SID_STR_LEN_MAX 16
#define MESID_STR_LEN_MAX 19
#define SID_SUFFIX_LENGTH 1
#define MESID_SUFFIX_LENGTH 4

#define IS_GPS(sid) is_gps((sid).code)
#define IS_GLO(sid) is_glo((sid).code)
#define IS_SBAS(sid) is_sbas((sid).code)
#define IS_BDS2(sid) is_bds2((sid).code)
#define IS_QZSS(sid) is_qzss((sid).code)
#define IS_GAL(sid) is_gal((sid).code)

#define SAT_INVALID (-1)
#define SID_UNKNOWN ((gnss_signal_t){0, CODE_INVALID})

/** Constellation identifier. */
typedef enum constellation_e {
  CONSTELLATION_INVALID = -1,
  CONSTELLATION_GPS,
  CONSTELLATION_SBAS,
  CONSTELLATION_GLO,
  CONSTELLATION_BDS,
  CONSTELLATION_QZS,
  CONSTELLATION_GAL,
  CONSTELLATION_COUNT,
} constellation_t;

static inline char constellation_to_char(constellation_t cons) {
  switch (cons) {
    case CONSTELLATION_GPS:
      return 'G';
    case CONSTELLATION_SBAS:
      return 'S';
    case CONSTELLATION_GLO:
      return 'R';
    case CONSTELLATION_BDS:
      return 'C';
    case CONSTELLATION_QZS:
      return 'J';
    case CONSTELLATION_GAL:
      return 'E';
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      return '?';
  }
}

/** Code identifier. */
typedef enum code_e {
  CODE_INVALID = -1,
  CODE_GPS_L1CA = 0,  /* GPS L1CA: BPSK(1) */
  CODE_GPS_L2CM = 1,  /* GPS L2C: 2 x BPSK(0.5) */
  CODE_SBAS_L1CA = 2, /* SBAS L1: BPSK(1) */
  CODE_GLO_L1OF = 3,  /* GLONASS L1OF: FDMA BPSK(0.5) */
  CODE_GLO_L2OF = 4,  /* GLONASS L2OF: FDMA BPSK(0.5) */
  CODE_GPS_L1P = 5,   /* GPS L1P(Y): encrypted BPSK(10) */
  CODE_GPS_L2P = 6,   /* GPS L2P(Y): encrypted BPSK(10) */
  CODE_GPS_L2CL = 7,
  CODE_GPS_L2CX = 8,
  CODE_GPS_L5I = 9, /* GPS L5: QPSK(10) at 1150*f0 */
  CODE_GPS_L5Q = 10,
  CODE_GPS_L5X = 11,
  CODE_BDS2_B1 = 12, /* BDS2 B1I: BPSK(2) at 1526*f0 */
  CODE_BDS2_B2 = 13, /* BDS2 B2I: BPSK(2) at 1180*f0 */
  CODE_GAL_E1B = 14, /* Galileo E1: CASM CBOC(1,1) at 1540*f0 */
  CODE_GAL_E1C = 15,
  CODE_GAL_E1X = 16,
  CODE_GAL_E6B = 17, /* Galileo E6: CASM BPSK(5) at 1250*f0 */
  CODE_GAL_E6C = 18,
  CODE_GAL_E6X = 19,
  CODE_GAL_E7I = 20, /* Galileo E5b: QPSK(10) at 1180*f0 */
  CODE_GAL_E7Q = 21,
  CODE_GAL_E7X = 22,
  CODE_GAL_E8I = 23, /* Galileo E5AltBOC(15,10) at 1165*f0 */
  CODE_GAL_E8Q = 24,
  CODE_GAL_E8X = 25,
  CODE_GAL_E5I = 26, /* Galileo E5a: QPSK(10) at 1150*f0 */
  CODE_GAL_E5Q = 27,
  CODE_GAL_E5X = 28,
  CODE_GLO_L1P = 29,  /* GLONASS L1P: encrypted */
  CODE_GLO_L2P = 30,  /* GLONASS L2P: encrypted */
  CODE_QZS_L1CA = 31, /* QZSS L1CA: BPSK(1) at 1540*f0 */
  CODE_QZS_L1CI = 32, /* QZSS L1C: TM-BOC at 1540*f0 */
  CODE_QZS_L1CQ = 33,
  CODE_QZS_L1CX = 34,
  CODE_QZS_L2CM = 35, /* QZSS L2C: 2 x BPSK(0.5) at 1200*f0 */
  CODE_QZS_L2CL = 36,
  CODE_QZS_L2CX = 37,
  CODE_QZS_L5I = 38, /* QZSS L5: QPSK(10) at 1150*f0 */
  CODE_QZS_L5Q = 39,
  CODE_QZS_L5X = 40,
  CODE_SBAS_L5I = 41, /* SBAS L5: ? at 1150*f0 */
  CODE_SBAS_L5Q = 42,
  CODE_SBAS_L5X = 43,
  CODE_BDS3_B1CI = 44, /* BDS3 B1C: TM-BOC at 1540*f0 */
  CODE_BDS3_B1CQ = 45,
  CODE_BDS3_B1CX = 46,
  CODE_BDS3_B5I = 47, /* BDS3 B2a: QPSK(10) at 1150*f0 */
  CODE_BDS3_B5Q = 48,
  CODE_BDS3_B5X = 49,
  CODE_BDS3_B7I = 50, /* BDS3 B2b: QPSK(10) at 1180*f0 */
  CODE_BDS3_B7Q = 51,
  CODE_BDS3_B7X = 52,
  CODE_BDS3_B3I = 53, /* BDS3 B3I: QPSK(10) at 1240*f0 */
  CODE_BDS3_B3Q = 54,
  CODE_BDS3_B3X = 55,
  CODE_GPS_L1CI = 56, /* GPS L1C: TM-BOC at 1540*f0 */
  CODE_GPS_L1CQ = 57,
  CODE_GPS_L1CX = 58,
  CODE_AUX_GPS = 59, /* Auxiliary antenna signals */
  CODE_AUX_SBAS = 60,
  CODE_AUX_GAL = 61,
  CODE_AUX_QZS = 62,
  CODE_AUX_BDS = 63,
  CODE_COUNT
} code_t;

/** GNSS signal identifier. */
typedef struct {
  u16 sat;
  code_t code;
} gnss_signal_t;

/** GNSS signal identifier for internal ME processing.
 *
 *  GPS signals are encoded similarly with gnss_signal_t,
 *  sat range 1 - 32.
 *
 *  GLO signals have their sat field represent the frequency slot FCN,
 *  sat range 1 - 14 (which is FCN -7..6 shifted by 8).
 */
typedef struct {
  u16 sat;
  code_t code;
} me_gnss_signal_t;

#define MAX_SBAS_SATS_PER_SYSTEM 3

typedef enum sbas_system_e {
  SBAS_NONE = -1,
  SBAS_WAAS = 0,
  SBAS_EGNOS,
  SBAS_GAGAN,
  SBAS_MSAS,
  SBAS_COUNT
} sbas_system_t;

typedef bool (*sid_eq_fn)(const gnss_signal_t a);

/** Are tracking codes equal or being treated as equivalent? L2CM/L1CA
 * observations on the rover, and L2P/L1P observations on the base are treated
 * as equivalent signals.
 *
 * \remark pcode_equiv and is_l2_sid are related to handling of L2P and L2CM
 *         observations here:
 *         https://github.com/swift-nav/estimation_team_planning/issues/215. In
 *         the future, we'll want to correct/canonicalize L2P signals using
 *         has_mixed_l2_obs in signal.h.
 *
 * \param a     code a
 * \param b     code b
 * \return  True if a and b are both in the same frequency band, but have
 *          different tracking modes.
 */
static inline bool code_equiv(const code_t a, const code_t b) {
  /* TODO GLO: Check GLO status. */
  if (a == b) {
    return true;
  } else if (a == CODE_GPS_L2CM && b == CODE_GPS_L2P) {
    return true;
  } else if (a == CODE_GPS_L2P && b == CODE_GPS_L2CM) {
    return true;
  } else if (a == CODE_GPS_L1P && b == CODE_GPS_L1CA) {
    return true;
  } else if (a == CODE_GPS_L1CA && b == CODE_GPS_L1P) {
    return true;
  } else {
    return false;
  }
}

/** Is this a GPS signal?
 *
 * \param   code  Code to check.
 * \return  True if this is a GPS signal.
 */
static inline bool is_gps(const code_t code) {
  return (CODE_GPS_L1CA == code) || (CODE_GPS_L2CM == code) ||
         (CODE_GPS_L2CL == code) || (CODE_GPS_L2CX == code) ||
         (CODE_GPS_L1P == code) || (CODE_GPS_L2P == code) ||
         (CODE_GPS_L5I == code) || (CODE_GPS_L5Q == code) ||
         (CODE_GPS_L5X == code) || (CODE_AUX_GPS == code);
}

/** Is this an SBAS signal?
 *
 * \param   code  Code to check.
 * \return  True if this is an SBAS signal.
 */
static inline bool is_sbas(const code_t code) {
  return (CODE_SBAS_L1CA == code) || (CODE_SBAS_L5I == code) ||
         (CODE_SBAS_L5Q == code) || (CODE_SBAS_L5X == code) ||
         (CODE_AUX_SBAS == code);
}

/** Is this a GLO signal?
 *
 * \param   code  Code to check.
 * \return  True if this is a GLO signal.
 */
static inline bool is_glo(const code_t code) {
  return (CODE_GLO_L1OF == code) || (CODE_GLO_L2OF == code) ||
         (CODE_GLO_L1P == code) || (CODE_GLO_L2P == code);
}

/** Is this a BDS2 signal?
 *
 * \param a ME GNSS signal identifier
 * \return  True if this is a Beidou generation 2 signal.
 */
static inline bool is_bds2(const code_t code) {
  return (CODE_BDS2_B1 == code) || (CODE_BDS2_B2 == code) ||
         (CODE_BDS3_B5I == code) || (CODE_BDS3_B5Q == code) ||
         (CODE_BDS3_B5X == code) || (CODE_AUX_BDS == code);
}

/** Is this a Galileo signal?
 *
 * \param   code  Code to check.
 * \return  True if this is a Galileo signal.
 */
static inline bool is_gal(const code_t code) {
  return (CODE_GAL_E1B == code) || (CODE_GAL_E1C == code) ||
         (CODE_GAL_E1X == code) || (CODE_GAL_E6B == code) ||
         (CODE_GAL_E6C == code) || (CODE_GAL_E6X == code) ||
         (CODE_GAL_E7I == code) || (CODE_GAL_E7Q == code) ||
         (CODE_GAL_E7X == code) || (CODE_GAL_E8I == code) ||
         (CODE_GAL_E8Q == code) || (CODE_GAL_E8X == code) ||
         (CODE_GAL_E5I == code) || (CODE_GAL_E5Q == code) ||
         (CODE_GAL_E5X == code) || (CODE_GAL_E5Q == code) ||
         (CODE_AUX_GAL == code);
}

/** Is this a QZSS signal?
 *
 * \param   code  Code to check.
 * \return  True if this is a QZSS signal.
 */
static inline bool is_qzss(const code_t code) {
  return (CODE_QZS_L1CA == code) || (CODE_QZS_L2CM == code) ||
         (CODE_QZS_L2CL == code) || (CODE_QZS_L2CX == code) ||
         (CODE_QZS_L5I == code) || (CODE_QZS_L5Q == code) ||
         (CODE_QZS_L5X == code) || (CODE_AUX_QZS == code);
}

constellation_t sid_to_constellation(gnss_signal_t sid);

static inline code_t constellation_to_l1_code(constellation_t constellation) {
  switch (constellation) {
    case CONSTELLATION_GPS:
      return CODE_GPS_L1CA;
    case CONSTELLATION_SBAS:
      return CODE_SBAS_L1CA;
    case CONSTELLATION_GLO:
      return CODE_GLO_L1OF;
    case CONSTELLATION_BDS:
      return CODE_BDS2_B1;
    case CONSTELLATION_QZS:
      return CODE_QZS_L1CA;
    case CONSTELLATION_GAL:
      return CODE_GAL_E1B;
    case CONSTELLATION_INVALID:
    case CONSTELLATION_COUNT:
    default:
      return CODE_INVALID;
  }
}

static inline code_t sid_to_l1_code(gnss_signal_t sid) {
  return constellation_to_l1_code(sid_to_constellation(sid));
}

/** Determine if a code is valid.
 *
 * \param code    Code to use.
 * \return true if code is valid, false otherwise
 */
static inline bool code_valid(code_t code) {
  return ((code >= 0) && (code < CODE_COUNT));
}

/** Signal comparison function. */
static inline int sid_compare(const gnss_signal_t a, const gnss_signal_t b) {
  /* Signal code are not sorted in order per constellation
   * (e.g. GLO L1C ~ 3 and GPS L2P ~ 6).
   * As some of our functions relies on comparing ordered sets of signals,
   * this can cause issues.
   * Therefore, in this function, we enforce the ordering per
   * constellation/frequency/satellite */
  if ((code_valid(a.code)) && code_valid(b.code)) {
    if (sid_to_constellation(a) == sid_to_constellation(b)) {
      if (code_equiv(a.code, b.code)) {
        return a.sat - b.sat;
      } else {
        return a.code - b.code;
      }
    } else {
      return sid_to_constellation(a) - sid_to_constellation(b);
    }
  } else {
    if (code_equiv(a.code, b.code)) {
      return a.sat - b.sat;
    }
    return a.code - b.code;
  }
}

/** Is GLO frequency valid?
 *
 * \param a FLO frequency slot
 * \return  True if GLO frequency slot is in appropriate range.
 */
static inline bool glo_fcn_is_valid(u16 fcn) {
  return (fcn >= GLO_MIN_FCN) && (fcn <= GLO_MAX_FCN);
}

/** Is GLO orbital slot valid?
 *
 * \param a GLO orbital slot
 * \return  True if GLO orbital slot is in appropriate range.
 */
static inline bool glo_slot_id_is_valid(u16 slot) {
  return (slot <= NUM_SATS_GLO) && (slot > 0);
}

constellation_t mesid_to_constellation(const me_gnss_signal_t mesid);
/** ME signal comparison function. */
static inline int mesid_compare(const me_gnss_signal_t a,
                                const me_gnss_signal_t b) {
  /* Signal code are not sorted in order per constellation
   * (e.g. GLO L1C ~ 3 and GPS L2P ~ 6).
   * As some of our functions relies on comparing ordered sets of signals,
   * this can cause issues.
   * Therefore, in this function, we enforce the ordering per
   * constellation/frequency/satellite */
  if ((code_valid(a.code)) && code_valid(b.code)) {
    if (mesid_to_constellation(a) == mesid_to_constellation(b)) {
      if (code_equiv(a.code, b.code)) {
        return a.sat - b.sat;
      } else {
        return a.code - b.code;
      }
    } else {
      return mesid_to_constellation(a) - mesid_to_constellation(b);
    }
  } else {
    return a.code - b.code;
  }
}

#ifdef __cplusplus
static inline bool operator==(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) == 0;
}

static inline bool operator!=(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) != 0;
}

static inline bool operator<(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) < 0;
}

static inline bool operator>(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) > 0;
}

static inline bool operator<=(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) <= 0;
}

static inline bool operator>=(const gnss_signal_t &a, const gnss_signal_t &b) {
  return sid_compare(a, b) >= 0;
}
#endif /* __cplusplus */

/** Untyped signal comparison function. */
static inline int cmp_sid_sid(const void *a, const void *b) {
  return sid_compare(*(const gnss_signal_t *)a, *(const gnss_signal_t *)b);
}

/** Signal equality function. */
static inline bool sid_is_equal(const gnss_signal_t a, const gnss_signal_t b) {
  return sid_compare(a, b) == 0;
}

/** ME signal equality function. */
static inline bool mesid_is_equal(const me_gnss_signal_t a,
                                  const me_gnss_signal_t b) {
  return mesid_compare(a, b) == 0;
}

/* Logging macros */
#define _LOG_SID(func, sid, format, ...)          \
  do {                                            \
    char sid_str[SID_STR_LEN_MAX];                \
    sid_to_string(sid_str, sizeof(sid_str), sid); \
    func("%s " format, sid_str, ##__VA_ARGS__);   \
  } while (false)

#define log_error_sid(sid, format, ...) \
  _LOG_SID(log_error, sid, format, ##__VA_ARGS__)

#define log_warn_sid(sid, format, ...) \
  _LOG_SID(log_warn, sid, format, ##__VA_ARGS__)

#define log_info_sid(sid, format, ...) \
  _LOG_SID(log_info, sid, format, ##__VA_ARGS__)

#define log_debug_sid(sid, format, ...) \
  _LOG_SID(log_debug, sid, format, ##__VA_ARGS__)

#define detailed_log_error_sid(sid, format, ...) \
  _LOG_SID(detailed_log_error, sid, format, ##__VA_ARGS__)

#define detailed_log_warn_sid(sid, format, ...) \
  _LOG_SID(detailed_log_warn, sid, format, ##__VA_ARGS__)

#define detailed_log_info_sid(sid, format, ...) \
  _LOG_SID(detailed_log_info, sid, format, ##__VA_ARGS__)

#define detailed_log_debug_sid(sid, format, ...) \
  _LOG_SID(detailed_log_debug, sid, format, ##__VA_ARGS__)

#define _LOG_MESID(func, mesid, format, ...)              \
  do {                                                    \
    char mesid_str[MESID_STR_LEN_MAX];                    \
    mesid_to_string(mesid_str, sizeof(mesid_str), mesid); \
    func("%s " format, mesid_str, ##__VA_ARGS__);         \
  } while (false)

#define log_error_mesid(mesid, format, ...) \
  _LOG_MESID(log_error, mesid, format, ##__VA_ARGS__)

#define log_warn_mesid(mesid, format, ...) \
  _LOG_MESID(log_warn, mesid, format, ##__VA_ARGS__)

#define log_info_mesid(mesid, format, ...) \
  _LOG_MESID(log_info, mesid, format, ##__VA_ARGS__)

#define log_debug_mesid(mesid, format, ...) \
  _LOG_MESID(log_debug, mesid, format, ##__VA_ARGS__)

#define detailed_log_error_mesid(mesid, format, ...) \
  _LOG_MESID(detailed_log_error, mesid, format, ##__VA_ARGS__)

#define detailed_log_warn_mesid(mesid, format, ...) \
  _LOG_MESID(detailed_log_warn, mesid, format, ##__VA_ARGS__)

#define detailed_log_info_mesid(mesid, format, ...) \
  _LOG_MESID(detailed_log_info, mesid, format, ##__VA_ARGS__)

#define detailed_log_debug_mesid(mesid, format, ...) \
  _LOG_MESID(detailed_log_debug, mesid, format, ##__VA_ARGS__)

/* \} */

gnss_signal_t construct_sid(code_t code, u16 sat);
me_gnss_signal_t construct_mesid(code_t code, u16 sat);
gnss_signal_t mesid2sid(const me_gnss_signal_t mesid, u16 glo_slot_id);
int sat_code_to_string(
    char *str_buf, size_t suffix_len, const char *suffix, u16 sat, code_t code);
int sid_to_string(char *s, int n, const gnss_signal_t sid);
int mesid_to_string(char *s, int n, const me_gnss_signal_t mesid);
bool sid_valid(gnss_signal_t sid);
bool mesid_valid(const me_gnss_signal_t mesid);
bool constellation_valid(constellation_t constellation);
gnss_signal_t sid_from_code_index(code_t code, u16 code_index);
me_gnss_signal_t mesid_from_code_index(code_t code, u16 code_index);
u16 sid_to_code_index(gnss_signal_t sid);
u16 mesid_to_code_index(const me_gnss_signal_t mesid);
double mesid_to_carr_freq(const me_gnss_signal_t mesid);
constellation_t code_to_constellation(code_t code);
double sid_to_carr_freq(gnss_signal_t sid);
double sid_to_lambda(gnss_signal_t sid);
const char *code_to_string(const code_t code);
u32 code_to_chip_count(code_t code);
double code_to_chip_rate(code_t code);
double mesid_to_carr_to_code(const me_gnss_signal_t mesid);
u16 code_to_prn_period_ms(code_t code);
bool code_requires_direct_acq(code_t code);
float code_to_sv_doppler_min(code_t code);
float code_to_sv_doppler_max(code_t code);
bool code_requires_decoder(code_t code);
u16 constellation_to_sat_count(constellation_t gnss);
const u8 *get_sbas_prn_list(sbas_system_t sbas_system);
sbas_system_t get_sbas_system(const gnss_signal_t sid);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_SIGNAL_H */
