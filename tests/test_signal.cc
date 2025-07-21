#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swiftnav/constants.h>
#include <swiftnav/glo_map.h>
#include <swiftnav/signal.h>

#include "check_utils.h"

/* Include singal.c here to have a chance to turn off asserts.
   Otherwise the code lines with asserts cannot be covered by
   tests and it will lower test coverage statistics. */
#ifndef NDEBUG
#define NDEBUG
#endif
#include "../src/signal.c"

namespace {

#define ARRAY_COUNT(arr) ((sizeof(arr) / sizeof(arr[0])))

const struct code_data_element {
  code_t code;
  u16 sat_count;
} code_data[] = {
    {CODE_GPS_L1CA, NUM_SIGNALS_GPS_L1CA},
    {CODE_AUX_GPS, NUM_SIGNALS_GPS_L1CA},
    {CODE_GPS_L2CM, NUM_SIGNALS_GPS_L2C},
    {CODE_GPS_L2CL, NUM_SIGNALS_GPS_L2C},
    {CODE_GPS_L2CX, NUM_SIGNALS_GPS_L2C},
    {CODE_GPS_L5I, NUM_SIGNALS_GPS_L5},
    {CODE_GPS_L5Q, NUM_SIGNALS_GPS_L5},
    {CODE_GPS_L5X, NUM_SIGNALS_GPS_L5},
    {CODE_GPS_L1CI, NUM_SIGNALS_GPS_L1C},
    {CODE_GPS_L1CQ, NUM_SIGNALS_GPS_L1C},
    {CODE_GPS_L1CX, NUM_SIGNALS_GPS_L1C},
    {CODE_GPS_L1P, NUM_SIGNALS_GPS_L1P},
    {CODE_GPS_L2P, NUM_SIGNALS_GPS_L2P},

    {CODE_SBAS_L1CA, NUM_SIGNALS_SBAS_L1CA},
    {CODE_AUX_SBAS, NUM_SIGNALS_SBAS_L1CA},
    {CODE_SBAS_L5I, NUM_SIGNALS_SBAS_L5},
    {CODE_SBAS_L5Q, NUM_SIGNALS_SBAS_L5},
    {CODE_SBAS_L5X, NUM_SIGNALS_SBAS_L5},

    {CODE_GLO_L1OF, NUM_SIGNALS_GLO_L1OF},
    {CODE_GLO_L2OF, NUM_SIGNALS_GLO_L2OF},
    {CODE_GLO_L1P, NUM_SIGNALS_GLO_L1P},
    {CODE_GLO_L2P, NUM_SIGNALS_GLO_L2P},

    {CODE_BDS2_B1, NUM_SIGNALS_BDS2_B1},
    {CODE_AUX_BDS, NUM_SIGNALS_BDS2_B1},
    {CODE_BDS2_B2, NUM_SIGNALS_BDS2_B2},
    {CODE_BDS3_B1CI, NUM_SIGNALS_BDS3_B1C},
    {CODE_BDS3_B1CQ, NUM_SIGNALS_BDS3_B1C},
    {CODE_BDS3_B1CX, NUM_SIGNALS_BDS3_B1C},
    {CODE_BDS3_B5I, NUM_SIGNALS_BDS3_B5},
    {CODE_BDS3_B5Q, NUM_SIGNALS_BDS3_B5},
    {CODE_BDS3_B5X, NUM_SIGNALS_BDS3_B5},
    {CODE_BDS3_B7I, NUM_SIGNALS_BDS3_B7},
    {CODE_BDS3_B7Q, NUM_SIGNALS_BDS3_B7},
    {CODE_BDS3_B7X, NUM_SIGNALS_BDS3_B7},
    {CODE_BDS3_B3I, NUM_SIGNALS_BDS3_B3},
    {CODE_BDS3_B3Q, NUM_SIGNALS_BDS3_B3},
    {CODE_BDS3_B3X, NUM_SIGNALS_BDS3_B3},

    {CODE_GAL_E1B, NUM_SIGNALS_GAL_E1},
    {CODE_GAL_E1C, NUM_SIGNALS_GAL_E1},
    {CODE_GAL_E1X, NUM_SIGNALS_GAL_E1},
    {CODE_AUX_GAL, NUM_SIGNALS_GAL_E1},
    {CODE_GAL_E6B, NUM_SIGNALS_GAL_E6},
    {CODE_GAL_E6C, NUM_SIGNALS_GAL_E6},
    {CODE_GAL_E6X, NUM_SIGNALS_GAL_E6},
    {CODE_GAL_E7I, NUM_SIGNALS_GAL_E7},
    {CODE_GAL_E7Q, NUM_SIGNALS_GAL_E7},
    {CODE_GAL_E7X, NUM_SIGNALS_GAL_E7},
    {CODE_GAL_E8I, NUM_SIGNALS_GAL_E8},
    {CODE_GAL_E8Q, NUM_SIGNALS_GAL_E8},
    {CODE_GAL_E8X, NUM_SIGNALS_GAL_E8},
    {CODE_GAL_E5I, NUM_SIGNALS_GAL_E5},
    {CODE_GAL_E5Q, NUM_SIGNALS_GAL_E5},
    {CODE_GAL_E5X, NUM_SIGNALS_GAL_E5},

    {CODE_QZS_L1CA, NUM_SIGNALS_QZS_L1},
    {CODE_AUX_QZS, NUM_SIGNALS_QZS_L1},
    {CODE_QZS_L1CI, NUM_SIGNALS_QZS_L1C},
    {CODE_QZS_L1CQ, NUM_SIGNALS_QZS_L1C},
    {CODE_QZS_L1CX, NUM_SIGNALS_QZS_L1C},
    {CODE_QZS_L2CM, NUM_SIGNALS_QZS_L2C},
    {CODE_QZS_L2CL, NUM_SIGNALS_QZS_L2C},
    {CODE_QZS_L2CX, NUM_SIGNALS_QZS_L2C},
    {CODE_QZS_L5I, NUM_SIGNALS_QZS_L5},
    {CODE_QZS_L5Q, NUM_SIGNALS_QZS_L5},
    {CODE_QZS_L5X, NUM_SIGNALS_QZS_L5},
};

const struct constellation_data_element {
  constellation_t constellation;
  u16 sat_count;
  u16 code_count;
} constellation_data[] = {
    {CONSTELLATION_GPS, NUM_SIGNALS_GPS, NUM_CODES_GPS},
    {CONSTELLATION_SBAS, NUM_SIGNALS_SBAS, NUM_CODES_SBAS},
    {CONSTELLATION_GLO, NUM_SIGNALS_GLO, NUM_CODES_GLO},
    {CONSTELLATION_BDS, NUM_SIGNALS_BDS, NUM_CODES_BDS},
    {CONSTELLATION_QZS, NUM_SIGNALS_QZS, NUM_CODES_QZS},
    {CONSTELLATION_GAL, NUM_SIGNALS_GAL, NUM_CODES_GAL},
};

TEST(TestSignal, CodeTableConsistency) {
  for (size_t i = 0; i < CODE_COUNT; i++) {
    EXPECT_EQ((code_e)i, code_table[i].code);
  }
}

TEST(TestSignal, SignalAggregates) {
  EXPECT_EQ(ARRAY_COUNT(code_data), CODE_COUNT);

  EXPECT_EQ(ARRAY_COUNT(constellation_data), CONSTELLATION_COUNT);

  u16 constellation_code_counts[CONSTELLATION_COUNT];
  memset(constellation_code_counts, 0, sizeof(constellation_code_counts));
  for (const auto &i : code_data) {
    const struct code_data_element *e = &i;
    constellation_t constellation = code_to_constellation(e->code);
    constellation_code_counts[constellation]++;
  }
  for (const auto &i : constellation_data) {
    const struct constellation_data_element *e = &i;
    EXPECT_EQ(e->code_count, constellation_code_counts[e->constellation]);
  }
}

TEST(TestSignal, SignalFromIndex) {
  for (const auto &i : code_data) {
    const struct code_data_element *e = &i;
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 code_index = 0; code_index < sat_count; code_index++) {
      gnss_signal_t sid = sid_from_code_index(code, code_index);

      EXPECT_TRUE(sid_valid(sid));
      EXPECT_EQ(sid_to_code_index(sid), code_index);
    }
  }
}

