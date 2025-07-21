#ifndef LIBSWIFTNAV_SRC_GEOID_MODEL_H
#define LIBSWIFTNAV_SRC_GEOID_MODEL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GeoidModel {
  const float *data;
  float lat_spacing;
  float lon_spacing;
  size_t n_lat;
  size_t n_lon;
};

const struct GeoidModel *get_geoid_model_15_minute(void);
const struct GeoidModel *get_geoid_model_1_degree(void);

#ifdef __cplusplus
}
#endif

#endif
