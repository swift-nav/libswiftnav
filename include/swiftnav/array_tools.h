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

#ifndef ARRAY_TOOLS_H
#define ARRAY_TOOLS_H

#include <stdbool.h>

#include <swiftnav/common.h>

/** Check if value is in array.
 *
 * \param array Array of values
 * \param count Length of array
 * \param value Value to look for
 */
static inline bool is_value_in_array(const u8 array[const],
                                     u8 count,
                                     u8 value) {
  for (u8 i = 0; i < count; i++) {
    if (array[i] == value) {
      return true;
    }
  }

  return false;
}

static inline bool is_value_in_array_u16(const u16 array[const],
                                         u8 count,
                                         u16 value) {
  for (u8 i = 0; i < count; i++) {
    if (array[i] == value) {
      return true;
    }
  }

  return false;
}

#endif  // ARRAY_TOOLS_H