TEST(TestSignal, SignalProperties) {
  const struct test_case {
    gnss_signal_t sid;
    bool valid;
    const char *str;
  } test_cases[] = {
      {.sid = {.sat = 0, .code = CODE_INVALID}, .valid = false},
      {.sid = {.sat = 0, .code = CODE_COUNT}, .valid = false},
      {.sid = {.sat = 1, .code = CODE_INVALID}, .valid = false},
      {
          .sid = {.sat = 0, .code = CODE_GPS_L1CA},
          .valid = false,
      },
      {.sid = {.sat = 1, .code = CODE_GPS_L1CA},
       .valid = true,
       .str = "GPS L1CA 1"},
      {.sid = {.sat = 1, .code = CODE_GPS_L2CM},
       .valid = true,
       .str = "GPS L2CM 1"},
      {.sid = {.sat = 1, .code = CODE_SBAS_L1CA}, .valid = false},
      {.sid = {.sat = 32, .code = CODE_GPS_L1CA},
       .valid = true,
       .str = "GPS L1CA 32"},
      {.sid = {.sat = 33, .code = CODE_GPS_L1CA}, .valid = false},
      {.sid = {.sat = 0, .code = CODE_SBAS_L1CA}, .valid = false},
      {.sid = {.sat = 119, .code = CODE_SBAS_L1CA}, .valid = false},
      {.sid = {.sat = 120, .code = CODE_SBAS_L1CA},
       .valid = true,
       .str = "SBAS L1 120"},
      {.sid = {.sat = 120, .code = CODE_GPS_L1CA}, .valid = false},
      {.sid = {.sat = 138, .code = CODE_SBAS_L1CA},
       .valid = true,
       .str = "SBAS L1 138"},
      {.sid = {.sat = 139, .code = CODE_SBAS_L1CA}, .valid = false},
      {
          .sid = {.sat = 0, .code = CODE_GLO_L1OF},
          .valid = false,
      },
      {.sid = {.sat = 1, .code = CODE_GLO_L1OF},
       .valid = true,
       .str = "GLO L1OF 1"},
      {.sid = {.sat = 28, .code = CODE_GLO_L1OF},
       .valid = true,
       .str = "GLO L1OF 28"},
      {.sid = {.sat = 29, .code = CODE_GLO_L1OF}, .valid = false},
      {
          .sid = {.sat = 0, .code = CODE_GLO_L2OF},
          .valid = false,
      },
      {.sid = {.sat = 1, .code = CODE_GLO_L2OF},
       .valid = true,
       .str = "GLO L2OF 1"},
      {.sid = {.sat = 28, .code = CODE_GLO_L2OF},
       .valid = true,
       .str = "GLO L2OF 28"},
      {.sid = {.sat = 29, .code = CODE_GLO_L2OF}, .valid = false},
      {.sid = {.sat = 0, .code = CODE_GPS_L1P},
       .valid = false,
       .str = "GPS L1P 0"},
      {.sid = {.sat = 1, .code = CODE_GPS_L1P},
       .valid = true,
       .str = "GPS L1P 1"},
      {.sid = {.sat = 24, .code = CODE_GPS_L1P},
       .valid = true,
       .str = "GPS L1P 24"},
      {.sid = {.sat = 0, .code = CODE_GPS_L2P},
       .valid = false,
       .str = "GPS L2P 0"},
      {.sid = {.sat = 1, .code = CODE_GPS_L2P},
       .valid = true,
       .str = "GPS L2P 1"},
      {.sid = {.sat = 24, .code = CODE_GPS_L2P},
       .valid = true,
       .str = "GPS L2P 24"},
      {.sid = {.sat = 0, .code = CODE_BDS2_B1}, .valid = false},
      {.sid = {.sat = 1, .code = CODE_BDS2_B1},
       .valid = true,
       .str = "BDS B1 1"},
      {.sid = {.sat = 41, .code = CODE_BDS2_B1},
       .valid = true,
       .str = "BDS B1 41"},
      {.sid = {.sat = 65, .code = CODE_BDS2_B1}, .valid = false},
  };

  for (const auto &i : test_cases) {
    const struct test_case *t = &i;
    bool valid = sid_valid(t->sid);
    EXPECT_EQ(t->valid, valid);
    if (valid) {
      gnss_signal_t sid =
          sid_from_code_index(t->sid.code, sid_to_code_index(t->sid));
      EXPECT_TRUE(sid_is_equal(t->sid, sid));
      char str[SID_STR_LEN_MAX] = {0};
      u32 ret = sid_to_string(str, sizeof(str), sid);
      EXPECT_EQ(ret, strlen(t->str));
      EXPECT_STREQ(str, t->str);
      EXPECT_TRUE(constellation_valid(code_to_constellation(sid.code)));
    }
    EXPECT_FALSE(constellation_valid(CONSTELLATION_COUNT))
        << "constellation_valid failed for constellation "
        << CONSTELLATION_COUNT;
  }
}

