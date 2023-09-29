
#include <check.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/troposphere.h>

#include "check_suites.h"

START_TEST(test_calc_troposphere) {
  const double d_tol = 1e-4;

  /* some tests against "true" values computed with UNB3M.f */
  /* http://www2.unb.ca/gge/Personnel/Santos/UNB_pack.pdf */

  double lat = 40 * D2R;
  double h = 1300.0;
  double doy = 32.5;
  double el = 45 * D2R;
  double d_true = 2.8567;

  double d_tropo = calc_troposphere(doy, lat, h, el);

  fail_unless(
      fabs(d_tropo - d_true) < d_tol,
      "Distance didn't match hardcoded correct values %0.5f. Saw: %.5f\n",
      d_true,
      d_tropo);

  lat = -10 * D2R;
  h = 0.0;
  doy = 180.5;
  el = 20 * D2R;
  d_true = 7.4942;

  d_tropo = calc_troposphere(doy, lat, h, el);

  fail_unless(
      fabs(d_tropo - d_true) < d_tol,
      "Distance didn't match hardcoded correct values %0.5f. Saw: %.5f\n",
      d_true,
      d_tropo);

  lat = 75 * D2R;
  h = 0.0;
  doy = 50.5;
  el = 10 * D2R;
  d_true = 12.90073;

  d_tropo = calc_troposphere(doy, lat, h, el);

  fail_unless(
      fabs(d_tropo - d_true) < d_tol,
      "Distance didn't match hardcoded correct values %0.5f. Saw: %.5f\n",
      d_true,
      d_tropo);

  /* altitude sanity tests */
  double max_tropo_correction = 30.0;
  h = -5000;
  d_tropo = calc_troposphere(doy, lat, h, el);

  fail_unless(fabs(d_tropo) < max_tropo_correction,
              "Sanity test fail at altitude %0.5f. : Correction was %.5f\n",
              h,
              d_tropo);

  h = 12000;
  d_tropo = calc_troposphere(doy, lat, h, el);

  fail_unless(fabs(d_tropo) < max_tropo_correction,
              "Sanity test fail at altitude %0.5f. : Correction was %.5f\n",
              h,
              d_tropo);

  /* satellite elevation sanity tests */
  h = 100;
  double elevation_testcases[] = {1e-3, 1e-4, 1e-5, 0, -1e3, -0.1};
  max_tropo_correction = 100.0;

  for (u8 i = 0; i < sizeof(elevation_testcases) / sizeof(double); i++) {
    el = elevation_testcases[i];
    d_tropo = calc_troposphere(doy, lat, h, el);
    fail_unless(fabs(d_tropo) < max_tropo_correction,
                "Sanity test fail at satellite elevation %0.5f. : Correction "
                "was %.5f\n",
                el,
                d_tropo);
  }
}
END_TEST

Suite *troposphere_suite(void) {
  Suite *s = suite_create("Troposphere");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_calc_troposphere);
  suite_add_tcase(s, tc_core);

  return s;
}
