/*
 * Copyright (C) 2017 Swift Navigation Inc.
 * Contact: Dmitry Tatarinov <dmitry.tatarinov@exafore.com>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <gtest/gtest.h>
#include <swiftnav/glo_map.h>
#include <swiftnav/logging.h>

#define FCN_TEST_VAL 14
#define SLOT_ID_TEST_VAL_1 5
#define SLOT_ID_TEST_VAL_2 10

namespace {

void glo_map_lock(void) {}
void glo_map_unlock(void) {}

TEST(TestGloMap, GloMap) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  for (u16 i = 1; i <= NUM_SATS_GLO; i++) {
    gnss_signal_t glo_sid = construct_sid(CODE_GLO_L1OF, i);
    glo_map_set_slot_id(FCN_TEST_VAL, /*glo_slot_id=*/i);

    EXPECT_TRUE(glo_map_valid(glo_sid));

    u16 fcn = glo_map_get_fcn(glo_sid);
    EXPECT_EQ(fcn, FCN_TEST_VAL);

    glo_map_clear_slot_id(i);

    EXPECT_FALSE(glo_map_valid(glo_sid));
  }

  u16 slot_id1, slot_id2;
  u8 si_num;
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  EXPECT_EQ(si_num, 0);
  EXPECT_EQ(slot_id1, 0);
  EXPECT_EQ(slot_id2, 0);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_1);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  EXPECT_EQ(si_num, 1);
  EXPECT_EQ(slot_id1, SLOT_ID_TEST_VAL_1);
  EXPECT_EQ(slot_id2, 0);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_1);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_2);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  EXPECT_EQ(si_num, 2);
  EXPECT_EQ(slot_id1, SLOT_ID_TEST_VAL_1);
  EXPECT_EQ(slot_id2, SLOT_ID_TEST_VAL_2);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_1);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_2);
  glo_map_set_slot_id(FCN_TEST_VAL, SLOT_ID_TEST_VAL_2 + 1);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  EXPECT_EQ(si_num, 2);
  EXPECT_EQ(slot_id1, SLOT_ID_TEST_VAL_1);
  EXPECT_EQ(slot_id2, SLOT_ID_TEST_VAL_2);
}

}  // namespace