TEST(TestSignal, SignalCompare) {
  gnss_signal_t sids[NUM_SIGNALS];
  u32 signal_index = 0;

  for (const auto &i : code_data) {
    const struct code_data_element *e = &i;
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 sat_index = 0; sat_index < sat_count; sat_index++) {
      gnss_signal_t sid = sid_from_code_index(code, sat_index);
      sids[signal_index++] = sid;
    }
  }

  qsort(sids, NUM_SIGNALS, sizeof(gnss_signal_t), cmp_sid_sid);

  for (u32 i = 1; i < NUM_SIGNALS; i++) {
    EXPECT_FALSE((sid_is_equal(sids[i], sids[i - 1])))
        << "signal index " << i << " not unique";
    EXPECT_TRUE(sid_compare(sids[i], sids[i - 1]) >= 0)
        << "signal index " << i << " not in order";
  }
}

TEST(TestSignal, SignalConstruction) {
  for (const auto &i : code_data) {
    const struct code_data_element *e = &i;
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 code_index = 0; code_index < sat_count; code_index++) {
      gnss_signal_t sid = sid_from_code_index(code, code_index);
      gnss_signal_t csid = construct_sid(sid.code, sid.sat);
      EXPECT_TRUE(sid_valid(csid));
      EXPECT_TRUE(sid_is_equal(sid, csid));
    }
  }
}

