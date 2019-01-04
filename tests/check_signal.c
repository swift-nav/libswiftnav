#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "check_suites.h"
#include "common/check_utils.h"

#include <swiftnav/constants.h>
#include <swiftnav/signal.h>

/* Include singal.c here to have a chance to turn off asserts.
   Otherwise the code lines with asserts cannot be covered by
   tests and it will lower test coverage statistics. */
#define NDEBUG
#include <../src/signal.c>

#define ARRAY_COUNT(arr) ((sizeof(arr) / sizeof(arr[0])))

static const struct code_data_element {
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

static const struct constellation_data_element {
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

START_TEST(test_signal_aggregates) {
  fail_unless(ARRAY_COUNT(code_data) == CODE_COUNT,
              "missing code entry in code_data[]");

  fail_unless(ARRAY_COUNT(constellation_data) == CONSTELLATION_COUNT,
              "missing constellation entry in constellation_data[]");

  u16 constellation_code_counts[CONSTELLATION_COUNT];
  memset(constellation_code_counts, 0, sizeof(constellation_code_counts));
  for (u32 i = 0; i < ARRAY_COUNT(code_data); i++) {
    const struct code_data_element *e = &code_data[i];
    constellation_t constellation = code_to_constellation(e->code);
    constellation_code_counts[constellation]++;
  }
  for (u32 i = 0; i < ARRAY_COUNT(constellation_data); i++) {
    const struct constellation_data_element *e = &constellation_data[i];
    fail_unless(e->code_count == constellation_code_counts[e->constellation],
                "invalid code count definition for code %d",
                e->constellation);
  }
}
END_TEST

START_TEST(test_signal_from_index) {
  for (u32 i = 0; i < ARRAY_COUNT(code_data); i++) {
    const struct code_data_element *e = &code_data[i];
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 code_index = 0; code_index < sat_count; code_index++) {
      gnss_signal_t sid = sid_from_code_index(code, code_index);

      fail_unless(sid_valid(sid),
                  "signal from code index not valid: "
                  "code %d code index %d",
                  code,
                  code_index);
      fail_unless(sid_to_code_index(sid) == code_index,
                  "signal from code index to code index failed: "
                  "code %d code index %d",
                  code,
                  code_index);
    }
  }
}
END_TEST

START_TEST(test_signal_properties) {
  const struct test_case {
    gnss_signal_t sid;
    bool valid;
    char str[SID_STR_LEN_MAX];
  } test_cases[] = {
      {.sid = {.code = CODE_INVALID, .sat = 0}, .valid = false},
      {.sid = {.code = CODE_COUNT, .sat = 0}, .valid = false},
      {.sid = {.code = CODE_INVALID, .sat = 1}, .valid = false},
      {
          .sid = {.code = CODE_GPS_L1CA, .sat = 0},
          .valid = false,
      },
      {.sid = {.code = CODE_GPS_L1CA, .sat = 1},
       .valid = true,
       .str = "GPS L1CA 1"},
      {.sid = {.code = CODE_GPS_L2CM, .sat = 1},
       .valid = true,
       .str = "GPS L2CM 1"},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 1}, .valid = false},
      {.sid = {.code = CODE_GPS_L1CA, .sat = 32},
       .valid = true,
       .str = "GPS L1CA 32"},
      {.sid = {.code = CODE_GPS_L1CA, .sat = 33}, .valid = false},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 0}, .valid = false},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 119}, .valid = false},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 120},
       .valid = true,
       .str = "SBAS L1 120"},
      {.sid = {.code = CODE_GPS_L1CA, .sat = 120}, .valid = false},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 138},
       .valid = true,
       .str = "SBAS L1 138"},
      {.sid = {.code = CODE_SBAS_L1CA, .sat = 139}, .valid = false},
      {
          .sid = {.code = CODE_GLO_L1OF, .sat = 0},
          .valid = false,
      },
      {.sid = {.code = CODE_GLO_L1OF, .sat = 1},
       .valid = true,
       .str = "GLO L1OF 1"},
      {.sid = {.code = CODE_GLO_L1OF, .sat = 28},
       .valid = true,
       .str = "GLO L1OF 28"},
      {.sid = {.code = CODE_GLO_L1OF, .sat = 29}, .valid = false},
      {
          .sid = {.code = CODE_GLO_L2OF, .sat = 0},
          .valid = false,
      },
      {.sid = {.code = CODE_GLO_L2OF, .sat = 1},
       .valid = true,
       .str = "GLO L2OF 1"},
      {.sid = {.code = CODE_GLO_L2OF, .sat = 28},
       .valid = true,
       .str = "GLO L2OF 28"},
      {.sid = {.code = CODE_GLO_L2OF, .sat = 29}, .valid = false},
      {.sid = {.code = CODE_GPS_L1P, .sat = 0},
       .valid = false,
       .str = "GPS L1P 0"},
      {.sid = {.code = CODE_GPS_L1P, .sat = 1},
       .valid = true,
       .str = "GPS L1P 1"},
      {.sid = {.code = CODE_GPS_L1P, .sat = 24},
       .valid = true,
       .str = "GPS L1P 24"},
      {.sid = {.code = CODE_GPS_L2P, .sat = 0},
       .valid = false,
       .str = "GPS L2P 0"},
      {.sid = {.code = CODE_GPS_L2P, .sat = 1},
       .valid = true,
       .str = "GPS L2P 1"},
      {.sid = {.code = CODE_GPS_L2P, .sat = 24},
       .valid = true,
       .str = "GPS L2P 24"},
  };

  for (u32 i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
    const struct test_case *t = &test_cases[i];
    bool valid = sid_valid(t->sid);
    fail_unless(t->valid == valid, "test signal %d validity incorrect", i);
    if (valid) {
      gnss_signal_t sid =
          sid_from_code_index(t->sid.code, sid_to_code_index(t->sid));
      fail_unless(sid_is_equal(t->sid, sid),
                  "test signal %d code index conversion failed",
                  sid.code);
      char str[SID_STR_LEN_MAX] = {0};
      u32 ret = sid_to_string(str, sizeof(str), sid);
      fail_unless((strcmp(str, t->str) == 0) && (ret == strlen(t->str)),
                  "signal to string \"%s\" failed",
                  t->str);
      fail_unless(constellation_valid(code_to_constellation(sid.code)),
                  "constellation_valid failed for code %d",
                  sid.code);
    }
    fail_unless(!constellation_valid(CONSTELLATION_COUNT),
                "constellation_valid failed for constellation %d",
                CONSTELLATION_COUNT);
  }
}
END_TEST

