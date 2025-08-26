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

#include <gtest/gtest.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <swiftnav/constants.h>
#include <swiftnav/shm.h>

namespace {

TEST(TestShm, ShmGpsDecodeShiEphemeris) {
  u32 sf1w3 = 0x3f122c34;
  u8 shi_ephemeris;

  shm_gps_decode_shi_ephemeris(sf1w3, &shi_ephemeris);

  EXPECT_EQ(shi_ephemeris, 0x2c);
}

TEST(TestShm, CheckNavDhi) {
  for (u8 dhi = 0; dhi < NAV_DHI_COUNT; ++dhi) {
    for (u16 ignored = 0; ignored <= UCHAR_MAX; ++ignored) {
      bool res = check_nav_dhi((dhi << 5), ignored);
      bool expected = (NAV_DHI_OK == dhi) || ((ignored & 1 << dhi) != 0);
      EXPECT_EQ(res, expected);
    }
  }
}

}  // namespace
