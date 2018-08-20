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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

/* Set packing based upon toolchain */
#if defined(__GNUC__) || defined(__clang__)

#define SBP_PACK_START /* Intentionally empty */
#define SBP_PACK_END   /* Intentionally empty */
#define SBP_ATTR_PACKED __attribute__((packed))

#elif defined(_MSC_VER)

#define SBP_PACK_START __pragma(pack(1));
#define SBP_PACK_END __pragma(pack());
#define SBP_ATTR_PACKED /* Intentionally empty */

#else

#if !defined(SBP_PACK_START) || !defined(SBP_PACK_END) || \
    !defined(SBP_ATTR_PACKED)
#error Unknown compiler, please override SBP_PACK_START, SBP_PACK_END, and SBP_ATTR_PACKED
#endif

#endif /* toolchaing packing macros */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBSWIFTNAV_COMMON_H */
