#include <gtest/gtest.h>
#include <math.h>
#include <stdio.h>
#include <swiftnav/constants.h>
#include <swiftnav/coord_system.h>
#include <swiftnav/single_epoch_solver.h>

#include "check_utils.h"
#include "test_data.h"

namespace {

using namespace test_data;

TEST(TestPvt, PvtFailedRepair) {
  u8 n_used = 5;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8};

  calc_PVT(n_used,
           nms,
           &tor,
           false,
           true,
           &obs_mask_config,
           ALL_CONSTELLATIONS,
           &soln,
           &dops,
           &raim_removed_sids);
  /* PVT repair requires at least 6 measurements. */
  EXPECT_EQ(soln.valid, 0);
}

TEST(TestPvt, PvtRepair) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};
  gnss_signal_t expected_removed_sid = {.sat = 9, .code = CODE_GPS_L1CA};

  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 1);
  EXPECT_EQ(soln.n_sigs_used, n_used - 1);
  EXPECT_EQ(soln.n_sats_used, n_used - 1);
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid))
      << "Unexpected RAIM removed SID!\n";
}

TEST(TestPvt, PvtRaimSingular) {
  /* test the case of bug 946 where extreme pseudorange errors lead to singular
   * geometry */
  u8 n_used = 9;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  navigation_measurement_t nm1_broken = nm1;
  navigation_measurement_t nm2_broken = nm2;
  nm1_broken.raw_pseudorange += 5e8;
  nm2_broken.raw_pseudorange -= 2e7;

  const navigation_measurement_t nms[9] = {
      nm1_broken, nm2_broken, nm3, nm4, nm5, nm6, nm7, nm9, nm10};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, -4);
}

TEST(TestPvt, PvtVelRepair) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};
  gnss_signal_t expected_removed_sid = {.sat = 5, .code = CODE_GPS_L1CA};

  const navigation_measurement_t nms[6] = {nm2, nm3, nm4, nm5, nm6b, nm7};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 1);
  EXPECT_EQ(soln.n_sigs_used, n_used - 1);
  EXPECT_EQ(soln.n_sats_used, n_used - 1);
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid))
      << "Unexpected RAIM removed SID!\n";
}

TEST(TestPvt, PvtRepairMultifailure) {
  u8 n_used = 7;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};
  gnss_signal_t expected_removed_sid = {.sat = 9, .code = CODE_GPS_L1CA};

  const navigation_measurement_t nms[8] = {
      nm1, nm2, nm3, nm7, nm10b, nm5, nm6, nm7};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 1);
  EXPECT_EQ(soln.n_sigs_used, n_used - 2);
  EXPECT_EQ(soln.n_sats_used, n_used - 2);
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid))
      << "Unexpected RAIM removed SID!\n";
}

TEST(TestPvt, PvtRaimGpsL1caOnly) {
  /* 9 L1CA signals (one broken) and 1 L2CM signal */
  u8 n_used = 10;
  u8 n_gps_l1ca = 9;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{false, 25}};
  gnss_signal_t expected_removed_sid = {.sat = 9, .code = CODE_GPS_L1CA};

  const navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 1);
  EXPECT_EQ(soln.n_sigs_used, n_gps_l1ca - 1);
  EXPECT_EQ(soln.n_sats_used, n_gps_l1ca - 1);
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid))
      << "Unexpected RAIM removed SID!\n";
}

TEST(TestPvt, PvtOutlierGpsL1caOnly) {
  /* 9 L1CA signals and 1 (broken) L2CM signal */
  u8 n_used = 9;
  u8 n_gps_l1ca = 8;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  const navigation_measurement_t nms[9] = {
      nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10b};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);

  EXPECT_EQ(code, 0);
  EXPECT_EQ(soln.n_sigs_used, n_gps_l1ca);
  EXPECT_EQ(soln.n_sats_used, n_gps_l1ca);
}

// Regression test for PIKSI-191
TEST(TestPvt, CalcPvtExcludeGal) {
  u8 n_used = 8;
  u8 n_gps_l1ca = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  // Mixing GPS and GAL satellites
  navigation_measurement_t nms[9] = {
      nm3, gal_nm1, gal_nm2, nm5, nm6, nm7, nm8, nm9};

  // Now using predicate GPS_ONLY would trigger an assert in outlier detection
  // which is called from within calc_PVT()
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     GPS_ONLY,
                     &soln,
                     &dops,
                     &raim_removed_sids);

  EXPECT_EQ(code, 0);
  EXPECT_EQ(soln.n_sigs_used, n_gps_l1ca);
  EXPECT_EQ(soln.n_sats_used, n_gps_l1ca);
}

TEST(TestPvt, PvtFlagOutlierBias) {
  /* 8 L1CA signals and 2 L2CM signals */
  u8 n_used = 9;
  u8 n_gps_l1ca = 7;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  navigation_measurement_t nm10_bias = nm10;
  navigation_measurement_t nm11_bias = nm11;

  /* add a common bias of 120 m to the L2CM measurements */
  nm10_bias.raw_pseudorange += 120;
  nm11_bias.raw_pseudorange += 120;

  /* healthy measurements, with bias on L2 */
  navigation_measurement_t nms[9] = {
      nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm10_bias, nm11_bias};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     false,
                     &obs_mask_config,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 0);
  EXPECT_EQ(soln.n_sigs_used, n_gps_l1ca);
  EXPECT_EQ(soln.n_sats_used, n_gps_l1ca);

  /* add outlier to one of the L2 measurements  */
  nm11_bias.raw_pseudorange += 1000;
  nms[8] = nm11_bias;

  code = calc_PVT(n_used,
                  nms,
                  &tor,
                  false,
                  false,
                  &obs_mask_config,
                  GPS_L1CA_WHEN_POSSIBLE,
                  &soln,
                  &dops,
                  &raim_removed_sids);

  EXPECT_EQ(code, 0);
  EXPECT_EQ(soln.n_sigs_used, n_gps_l1ca);
  EXPECT_EQ(soln.n_sats_used, n_gps_l1ca);
}

