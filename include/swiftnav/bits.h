/*
 * Copyright (C) 2013, 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_BITS_H
#define LIBSWIFTNAV_BITS_H

#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* See https://github.com/wine-mirror/wine/blob/master/include/windows.h#L22
 * If enabled, MSVC will give out warning for the sign extend macros:
 * 'unnamed type definition in parentheses'
 */
#if defined(_MSC_VER) && (_MSC_VER >= 800) && !defined(__cplusplus)
#pragma warning(disable : 4116)
#endif

/**
 * Sign extension macro for 32-bit integers.
 *
 * \param n_bits Number of signed bits in the argument
 * \param arg    Unsigned ordinal for sign extension
 *
 * \return 32-bit signed integer with 2-complement of \a n_bits in \a arg
 */
#define BITS_SIGN_EXTEND_32(n_bits, arg) \
  ((struct { s32 bits : (n_bits); }){.bits = (arg)}.bits)
/**
 * Sign extension macro for 64-bit integers.
 *
 * \param n_bits Number of signed bits in the argument
 * \param arg    Unsigned ordinal for sign extension
 *
 * \return 64-bit signed integer with 2-complement of \a n_bits in \a arg
 */
#define BITS_SIGN_EXTEND_64(n_bits, arg) \
  ((struct { s64 bits : (n_bits); }){.bits = (arg)}.bits)

u8 parity(u32 x);
u16 bytes_interleave(const u8 x, const u8 y);
u32 getbitu(const u8 *buff, u32 pos, u8 len);
u64 getbitul(const u8 *buff, u32 pos, u8 len);
s32 getbits(const u8 *buff, u32 pos, u8 len);
s64 getbitsl(const u8 *buff, u32 pos, u8 len);
void setbitu(u8 *buff, u32 pos, u32 len, u32 data);
void setbits(u8 *buff, u32 pos, u32 len, s32 data);
void setbitul(u8 *buff, u32 pos, u32 len, u64 data);
void setbitsl(u8 *buff, u32 pos, u32 len, s64 data);
void bitcopy(
    void *dst, u32 dst_index, const void *src, u32 src_index, u32 count);
void bitshl(void *buf, u32 size, u32 shift);
u8 count_bits_u64(u64 v, u8 bv);
u8 count_bits_u32(u32 v, u8 bv);
u8 count_bits_u16(u16 v, u8 bv);
u8 count_bits_u8(u8 v, u8 bv);

enum endianess {
  SWIFT_LITTLE_ENDIAN = 0,  // So that a zero'd out value (eg in a struct)
                            // defaults to little endian
  SWIFT_BIG_ENDIAN,
};

static inline enum endianess get_endianess(void) {
  u16 v = 1;
  u8 *ptr = (u8 *)&v;
  return (*ptr == 1) ? SWIFT_LITTLE_ENDIAN : SWIFT_BIG_ENDIAN;
}

static inline u16 byte_swap_16(u16 v) { return (u16)((v >> 8) | (v << 8)); }

static inline u32 byte_swap_32(u32 v) {
  return (v >> 24) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | (v << 24);
}

static inline u64 byte_swap_64(u64 v) {
  return (v >> 56) | ((v >> 40) & 0xff00) | ((v >> 24) & 0xff0000) |
         ((v >> 8) & 0xff000000) | ((v << 8) & 0xff00000000) |
         ((v << 24) & 0xff0000000000) | ((v << 40) & 0xff000000000000) |
         ((v << 56) & 0xff00000000000000);
}

static inline u16 htobe_16(u16 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_16(v);
}

static inline u16 htole_16(u16 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_16(v);
}

static inline u32 htobe_32(u32 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_32(v);
}

static inline u32 htole_32(u32 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_32(v);
}

static inline u64 htobe_64(u64 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_64(v);
}

static inline u64 htole_64(u64 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_64(v);
}

static inline u16 betoh_16(u16 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_16(v);
}

static inline u16 letoh_16(u16 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_16(v);
}

static inline u32 betoh_32(u32 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_32(v);
}

static inline u32 letoh_32(u32 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_32(v);
}

static inline u64 betoh_64(u64 v) {
  return get_endianess() == SWIFT_BIG_ENDIAN ? v : byte_swap_64(v);
}

static inline u64 letoh_64(u64 v) {
  return get_endianess() == SWIFT_LITTLE_ENDIAN ? v : byte_swap_64(v);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_BITS_H */