void glo_map_lock(void) {}
void glo_map_unlock(void) {}

TEST(TestSignal, SignalSidToCarrFreq) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  double carr_freq;
  gnss_signal_t sid = construct_sid(CODE_GPS_L2CM, GPS_FIRST_PRN);
  carr_freq = sid_to_carr_freq(sid);
  EXPECT_EQ(GPS_L2_HZ, carr_freq);

  sid = construct_sid(CODE_GPS_L1CA, GPS_FIRST_PRN);
  carr_freq = sid_to_carr_freq(sid);
  EXPECT_EQ(GPS_L1_HZ, carr_freq);

  /* check all GLO frequency and orbital slots */
  for (u16 sat = GLO_FIRST_PRN; sat <= NUM_SATS_GLO; sat++) {
    for (u16 fcn = GLO_MIN_FCN; fcn <= GLO_MAX_FCN; fcn++) {
      /* L2 first */
      /* map orb and fcn slots */
      glo_map_set_slot_id(fcn, sat);
      sid = construct_sid(CODE_GLO_L2OF, sat);
      carr_freq = sid_to_carr_freq(sid);
      EXPECT_EQ((GLO_L2_HZ +
                 (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L2_DELTA_HZ),
                carr_freq);
      /* now L1 */
      sid = construct_sid(CODE_GLO_L1OF, sat);
      carr_freq = sid_to_carr_freq(sid);
      EXPECT_EQ((GLO_L1_HZ +
                 (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) * GLO_L1_DELTA_HZ),
                carr_freq);
    }
  }
}

TEST(TestSignal, SignalSidToLambda) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  double lambda;

  gnss_signal_t sid = construct_sid(CODE_GPS_L2CM, GPS_FIRST_PRN);
  lambda = sid_to_lambda(sid);
  EXPECT_EQ((GPS_C / GPS_L2_HZ), lambda);

  sid = construct_sid(CODE_GPS_L1CA, GPS_FIRST_PRN);
  lambda = sid_to_lambda(sid);
  EXPECT_EQ((GPS_C / GPS_L1_HZ), lambda);

  /* check all GLO frequency and orbital slots */
  for (u16 orb_slot = GLO_FIRST_PRN; orb_slot <= NUM_SATS_GLO; orb_slot++) {
    for (u16 fcn = GLO_MIN_FCN; fcn <= GLO_MAX_FCN; fcn++) {
      /* L2 first */
      /* map orb and fcn slots */
      glo_map_set_slot_id(fcn, orb_slot);
      sid = construct_sid(CODE_GLO_L2OF, orb_slot);
      lambda = sid_to_lambda(sid);
      EXPECT_EQ((GPS_C / (GLO_L2_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                          GLO_L2_DELTA_HZ)),
                lambda);
      /* now L1 */
      sid = construct_sid(CODE_GLO_L1OF, orb_slot);
      lambda = sid_to_lambda(sid);
      EXPECT_EQ((GPS_C / (GLO_L1_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                          GLO_L1_DELTA_HZ)),
                lambda);
    }
  }
}