TEST(TestPvt, DisablePvtRaim) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{false, 25}};

  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_EQ(code, 2);
  EXPECT_EQ(soln.valid, 1) << "Solution should be valid!";
}

TEST(TestPvt, DisablePvtVelocity) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  const navigation_measurement_t nms[9] = {
      nm1_no_doppler, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_GE(code, 0);
  EXPECT_EQ(soln.valid, 1);
  EXPECT_EQ(soln.vel_ned[0], 0.0);
  EXPECT_EQ(soln.vel_ned[1], 0.0);
  EXPECT_EQ(soln.vel_ned[2], 0.0);
  EXPECT_EQ(soln.vel_ecef[0], 0.0);
  EXPECT_EQ(soln.vel_ecef[1], 0.0);
  EXPECT_EQ(soln.vel_ecef[2], 0.0);
}

TEST(TestPvt, CountSats) {
  u8 n_used = 10;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{false, 25}};

  const navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     false,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_GE(code, 0);
  EXPECT_EQ(soln.valid, 1);
  EXPECT_EQ(soln.n_sigs_used, 10);
  EXPECT_EQ(soln.n_sats_used, 9);
}

TEST(TestPvt, CountSatsL1caOnly) {
  u8 n_used = 10;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  /* 10 signals of which one is GPS L2 and others GPS L1 */
  const navigation_measurement_t nms[10] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9, nm10};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     false,
                     &obs_mask_config,
                     GPS_L1CA_WHEN_POSSIBLE,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_GE(code, 0);
  EXPECT_EQ(soln.valid, 1);
  EXPECT_EQ(soln.n_sigs_used, 9);
  EXPECT_EQ(soln.n_sats_used, 9);
}

TEST(TestPvt, Dops) {
  u8 n_used = 6;
  gnss_solution soln;
  dops_t dops = {.pdop = 22, .gdop = 22, .tdop = 22, .hdop = 22, .vdop = 22};
  dops_t truedops = {.pdop = 2.69955,
                     .gdop = 3.07696,
                     .tdop = 1.47652,
                     .hdop = 1.76157,
                     .vdop = 2.04559};
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{false, 25}};

  const double dop_tol = 1e-3;

  const navigation_measurement_t nms[6] = {nm1, nm2, nm3, nm4, nm5, nm6};

  /* disable raim check */
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);
  EXPECT_GE(code, 0);
  EXPECT_EQ(soln.valid, 1);
  EXPECT_TRUE(fabs(dops.pdop * dops.pdop -
                   (dops.vdop * dops.vdop + dops.hdop * dops.hdop)) < dop_tol);
  double dop_err =
      fabs(dops.pdop - truedops.pdop) + fabs(dops.gdop - truedops.gdop) +
      fabs(dops.tdop - truedops.tdop) + fabs(dops.hdop - truedops.hdop) +
      fabs(dops.vdop - truedops.vdop);
  EXPECT_LT(dop_err, dop_tol);
}

TEST(TestPvt, PvtSuccessfulRepairWithCn0Mask) {
  /* Emulate TES-238 scenario */

  u8 n_used = 9;
  gnss_solution soln;
  dops_t dops;
  gnss_sid_set_t raim_removed_sids;
  obs_mask_config_t obs_mask_config = {{true, 25}};

  // Given erroneous pseudorange measurement with C/N0 above mask threshold (sat
  // 2)
  navigation_measurement_t nm3_bias = nm3;
  nm3_bias.raw_pseudorange += 4350;

  // Given erroneous measurement with C/N0 below mask threshold and pseudorange
  // that if processed, does not allow for successful RAIM repair
  navigation_measurement_t nm4_bias_low_cn0 = nm4;
  nm4_bias_low_cn0.raw_pseudorange += 4134;
  nm4_bias_low_cn0.cn0 = 21;

  // When calc_PVT is attempted with measurements defined above and C/N0
  // observation masking enabled
  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3_bias, nm4_bias_low_cn0, nm5, nm6, nm7, nm8, nm9};
  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     false,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     &soln,
                     &dops,
                     &raim_removed_sids);

  // Expect successful RAIM repair
  gnss_signal_t expected_removed_sid_1 = {.sat = 9, .code = CODE_GPS_L1CA};
  gnss_signal_t expected_removed_sid_2 = {.sat = 2, .code = CODE_GPS_L1CA};
  EXPECT_EQ(code, 1);
  EXPECT_EQ(soln.n_sigs_used, n_used - 3);
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid_1))
      << "Unexpected RAIM removed SID!\n";
  EXPECT_TRUE(sid_set_contains(&raim_removed_sids, expected_removed_sid_2))
      << "Unexpected RAIM removed SID!\n";
}

}  // namespace
