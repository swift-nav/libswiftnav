/*
 * Copyright (C) 2017 Swift Navigation Inc.
 * Contact: Dmitry Tatarinov <dmitry.tatarinov@exafore.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef SRC_GLO_MAP_H_
#define SRC_GLO_MAP_H_

#include <swiftnav/signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void glo_map_init(void (*lock_cb)(void), void (*unlock_cb)(void));
bool glo_map_valid(const gnss_signal_t sid);
void glo_map_set_slot_id(u16 fcn, u16 glo_slot_id);
u16 glo_map_get_fcn(gnss_signal_t sid);
void glo_map_clear_slot_id(u16 glo_slot_id);
void glo_map_clear_all(void);
void glo_map_fill_dummy_data(void);
u8 glo_map_get_slot_id(const u16 fcn, u16 *slot_id1, u16 *slot_id2);
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* SRC_GLO_MAP_H_ */
