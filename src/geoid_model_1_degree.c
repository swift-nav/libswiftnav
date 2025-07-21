#include "geoid_model_1_degree.inc"

#include "geoid_model.h"

static const struct GeoidModel model = {
    .data = &GEOID[0][0],
    .lat_spacing = LAT_GRID_SPACING_DEG,
    .lon_spacing = LON_GRID_SPACING_DEG,
    .n_lat = sizeof(GEOID[0]) / sizeof(GEOID[0][0]),
    .n_lon = sizeof(GEOID) / sizeof(GEOID[0]),
};

const struct GeoidModel *get_geoid_model_1_degree(void) { return &model; }
