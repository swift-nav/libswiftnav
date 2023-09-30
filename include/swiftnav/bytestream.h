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
  enum endianess endianess;
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

static inline bool swiftnav_bytestream_decode_u8(swiftnav_bytestream_t *buf,
                                                 u8 *dest) {
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), dest)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  return true;
}

static inline bool swiftnav_bytestream_decode_s8(swiftnav_bytestream_t *buf,
                                                 s8 *dest) {
  u8 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), &v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_u16(swiftnav_bytestream_t *buf,
                                                  u16 *dest) {
  u16 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_16(v) : letoh_16(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_s16(swiftnav_bytestream_t *buf,
                                                  s16 *dest) {
  u16 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_16(v) : letoh_16(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_u32(swiftnav_bytestream_t *buf,
                                                  u32 *dest) {
  u32 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_32(v) : letoh_32(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_s32(swiftnav_bytestream_t *buf,
                                                  s32 *dest) {
  u32 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_32(v) : letoh_32(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_u64(swiftnav_bytestream_t *buf,
                                                  u64 *dest) {
  u64 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_64(v) : letoh_64(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

static inline bool swiftnav_bytestream_decode_s64(swiftnav_bytestream_t *buf,
                                                  s64 *dest) {
  u64 v;
  if (!swiftnav_bytestream_get_bytes(buf, 0, sizeof(*dest), (uint8_t *)&v)) {
    return false;
  }
  swiftnav_bytestream_remove(buf, sizeof(*dest));
  v = (buf->endianess == SWIFT_BIG_ENDIAN) ? betoh_64(v) : letoh_64(v);
  memcpy(dest, &v, sizeof(v));
  return true;
}

#ifdef __cplusplus
}
#endif

#endif