START_TEST(test_signal_compare) {
  gnss_signal_t sids[NUM_SIGNALS];
  u32 signal_index = 0;

  for (u32 i = 0; i < ARRAY_COUNT(code_data); i++) {
    const struct code_data_element *e = &code_data[i];
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 sat_index = 0; sat_index < sat_count; sat_index++) {
      gnss_signal_t sid = sid_from_code_index(code, sat_index);
      sids[signal_index++] = sid;
    }
  }

  qsort(sids, NUM_SIGNALS, sizeof(gnss_signal_t), cmp_sid_sid);

  for (u32 i = 1; i < NUM_SIGNALS; i++) {
    fail_unless(!(sid_is_equal(sids[i], sids[i - 1]) &&
                  !code_equiv(sids[i].code, sids[i - 1].code)),
                "signal index %d not unique",
                i);
    fail_unless(sid_compare(sids[i], sids[i - 1]) >= 0,
                "signal index %d not in order",
                i);
  }
}
END_TEST

START_TEST(test_signal_construction) {
  for (u32 i = 0; i < ARRAY_COUNT(code_data); i++) {
    const struct code_data_element *e = &code_data[i];
    code_t code = e->code;
    u16 sat_count = e->sat_count;
    for (u16 code_index = 0; code_index < sat_count; code_index++) {
      gnss_signal_t sid = sid_from_code_index(code, code_index);
      gnss_signal_t csid = construct_sid(sid.code, sid.sat);
      fail_unless(sid_valid(csid),
                  "constructed signal not valid: "
                  "code %d code index %d",
                  code,
                  code_index);
      fail_unless(sid_is_equal(sid, csid),
                  "constructed signal mismatch: "
                  "code %d code index %d",
                  code,
                  code_index);
    }
  }
}
END_TEST

