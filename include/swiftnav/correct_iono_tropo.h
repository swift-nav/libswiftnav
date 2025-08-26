/*
 * Copyright (c) 2018 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_CORRECT_IONO_TROPO_H
#define LIBSWIFTNAV_CORRECT_IONO_TROPO_H

#include <swiftnav/coord_system.h>
#include <swiftnav/ionosphere.h>
#include <swiftnav/nav_meas.h>

#ifdef __cplusplus
extern "C" {
#endif

void correct_iono(const double *pos_ecef,
                  const ionosphere_t *iono_params,
                  u8 n_meas,
                  navigation_measurement_t *nav_meas);

void correct_tropo(const double *pos_ecef,
                   u8 n_meas,
                   navigation_measurement_t *nav_meas);

#ifdef __cplusplus
}  // end extern "C"
#endif

#endif  // LIBSWIFTNAV_CORRECT_IONO_TROPO_H
