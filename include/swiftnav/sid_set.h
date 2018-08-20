/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_SID_SET_H
#define LIBSWIFTNAV_SID_SET_H

#include <swiftnav/common.h>
#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  u64 sats[CODE_COUNT];
} gnss_sid_set_t;

void sid_set_init(gnss_sid_set_t *sid_set);
void sid_set_add(gnss_sid_set_t *sid_set, gnss_signal_t sid);
void sid_set_remove(gnss_sid_set_t *sid_set, gnss_signal_t sid);
u32 sid_set_get_sat_count(const gnss_sid_set_t *sid_set);
u32 sid_set_get_sig_count(const gnss_sid_set_t *sid_set);
bool sid_set_contains(const gnss_sid_set_t *sid_set, gnss_signal_t sid);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_SID_SET_H */