TEST(TestSignal, SignalCodeToChipCount) {
  u32 chip_count;

  chip_count = code_to_chip_count(CODE_GPS_L1CA);
  EXPECT_EQ(GPS_L1CA_CHIPS_NUM, chip_count);

  chip_count = code_to_chip_count(CODE_SBAS_L1CA);
  EXPECT_EQ(GPS_L1CA_CHIPS_NUM, chip_count);

  chip_count = code_to_chip_count(CODE_GPS_L2CM);
  EXPECT_EQ(GPS_L2CM_CHIPS_NUM, chip_count);

  chip_count = code_to_chip_count(CODE_GPS_L2CL);
  EXPECT_EQ(GPS_L2CL_CHIPS_NUM, chip_count);

  /* check unsupported branch for code coverage stats */
  chip_count = code_to_chip_count(CODE_GLO_L2OF);
}

TEST(TestSignal, SignalCodeToChipRate) {
  double chip_rate;

  chip_rate = code_to_chip_rate(CODE_GPS_L1CA);
  EXPECT_EQ(GPS_CA_CHIPPING_RATE, chip_rate);

  chip_rate = code_to_chip_rate(CODE_SBAS_L1CA);
  EXPECT_EQ(GPS_CA_CHIPPING_RATE, chip_rate);

  chip_rate = code_to_chip_rate(CODE_GPS_L2CM);
  EXPECT_EQ(GPS_CA_CHIPPING_RATE, chip_rate);

  /* check unsupported branch for code coverage stats */
  chip_rate = code_to_chip_rate(CODE_GLO_L2OF);
}

TEST(TestSignal, SignalCodeRequiresDirectAcq) {
  bool req;

  req = code_requires_direct_acq(CODE_GPS_L1CA);
  EXPECT_EQ(true, req);

  req = code_requires_direct_acq(CODE_GPS_L2CM);
  EXPECT_EQ(false, req);
}

TEST(TestSignal, SignalCodeToPrnPeriod) {
  u16 period;

  period = code_to_prn_period_ms(CODE_GPS_L1CA);
  EXPECT_EQ(1, period);

  period = code_to_prn_period_ms(CODE_GPS_L2CM);
  EXPECT_EQ(20, period);

  period = code_to_prn_period_ms(CODE_GLO_L1OF);
  EXPECT_EQ(1, period);
}

TEST(TestSignal, SidSystemCheck) {
  for (u8 i = 0; i < CODE_COUNT; i++) {
    bool gps =
        (i == CODE_GPS_L1CA) || (i == CODE_AUX_GPS) || (i == CODE_GPS_L2CM) ||
        (i == CODE_GPS_L2CL) || (i == CODE_GPS_L2CX) || (i == CODE_GPS_L1P) ||
        (i == CODE_GPS_L2P) || (i == CODE_GPS_L5I) || (i == CODE_GPS_L5Q) ||
        (i == CODE_GPS_L5X) || (i == CODE_GPS_L1CI) || (i == CODE_GPS_L1CQ) ||
        (i == CODE_GPS_L1CX);

    EXPECT_EQ(gps, IS_GPS(construct_sid((code_t)i, GPS_FIRST_PRN)));

    bool glo = (i == CODE_GLO_L1OF) || (i == CODE_GLO_L2OF) ||
               (i == CODE_GLO_L1P) || (i == CODE_GLO_L2P);

    EXPECT_EQ(glo, IS_GLO(construct_sid((code_t)i, GLO_FIRST_PRN)));

    bool sbas = (i == CODE_SBAS_L1CA) || (i == CODE_AUX_SBAS) ||
                (i == CODE_SBAS_L5I) || (i == CODE_SBAS_L5Q) ||
                (i == CODE_SBAS_L5X);

    EXPECT_EQ(sbas, IS_SBAS(construct_sid((code_t)i, SBAS_FIRST_PRN)));
  }
}

