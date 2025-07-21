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

#include <gtest/gtest.h>
#include <stdio.h>
#include <swiftnav/constants.h>
#include <swiftnav/ionosphere.h>

namespace {

TEST(TestIonosphere, CalcIonosphere) {
  gps_time_t t = {.tow = 479820, .wn = 1875};
  ionosphere_t i = {.a0 = 0.1583e-7,
                    .a1 = -0.7451e-8,
                    .a2 = -0.5960e-7,
                    .a3 = 0.1192e-6,
                    .b0 = 0.1290e6,
                    .b1 = -0.2130e6,
                    .b2 = 0.6554e5,
                    .b3 = 0.3277e6};
  double lat_u = -35.3 * D2R, lon_u = 149.1 * D2R;
  double a = 0.0 * D2R, e = 15.0 * D2R;
  double d_true = 7.202;

  const double d_tol = 1e-3;

  double d_l1 = calc_ionosphere(&t, lat_u, lon_u, a, e, &i);
  double d_err = fabs(d_l1 - d_true);

  EXPECT_LT(d_err, d_tol);

  t.wn = 1042;
  t.tow = 593100;
  i.a0 = 0.3820e-7;
  i.a1 = 0.1490e-7;
  i.a2 = -0.1790e-6;
  i.a3 = 0.0;
  i.b0 = 0.1430e6;
  i.b1 = 0.0;
  i.b2 = -0.3280e6;
  i.b3 = 0.1130e6;
  lat_u = 40.0 * D2R;
  lon_u = 260.0 * D2R;
  a = 210.0 * D2R;
  e = 20.0 * D2R;
  d_true = 23.784;

  d_l1 = calc_ionosphere(&t, lat_u, lon_u, a, e, &i);
  d_err = fabs(d_l1 - d_true);

  EXPECT_LT(d_err, d_tol);

  t.wn = 1042;
  t.tow = 345600;
  i.a0 = 1.304e-8;
  i.a1 = 0;
  i.a2 = -5.96e-8;
  i.a3 = 5.96e-8;
  i.b0 = 1.106e5;
  i.b1 = -65540.0;
  i.b2 = -2.621e5;
  i.b3 = 3.932e5;
  lat_u = 0.70605;
  lon_u = -0.076233;
  a = 2.62049;
  e = 0.2939;
  d_true = 3.4929;

  d_l1 = calc_ionosphere(&t, lat_u, lon_u, a, e, &i);
  d_err = fabs(d_l1 - d_true);

  EXPECT_LT(d_err, d_tol);
}

TEST(TestIonosphere, DecodeIonoParameters) {
#define tol 1e-12
  struct {
    u32 frame_words[8];
    ionosphere_t result;
  } t_case = {.frame_words =
                  {/* 4th SF real data at 11-May-2016 */
                   0x1e0300c9,
                   0x7fff8c24,
                   0x23fbdc2,
                   0,
                   0,
                   0,
                   0,
                   0},
              .result = {
                  /* reference data provided by u-blox receiver */
                  .a0 = 0.0000000111758,
                  .a1 = 0.0000000223517,
                  .a2 = -0.0000000596046,
                  .a3 = -0.0000001192092,
                  .b0 = 98304.0,
                  .b1 = 131072.0,
                  .b2 = -131072.0,
                  .b3 = -589824.0,
              }};
  ionosphere_t i;
  decode_iono_parameters(t_case.frame_words, &i);
  EXPECT_NEAR(i.a0, t_case.result.a0, tol);
  EXPECT_NEAR(i.a1, t_case.result.a1, tol);
  EXPECT_NEAR(i.a2, t_case.result.a2, tol);
  EXPECT_NEAR(i.a3, t_case.result.a3, tol);
  EXPECT_NEAR(i.b0, t_case.result.b0, tol);
  EXPECT_NEAR(i.b1, t_case.result.b1, tol);
  EXPECT_NEAR(i.b2, t_case.result.b2, tol);
  EXPECT_NEAR(i.b3, t_case.result.b3, tol);
}

}  // namespace
