/*
 * Copyright (C) 2012 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *          Fergus Noble <fergus@swift-nav.com>
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** \defgroup common Common definitions
 * Common definitions used throughout the library.
 * \{ */

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif /* ABS */
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif /* MIN */
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif  /* MAX */
#define CLAMP_DIFF(a, b) (MAX((a), (b)) - (b))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ARRAY_SIZE2(a) (sizeof(a) / sizeof((a)[0][0]))

#define SIGN(a) ((a >= 0) ? +1 : -1)

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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef COMMON_INT_TYPES
#define COMMON_INT_TYPES

/** \defgroup common_inttypes Integer types
 * Specified-width integer type definitions for shorter and nicer code.
 *
 * These should be used in preference to unspecified width types such as
 * `int` which can lead to portability issues between different platforms.
 * \{ */

/** Signed 8-bit integer. */
typedef int8_t s8;
/** Signed 16-bit integer. */
typedef int16_t s16;
/** Signed 32-bit integer. */
typedef int32_t s32;
/** Signed 64-bit integer. */
typedef int64_t s64;
/** Unsigned 8-bit integer. */
typedef uint8_t u8;
/** Unsigned 16-bit integer. */
typedef uint16_t u16;
/** Unsigned 32-bit integer. */
typedef uint32_t u32;
/** Unsigned 64-bit integer. */
typedef uint64_t u64;

#endif /* COMMON_INT_TYPES */

/** \} */

/** \} */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_COMMON_H */
