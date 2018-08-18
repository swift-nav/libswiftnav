/*
 * Copyright (C) 2010 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_EDC_H
#define LIBSWIFTNAV_EDC_H

#include <stdbool.h>
#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 crc24q(const u8 *buf, u32 len, u32 crc);
u32 crc24q_bits(u32 crc, const u8 *buf, u32 n_bits, bool invert);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_EDC_H */