START_TEST(test_code_equivalence) {
  const struct test_case {
    gnss_signal_t sid_a;
    gnss_signal_t sid_b;
    bool equiv;
    char str[20];
  } test_cases[] = {
      {.sid_a = {.code = CODE_GPS_L2P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L2P, .sat = 1},
       .equiv = true,
       .str = "GPS L2P GPS L2P"},
      {.sid_a = {.code = CODE_GPS_L1P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L1P, .sat = 1},
       .equiv = true,
       .str = "GPS L1P GPS L1P"},
      {.sid_a = {.code = CODE_GPS_L2P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L2CM, .sat = 1},
       .equiv = true,
       .str = "GPS L2P GPS L2CM"},
      {.sid_a = {.code = CODE_GPS_L1P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L1CA, .sat = 1},
       .equiv = true,
       .str = "GPS L1P GPS L1CA"},
      {.sid_a = {.code = CODE_GPS_L1P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L2CM, .sat = 1},
       .equiv = false,
       .str = "GPS L2P L1P GPS L2CM"},
      {.sid_a = {.code = CODE_GPS_L2P, .sat = 1},
       .sid_b = {.code = CODE_GPS_L1CA, .sat = 1},
       .equiv = false,
       .str = "GPS L2P GPS L1CA"},
      {.sid_a = {.code = CODE_GLO_L1OF, .sat = 0},
       .sid_b = {.code = CODE_GPS_L1CA, .sat = 0},
       .equiv = false,
       .str = "GLO L1CA GPS L1CA"},
  };
  for (u32 i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
    const struct test_case *t = &test_cases[i];
    fail_unless(code_equiv(t->sid_a.code, t->sid_b.code) == t->equiv,
                "Signal test for %s failed! Expected %d, got %d",
                t->str,
                t->equiv,
                code_equiv(t->sid_a.code, t->sid_a.code));
  }
}
END_TEST

static void glo_map_lock(void) {}
static void glo_map_unlock(void) {}

START_TEST(test_signal_sid_to_carr_freq) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  double carr_freq;
  gnss_signal_t sid = construct_sid(CODE_GPS_L2CM, GPS_FIRST_PRN);
  carr_freq = sid_to_carr_freq(sid);
  fail_unless(GPS_L2_HZ == carr_freq, "L2 carr freq error");

  sid = construct_sid(CODE_GPS_L1CA, GPS_FIRST_PRN);
  carr_freq = sid_to_carr_freq(sid);
  fail_unless(GPS_L1_HZ == carr_freq, "L1 carr freq error");

  /* check all GLO frequency and orbital slots */
  for (u16 sat = GLO_FIRST_PRN; sat <= NUM_SATS_GLO; sat++) {
    for (u16 fcn = GLO_MIN_FCN; fcn <= GLO_MAX_FCN; fcn++) {
      /* L2 first */
      me_gnss_signal_t mesid = construct_mesid(CODE_GLO_L2OF, fcn);
      /* map orb and fcn slots, it does not matter what mesid.code is used */
      glo_map_set_slot_id(mesid, sat);
      sid = construct_sid(CODE_GLO_L2OF, sat);
      carr_freq = sid_to_carr_freq(sid);
      fail_unless((GLO_L2_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                   GLO_L2_DELTA_HZ) == carr_freq,
                  "Glonass L2 carrier error");
      /* now L1 */
      mesid = construct_mesid(CODE_GLO_L1OF, fcn);
      sid = construct_sid(CODE_GLO_L1OF, sat);
      carr_freq = sid_to_carr_freq(sid);
      fail_unless((GLO_L1_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                   GLO_L1_DELTA_HZ) == carr_freq,
                  "Glonass L1 carrier error");
    }
  }
}
END_TEST

START_TEST(test_signal_sid_to_lambda) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  double lambda;

  gnss_signal_t sid = construct_sid(CODE_GPS_L2CM, GPS_FIRST_PRN);
  lambda = sid_to_lambda(sid);
  fail_unless((GPS_C / GPS_L2_HZ) == lambda, "GPS L2 wavelength error");

  sid = construct_sid(CODE_GPS_L1CA, GPS_FIRST_PRN);
  lambda = sid_to_lambda(sid);
  fail_unless((GPS_C / GPS_L1_HZ) == lambda, "GPS L1 wavelength error");

  /* check all GLO frequency and orbital slots */
  for (u16 orb_slot = GLO_FIRST_PRN; orb_slot <= NUM_SATS_GLO; orb_slot++) {
    for (u16 fcn = GLO_MIN_FCN; fcn <= GLO_MAX_FCN; fcn++) {
      /* L2 first */
      me_gnss_signal_t mesid = construct_mesid(CODE_GLO_L2OF, fcn);
      /* map orb and fcn slots, it does not matter what mesid.code is used */
      glo_map_set_slot_id(mesid, orb_slot);
      sid = construct_sid(CODE_GLO_L2OF, orb_slot);
      lambda = sid_to_lambda(sid);
      fail_unless(
          (GPS_C / (GLO_L2_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                    GLO_L2_DELTA_HZ)) == lambda,
          "Glonass L2 wavelength error");
      /* now L1 */
      mesid = construct_mesid(CODE_GLO_L1OF, fcn);
      sid = construct_sid(CODE_GLO_L1OF, orb_slot);
      lambda = sid_to_lambda(sid);
      fail_unless(
          (GPS_C / (GLO_L1_HZ + (glo_map_get_fcn(sid) - GLO_FCN_OFFSET) *
                                    GLO_L1_DELTA_HZ)) == lambda,
          "Glonass L1 wavelength error");
    }
  }
}
END_TEST

