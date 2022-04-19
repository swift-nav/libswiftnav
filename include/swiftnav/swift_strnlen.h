#ifndef LIBSWIFTNAV_STRNLEN_H
#define LIBSWIFTNAV_STRNLEN_H

#include <stddef.h>

static inline size_t swift_strnlen(const char *str, size_t max) {
  size_t len = 0;
  while (len < max && str[len] != 0) {
    len++;
  }
  return len;
}

#endif
