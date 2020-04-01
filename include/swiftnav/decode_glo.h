/*
 * Copyright (C) 2020 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef LIBSWIFTNAV_DECODE_GLO_H
#define LIBSWIFTNAV_DECODE_GLO_H

#include <inttypes.h>
#include <stdbool.h>
#include <swiftnav/common.h>
#include <swiftnav/ephemeris.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u32 extract_word_glo(const glo_string_t *string, u16 bit_index, u8 n_bits);

s8 error_detection_glo(const glo_string_t *string);

bool decode_glo_string_1(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk);
bool decode_glo_string_2(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *toe);
bool decode_glo_string_3(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *toe);
bool decode_glo_string_4(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk,
                         glo_time_t *toe,
                         u8 *age_of_data_days);
bool decode_glo_string_5(const glo_string_t *string,
                         ephemeris_t *eph,
                         glo_time_t *tk,
                         glo_time_t *toe,
                         float *tau_gps_s);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* LIBSWIFTNAV_DECODE_GLO_H */
