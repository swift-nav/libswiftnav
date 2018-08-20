/*
 * Copyright (C) 2017 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <inttypes.h>
#include <string.h>

#include <swiftnav/logging.h>
#include <swiftnav/memcpy_s.h>

memcpy_s_t memcpy_s(void *dest,
                    size_t destsize,
                    const void *src,
                    size_t count) {
  if (NULL == dest) {
    log_error("memcpy_s error: destination NULL");
    return MEMCPY_S_DEST_NULL;
  }

  if (NULL == src) {
    log_error("memcpy_s error: source NULL");
    return MEMCPY_S_SRC_NULL;
  }

  if (0 == destsize) {
    log_error("memcpy_s error: destination size zero");
    return MEMCPY_S_DEST_SIZE_ZERO;
  }

  if (0 == count) {
    log_error("memcpy_s error: src size zero");
    return MEMCPY_S_SRC_SIZE_ZERO;
  }

  if (destsize < count) {
    log_error("memcpy_s error: src size %" PRIu64
              " greater than dest size %" PRIu64,
              (u64)count,
              (u64)destsize);
    return MEMCPY_S_OVERSIZED;
  }

  if (((src > dest) && (src < (void *)((u8 *)dest + destsize))) ||
      ((dest > src) && (dest < (void *)((u8 *)src + count)))) {
    log_error("memcpy_s error: overlap");
    return MEMCPY_S_OVERLAP;
  }

  memcpy(dest, src, count);

  return MEMCPY_S_OK;
}
