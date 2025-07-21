#include <gtest/gtest.h>
#include <swiftnav/correct_iono_tropo.h>
#include <swiftnav/single_epoch_solver.h>

#include "test_data.h"

namespace {

using namespace test_data;

static void run_calc_pvt(const u8 n_used,
                         const navigation_measurement_t nms[9],
                         gnss_solution *soln) {
  dops_t dops;
  obs_mask_config_t obs_mask_config = {{false, 25}};

  s8 code = calc_PVT(n_used,
                     nms,
                     &tor,
                     true,
                     true,
                     &obs_mask_config,
                     ALL_CONSTELLATIONS,
                     soln,
                     &dops,
                     nullptr);
  EXPECT_EQ(code, 2) << "Return code should be 2 (raim not used). Saw: "
                     << code;
  EXPECT_EQ(soln->valid, 1) << "Solution should be valid!";
}

TEST(TestCorrectIonoTropo, TestCorrectIono) {
  u8 n_used = 9;
  gnss_solution soln1;
  gnss_solution soln2;

  /* compute PVT with no iono correction */

  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  run_calc_pvt(n_used, nms, &soln1);

  EXPECT_NEAR(soln1.pos_ecef[0], -2715898.028, 0.001);
  EXPECT_NEAR(soln1.pos_ecef[1], -4266139.598, 0.001);
  EXPECT_NEAR(soln1.pos_ecef[2], 3891352.859, 0.001);

  /* compute PVT with iono correction */

  navigation_measurement_t nms_iono[9];
  memcpy(nms_iono, nms, sizeof(nms));
  correct_iono(soln1.pos_ecef, &DEFAULT_IONO_PARAMS, n_used, nms_iono);
  run_calc_pvt(n_used, nms_iono, &soln2);

  EXPECT_NEAR(soln2.pos_ecef[0], -2715896.609, 0.001);
  EXPECT_NEAR(soln2.pos_ecef[1], -4266137.564, 0.001);
  EXPECT_NEAR(soln2.pos_ecef[2], 3891350.932, 0.001);

  /* check position delta */

  double delta_x = soln1.pos_ecef[0] - soln2.pos_ecef[0];
  double delta_y = soln1.pos_ecef[1] - soln2.pos_ecef[1];
  double delta_z = soln1.pos_ecef[2] - soln2.pos_ecef[2];

  double pos_shift =
      sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
  EXPECT_NEAR(pos_shift, 3.141, 0.001);

  /* check PR/CP deltas */

  static const double expected_pr_deltas[9] = {-1.797691397369,
                                               -1.521543189883,
                                               -2.111414518207,
                                               -2.330238319933,
                                               -3.942552670836,
                                               -1.669149085879,
                                               -4.086640629917,
                                               -3.286862179637,
                                               -2.850360091776};

  for (int i = 0; i < n_used; i++) {
    double pr_delta = nms_iono[i].raw_pseudorange - nms[i].raw_pseudorange;
    EXPECT_NEAR(pr_delta, expected_pr_deltas[i], 0.001);

    double cp_delta = nms_iono[i].raw_carrier_phase - nms[i].raw_carrier_phase;
    EXPECT_NEAR(cp_delta,
                expected_pr_deltas[i] * (sid_to_carr_freq(nms[i].sid) / GPS_C),
                0.001);
  }
}

TEST(TestCorrectIonoTropo, TestCorrectTropo) {
  u8 n_used = 9;
  gnss_solution soln1;
  gnss_solution soln2;

  /* compute PVT with no tropo correction */

  const navigation_measurement_t nms[9] = {
      nm1, nm2, nm3, nm4, nm5, nm6, nm7, nm8, nm9};

  run_calc_pvt(n_used, nms, &soln1);

  EXPECT_NEAR(soln1.pos_ecef[0], -2715898.028, 0.001);
  EXPECT_NEAR(soln1.pos_ecef[1], -4266139.598, 0.001);
  EXPECT_NEAR(soln1.pos_ecef[2], 3891352.859, 0.001);

  /* compute PVT with tropo correction */

  navigation_measurement_t nms_tropo[9];
  memcpy(nms_tropo, nms, sizeof(nms));

  correct_tropo(soln1.pos_ecef, n_used, nms_tropo);
  run_calc_pvt(n_used, nms_tropo, &soln2);

  EXPECT_NEAR(soln2.pos_ecef[0], -2715896.697, 0.001);
  EXPECT_NEAR(soln2.pos_ecef[1], -4266137.874, 0.001);
  EXPECT_NEAR(soln2.pos_ecef[2], 3891351.398, 0.001);

  /* check position delta */

  double delta_x = soln1.pos_ecef[0] - soln2.pos_ecef[0];
  double delta_y = soln1.pos_ecef[1] - soln2.pos_ecef[1];
  double delta_z = soln1.pos_ecef[2] - soln2.pos_ecef[2];

  double pos_shift =
      sqrt(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
  EXPECT_NEAR(pos_shift, 2.623, 0.001);

  /* check PR/CP deltas */

  static const double expected_pr_deltas[9] = {-0.643832463771,
                                               -0.531037796289,
                                               -0.768872588873,
                                               -0.866686545312,
                                               -2.582971405238,
                                               -0.594299942255,
                                               -2.985528819263,
                                               -1.532460253686,
                                               -1.163043931127};

  for (int i = 0; i < n_used; i++) {
    double pr_delta = nms_tropo[i].raw_pseudorange - nms[i].raw_pseudorange;
    EXPECT_NEAR(pr_delta, expected_pr_deltas[i], 0.001);

    double cp_delta = nms_tropo[i].raw_carrier_phase - nms[i].raw_carrier_phase;
    EXPECT_NEAR(cp_delta,
                -expected_pr_deltas[i] * (sid_to_carr_freq(nms[i].sid) / GPS_C),
                0.001);
  }
}

}  // namespace
