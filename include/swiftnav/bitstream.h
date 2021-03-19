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

typedef struct swiftnav_bitstream {
  const uint8_t *data;
  uint32_t len;
  uint32_t offset;
} swiftnav_bitstream_t;

static inline bool swiftnav_bitstream_would_overflow(
    const swiftnav_bitstream_t *buf, u32 pos, u32 len) {
  return (buf->offset + pos + len) > buf->len;
}

static inline void swiftnav_bitstream_init(swiftnav_bitstream_t *buf,
                                           const uint8_t *data,
                                           u32 len) {
  buf->data = data;
  buf->len = len;
  buf->offset = 0;
}

static inline bool swiftnav_bitstream_getbitu(const swiftnav_bitstream_t *buf,
                                              u32 *out,
                                              u32 pos,
                                              u32 len) {
  if (swiftnav_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitu(buf->data, buf->offset + pos, len);
  return true;
}

static inline bool swiftnav_bitstream_getbits(const swiftnav_bitstream_t *buf,
                                              s32 *out,
                                              u32 pos,
                                              u32 len) {
  if (swiftnav_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbits(buf->data, buf->offset + pos, len);
  return true;
}

static inline bool swiftnav_bitstream_getbitul(const swiftnav_bitstream_t *buf,
                                               u64 *out,
                                               u32 pos,
                                               u32 len) {
  if (swiftnav_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitul(buf->data, buf->offset + pos, len);
  return true;
}

static inline bool swiftnav_bitstream_getbitsl(const swiftnav_bitstream_t *buf,
                                               s64 *out,
                                               u32 pos,
                                               u32 len) {
  if (swiftnav_bitstream_would_overflow(buf, pos, len)) {
    return false;
  }
  *out = getbitsl(buf->data, buf->offset + pos, len);
  return true;
}

static inline void swiftnav_bitstream_remove(swiftnav_bitstream_t *buf,
                                             u32 len) {
  buf->offset += len;
}

static inline u32 swiftnav_bitstream_remaining(swiftnav_bitstream_t *buf) {
  if (buf->offset > buf->len) {
    return 0;
  }
  return buf->len - buf->offset;
}

#ifdef __cplusplus
}
#endif

#endif
