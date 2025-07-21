#include <gtest/gtest.h>
#include <inttypes.h>
#include <swiftnav/decode_glo.h>
#include <swiftnav/linear_algebra.h>
#include <swiftnav/shm.h>

namespace {

#define LOW_TOL 1e-6
#define HIGH_TOL 1e-1

ephemeris_t eph;

/*This input strings were taken from collected data,
 * refer to libswiftnav/tests/data/gloframesstream/raw_glo_frame_ascii.log
 * #GLORAWFRAMEA,USB1,0,73.0,SATTIME,1892,300827.000,00000000,8792,13498;
 * 3,55,4,1892,300827.077,52,52,15,
 * 01074396999b05c3a850b5,0,021760a5256204d9c15f66,0,0380269d60899a6d0e3123,0,
 * 04865d1cc0000000344918,0,050d100000000340000895,0,06ab81d85b1019f107b83c,0,
 * 0705d3179ea697fc554079,0,082c00148c06153f8e133e,0,0972222bcd4e97ff14be12,0,
 * 0aad8090a54019cb035d3f,0,0b3e2240201e97fc34fc39,0,0cae940cdc3c1e2786da9b,0,
 * 0d68bf54a4c697f115320b,0,0eaf8449b38c1e228932d8,0,0f815b653eee981314802a,0*f3837a1c
 */
glo_string_t strings_in[5] = {
    {{0xc3a850b5, 0x96999b05, 0x010743}}, /* 01074396999b05c3a850b5 */
    {{0xd9c15f66, 0xa5256204, 0x021760}}, /* 021760a5256204d9c15f66 */
    {{0x6d0e3123, 0x9d60899a, 0x038026}}, /* 0380269d60899a6d0e3123 */
    {{0x00344918, 0x1cc00000, 0x04865d}}, /* 04865d1cc0000000344918 */
    {{0x40000895, 0x3, 0x050d10}}         /* 050d100000000340000895 */
};

/* RAW strings above correspond to following data from same file:
 * #GLOEPHEMERISA,USB1,0,73.5,SATTIME,1892,300617.000,00000000,8d29,13498;
 * 55,4,1,4,1892,301517000,10783,104,0,0,59,0,
 * -1.4453039062500000e+07,-6.9681713867187500e+06,1.9873773925781250e+07, <--
 * X, Y, Z
 * -1.4125013351440430e+03,-2.3216266632080078e+03,-1.8360681533813477e+03, <--
 * Vx, Vy, Vz
 * 0.00000000000000000,0.00000000000000000,-2.79396772384643555e-06, <-- Ax, Ay,
 * Az
 * -9.71024855971336365e-05, <-- tau
 * 5.587935448e-09,
 * 1.81898940354585648e-12, <-- gamma
 * 52200,3,0,0,13*955c64e9
 */
double X = -1.4453039062500000e+07;
double Y = -6.9681713867187500e+06;
double Z = 1.9873773925781250e+07;
double VX = -1.4125013351440430e+03;
double VY = -2.3216266632080078e+03;
double VZ = -1.8360681533813477e+03;
double AX = 0;
double AY = 0;
double AZ = -2.79396772384643555e-06;
double GAMMA = 1.81898940354585648e-12;
double TAU = -9.71024855971336365e-05;

void e_out(void) {
  log_debug("GLO Ephemeris:\n");
  log_debug("\tSID: %u (code %u)\n", eph.sid.sat, eph.sid.code);
  log_debug("\tSource: %u\n", eph.source);
  log_debug("\tGPS time: TOE %f, WN %d\n", eph.toe.tow, eph.toe.wn);
  log_debug("\tURA: %f\n", eph.ura);
  log_debug("\tFit interval: %u\n", eph.fit_interval);
  log_debug("\tValid: %u\n", eph.valid);
  log_debug("\tHealth bits: 0x%02x\n", eph.health_bits);
  log_debug("\tgamma: %25.18f\n", eph.data.glo.gamma);
  log_debug("\ttau: %25.18f\n", eph.data.glo.tau);
  log_debug("\tX, Y, Z: %25.18f, %25.18f, %25.18f\n",
            eph.data.glo.pos[0],
            eph.data.glo.pos[1],
            eph.data.glo.pos[2]);
  log_debug("\tVX, VY, VZ: %25.18f, %25.18f, %25.18f\n",
            eph.data.glo.vel[0],
            eph.data.glo.vel[1],
            eph.data.glo.vel[2]);
  log_debug("\tAX, AY, AZ: %25.18f, %25.18f, %25.18f\n",
            eph.data.glo.acc[0],
            eph.data.glo.acc[1],
            eph.data.glo.acc[2]);
  EXPECT_NEAR(eph.data.glo.pos[0], X, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.pos[1], Y, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.pos[2], Z, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.vel[0], VX, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.vel[1], VY, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.vel[2], VZ, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.acc[0], AX, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.acc[1], AY, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.acc[2], AZ, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.tau, TAU, LOW_TOL);
  EXPECT_NEAR(eph.data.glo.gamma, GAMMA, LOW_TOL);
}

TEST(TestDecodeGlonass, ExtractGloWord) {
  u32 ret = 0;

  glo_string_t string;

  string.word[0] = 5;
  string.word[1] = 5;
  string.word[2] = 5;
  ret = extract_word_glo(&string, 1, 32);
  EXPECT_EQ(ret, 5);
  ret = extract_word_glo(&string, 33, 3);
  EXPECT_EQ(ret, 5);
  ret = extract_word_glo(&string, 65, 3);
  EXPECT_EQ(ret, 5);

  string.word[0] = 0x12345678;
  string.word[1] = 0xdeadbeef;
  string.word[2] = 0x87654321;
  ret = extract_word_glo(&string, 1, 32);
  EXPECT_EQ(ret, 0x12345678);
  ret = extract_word_glo(&string, 33, 32);
  EXPECT_EQ(ret, 0xdeadbeef);
  ret = extract_word_glo(&string, 65, 32);
  EXPECT_EQ(ret, 0x87654321);
  ret = extract_word_glo(&string, 49, 4);
  EXPECT_EQ(ret, 0xd);

  string.word[0] = 0xbeef0000;
  string.word[1] = 0x4321dead;
  string.word[2] = 0x00008765;
  ret = extract_word_glo(&string, 17, 32);
  EXPECT_EQ(ret, 0xdeadbeef);
  ret = extract_word_glo(&string, 49, 32);
  EXPECT_EQ(ret, 0x87654321);
  ret = extract_word_glo(&string, 49, 16);
  EXPECT_EQ(ret, 0x4321);
}

TEST(TestDecodeGlonass, ErrorCorrectionGlo) {
  const struct {
    glo_string_t str_in; /**< input string for test  */
    s8 ret;              /** result of the test */
  } test_case[] = {
      /* First, simply test one GLO nav message received from Novatel,
       * we trust Novatel, so no errors must be */
      {{{0xc90cfb3e, 0x9743a301, 0x010749}}, 0}, /* case 0 */
      {{{0xdd39f5fc, 0x24542d0c, 0x021760}}, 0},
      {{{0x653bc7e9, 0x1e8ead92, 0x038006}}, 0},
      {{{0x60342dfc, 0x41000002, 0x0481c7}}, 0},
      {{{0x40000895, 0x00000003, 0x050d10}}, 0},
      {{{0x530a7ecf, 0x059c4415, 0x06b082}}, 0},
      {{{0xfd94beb6, 0x7a577e97, 0x070f46}}, 0},
      {{{0xba02de6f, 0x988e6814, 0x08b101}}, 0},
      {{{0x12064831, 0x87767698, 0x09e1a6}}, 0},
      {{{0xaf870be5, 0x54ef2617, 0x0ab286}}, 0},
      {{{0x0f06ba41, 0x9a3f2698, 0x0b8f7c}}, 0},
      {{{0x2f012204, 0xf0c3c81a, 0x0cb309}}, 0},
      {{{0x1c858601, 0x10c47e98, 0x0da065}}, 0},
      {{{0x5205980b, 0xf49abc1a, 0x0eb40e}}, 0},
      {{{0x15454437, 0x2504e698, 0x0f8c09}}, 0},
      /* Second, take 1st string from other GLO nav message and introduce an
       * error in data bits */
      {{{0xc90cfb81, 0x9743a301, 0x010748}}, 0}, /* case 15, no errors  */
      {{{0xc90cfb81, 0x9743a301, 0x110748}}, 85},
      {{{0xc90cfb81, 0x1743a301, 0x010748}}, 64},
      {{{0x490cfb81, 0x9743a301, 0x010748}}, 32},
      {{{0xc90cfb81, 0x9743a300, 0x010748}}, 33},
      {{{0xc90cfb81, 0x9743a301, 0x010749}}, 65},
      {{{0xc90cfb81, 0x9743a301, 0x000748}}, 81},
      {{{0xc90c3b81, 0x9743a301, 0x010748}}, -1},
      {{{0xc90cfb81, 0x974fa301, 0x010748}}, -1},
      {{{0xc90cfb81, 0x9743a301, 0x01074b}}, -1},
      {{{0xc90cfb81, 0x9743a301, 0x010744}}, -1},
      {{{0xc90cfb81, 0x9aaaa301, 0x010748}}, -1},
      {{{0xc90cfb81, 0x9743a301, 0x010748}}, 0}, /* no errors here */
  };

  for (auto i : test_case) {
    s8 ret = error_detection_glo(&i.str_in);
    EXPECT_EQ(i.ret, ret);
  }
}

TEST(TestDecodeGlonass, DecodeEphemerisGlo) {
  gnss_signal_t sid = SID_UNKNOWN;

  decode_glo_ephemeris(strings_in, sid, /* utc_params = */ nullptr, &eph);

  e_out();
}

}  // namespace
