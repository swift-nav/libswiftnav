#ifndef LIBSWIFTNAV_BYTESTREAM_H
#define LIBSWIFTNAV_BYTESTREAM_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <swiftnav/bits.h>
#include <swiftnav/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct swiftnav_bytestream {
  const u8 *data;
  u32 len;
  u32 offset;
} swiftnav_bytestream_t;

static inline bool swiftnav_bytestream_would_overflow(
    const swiftnav_bytestream_t *buf, u32 index, u32 len) {
  return (buf->offset + index + len) > buf->len;
}

static inline void swiftnav_bytestream_init(swiftnav_bytestream_t *buf,
                                            const u8 *data,
                                            u32 len) {
  buf->data = data;
  buf->len = len;
  buf->offset = 0;
}

static inline bool swiftnav_bytestream_get_bytes(
    const swiftnav_bytestream_t *buf, u32 index, u32 len, u8 *dest) {
  if (swiftnav_bytestream_would_overflow(buf, index, len)) {
    return false;
  }
  memcpy(dest, buf->data + buf->offset + index, len);
  return true;
}

static inline void swiftnav_bytestream_remove(swiftnav_bytestream_t *buf,
                                              u32 len) {
  buf->offset += len;
}

static inline u32 swiftnav_bytestream_remaining(swiftnav_bytestream_t *buf) {
  if (buf->offset > buf->len) {
    return 0;
  }
  return buf->len - buf->offset;
}

#ifdef __cplusplus
}
#endif

#endif
