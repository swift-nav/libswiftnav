/*
 * Geoid model tests
 *
 * Contains code from
 * https://github.com/swift-nav/RTKLIB/blob/master/src/geoid.c in accordance
 * with the terms of the RTKLib licence
 */

#include <gtest/gtest.h>
#include <swiftnav/constants.h>
#include <swiftnav/geoid_model.h>

#include <cmath>

#include "../src/geoid_model.h"

namespace {

/* embedded geoid area range {W,E,S,N} (deg) */
const double range[4] = {0.0, 360.0, -90.0, 90.0};

/* bilinear interpolation ----------------------------------------------------*/
double interpb(const double* y, double a, double b) {
  return y[0] * (1.0 - a) * (1.0 - b) + y[1] * a * (1.0 - b) +
         y[2] * (1.0 - a) * b + y[3] * a * b;
}

const GeoidModel& model_1_degree = *get_geoid_model_1_degree();
const GeoidModel& model_15_minute = *get_geoid_model_15_minute();

/* lookup function for embedded geoid model */
float geoidh_emb(const double& lat,
                 const double& lon,
                 const GeoidModel& model) {
  double a, b, y[4];
  int i1, i2, j1, j2;

  if (lon < range[0] || range[1] < lon || lat < range[2] || range[3] < lat) {
    return 0.0;
  }
  a = (lon - range[0]) / model.lon_spacing;
  b = (lat - range[2]) / model.lat_spacing;
  i1 = static_cast<int>(a);
  a -= i1;
  i2 = i1 < 360 / model.lon_spacing ? i1 + 1 : i1;
  j1 = static_cast<int>(b);
  b -= j1;
  j2 = j1 < 180 / model.lat_spacing ? j1 + 1 : j1;
  y[0] = model.data[i1 * model.n_lat + j1];
  y[1] = model.data[i2 * model.n_lat + j1];
  y[2] = model.data[i1 * model.n_lat + j2];
  y[3] = model.data[i2 * model.n_lat + j2];
  return interpb(y, a, b);
}

/*
 * Perform direct lookups on GEOID to compare height values from the 1 degree
 * resolution geoid vs heights from the 15 minute resolution geoid for an
 * exact match (for all points which exist in the 1 degree grid)
 */
TEST(TestGeoidModel, CompareGeoidModelsMatchingPoints) {
  // 360 degrees of longitude (1 repeated)
  for (int lon = 0; lon < 361; lon++) {
    // 180 degrees of latitude (1 repeated)
    for (int lat = 0; lat < 181; lat++) {
      float val_1_degree =
          model_1_degree.data[lon * model_1_degree.n_lat + lat];
      float val_15_minute =
          model_15_minute.data[4 * lon * model_15_minute.n_lat + 4 * lat];

      EXPECT_TRUE(std::fabs(val_1_degree - val_15_minute) < 1e-4)
          << "Mismatch between 1 degree resolution and 0.25 degree "
             "resolution geoids: "
          << val_1_degree << " vs " << val_15_minute;
    }
  }
}

/*
 * Use geoidh_emb() to compare heights from the 1 degree resolution geoid vs
 * heights from the 15 minute resolution geoid for every 10th of a degree
 * (with bilinear interpolation).  Maximum expected difference is 15.2m (in
 * Hawaii).
 */
TEST(TestGeoidModel, CompareGeoidModelsTenthDegree) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;
      float val_1_degree = geoidh_emb(lat_deg, lon_deg, model_1_degree);
      float val_15_minute = geoidh_emb(lat_deg, lon_deg, model_15_minute);
      EXPECT_TRUE(std::fabs(val_1_degree - val_15_minute) < 15.5)
          << "Mismatch between 1 degree resolution and 0.25 degree "
             "resolution geoids "
             "at lat "
          << lat_deg << ", lon " << lon_deg << ": " << val_1_degree << " vs "
          << val_15_minute << ", delta "
          << std::fabs(val_1_degree - val_15_minute) << "\n";
    }
  }
}

/*
 * Compare heights from the 1 degree geoid (with bilinear interpolation) vs
 * heights from the 1 degree geoid (with bicubic interpolation) for every 10th
 * of a degree.  Maximum offset should be 3.69m (in Indonesia).
 */
TEST(TestGeoidModel, Compare1DegreeBilinearVsBicubic) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;

      float bilinear = geoidh_emb(lat_deg, lon_deg, model_1_degree);
      float bicubic = get_geoid_offset_1_degree(lat_deg * D2R, lon_deg * D2R);
      EXPECT_TRUE(std::fabs(bilinear - bicubic) < 3.69)
          << "Mismatch between bilinear and bicubic interpolation for 1 "
             "degree geoid "
             "at lat "
          << lat_deg << ", lon " << lon_deg << ": " << bilinear << "vs "
          << bicubic << ", delta " << std::fabs(bilinear - bicubic) << "\n";
    }
  }
}

/*
 * Compare heights from the 15 minute geoid (with bilinear interpolation) vs
 * heights from the 15 degree geoid (with bicubic interpolation) for every 10th
 * of a degree.  Maximum offset should be 1.33m (in Columbia)
 */
TEST(TestGeoidModel, Compare15MinuteBilinearVsBicubic) {
  for (int lon = 0; lon <= 3600; lon++) {
    for (int lat = -900; lat <= 900; lat++) {
      double lat_deg = lat / 10.;
      double lon_deg = lon / 10.;
      float bilinear = geoidh_emb(lat_deg, lon_deg, model_15_minute);
      float bicubic = get_geoid_offset_15_minute(lat_deg * D2R, lon_deg * D2R);
      EXPECT_TRUE(std::fabs(bilinear - bicubic) < 1.33)
          << "Mismatch between bilinear and bicubic interpolation for 15 "
             "minute grid "
             "at lat "
          << lat_deg << ", lon " << lon_deg << ": " << bilinear << "vs "
          << bicubic << ", delta " << std::fabs(bilinear - bicubic) << "\n";
    }
  }
}

}  // namespace
