#ifndef LIBSWIFTNAV_BITSTREAM_H
#define LIBSWIFTNAV_BITSTREAM_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <swiftnav/bits.h>
#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swiftnav_in_bitstream {
  const uint8_t *data;
  uint32_t len;
  uint32_t offset;
} swiftnav_in_bitstream_t;

typedef struct swiftnav_out_bitstream {
  uint8_t *data;
  uint32_t len;
  uint32_t offset;
} swiftnav_out_bitstream_t;

static inline bool swiftnav_in_bitstream_would_overflow(
    const swiftnav_in_bitstream_t *buf, u32 pos, u32 len) {
  return (buf->offset + pos + len) > buf->len;
}

static inline void swiftnav_in_bitstream_init(swiftnav_in_bitstream_t *buf,
                                              const uint8_t *data,
                                              u32 len) {
  buf->data = data;
  buf->len = len;
  buf->offset = 0;
}

static inline bool swiftnav_in_bitstream_getbitu(
    const swiftnav_in_bitstream_t *buf, u32 *out, u32 pos, u32 len) {
  if (swiftnav_in_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitu(buf->data, buf->offset + pos, (u8)len);
  return true;
}

static inline bool swiftnav_in_bitstream_getbits(
    const swiftnav_in_bitstream_t *buf, s32 *out, u32 pos, u32 len) {
  if (swiftnav_in_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbits(buf->data, buf->offset + pos, (u8)len);
  return true;
}

static inline bool swiftnav_in_bitstream_getbitul(
    const swiftnav_in_bitstream_t *buf, u64 *out, u32 pos, u32 len) {
  if (swiftnav_in_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitul(buf->data, buf->offset + pos, (u8)len);
  return true;
}

static inline bool swiftnav_in_bitstream_getbitsl(
    const swiftnav_in_bitstream_t *buf, s64 *out, u32 pos, u32 len) {
  if (swiftnav_in_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitsl(buf->data, buf->offset + pos, (u8)len);
  return true;
}

static inline void swiftnav_in_bitstream_remove(swiftnav_in_bitstream_t *buf,
                                                u32 len) {
  buf->offset += len;
}

static inline u32 swiftnav_in_bitstream_remaining(
    swiftnav_in_bitstream_t *buf) {
  if (buf->offset > buf->len) {
    return 0;
  }
  return buf->len - buf->offset;
}

static inline bool swiftnav_out_bitstream_would_overflow(
    const swiftnav_out_bitstream_t *buf, u32 pos, u32 len) {
  return (buf->offset + pos + len) > buf->len;
}
static inline void swiftnav_out_bitstream_init(swiftnav_out_bitstream_t *buf,
                                               uint8_t *data,
                                               u32 len) {
  buf->data = data;
  buf->len = len;
  buf->offset = 0;
}
static inline bool swiftnav_out_bitstream_setbitu(swiftnav_out_bitstream_t *buf,
                                                  const u32 *in,
                                                  u32 pos,
                                                  u32 len) {
  if (swiftnav_out_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  setbitu(buf->data, buf->offset + pos, (u8)len, *in);
  return true;
}

static inline bool swiftnav_out_bitstream_setbits(swiftnav_out_bitstream_t *buf,
                                                  const s32 *in,
                                                  u32 pos,
                                                  u32 len) {
  if (swiftnav_out_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  setbits(buf->data, buf->offset + pos, (u8)len, *in);
  return true;
}

static inline bool swiftnav_out_bitstream_setbitul(
    swiftnav_out_bitstream_t *buf, const u64 *in, u32 pos, u32 len) {
  if (swiftnav_out_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  setbitul(buf->data, buf->offset + pos, (u8)len, *in);
  return true;
}

static inline bool swiftnav_out_bitstream_setbitsl(
    swiftnav_out_bitstream_t *buf, const s64 *in, u32 pos, u32 len) {
  if (swiftnav_out_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  setbitsl(buf->data, buf->offset + pos, (u8)len, *in);
  return true;
}

static inline void swiftnav_out_bitstream_remove(swiftnav_out_bitstream_t *buf,
                                                 u32 len) {
  buf->offset += len;
}
static inline u32 swiftnav_out_bitstream_remaining(
    swiftnav_out_bitstream_t *buf) {
  if (buf->offset > buf->len) {
    return 0;
  }
  return buf->len - buf->offset;
}

#ifdef __cplusplus
}
#endif

#endif
