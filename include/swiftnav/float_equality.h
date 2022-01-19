#ifndef SWIFTNAV_FLOAT_EQUALITY_H
#define SWIFTNAV_FLOAT_EQUALITY_H

#include <math.h>
#include <stdbool.h>
#include <swiftnav/constants.h>

static inline bool double_equal(double a, double b) {
  return fabs(a - b) < FLOAT_EQUALITY_EPS;
}

static inline bool float_equal(float a, float b) {
  return double_equal((double)a, (double)b);
}

#endif
