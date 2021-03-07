/*
 * Copyright (C) 2020 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <assert.h>
#include <swiftnav/common.h>
#include <swiftnav/constants.h>
#include <swiftnav/float_equality.h>
#include <swiftnav/geoid_model.h>
#include <swiftnav/logging.h>

// limits after converting to degrees
#define MIN_LON 0
#define MAX_LON 360
#define MIN_LAT (-90)
#define MAX_LAT 90

#ifdef GEOID_MODEL_15_MINUTE_RESOLUTION
/* Geoid model with geoid heights derived from EGM2008 (0.25 x 0.25 deg grid) */
#include "geoid_model_15_minute.inc"
#else
/* Geoid model with geoid heights derived from EGM2008 (1 x 1 deg grid) */
#include "geoid_model_1_degree.inc"
#endif /* GEOID_MODEL_15_MINUTE_RESOLUTION */

/*
 * Get GEOID[x][y], accounting for wrap-around of longitude values
 */
static inline float get_geoid_val(int x, int y) {
  if (x >= (MAX_LON - MIN_LON) / LON_GRID_SPACING_DEG) {
    x -= (int)((MAX_LON - MIN_LON) / LON_GRID_SPACING_DEG);
  } else if (x < 0) {
    x += (int)((MAX_LON - MIN_LON) / LON_GRID_SPACING_DEG);
  }

  return GEOID[x][y];
}

/*
 * Perform bilinear interpolation of the height values in a single geoid cell
 * where the corners are located at:
 *
 * (x,y+1).(x+1,y+1)
 *   .         .
 * (x,y)...(x+1,y)
 *
 * 'fx' and 'fy' specify the fractional offset with respect to the (x,y)
 * corner.
 */
static float bilinear_interpolation(int x, int y, float fx, float fy) {
  /* Get the values at the four corners */
  float southwest = get_geoid_val(x, y);
  float southeast = get_geoid_val(x + 1, y);
  float northwest = get_geoid_val(x, y + 1);
  float northeast = get_geoid_val(x + 1, y + 1);

  /* Bilinear interpolation */
  return (1 - fy) * ((1 - fx) * southwest + fx * southeast) +
         fy * ((1 - fx) * northwest + fx * northeast);
}

/*
 * Perform cubic interpolation, i.e. evaluate the function:
 *
 * f(p0,p1,p2,p3,x) = p1 +
 *                    (-1/2*p0 + 1/2*p2)*x +
 *                    (p0-5/2*p1 + 2*p2 - 1/2*p3)*x^2 +
 *                    (-1/2*p0 + 3/2*p1 -3/2*p2 + 1/2*p3)*x^3
 */
static double cubic_interpolation(double p[4], double x) {
  return p[1] + 0.5 * x *
                    (p[2] - p[0] +
                     x * (2. * p[0] - 5. * p[1] + 4. * p[2] - p[3] +
                          x * (3. * (p[1] - p[2]) + p[3] - p[0])));
}

