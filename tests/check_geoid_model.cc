/*
 * Geoid model tests
 *
 * Contains code from
 * https://github.com/swift-nav/RTKLIB/blob/master/src/geoid.c in accordance
 * with the terms of the RTKLib licence
 */

#include <check.h>
#include <swiftnav/constants.h>
#include <swiftnav/geoid_model.h>

#ifdef __cplusplus
extern "C" {
#endif

/* embedded geoid area range {W,E,S,N} (deg) */
static const double range[4] = {0.0, 360.0, -90.0, 90.0};

/* bilinear interpolation ----------------------------------------------------*/
static double interpb(const double* y, double a, double b) {
  return y[0] * (1.0 - a) * (1.0 - b) + y[1] * a * (1.0 - b) +
         y[2] * (1.0 - a) * b + y[3] * a * b;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* lookup function for embedded geoid model */
template <typename T>
float geoidh_emb(const double& lat,
                 const double& lon,
                 const T& geoid,
                 const double& dlat,
                 const double& dlon) {
  double a, b, y[4];
  int i1, i2, j1, j2;

  if (lon < range[0] || range[1] < lon || lat < range[2] || range[3] < lat) {
    return 0.0;
  }
  a = (lon - range[0]) / dlon;
  b = (lat - range[2]) / dlat;
  i1 = (int)a;
  a -= i1;
  i2 = i1 < 360 / dlon ? i1 + 1 : i1;
  j1 = (int)b;
  b -= j1;
  j2 = j1 < 180 / dlat ? j1 + 1 : j1;
  y[0] = geoid[i1][j1];
  y[1] = geoid[i2][j1];
  y[2] = geoid[i1][j2];
  y[3] = geoid[i2][j2];
  return interpb(y, a, b);
}

// directly include the 1 degree geoid for testing purposes
namespace geoid_1_degree {
#include "src/geoid_model_1_degree.inc"
inline float geoidh_emb(const double& lat, const double& lon) {
  return ::geoidh_emb(
      lat, lon, GEOID, LAT_GRID_SPACING_DEG, LON_GRID_SPACING_DEG);
}
}  // namespace geoid_1_degree

// directly include the 0.25 degree geoid for testing purposes
namespace geoid_15_minute {
#include "src/geoid_model_15_minute.inc"
inline float geoidh_emb(const double& lat, const double& lon) {
  return ::geoidh_emb(
      lat, lon, GEOID, LAT_GRID_SPACING_DEG, LON_GRID_SPACING_DEG);
}
}  // namespace geoid_15_minute

#include <swiftnav/logging.h>
#undef log_error
#define log_error(...)

// directly include our implementation with 1 degree geoid for testing purposes
namespace src_geoid_model_1_degree {
#undef GEOID_MODEL_15_MINUTE_RESOLUTION
#include "src/geoid_model.c"
}  // namespace src_geoid_model_1_degree

// directly include our implementation with 0.25 degree geoid for testing
// purposes
namespace src_geoid_model_15_minute {
#define GEOID_MODEL_15_MINUTE_RESOLUTION
#include "src/geoid_model.c"
}  // namespace src_geoid_model_15_minute

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Perform direct lookups on GEOID to compare height values from the 1 degree
 * resolution geoid vs heights from the 15 minute resolution geoid for an
 * exact match (for all points which exist in the 1 degree grid)
 */
START_TEST(compare_geoid_models_matching_points) {
  // 360 degrees of longitude (1 repeated)
  for (int lon = 0; lon < 361; lon++) {
    // 180 degrees of latitude (1 repeated)
    for (int lat = 0; lat < 181; lat++) {
      float val_1_degree = geoid_1_degree::GEOID[lon][lat];
      float val_15_minute = geoid_15_minute::GEOID[lon * 4][lat * 4];

      fail_unless(fabs(val_1_degree - val_15_minute) < 1e-4,
                  "Mismatch between 1 degree resolution and 0.25 degree "
                  "resolution geoids: "
                  "%f vs %f\n",
                  val_1_degree,
                  val_15_minute);
    }
  }
}
END_TEST

/*
 * Use geoidh_emb() to compare heights from the 1 degree resolution geoid vs
 * heights from the 15 minute resolution geoid for every 10th of a degree
 * (with bilinear interpolation).  Maximum expected difference is 15.2m (in
 * Hawaii).
 */
START_TEST(compare_geoid_models_tenth_degree) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;
      float val_1_degree = geoid_1_degree::geoidh_emb(lat_deg, lon_deg);
      float val_15_minute = geoid_15_minute::geoidh_emb(lat_deg, lon_deg);
      fail_unless(fabs(val_1_degree - val_15_minute) < 15.5,
                  "Mismatch between 1 degree resolution and 0.25 degree "
                  "resolution geoids "
                  "at lat %g, lon %g: %f vs %f, delta %f\n",
                  lat_deg,
                  lon_deg,
                  val_1_degree,
                  val_15_minute,
                  fabs(val_1_degree - val_15_minute));
    }
  }
}
END_TEST

/*
 * Compare heights from the 1 degree geoid (with bilinear interpolation) vs
 * heights from the 1 degree geoid (with bicubic interpolation) for every 10th
 * of a degree.  Maximum offset should be 3.69m (in Indonesia).
 */
START_TEST(compare_1_degree_bilinear_vs_bicubic) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;

      float bilinear = geoid_1_degree::geoidh_emb(lat_deg, lon_deg);
      float bicubic = src_geoid_model_1_degree::get_geoid_offset(lat_deg * D2R,
                                                                 lon_deg * D2R);
      fail_unless(fabs(bilinear - bicubic) < 3.69,
                  "Mismatch between bilinear and bicubic interpolation for 1 "
                  "degree geoid "
                  "at lat %g, lon %g: %f vs %f, delta %f\n",
                  lat_deg,
                  lon_deg,
                  bilinear,
                  bicubic,
                  fabs(bilinear - bicubic));
    }
  }
}
END_TEST

/*
 * Compare heights from the 15 minute geoid (with bilinear interpolation) vs
 * heights from the 15 degree geoid (with bicubic interpolation) for every 10th
 * of a degree.  Maximum offset should be 1.33m (in Columbia)
 */
START_TEST(compare_15_minute_bilinear_vs_bicubic) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;
      float bilinear = geoid_15_minute::geoidh_emb(lat_deg, lon_deg);
      float bicubic = src_geoid_model_15_minute::get_geoid_offset(
          lat_deg * D2R, lon_deg * D2R);
      fail_unless(fabs(bilinear - bicubic) < 1.33,
                  "Mismatch between bilinear and bicubic interpolation for 15 "
                  "minute grid "
                  "at lat %g, lon %g: %f vs %f, delta %f\n",
                  lat_deg,
                  lon_deg,
                  bilinear,
                  bicubic,
                  fabs(bilinear - bicubic));
    }
  }
}
END_TEST

Suite* geoid_model_test_suite(void) {
  Suite* s = suite_create("Geoid model");

  TCase* tc_core = tcase_create("Core");
  tcase_add_test(tc_core, compare_geoid_models_matching_points);
  tcase_add_test(tc_core, compare_geoid_models_tenth_degree);
  tcase_add_test(tc_core, compare_1_degree_bilinear_vs_bicubic);
  tcase_add_test(tc_core, compare_15_minute_bilinear_vs_bicubic);
  suite_add_tcase(s, tc_core);

  return s;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
