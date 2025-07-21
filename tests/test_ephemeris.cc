#include <gtest/gtest.h>
#include <inttypes.h>
#include <swiftnav/almanac.h>
#include <swiftnav/ephemeris.h>
#include <swiftnav/linear_algebra.h>
#include <swiftnav/shm.h>

namespace {

/* Helper enum for triggering test behavior based on where toe lies within fit
 * interval for a sid. */
enum class ephemeris_toe_section_of_fit_interval_t {
  SBAS = 0,
  BEGINNING = 1,
  MIDDLE = 2,
};

/* Set thresholds so high that the unit tests
 * for ephemeris - almanac cross checks actually pass.
 * These should be modified for actual tests. */
#define VALID_ALM_ACCURACY (500000)
#define VALID_EPH_ACCURACY (500000)

const double EPHEMERIS_EPSILON_FOR_TOW_VALIDITY_CHECKS = 1.0e-6;

/* GPS ephemeris for tests.
 * See scenario ME-45. */
static const ephemeris_t gps_eph = {
    .sid = {.sat = 1, .code = CODE_GPS_L1CA},
    .toe = {.tow = 14400, .wn = 1916},
    .ura = 2.0,
    .fit_interval = 14400,
    .valid = 1,
    .health_bits = 0,
    .source = EPH_SOURCE_GPS_LNAV,
    .data =
        {
            .kepler =
                {
                    .tgd =
                        {
                            .gps_s = {5.122274160385132E-9, 0.0},
                        },
                    .crc = 198.9375,
                    .crs = 10.28125,
                    .cuc = 5.327165126800537E-7,
                    .cus = 9.521842002868652E-6,
                    .cic = -2.3655593395233154E-7,
                    .cis = -3.91155481338501E-8,
                    .dn = 4.5637615275575705E-9,
                    .m0 = 2.167759779416001,
                    .ecc = 0.005649387603625655,
                    .sqrta = 5153.644334793091,
                    .omega0 = 1.8718410336467348,
                    .omegadot = -7.896400345341237E-9,
                    .w = 0.4837085715349947,
                    .inc = 0.9649728717477063,
                    .inc_dot = 6.078824636017362E-10,
                    .af0 = 2.5494489818811417E-5,
                    .af1 = 1.2505552149377763E-12,
                    .af2 = 0.0,
                    .toc = {.tow = 14400, .wn = 1916},
                    .iodc = 2,
                    .iode = 2,
                },
        },
};

/* GPS almanac for tests.
 * See scenario ME-45. */
static const almanac_t gps_alm = {
    .sid = {.sat = 1, .code = CODE_GPS_L1CA},
    .toa = {.tow = 53248, .wn = 1916},
    .ura = 900,
    .fit_interval = 504000,
    .valid = 1,
    .health_bits = 0,
    .data =
        {
            .kepler =
                {
                    .m0 = 1.5509826579560628,
                    .ecc = 0.005649566650390625,
                    .sqrta = 5153.64453125,
                    .omega0 = 1.8715344586823712,
                    .omegadot = -7.897471818543825E-9,
                    .w = 0.4837084091510879,
                    .inc = 0.964996154674105,
                    .af0 = 2.574920654296875E-5,
                    .af1 = 0.0,
                },
        },
};

TEST(TestEphemeris, EphemerisAlmanacDivergence) {
  /* See scenario ME-45 description. */

  /* Initially just copy the original ephemeris. */
  ephemeris_t gps_eph_diverged;
  memcpy(&gps_eph_diverged, &gps_eph, sizeof(gps_eph_diverged));

  EXPECT_TRUE(ephemeris_equal(&gps_eph, &gps_eph_diverged));

  /* Next, let's modify and diverge the ephemeris.
   * Further modify these if the test case changes. */
  gps_eph_diverged.data.kepler.dn = 10.4154338446262e-009;
  gps_eph_diverged.data.kepler.m0 = 2.16970122385066e+000;

  EXPECT_FALSE(ephemeris_equal(&gps_eph, &gps_eph_diverged));

  /* First check point: start of the position test interval */
  gps_time_t t_start = gps_eph.toe;
  t_start.tow += -(double)gps_eph.fit_interval / 2.0;
  normalize_gps_time(&t_start);
  const satellite_orbit_type_t orbit_type = MEO;

  gps_time_t t = t_start;

  for (u8 i = 0; i < 3;
       ++i, t.tow += gps_eph.fit_interval / 2.0, normalize_gps_time(&t)) {
    double d;
    double _[3];
    double alm_sat_pos[3];
    double eph_sat_pos[3];
    double div_sat_pos[3];

    bool calc_alm_ok =
        (0 == calc_sat_state_almanac(&gps_alm, &t, alm_sat_pos, _, _, _, _));
    bool calc_eph_ok =
        (0 ==
         calc_sat_state_n(&gps_eph, &t, orbit_type, eph_sat_pos, _, _, _, _));
    bool calc_eph_div_ok =
        (0 == calc_sat_state_n(
                  &gps_eph_diverged, &t, orbit_type, div_sat_pos, _, _, _, _));

    /* Check successful sat state calculation. */
    EXPECT_TRUE(calc_alm_ok && calc_eph_ok && calc_eph_div_ok)
        << "Failure computing sat state! \n"
           "Alm success: %" PRIu8
           " \n"
           "Eph success: %" PRIu8
           " \n"
           "Eph diverged success: %" PRIu8 " \n"
        << (uint8_t)calc_alm_ok << (uint8_t)calc_eph_ok
        << (uint8_t)calc_eph_div_ok;

    /* Almanac vs. diverged ephemeris comparison. */

    /* Compute distance [m] */
    d = vector_distance(3, alm_sat_pos, div_sat_pos);

    EXPECT_TRUE(d <= VALID_ALM_ACCURACY)
        << "Almanac vs. diverging  check failed: \n"
           "Iteration: %" PRIu8
           " \n"
           "Distance: %lf \n"
           "Alm_sat_pos_x: %lf, Diverged_sat_pos_x: %lf \n"
           "Alm_sat_pos_y: %lf, Diverged_sat_pos_y: %lf \n"
           "Alm_sat_pos_z: %lf, Diverged_sat_pos_z: %lf"
        << i << d << alm_sat_pos[0] << div_sat_pos[0] << alm_sat_pos[1]
        << div_sat_pos[1] << alm_sat_pos[2] << div_sat_pos[2];

    /* Ephemeris vs. diverged ephemeris comparison. */

    /* Compute distance [m] */
    d = vector_distance(3, eph_sat_pos, div_sat_pos);

    EXPECT_TRUE(d <= VALID_EPH_ACCURACY)
        << "Ephemeris vs. diverging ephemeris check failed: \n"
           "Iteration: %" PRIu8
           " \n"
           "Distance: %lf \n"
           "Eph_sat_pos_x: %lf, Diverged_sat_pos_x: %lf \n"
           "Eph_sat_pos_y: %lf, Diverged_sat_pos_y: %lf \n"
           "Eph_sat_pos_z: %lf, Diverged_sat_pos_z: %lf"
        << i << d << eph_sat_pos[0] << div_sat_pos[0] << eph_sat_pos[1]
        << div_sat_pos[1] << eph_sat_pos[2] << div_sat_pos[2];
  }
}

TEST(TestEphemeris, EphemerisEqual) {
  ephemeris_t a;
  ephemeris_t b;

  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));

  EXPECT_TRUE(ephemeris_equal(&a, &b));

  a.valid = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.health_bits = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.sat = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = static_cast<code_e>(1);
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = static_cast<code_e>(1);
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.toe.wn = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.toe.tow = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.ura = 1;
  EXPECT_TRUE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.fit_interval = 1;
  EXPECT_TRUE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.tgd.gps_s[0] = 1.0;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.crs = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.crc = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.cuc = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.cus = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.cic = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.cis = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.dn = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.m0 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.ecc = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.sqrta = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.omega0 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.omegadot = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.w = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.inc = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.inc_dot = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.af0 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.af1 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.af2 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.iode = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.iodc = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.toc.wn = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GPS_L1CA;
  a.data.kepler.toc.tow = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.pos[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.pos[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.pos[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.vel[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.vel[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.vel[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.acc[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.acc[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.acc[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.a_gf0 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_SBAS_L1CA;
  a.data.xyz.a_gf1 = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.gamma = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.tau = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.d_tau = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.iod = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.fcn = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.pos[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.pos[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.pos[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.vel[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.vel[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.vel[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.acc[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.acc[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L1OF;
  a.data.glo.acc[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.gamma = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.tau = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.d_tau = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.iod = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.fcn = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.pos[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.pos[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.pos[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.vel[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.vel[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.vel[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.acc[0] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.acc[1] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));

  a.sid.code = CODE_GLO_L2OF;
  a.data.glo.acc[2] = 1;
  EXPECT_FALSE(ephemeris_equal(&a, &b));
  memset(&a, 0, sizeof(a));
}

TEST(TestEphemeris, EphemerisHealth) {
  const struct test_case {
    ephemeris_t e;
    bool healthy;
  } test_cases[] = {
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L1CA},
               .ura = 2.0,
               .valid = 1u,
               .health_bits = 0,
           },
       .healthy = true},
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L1CA},
               .ura = 200.0,
               .valid = 0u,
               .health_bits = 0,
           },
       .healthy = true},
      {.e =
           {
               .sid = {.sat = 32, .code = CODE_GPS_L2CM},
               .ura = 2000.0,
               .valid = 1u,
               .health_bits = 0,
           },
       .healthy = true},
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L1CA},
               .ura = 33333.0,
               .valid = 1u,
               .health_bits = 0,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L1CA},
               .ura = INVALID_URA_VALUE,
               .valid = 1u,
               .health_bits = 0,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L1CA},
               .ura = -100.0,
               .valid = 1u,
               .health_bits = 0,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 11, .code = CODE_GPS_L1CA},
               .ura = 1.0,
               .valid = 1u,
               .health_bits = 0x3F,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 12, .code = CODE_GPS_L1CA},
               .ura = 1.0,
               .valid = 1u,
               .health_bits = 0x2A,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 13, .code = CODE_GPS_L2CM},
               .ura = 4000.0,
               .valid = 1u,
               .health_bits = 0x2E,
           },
       .healthy = false},
      {.e =
           {
               .sid = {.sat = 1, .code = CODE_GPS_L2CM},
               .ura = 4000.0,
               .valid = 1u,
               .health_bits = 0x2A,
           },
       .healthy = true},
      {.e =
           {
               .sid = {.sat = 22, .code = CODE_GPS_L1CA},
               .ura = 10.0,
               .valid = 1u,
               .health_bits = 0x2E,
           },
       .healthy = false},
  };

  for (u8 i = 0; i < (sizeof(test_cases) / sizeof(test_cases[0])); i++) {
    const struct test_case *t = &test_cases[i];
    EXPECT_EQ(t->healthy, ephemeris_healthy(&t->e, t->e.sid.code))
        << "test signal " << i << " healthy incorrect";
  }
}

TEST(TestEphemeris, Test6bitHealthWord) {
  /*
   * u8 gps_healthy(u8 health_bits, code_t code)
   */

  const struct test_case {
    u8 health_bits;
    code_t code;
    u8 result;
  } test_cases[] = {
      {.health_bits = 0, .code = CODE_GPS_L1CA, .result = 1},
      {.health_bits = 0, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x2B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x2B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x0B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x0B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x0B, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x0B, .code = CODE_GPS_L2CM, .result = 1},
      {.health_bits = 0x2E, .code = CODE_GPS_L1CA, .result = 0},
      {.health_bits = 0x2E, .code = CODE_GPS_L2CM, .result = 0},
      {.health_bits = 0x0E, .code = CODE_GPS_L1CA, .result = 1},
      {.health_bits = 0x0E, .code = CODE_GPS_L2CM, .result = 0},
      {.health_bits = 0x04, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x07, .code = CODE_GPS_L2P, .result = 0},
      {.health_bits = 0x20, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x20, .code = CODE_GPS_L1P, .result = 0},
      {.health_bits = 0x01, .code = CODE_GPS_L2P, .result = 0},
      {.health_bits = 0, .code = CODE_GPS_L5I, .result = 1},
  };
  for (const auto &i : test_cases) {
    const struct test_case *t = &i;
    EXPECT_EQ(t->result, check_6bit_health_word(t->health_bits, t->code))
        << "check_6bit_health_word(%d, %d) test failed (%d)" << t->health_bits
        << t->code << t->result;
  }
}

TEST(TestEphemeris, EphemerisBds) {
  /* clang-format off */
  static const ephemeris_t ephe_exp = {
    .sid = {
      .sat = 25,
      .code = static_cast<code_e>(12)
    },
    .toe = {
        .tow = 460800.000000,
        .wn = 2091
    },
    .ura = 2.000000,
    .fit_interval = 0,
    .valid = 0,
    .health_bits = 0,
    .source = EPH_SOURCE_BDS_D1_D2_NAV,
    .data = {.kepler = {
      .tgd = {
        .bds_s = {-2.99999997e-10, -2.99999997e-10}
      },
      .crc = 167.140625,
      .crs = -18.828125,
      .cuc = -9.0105459094047546e-07,
      .cus = 9.4850547611713409e-06,
      .cic = -4.0978193283081055e-08,
      .cis = 1.0104849934577942e-07,
      .dn = 3.9023054038264214e-09,
      .m0 = 0.39869951815527438,
      .ecc = 0.00043709692545235157,
      .sqrta = 5282.6194686889648,
      .omega0 = 2.2431156200949509,
      .omegadot = -6.6892072037584707e-09,
      .w = 0.39590413040186828,
      .inc = 0.95448398903792575,
      .inc_dot = -6.2716898124832475e-10,
      .af0 = -0.00050763087347149849,
      .af1 = -1.3019807454384136e-11,
      .af2 = 0.000000,
      .toc = {
        .tow = 460800,
        .wn = 2091
      },
      .iodc = 160,
      .iode = 160
    }}
  };

  static const u32 words[3][10] = {
    {0x38901714, 0x5F81035, 0x5BEE184, 0x3FDF95, 0x3D0B09CA,
     0x3C47CDE6, 0x19AC7AD, 0x24005E73, 0x2ED79F72, 0x38D7A13C},

    {0x38902716, 0x610AAF9, 0x2EFE1C86, 0x1103E979, 0x18E80030,
     0x394A8A9E, 0x4F9109A, 0x29C9FE18, 0x34BA516C, 0x13D2B18F},

    {0x38903719, 0x62B0869, 0x4DC786, 0x1087FF8F, 0x3D47FD49,
     0x2DAE0084, 0x1B3C9264, 0xB6C9161, 0x1B58811D, 0x2DC18C7}
  };
  /* clang-format on */

  gnss_signal_t sid = {.sat = 25, .code = static_cast<code_e>(12)};

  ephemeris_t ephe;
  memset(&ephe, 0, sizeof(ephe));
  decode_bds_d1_ephemeris(words, sid, &ephe);

  EXPECT_TRUE(ephemeris_equal(&ephe_exp, &ephe));
}

TEST(TestEphemeris, EphemerisGal) {
  /* clang-format off */
  static const ephemeris_t ephe_exp = {
    .sid = {
      .sat = 8,
      .code = static_cast<code_e>(14)
    },
    .toe = {
      .tow = 135000.,
      .wn = 2090
    },
    .ura = 3.120000,
    .fit_interval = 14400,
    .valid = 1,
    .health_bits = 0,
    .source = EPH_SOURCE_GAL_INAV,
    .data = {.kepler = {
      .tgd = {
        .gal_s = {-5.5879354476928711e-09, -6.5192580223083496e-09}
      },
      .crc = 62.375,
      .crs = -54.0625,
      .cuc = -2.3748725652694702e-06,
      .cus = 1.2902542948722839e-05,
      .cic = 7.4505805969238281e-09,
      .cis = 4.6566128730773926e-08,
      .dn = 2.9647663515616992e-09,
      .m0 = 1.1731263781996162,
      .ecc = 0.00021702353842556477,
      .sqrta = 5440.6276874542236,
      .omega0 = 0.7101536200630526,
      .omegadot = -5.363080536688408e-09,
      .w = 0.39999676368790066,
      .inc = 0.95957029480011957,
      .inc_dot = 4.3751822439020375e-10,
      .af0 = 0.0062288472545333198,
      .af1 = -5.4427573559223666e-12,
      .af2 = 0,

      .toc = {
        .tow = 135000,
        .wn = 2090
      },
      .iodc = 97,
      .iode = 97,
    }
  }};

  static const u8 words[5][GAL_INAV_CONTENT_BYTE] = {
    {  0x4, 0x61, 0x23, 0x28, 0xBF, 0x30, 0x9B, 0xA0,  0x0, 0x71, 0xC8, 0x6A, 0xA8, 0x14, 0x16, 0x7},
    {  0x8, 0x61, 0x1C, 0xEF, 0x2B, 0xC3, 0x27, 0x18, 0xAE, 0x65, 0x10, 0x4C, 0x1E, 0x1A, 0x13, 0x25},
    {  0xC, 0x61, 0xFF, 0xC5, 0x58, 0x20, 0x6D, 0xFB,  0x5, 0x1B,  0xF,  0x7, 0xCC, 0xF9, 0x3E, 0x6B},
    { 0x10, 0x61, 0x20,  0x0, 0x10,  0x0, 0x64, 0x8C, 0xA0, 0xCC, 0x1B, 0x5B, 0xBF, 0xFE, 0x81, 0x1},
    { 0x14, 0x50, 0x80, 0x20,  0x5, 0x81, 0xF4, 0x7C, 0x80, 0x21, 0x51,  0x9, 0xB6, 0xAA, 0xAA, 0xAA}
  };
  /* clang-format on */

  ephemeris_t ephe;
  memset(&ephe, 0, sizeof(ephe));
  decode_gal_ephemeris(words, &ephe);
  ephe.sid.code = CODE_GAL_E1B;
  ephe.valid = 1;

  EXPECT_TRUE(ephemeris_equal(&ephe_exp, &ephe));
}

void test_ephemeris_validity_window(
    const ephemeris_t &eph,
    gps_time_t &t_valid,
    ephemeris_toe_section_of_fit_interval_t toe_section) {
  ephemeris_validity_window_t window =
      ephemeris_validity_window(&eph, &t_valid);

  gps_time_t lower_bound = eph.toe;
  gps_time_t upper_bound = eph.toe;
  if (toe_section == ephemeris_toe_section_of_fit_interval_t::MIDDLE) {
    lower_bound.tow -= eph.fit_interval / 2.0;
    upper_bound.tow += eph.fit_interval / 2.0;
  } else if (toe_section ==
             ephemeris_toe_section_of_fit_interval_t::BEGINNING) {
    upper_bound.tow += eph.fit_interval;
  } else if (toe_section == ephemeris_toe_section_of_fit_interval_t::SBAS) {
    upper_bound.tow += SBAS_FIT_INTERVAL_SECONDS;
  }
  EXPECT_EQ(window.bgn, lower_bound);
  EXPECT_EQ(window.end, upper_bound);

  gps_time_t below_lower_bound = lower_bound;
  below_lower_bound.tow -= EPHEMERIS_EPSILON_FOR_TOW_VALIDITY_CHECKS;
  gps_time_t above_upper_bound = upper_bound;
  above_upper_bound.tow += EPHEMERIS_EPSILON_FOR_TOW_VALIDITY_CHECKS;
  EXPECT_EQ(ephemeris_valid(&eph, &lower_bound), 1);
  EXPECT_EQ(ephemeris_valid(&eph, &upper_bound), 1);
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  if (IS_SBAS(eph.sid)) {
    EXPECT_EQ(ephemeris_valid(&eph, &below_lower_bound), 1);
    EXPECT_EQ(ephemeris_valid(&eph, &above_upper_bound), 1);
  } else {
    EXPECT_EQ(ephemeris_valid(&eph, &below_lower_bound), 0);
    EXPECT_EQ(ephemeris_valid(&eph, &above_upper_bound), 0);
  }
}

TEST(TestEphemeris, EphemerisValidityWindow) {
  ephemeris_t eph;
  gps_time_t t_valid = gps_eph.toe;

  eph = gps_eph;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::MIDDLE);

  eph = gps_eph;
  t_valid = gps_eph.toe;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::MIDDLE);

  eph = gps_eph;
  t_valid = gps_eph.toe;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::BEGINNING);

  eph = gps_eph;
  t_valid = gps_eph.toe;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::BEGINNING);

  eph = gps_eph;
  t_valid = gps_eph.toe;
  eph.sid.code = CODE_GLO_L1OF;
  eph.source = EPH_SOURCE_GLO_FDMA;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::MIDDLE);

  eph = gps_eph;
  t_valid = gps_eph.toe;
  eph.sid.code = CODE_SBAS_L1CA;
  test_ephemeris_validity_window(
      eph, t_valid, ephemeris_toe_section_of_fit_interval_t::SBAS);
}

TEST(TestEphemeris, EphemerisValid) {
  const gps_time_t t_valid = gps_eph.toe;
  const gps_time_t t_late = {
      .tow =
          t_valid.tow + static_cast<double>(gps_eph.fit_interval) / 2.0 + 1.0,
      .wn = t_valid.wn,
  };
  const gps_time_t t_late_gal = {
      .tow = t_valid.tow + static_cast<double>(gps_eph.fit_interval) + 1.0,
      .wn = t_valid.wn,
  };
  ephemeris_t eph;

  EXPECT_EQ(ephemeris_valid(nullptr, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(nullptr, &t_valid), EPH_NULL);

  eph = gps_eph;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_GLO_L1OF;
  eph.source = EPH_SOURCE_GLO_FDMA;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_SBAS_L1CA;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.valid = 0u;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID);

  eph = gps_eph;
  eph.toe.wn = 0;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_WN_EQ_0);

  eph = gps_eph;
  eph.fit_interval = 0;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_FIT_INTERVAL_EQ_0);

  eph = gps_eph;
  eph.health_bits = 0x3F;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);

  eph = gps_eph;
  eph.data.kepler.iodc = 1;
  eph.data.kepler.iode = 3;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.data.kepler.iodc = GPS_IODC_MAX;
  eph.data.kepler.iode = GPS_IODE_MAX;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.data.kepler.iodc = GPS_IODC_MAX + 1;
  eph.data.kepler.iode = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.data.kepler.iodc = GPS_IODC_MAX + 1;
  eph.data.kepler.iode = GPS_IODE_MAX + 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  EXPECT_EQ(ephemeris_valid(&eph, &t_late), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_late), EPH_TOO_OLD);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  eph.health_bits = 0x3F;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  eph.data.kepler.iodc = 1;
  eph.data.kepler.iode = 3;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  eph.data.kepler.iodc = GPS_IODC_MAX;
  eph.data.kepler.iode = GPS_IODE_MAX;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  eph.data.kepler.iodc = GPS_IODC_MAX + 1;
  eph.data.kepler.iode = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  eph.data.kepler.iodc = GPS_IODC_MAX + 1;
  eph.data.kepler.iode = GPS_IODE_MAX + 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_QZS_L1CA;
  eph.source = EPH_SOURCE_QZS_LNAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_late), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_late), EPH_TOO_OLD);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  eph.health_bits = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  eph.data.kepler.iodc = 1;
  eph.data.kepler.iode = 3;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  eph.data.kepler.iodc = GAL_IOD_NAV_MAX;
  eph.data.kepler.iode = GAL_IOD_NAV_MAX;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  eph.data.kepler.iodc = GAL_IOD_NAV_MAX + 1;
  eph.data.kepler.iode = GAL_IOD_NAV_MAX + 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_GAL_E1B;
  eph.source = EPH_SOURCE_GAL_INAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_late_gal), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_late_gal), EPH_TOO_OLD);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  eph.health_bits = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  eph.data.kepler.iodc = 1;
  eph.data.kepler.iode = 3;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  eph.data.kepler.iodc = BDS2_IODC_MAX;
  eph.data.kepler.iode = BDS2_IODE_MAX;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  eph.data.kepler.iodc = BDS2_IODC_MAX + 1;
  eph.data.kepler.iode = BDS2_IODE_MAX + 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_BDS2_B1;
  eph.source = EPH_SOURCE_BDS_D1_D2_NAV;
  EXPECT_EQ(ephemeris_valid(&eph, &t_late_gal), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_late_gal), EPH_TOO_OLD);

  eph = gps_eph;
  eph.sid.code = CODE_BDS3_B1CI;
  eph.source = EPH_SOURCE_BDS_BCNAV1;
  eph.data.kepler.iodc = 1;
  eph.data.kepler.iode = 3;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_BDS3_B1CI;
  eph.source = EPH_SOURCE_BDS_BCNAV1;
  eph.data.kepler.iodc = BDS3_IODC_MAX;
  eph.data.kepler.iode = BDS3_IODE_MAX;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 1);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_VALID);

  eph = gps_eph;
  eph.sid.code = CODE_BDS3_B1CI;
  eph.source = EPH_SOURCE_BDS_BCNAV1;
  eph.data.kepler.iodc = BDS3_IODC_MAX + 1;
  eph.data.kepler.iode = BDS3_IODE_MAX + 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_INVALID_IOD);

  eph = gps_eph;
  eph.sid.code = CODE_GLO_L1OF;
  eph.source = EPH_SOURCE_GLO_FDMA;
  eph.health_bits = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);

  eph = gps_eph;
  eph.sid.code = CODE_GLO_L1OF;
  eph.source = EPH_SOURCE_GLO_FDMA;
  EXPECT_EQ(ephemeris_valid(&eph, &t_late), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_late), EPH_TOO_OLD);

  eph = gps_eph;
  eph.sid.code = CODE_SBAS_L1CA;
  eph.health_bits = 1;
  EXPECT_EQ(ephemeris_valid(&eph, &t_valid), 0);
  EXPECT_EQ(ephemeris_valid_detailed(&eph, &t_valid), EPH_UNHEALTHY);
}
}  // namespace