TEST(TestSignal, SbasPrnList) {
  for (u8 prn = SBAS_FIRST_PRN; prn < SBAS_FIRST_PRN + NUM_SATS_SBAS; prn++) {
    gnss_signal_t sid = {prn, CODE_SBAS_L1CA};
    sbas_system_t sbas_system = get_sbas_system(sid);
    EXPECT_LE(sbas_system, SBAS_COUNT);
  }

  for (sbas_system_t sbas_system = static_cast<sbas_system_t>(0);
       sbas_system < SBAS_COUNT;
       sbas_system = static_cast<sbas_system_t>(sbas_system + 1)) {
    for (unsigned char prn : sbas_prn_table[sbas_system].prn_list) {
      EXPECT_TRUE(prn == 0 || prn >= SBAS_FIRST_PRN);
      EXPECT_LT(prn, SBAS_FIRST_PRN + NUM_SATS_SBAS);

      if (prn >= SBAS_FIRST_PRN) {
        gnss_signal_t sid = {prn, CODE_SBAS_L1CA};
        EXPECT_EQ(get_sbas_system(sid), sbas_system);
      }
    }
  }
}

TEST(TestSignal, SignalHashes) {
  for (s32 test_round = 0; test_round < 1000000; ++test_round) {
    gnss_signal_t sid1, sid2;
    sid1.code = (code_t)(rand() % CODE_COUNT);
    sid1.sat = static_cast<u16>(rand() % MAX_NUM_SATS);
    sid2.code = static_cast<code_t>(rand() % CODE_COUNT);
    sid2.sat = (u16)(rand() % MAX_NUM_SATS);

    constellation_t sid1_constellation = sid_to_constellation(sid1);
    constellation_t sid2_constellation = sid_to_constellation(sid2);

    int comparison = sid_compare(sid1, sid2);

    if (sid1_constellation < sid2_constellation) {
      EXPECT_LT(comparison, 0);
    } else if (sid1_constellation > sid2_constellation) {
      EXPECT_GT(comparison, 0);
    } else {
      if (sid1.code < sid2.code) {
        EXPECT_LT(comparison, 0);
      } else if (sid1.code > sid2.code) {
        EXPECT_GT(comparison, 0);
      } else {
        if (sid1.sat < sid2.sat) {
          EXPECT_LT(comparison, 0);
        } else if (sid1.sat > sid2.sat) {
          EXPECT_GT(comparison, 0);
        } else {
          EXPECT_EQ(comparison, 0);
        }
      }
    }
  }
}

TEST(TestSignal, ConstellationToString) {
  EXPECT_STREQ("GPS", constellation_to_string(CONSTELLATION_GPS));
  EXPECT_STREQ("GPS", constellation_to_string(CONSTELLATION_GPS));
  EXPECT_STREQ("SBAS", constellation_to_string(CONSTELLATION_SBAS));
  EXPECT_STREQ("GLO", constellation_to_string(CONSTELLATION_GLO));
  EXPECT_STREQ("BDS", constellation_to_string(CONSTELLATION_BDS));
  EXPECT_STREQ("QZS", constellation_to_string(CONSTELLATION_QZS));
  EXPECT_STREQ("GAL", constellation_to_string(CONSTELLATION_GAL));
}

TEST(TestSignal, SubConstellationToString) {
  EXPECT_STREQ("GPS", sub_constellation_to_string(SUB_CONSTELLATION_GPS));
  EXPECT_STREQ("SBAS", sub_constellation_to_string(SUB_CONSTELLATION_SBAS));
  EXPECT_STREQ("GLO", sub_constellation_to_string(SUB_CONSTELLATION_GLO));
  EXPECT_STREQ("BDS2", sub_constellation_to_string(SUB_CONSTELLATION_BDS2));
  EXPECT_STREQ("BDS3", sub_constellation_to_string(SUB_CONSTELLATION_BDS3));
  EXPECT_STREQ("QZS", sub_constellation_to_string(SUB_CONSTELLATION_QZS));
  EXPECT_STREQ("GAL", sub_constellation_to_string(SUB_CONSTELLATION_GAL));
}

TEST(TestSignal, SubConstellationToConstellation) {
  EXPECT_EQ(CONSTELLATION_GPS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_GPS));
  EXPECT_EQ(CONSTELLATION_GPS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_GPS));
  EXPECT_EQ(CONSTELLATION_SBAS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_SBAS));
  EXPECT_EQ(CONSTELLATION_GLO,
            sub_constellation_to_constellation(SUB_CONSTELLATION_GLO));
  EXPECT_EQ(CONSTELLATION_BDS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_BDS2));
  EXPECT_EQ(CONSTELLATION_BDS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_BDS3));
  EXPECT_EQ(CONSTELLATION_QZS,
            sub_constellation_to_constellation(SUB_CONSTELLATION_QZS));
  EXPECT_EQ(CONSTELLATION_GAL,
            sub_constellation_to_constellation(SUB_CONSTELLATION_GAL));
}

}  // namespace