/* Return the geoid offset */
float get_geoid_offset(double lat_rad, double lon_rad) {
  /* Convert to degrees, returning 0.0 if out of bounds */
  float lat_deg = (float)(R2D * lat_rad);
  if (lat_deg > MAX_LAT || lat_deg < MIN_LAT) {
    log_error("Invalid latitude passed to get_geoid_offset: %lf", lat_rad);
    return 0.0;
  }

  float lon_deg = (float)(R2D * lon_rad);
  if (lon_deg < 0.) {
    lon_deg += 360.f;
  }
  if (lon_deg > MAX_LON || lon_deg < MIN_LON) {
    log_error("Invalid longitude passed to get_geoid_offset: %lf", lon_rad);
    return 0.0;
  }

  float fx, fy;    // fractional offset from cell corners
  float ixf, iyf;  // integer offset of cell corners

  fy = modff((lat_deg - MIN_LAT) / LAT_GRID_SPACING_DEG, &iyf);
  fx = modff((lon_deg - MIN_LON) / LON_GRID_SPACING_DEG, &ixf);

  int ix = (int)ixf;
  int iy = (int)iyf;

  /*
   * Special Case 1: if lat is +90 then use the geoid value directly (note:
   * at this latitude, height is same regardless of value of x)
   */
  if (iy == (int)((MAX_LAT - MIN_LAT) / LAT_GRID_SPACING_DEG)) {
    return GEOID[ix][iy];
  }

  /*
   * The general idea for performing bicubic interpolation is as follows:
   *
   * Given a grid of points:
   *
   * p03 p13 p23 p33
   * p02 p12 p22 p32
   * p01 p11 p21 p31
   * p00 p10 p20 p30
   *
   * We can perform interpolation in the region bounded by p11, p12, p21 and
   * p22 using:
   *
   * g(x,y) = f(f(p00,p01,p02,p03,y), f(p10,p11,p12,p13,y),
   *            f(p20,p21,p22,p23,y), f(p30,p31,p32,p33,y), x)
   *
   * We want to perform interpolation on the cell starting at (ix,iy), meaning
   * that we need the heights from the following locations:
   *
   * (ix-1,iy+2) (ix  ,iy+2) (ix+1,iy+2) (ix+2,iy+2)
   * (ix-1,iy+1) (ix  ,iy+1) (ix+1,iy+1) (ix+2,iy+1)
   * (ix-1,iy  ) (ix  ,iy  ) (ix+1,iy  ) (ix+2,iy  )
   * (ix-1,iy-1) (ix  ,iy-1) (ix+1,iy-1) (ix+2,iy-1)
   */
  if (iy > 0 && iy < (MAX_LAT - MIN_LAT) / LAT_GRID_SPACING_DEG - 1) {
    int y0, y1, y2, y3;

    if (iy == (int)((MAX_LAT - MIN_LAT) / LAT_GRID_SPACING_DEG - 2)) {
      /*
       * Special Case 2: for latitudes in range
       * [90 - 2 * LAT_GRID_SPACING, 90 - LAT_GRID_SPACING)
       * we flip the Y indices and the fractional offset so that
       * the height values correspond to:
       *
       * (ix-1,iy-1) (ix  ,iy-1) (ix+1,iy-1) (ix+2,iy-1)
       * (ix-1,iy  ) (ix  ,iy  ) (ix+1,iy  ) (ix+2,iy  )
       * (ix-1,iy+1) (ix  ,iy+1) (ix+1,iy+1) (ix+2,iy+1)
       * (ix-1,iy+2) (ix  ,iy+2) (ix+1,iy+2) (ix+2,iy+2)
       */
      y0 = iy + 2;
      y1 = iy + 1;
      y2 = iy;
      y3 = iy - 1;

      fy = 1.f - fy;
    } else {
      y0 = iy - 1;
      y1 = iy;
      y2 = iy + 1;
      y3 = iy + 2;
    }

    double heights[4][4] = {{get_geoid_val(ix - 1, y0),
                             get_geoid_val(ix - 1, y1),
                             get_geoid_val(ix - 1, y2),
                             get_geoid_val(ix - 1, y3)},
                            {get_geoid_val(ix, y0),
                             get_geoid_val(ix, y1),
                             get_geoid_val(ix, y2),
                             get_geoid_val(ix, y3)},
                            {get_geoid_val(ix + 1, y0),
                             get_geoid_val(ix + 1, y1),
                             get_geoid_val(ix + 1, y2),
                             get_geoid_val(ix + 1, y3)},
                            {get_geoid_val(ix + 2, y0),
                             get_geoid_val(ix + 2, y1),
                             get_geoid_val(ix + 2, y2),
                             get_geoid_val(ix + 2, y3)}};

    double arr[4] = {cubic_interpolation(heights[0], fy),
                     cubic_interpolation(heights[1], fy),
                     cubic_interpolation(heights[2], fy),
                     cubic_interpolation(heights[3], fy)};
    return (float)cubic_interpolation(arr, fx);

  } /* else Special Case 3: perform bilinear interpolation if latitude
     * is in range [-90,-90 + LAT_GRID_SPACING_DEG) or
     * [90 - LAT_GRID_SPACING, 90) */

  return bilinear_interpolation(ix, iy, fx, fy);
}
