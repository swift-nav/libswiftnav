/*
 * Copyright (C) 2012 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_TROPOSPHERE_H
#define LIBSWIFTNAV_TROPOSPHERE_H

#include <swiftnav/gnss_time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* tropo correction is not applicable above max altitude */
/* value is 11km which is the top of the troposphere according to ICAO's
 * International Standard Atmosphere. */
#define MAX_ALTITUDE 11e3

/* truncate satellite elevations near or below zero [deg] */
#define MIN_SAT_ELEVATION 0.1

double calc_troposphere(const gps_time_t *t_gps,
                        double lat,
                        double h,
                        double el);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_TROPOSPHERE_H */
