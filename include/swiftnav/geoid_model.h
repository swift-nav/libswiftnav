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

#ifndef GEOID_MODEL_H
#define GEOID_MODEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// range for lat_rad is (-M_PI_2, M_PI_2)
// range for lon_rad is (-2 * M_PI, 2 * M_PI)
float get_geoid_offset(double lat_rad, double lon_rad);

typedef enum {
  GEOID_MODEL_NONE = 0,
  GEOID_MODEL_EGM96 = 1,
  GEOID_MODEL_EGM2008 = 2,
} geoid_model_t;

geoid_model_t get_geoid_model(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GEOID_MODEL_H */
