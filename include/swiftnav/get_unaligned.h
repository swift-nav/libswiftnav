#ifndef SWIFTNAV_GET_UNALIGNED_H
#define SWIFTNAV_GET_UNALIGNED_H

#include <cstddef>

template <typename T, typename U>
T get_unaligned(const U *buf, size_t offset) {
  T value;
  memcpy(&value, reinterpret_cast<const char *>(buf) + offset, sizeof(T));
  return value;
}

#endif