START_TEST(test_signal_code_to_chip_count) {
  u32 chip_count;

  chip_count = code_to_chip_count(CODE_GPS_L1CA);
  fail_unless(GPS_L1CA_CHIPS_NUM == chip_count, "GPS_L1CA_CHIPS_NUM error");

  chip_count = code_to_chip_count(CODE_SBAS_L1CA);
  fail_unless(GPS_L1CA_CHIPS_NUM == chip_count, "CODE_SBAS_L1CA error");

  chip_count = code_to_chip_count(CODE_GPS_L2CM);
  fail_unless(GPS_L2CM_CHIPS_NUM == chip_count, "CODE_GPS_L2CM error");

  chip_count = code_to_chip_count(CODE_GPS_L2CL);
  fail_unless(GPS_L2CL_CHIPS_NUM == chip_count, "CODE_GPS_L2CL error");

  /* check unsupported branch for code coverage stats */
  chip_count = code_to_chip_count(CODE_GLO_L2OF);
}
END_TEST

START_TEST(test_signal_code_to_chip_rate) {
  double chip_rate;

  chip_rate = code_to_chip_rate(CODE_GPS_L1CA);
  fail_unless(GPS_CA_CHIPPING_RATE == chip_rate, "GPS_CA_CHIPPING_RATE error");

  chip_rate = code_to_chip_rate(CODE_SBAS_L1CA);
  fail_unless(GPS_CA_CHIPPING_RATE == chip_rate,
              "CODE_SBAS_L1CA chip rate error");

  chip_rate = code_to_chip_rate(CODE_GPS_L2CM);
  fail_unless(GPS_CA_CHIPPING_RATE == chip_rate,
              "CODE_GPS_L2CM chip rate error");

  /* check unsupported branch for code coverage stats */
  chip_rate = code_to_chip_rate(CODE_GLO_L2OF);
}
END_TEST

START_TEST(test_signal_code_requires_direct_acq) {
  bool requires;

  requires = code_to_chip_rate(CODE_GPS_L1CA);
  fail_unless(true == requires, "CODE_GPS_L1CA requires code_to_chip_rate");

  requires = code_requires_direct_acq(CODE_GPS_L2CM);
  fail_unless(false == requires,
              "CODE_GPS_L2CM requires code_requires_direct_acq");
}
END_TEST

START_TEST(test_signal_code_to_carr_to_code) {
  double c2c;

  c2c = mesid_to_carr_to_code(construct_mesid(CODE_GPS_L1CA, 1));
  fail_unless(within_epsilon(1540, c2c),
              "Incorrect GPS L1CA carr_to_code (%lf vs %lf)",
              1540.f,
              c2c);

  c2c = mesid_to_carr_to_code(construct_mesid(CODE_GPS_L2CM, 1));
  fail_unless(within_epsilon(1200, c2c),
              "Incorrect GPS L2CM carr_to_code (%lf vs %lf)",
              1200.f,
              c2c);

  c2c = mesid_to_carr_to_code(construct_mesid(CODE_GLO_L1OF, 1));
  fail_unless(within_epsilon(3127.3238748, c2c),
              "Incorrect GLO L1OF FCN 1 carr_to_code (%lf vs %lf)",
              3127.3238748,
              c2c);
}
END_TEST

START_TEST(test_signal_code_to_prn_period) {
  u16 period;

  period = code_to_prn_period_ms(CODE_GPS_L1CA);
  fail_unless(1 == period, "period not 1");

  period = code_to_prn_period_ms(CODE_GPS_L2CM);
  fail_unless(20 == period, "period not 1");

  period = code_to_prn_period_ms(CODE_GLO_L1OF);
  fail_unless(1 == period, "period not 1");
}
END_TEST

