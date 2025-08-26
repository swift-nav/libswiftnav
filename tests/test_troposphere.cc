
#include <gtest/gtest.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/troposphere.h>

namespace {

TEST(TestTroposphere, CalcTroposphere) {
  const double d_tol = 1e-4;

  /* some tests against "true" values computed with UNB3M.f */
  /* http://www2.unb.ca/gge/Personnel/Santos/UNB_pack.pdf */

  double lat = 40 * D2R;
  double h = 1300.0;
  double doy = 32.5;
  double el = 45 * D2R;
  double d_true = 2.8567;

  double d_tropo = calc_troposphere(doy, lat, h, el);

  EXPECT_TRUE(fabs(d_tropo - d_true) < d_tol)
      << "Distance didn't match hardcoded correct values " << d_true
      << ". Saw: " << d_tropo;

  lat = -10 * D2R;
  h = 0.0;
  doy = 180.5;
  el = 20 * D2R;
  d_true = 7.4942;

  d_tropo = calc_troposphere(doy, lat, h, el);

  EXPECT_TRUE(fabs(d_tropo - d_true) < d_tol)
      << "Distance didn't match hardcoded correct values " << d_true
      << ". Saw: " << d_tropo;

  lat = 75 * D2R;
  h = 0.0;
  doy = 50.5;
  el = 10 * D2R;
  d_true = 12.90073;

  d_tropo = calc_troposphere(doy, lat, h, el);

  EXPECT_TRUE(fabs(d_tropo - d_true) < d_tol)
      << "Distance didn't match hardcoded correct values " << d_true
      << ". Saw: " << d_tropo;

  /* altitude sanity tests */
  double max_tropo_correction = 30.0;
  h = -5000;
  d_tropo = calc_troposphere(doy, lat, h, el);

  EXPECT_TRUE(fabs(d_tropo) < max_tropo_correction)
      << "Sanity test fail at altitude " << h << ". : Correction was "
      << d_tropo;

  h = 12000;
  d_tropo = calc_troposphere(doy, lat, h, el);

  EXPECT_TRUE(fabs(d_tropo) < max_tropo_correction)
      << "Sanity test fail at altitude " << h << ". : Correction was "
      << d_tropo;

  /* satellite elevation sanity tests */
  h = 100;
  double elevation_testcases[] = {1e-3, 1e-4, 1e-5, 0, -1e3, -0.1};
  max_tropo_correction = 100.0;

  for (double elevation_testcase : elevation_testcases) {
    el = elevation_testcase;
    d_tropo = calc_troposphere(doy, lat, h, el);
    EXPECT_LT(fabs(d_tropo), max_tropo_correction)
        << "Sanity test fail at satellite elevation  " << el
        << ". : Correction "
           "was "
        << d_tropo;
  }
}

}  // namespace
