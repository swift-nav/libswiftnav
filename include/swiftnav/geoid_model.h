/*------------------------------------------------------------------------------
 * Reproduced from https://github.com/swift-nav/RTKLIB/blob/master/src/geoid.c
 * according to the RTKLib licence
 *
 *          Copyright (C) 2007-2013 by T.TAKASU, All rights reserved.
 *-----------------------------------------------------------------------------*/

#ifndef GEOID_MODEL_H
#define GEOID_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MIN_LON 0
#define MAX_LON 360
#define MIN_LAT (-90)
#define MAX_LAT 90

#define LAT_GRID_SPACING_DEG 1
#define LON_GRID_SPACING_DEG 1

extern const float GEOID[(MAX_LON - MIN_LON) / LON_GRID_SPACING_DEG + 1]
                        [(MAX_LAT - MIN_LAT) / LAT_GRID_SPACING_DEG + 1];

float get_geoid_offset(double lat_rad, double lon_rad);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GEOID_MODEL_H */
