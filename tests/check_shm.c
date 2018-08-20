/*
 * Copyright (C) 2016 Swift Navigation Inc.
 * Contact: Swift Navigation <dev@swiftnav.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <check.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

#include <swiftnav/constants.h>
#include <swiftnav/shm.h>
#include "check_suites.h"

START_TEST(test_shm_gps_decode_shi_ephemeris) {
  u32 sf1w3 = 0x3f122c34;
  u8 shi_ephemeris;

  shm_gps_decode_shi_ephemeris(sf1w3, &shi_ephemeris);

  fail_unless(shi_ephemeris == 0x2c,
              "shm_gps_decode_shi_ephemeris() returns 0x%x for 0x%x\n",
              shi_ephemeris,
              sf1w3);
}
END_TEST

START_TEST(test_check_nav_dhi) {
  for (u8 dhi = 0; dhi < NAV_DHI_COUNT; ++dhi) {
    for (u16 ignored = 0; ignored <= UCHAR_MAX; ++ignored) {
      bool res = check_nav_dhi((dhi << 5), ignored);
      bool expected = (NAV_DHI_OK == dhi) || (ignored & 1 << dhi);
      fail_unless(res == expected,
                  "check_nav_dhi(%" PRIu8 ", %" PRIu8 ") failed",
                  (dhi << 5),
                  ignored);
    }
  }
}
END_TEST

Suite *shm_suite(void) {
  Suite *s = suite_create("SHM");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_shm_gps_decode_shi_ephemeris);
  tcase_add_test(tc_core, test_check_nav_dhi);
  suite_add_tcase(s, tc_core);

  return s;
}