START_TEST(test_sid_system_check) {
  for (u8 i = 0; i < CODE_COUNT; i++) {
    bool gps =
        (i == CODE_GPS_L1CA) || (i == CODE_AUX_GPS) || (i == CODE_GPS_L2CM) ||
        (i == CODE_GPS_L2CL) || (i == CODE_GPS_L2CX) || (i == CODE_GPS_L1P) ||
        (i == CODE_GPS_L2P) || (i == CODE_GPS_L5I) || (i == CODE_GPS_L5Q) ||
        (i == CODE_GPS_L5X) || (i == CODE_GPS_L1CI) || (i == CODE_GPS_L1CQ) ||
        (i == CODE_GPS_L1CX);

    fail_unless(gps == IS_GPS(construct_sid(i, GPS_FIRST_PRN)),
                "is_gps_sid fail");
    fail_unless(gps == IS_GPS(construct_mesid(i, GPS_FIRST_PRN)),
                "is_gps_mesid fail");

    bool glo = (i == CODE_GLO_L1OF) || (i == CODE_GLO_L2OF) ||
               (i == CODE_GLO_L1P) || (i == CODE_GLO_L2P);

    fail_unless(glo == IS_GLO(construct_sid(i, GLO_FIRST_PRN)),
                "is_glo_sid fail");
    fail_unless(glo == IS_GLO(construct_mesid(i, GLO_FIRST_PRN)),
                "is_glo_mesid fail");

    bool sbas = (i == CODE_SBAS_L1CA) || (i == CODE_AUX_SBAS) ||
                (i == CODE_SBAS_L5I) || (i == CODE_SBAS_L5Q) ||
                (i == CODE_SBAS_L5X);

    fail_unless(sbas == IS_SBAS(construct_sid(i, SBAS_FIRST_PRN)),
                "is_sbas_sid fail");
    fail_unless(sbas == IS_SBAS(construct_mesid(i, SBAS_FIRST_PRN)),
                "is_sbas_mesid fail");
  }
}
END_TEST

START_TEST(test_sbas_prn_list) {
  for (u8 prn = SBAS_FIRST_PRN; prn < SBAS_FIRST_PRN + NUM_SATS_SBAS; prn++) {
    gnss_signal_t sid = {prn, CODE_SBAS_L1CA};
    sbas_system_t sbas_system = get_sbas_system(sid);
    fail_unless(sbas_system < SBAS_COUNT);
  }

  for (sbas_system_t sbas_system = (sbas_system_t)0; sbas_system < SBAS_COUNT;
       sbas_system = (sbas_system_t)(sbas_system + 1)) {
    for (u8 i = 0; i < MAX_SBAS_SATS_PER_SYSTEM; i++) {
      u8 prn = sbas_prn_table[sbas_system].prn_list[i];
      fail_unless(prn == 0 || prn >= SBAS_FIRST_PRN,
                  "invalid PRN in sbas_prn_table");
      fail_unless(prn < SBAS_FIRST_PRN + NUM_SATS_SBAS,
                  "invalid PRN in sbas_prn_table");

      if (prn >= SBAS_FIRST_PRN) {
        gnss_signal_t sid = {prn, CODE_SBAS_L1CA};
        fail_unless(get_sbas_system(sid) == sbas_system);
      }
    }
  }
}
END_TEST

Suite *signal_test_suite(void) {
  Suite *s = suite_create("Signal");
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_signal_aggregates);
  tcase_add_test(tc_core, test_signal_from_index);
  tcase_add_test(tc_core, test_signal_properties);
  tcase_add_test(tc_core, test_signal_compare);
  tcase_add_test(tc_core, test_signal_construction);
  tcase_add_test(tc_core, test_code_equivalence);
  tcase_add_test(tc_core, test_signal_sid_to_carr_freq);
  tcase_add_test(tc_core, test_signal_code_to_chip_count);
  tcase_add_test(tc_core, test_signal_code_to_chip_rate);
  tcase_add_test(tc_core, test_signal_code_requires_direct_acq);
  tcase_add_test(tc_core, test_signal_sid_to_lambda);
  tcase_add_test(tc_core, test_signal_code_to_carr_to_code);
  tcase_add_test(tc_core, test_signal_code_to_prn_period);
  tcase_add_test(tc_core, test_sid_system_check);
  tcase_add_test(tc_core, test_sbas_prn_list);
  suite_add_tcase(s, tc_core);
  return s;
}
