/*
 * Copyright (C) 2018 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_SBAS_RAW_DATA_H
#define LIBSWIFTNAV_SBAS_RAW_DATA_H

#include <swiftnav/common.h>
#include <swiftnav/gnss_time.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SBAS_RAW_PAYLOAD_LENGTH 27

typedef struct {
  gnss_signal_t sid;
  gps_time_t time_of_transmission;
  u8 message_type;
  u8 data[SBAS_RAW_PAYLOAD_LENGTH];
} sbas_raw_data_t;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* SBAS_RAW_DATA_H */
