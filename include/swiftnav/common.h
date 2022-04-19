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

#ifndef LIBSWIFTNAV_COMMON_H
#define LIBSWIFTNAV_COMMON_H

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#ifdef _MSC_VER
#include <malloc.h>
#define LSN_NEW_ARRAY(Name, ItemCount, Type)         \
  Type *Name = _malloca(sizeof(Type) * (ItemCount)); \
  assert(Name != NULL);                              \
  memset(Name, 0, sizeof(Type) * (ItemCount))
#define LSN_FREE_ARRAY(ptr) _freea((void *)ptr)
#else
#define LSN_NEW_ARRAY(Name, ItemCount, Type)                         \
  Type Name[ItemCount];                                              \
  for (size_t _x_##Name = 0; _x_##Name < (ItemCount); _x_##Name++) { \
    (Name)[_x_##Name] = 0;                                           \
  }
#define LSN_FREE_ARRAY(ptr)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup common Common definitions
 * Common definitions used throughout the library.
 * \{ */

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define CLAMP_DIFF(a, b) (MAX((a), (b)) - (b))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ARRAY_SIZE2(a) (sizeof(a) / sizeof((a)[0][0]))
#define SIGN(a) (((a) >= 0) ? +1 : -1)

/* See http://c-faq.com/cpp/multistmt.html for
 * and explaination of the do {} while(0)
 */
#define DO_EVERY(n, cmd)                        \
  do {                                          \
    static u32 do_every_count = 0;              \
    if ((n) > 0 && do_every_count % (n) == 0) { \
      cmd;                                      \
    }                                           \
    do_every_count++;                           \
  } while (0)

#ifndef COMMON_INT_TYPES
#define COMMON_INT_TYPES

/** \defgroup common_inttypes Integer types
 * Specified-width integer type definitions for shorter and nicer code.
 *
 * These should be used in preference to unspecified width types such as
 * `int` which can lead to portability issues between different platforms.
 * \{ */

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#endif /* COMMON_INT_TYPES */

/** \} */
/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_COMMON_H */
