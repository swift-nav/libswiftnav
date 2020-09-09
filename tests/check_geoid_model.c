/*------------------------------------------------------------------------------
 * Reproduced from https://github.com/swift-nav/RTKLIB/blob/master/src/geoid.c
 *according to the RTKLib licence
 *
 *          Copyright (C) 2007-2013 by T.TAKASU, All rights reserved.
 *-----------------------------------------------------------------------------*/

#include <check.h>

#include <swiftnav/constants.h>
#include <swiftnav/geoid_model.h>

#include "check_suites.h"

static const double range[4]; /* embedded geoid area range {W,E,S,N} (deg) */

static const double range[] = {0.00, 360.00, -90.00, 90.00};

/* bilinear interpolation ----------------------------------------------------*/
static double interpb(const double *y, double a, double b) {
  return y[0] * (1.0 - a) * (1.0 - b) + y[1] * a * (1.0 - b) +
         y[2] * (1.0 - a) * b + y[3] * a * b;
}
/* embedded geoid model ------------------------------------------------------*/
static double geoidh_emb(const double *pos) {
  const double dlon = 1.0, dlat = 1.0;
  double a, b, y[4];
  int i1, i2, j1, j2;

  if (pos[1] < range[0] || range[1] < pos[1] || pos[0] < range[2] ||
      range[3] < pos[0]) {
    return 0.0;
  }
  a = (pos[1] - range[0]) / dlon;
  b = (pos[0] - range[2]) / dlat;
  i1 = (int)a;
  a -= i1;
  i2 = i1 < 360 ? i1 + 1 : i1;
  j1 = (int)b;
  b -= j1;
  j2 = j1 < 180 ? j1 + 1 : j1;
  y[0] = GEOID[i1][j1];
  y[1] = GEOID[i2][j1];
  y[2] = GEOID[i1][j2];
  y[3] = GEOID[i2][j2];
  return interpb(y, a, b);
}

START_TEST(test_geoid_model) {
  for (int lat = 0; lat < 314; ++lat) {
    for (int lon = 0; lon < 628; ++lon) {
      double rtk_lib_pos[2] = {R2D * (lat / 100 - M_PI / 2),
                               R2D * (float)(lon / 100)};
      double rtk_lib = geoidh_emb(rtk_lib_pos);

      double pos[2] = {lat / 100 - M_PI / 2, lon / 100 - M_PI};
      float geoid_offset = get_geoid_offset(pos[0], pos[1]);

      fail_unless(fabs(geoid_offset - rtk_lib) < 1e-4,
                  "Sanity test fail at lat, lon %0.5f, %0.5f. : difference "
                  "was %.5f\n",
                  pos[0],
                  pos[1],
                  fabs(geoid_offset - rtk_lib));
    }
  }
}
END_TEST

Suite *geoid_model_suite_suite(void) {
  Suite *s = suite_create("Geoid_model");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_geoid_model);
  suite_add_tcase(s, tc_core);

  return s;
}
