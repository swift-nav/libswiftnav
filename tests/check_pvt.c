#include <check.h>
#include <math.h>
#include <stdio.h>

#include <swiftnav/constants.h>
#include <swiftnav/coord_system.h>
#include <swiftnav/single_epoch_solver.h>
#include "check_suites.h"
#include "check_utils.h"

#define TOR_WN 1939
#define TOR_TOW 42.0

/* time of reception for the tests */
static const gps_time_t tor = {.wn = TOR_WN, .tow = TOR_TOW};

static navigation_measurement_t nm1 = {
    .sid = {.sat = 9, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 23946993.888943646,
    .raw_pseudorange = 23946993.888943646,
    .sat_pos = {-19477278.087422125, -7649508.9457812719, 16674633.163554827},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm1_no_doppler = {
    .sid = {.sat = 9, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 23946993.888943646,
    .raw_pseudorange = 23946993.888943646,
    .sat_pos = {-19477278.087422125, -7649508.9457812719, 16674633.163554827},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID};

static navigation_measurement_t nm2 = {
    .sid = {.sat = 1, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 22932174.156858064,
    .raw_pseudorange = 22932174.156858064,
    .sat_pos = {-9680013.5408340245, -15286326.354385279, 19429449.383770257},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm3 = {
    .sid = {.sat = 2, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 24373231.648055989,
    .raw_pseudorange = 24373231.648055989,
    .sat_pos = {-19858593.085281931, -3109845.8288993631, 17180320.439503901},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm4 = {
    .sid = {.sat = 3, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 24779663.252316438,
    .raw_pseudorange = 24779663.252316438,
    .sat_pos = {6682497.8716542246, -14006962.389166718, 21410456.275678463},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm5 = {
    .sid = {.sat = 4, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 26948717.022331879,
    .raw_pseudorange = 26948717.022331879,
    .sat_pos = {7415370.9916331079, -24974079.044485383, -3836019.0262199985},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm6 = {
    .sid = {.sat = 5, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 23327405.435463827,
    .raw_pseudorange = 23327405.435463827,
    .sat_pos = {-2833466.1648670658, -22755197.793894723, 13160322.082875408},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

/* doppler measurement broken */
static navigation_measurement_t nm6b = {
    .sid = {.sat = 5, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 23327405.435463827,
    .raw_pseudorange = 23327405.435463827,
    .sat_pos = {-2833466.1648670658, -22755197.793894723, 13160322.082875408},
    .sat_vel = {0, 0, 0},
    .cn0 = 40,
    .lock_time = 5,
    .measured_doppler = 10000, /* Doppler outlier */
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID | NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm7 = {
    .sid = {.sat = 6, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 27371419.016328193,
    .raw_pseudorange = 27371419.016328193,
    .sat_pos = {14881660.383624561, -5825253.4316490609, 21204679.68313824},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm8 = {
    .sid = {.sat = 7, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 26294221.697782904,
    .raw_pseudorange = 26294221.697782904,
    .sat_pos = {12246530.477279386, -22184711.955107089, 7739084.2855069181},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm9 = {
    .sid = {.sat = 8, .code = CODE_GPS_L1CA},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 25781999.479948733,
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm10 = {
    .sid = {.sat = 8, .code = CODE_GPS_L2CM},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 25781999.479948733,
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

/* broken measurement */
static navigation_measurement_t nm10b = {
    .sid = {.sat = 8, .code = CODE_GPS_L2CM},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 25781999.479948733 + 30000,
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

static navigation_measurement_t nm11 = {
    .sid = {.sat = 11, .code = CODE_GPS_L2CM},
    .tot = {.wn = TOR_WN, .tow = TOR_TOW - 0.077},
    .pseudorange = 25781999.479948733,
    .raw_pseudorange = 25781999.479948733,
    .sat_pos = {-25360766.249484103, -1659033.490658124, 7821492.0398916304},
    .lock_time = 5,
    .flags = NAV_MEAS_FLAG_CODE_VALID | NAV_MEAS_FLAG_MEAS_DOPPLER_VALID |
             NAV_MEAS_FLAG_PHASE_VALID};

START_TEST(test_pvt_failed_repair) {
  u8 n_used = 5;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nms[9] = {nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8};

  calc_PVT(n_used,
           nms,
           &tor,
           false,
           true,
           ALL_CONSTELLATIONS,
           &soln,
           &dops,
           &raim_removed_sids);
  /* PVT repair requires at least 6 measurements. */
  fail_unless(soln.valid == 0, "Solution should be invalid!");
}
END_TEST

START_TEST(test_pvt_repair) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L1CA, .sat = 9};

  navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 1, "Return code should be 1 (pvt repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_used - 1,
              "n_sigs_used should be %u. Saw: %u\n",
              n_used - 1,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_used - 1,
              "n_sats_used should be %u. Saw: %u\n",
              n_used - 1,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_pvt_raim_singular) {
  /* test the case of bug 946 where extreme pseudorange errors lead to singular
   * geometry */
  u8 n_used = 9;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nm1_broken = nm1;
  navigation_measurement_t nm2_broken = nm2;
  nm1_broken.pseudorange += 5e8;
  nm2_broken.pseudorange -= 2e7;

  navigation_measurement_t nms[9] = {
      nm1_broken, nm2_broken, nm3, nm4, nm5, nm6, nm7, nm9, nm10};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == -4, "Return code should be -4 (RAIM failed). Saw: %d\n", code);
}
END_TEST

START_TEST(test_pvt_vel_repair) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L1CA, .sat = 5};

  navigation_measurement_t nms[6] = {nm2, nm3, nm4, nm5, nm6b, nm7};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 1, "Return code should be 1 (pvt repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_used - 1,
              "n_sigs_used should be %u. Saw: %u\n",
              n_used - 1,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_used - 1,
              "n_sats_used should be %u. Saw: %u\n",
              n_used - 1,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_pvt_repair_multifailure) {
  u8 n_used = 7;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L1CA, .sat = 9};

  navigation_measurement_t nms[8] = {nm1, nm2, nm3, nm7, nm10b, nm5, nm6, nm7};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 1, "Return code should be 1 (pvt repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_used - 2,
              "n_sigs_used should be %u. Saw: %u\n",
              n_used - 2,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_used - 2,
              "n_sats_used should be %u. Saw: %u\n",
              n_used - 2,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_pvt_raim_gps_l1ca_only) {
  /* 9 L1CA signals (one broken) and 1 L2CM signal */
  u8 n_used = 10;
  u8 n_gps_l1ca = 9;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L1CA, .sat = 9};

  navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 1, "Return code should be 1 (pvt repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_gps_l1ca - 1,
              "n_sigs_used should be %u. Saw: %u\n",
              n_gps_l1ca - 1,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_gps_l1ca - 1,
              "n_sats_used should be %u. Saw: %u\n",
              n_gps_l1ca - 1,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_pvt_outlier_gps_l1ca_only) {
  /* 9 L1CA signals and 1 (broken) L2CM signal */
  u8 n_used = 9;
  u8 n_gps_l1ca = 8;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L2CM, .sat = 8};

  navigation_measurement_t nms[9] = {
      nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10b};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 1, "Return code should be 1 (pvt repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_gps_l1ca,
              "n_sigs_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_gps_l1ca,
              "n_sats_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_pvt_flag_outlier_bias) {
  /* 8 L1CA signals and 2 L2CM signals */
  u8 n_used = 9;
  u8 n_gps_l1ca = 7;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nm10_bias = nm10;
  navigation_measurement_t nm11_bias = nm11;

  /* add a common bias of 120 m to the L2CM measurements */
  nm10_bias.pseudorange += 120;
  nm11_bias.pseudorange += 120;

  /* healthy measurements, with bias on L2 */
  navigation_measurement_t nms[9] = {
      nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm10_bias, nm11_bias};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(code == 0, "Return code should be 0 (success). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_gps_l1ca,
              "n_sigs_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_gps_l1ca,
              "n_sats_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sats_used);

  /* add outlier to one of the L2 measurements  */
  nm11_bias.pseudorange += 1000;
  nms[8] = nm11_bias;

  code = calc_PVT(n_used,
                  nms,
                  &tor,
                  false,
                  false,
                  GPS_L1CA_WHEN_POSSIBLE,
                  &soln,
                  &dops,
                  &raim_removed_sids);

  gnss_signal_t expected_removed_sid = {.code = CODE_GPS_L2CM, .sat = 8};

  fail_unless(
      code == 1, "Return code should be 1 (RAIM repair). Saw: %d\n", code);
  fail_unless(soln.n_sigs_used == n_gps_l1ca,
              "n_sigs_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == n_gps_l1ca,
              "n_sats_used should be %u. Saw: %u\n",
              n_gps_l1ca,
              soln.n_sats_used);
  fail_unless(sid_set_contains(&raim_removed_sids, expected_removed_sid),
              "Unexpected RAIM removed SID!\n");
}
END_TEST

START_TEST(test_disable_pvt_raim) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     true,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code == 2, "Return code should be 2 (raim not used). Saw: %d\n", code);
  fail_unless(soln.valid == 1, "Solution should be valid!");
}
END_TEST

START_TEST(test_disable_pvt_velocity) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nms[9] = {
      nm1_no_doppler, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code >= 0, "Return code should be >=0 (success). Saw: %d\n", code);
  fail_unless(soln.valid == 1, "Solution should be valid!");
  fail_unless((soln.vel_ned[0] == 0.0) && (soln.vel_ned[1] == 0.0) &&
                  (soln.vel_ned[2] == 0.0),
              "Velocity NED was not zero.  "
              "Saw: %.5f, %.5f, %.5f\n",
              soln.vel_ned[0],
              soln.vel_ned[1],
              soln.vel_ned[2]);
  fail_unless((soln.vel_ecef[0] == 0.0) && (soln.vel_ecef[1] == 0.0) &&
                  (soln.vel_ecef[2] == 0.0),
              "Velocity ECEF was not zero.  "
              "Saw: %.5f, %.5f, %.5f\n",
              soln.vel_ecef[0],
              soln.vel_ecef[1],
              soln.vel_ecef[2]);
}
END_TEST

START_TEST(test_count_sats) {
  u8 n_used = 10;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     false,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code >= 0, "Return code should be >=0 (success). Saw: %d\n", code);
  fail_unless(soln.valid == 1, "Solution should be valid!");
  fail_unless(soln.n_sigs_used == 10,
              "n_sigs_used should be 10. Saw: %u\n",
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == 9,
              "n_sats_used should be 9. Saw: %u\n",
              soln.n_sats_used);
}
END_TEST

START_TEST(test_count_sats_l1ca_only) {
  u8 n_used = 10;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;

  /* 10 signals of which one is GPS L2 and others GPS L1 */
  navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     false,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code >= 0, "Return code should be >=0 (success). Saw: %d\n", code);
  fail_unless(soln.valid == 1, "Solution should be valid!");
  fail_unless(soln.n_sigs_used == 9,
              "n_sigs_used should be 9. Saw: %u\n",
              soln.n_sigs_used);
  fail_unless(soln.n_sats_used == 9,
              "n_sats_used should be 9. Saw: %u\n",
              soln.n_sats_used);
}
END_TEST

START_TEST(test_dops) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops = {.pdop = 22, .gdop = 22, .tdop = 22, .hdop = 22, .vdop = 22};
  dops_t truedops = {.pdop = 2.69955,
                     .gdop = 3.07696,
                     .tdop = 1.47652,
                     .hdop = 1.76157,
                     .vdop = 2.04559};
  gnss_sid_set_t raim_removed_sids;

  const double dop_tol = 1e-3;

  navigation_measurement_t nms[6] = {nm1, nm2, nm3, nm4, nm5, nm6};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  fail_unless(
      code >= 0, "Return code should be >=0 (success). Saw: %d\n", code);
  fail_unless(soln.valid == 1, "Solution should be valid!");
  fail_unless(fabs(dops.pdop * dops.pdop -
                   (dops.vdop * dops.vdop + dops.hdop * dops.hdop)) < dop_tol,
              "HDOP^2 + VDOP^2 != PDOP^2.  Saw: %.5f, %.5f, %.5f, %.5f, %.5f\n",
              dops.pdop,
              dops.gdop,
              dops.tdop,
              dops.hdop,
              dops.vdop);
  double dop_err =
      fabs(dops.pdop - truedops.pdop) + fabs(dops.gdop - truedops.gdop) +
      fabs(dops.tdop - truedops.tdop) + fabs(dops.hdop - truedops.hdop) +
      fabs(dops.vdop - truedops.vdop);
  fail_unless(dop_err < dop_tol,
              "DOPs don't match hardcoded correct values.  "
              "Saw: %.5f, %.5f, %.5f, %.5f, %.5f\n",
              dops.pdop,
              dops.gdop,
              dops.tdop,
              dops.hdop,
              dops.vdop);
}
END_TEST

Suite *pvt_test_suite(void) {
  Suite *s = suite_create("PVT Solver");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_pvt_repair);
  tcase_add_test(tc_core, test_pvt_vel_repair);
  tcase_add_test(tc_core, test_pvt_repair_multifailure);
  tcase_add_test(tc_core, test_pvt_raim_gps_l1ca_only);
  tcase_add_test(tc_core, test_pvt_outlier_gps_l1ca_only);
  tcase_add_test(tc_core, test_pvt_flag_outlier_bias);
  tcase_add_test(tc_core, test_pvt_failed_repair);
  tcase_add_test(tc_core, test_pvt_raim_singular);
  tcase_add_test(tc_core, test_disable_pvt_raim);
  tcase_add_test(tc_core, test_disable_pvt_velocity);
  tcase_add_test(tc_core, test_count_sats);
  tcase_add_test(tc_core, test_count_sats_l1ca_only);
  tcase_add_test(tc_core, test_dops);
  suite_add_tcase(s, tc_core);

  return s;
}
