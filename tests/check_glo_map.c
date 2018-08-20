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

#include <check.h>

#include <swiftnav/glo_map.h>
#include <swiftnav/logging.h>
#include "check_suites.h"

#define FCN_TEST_VAL 14
#define SLOT_ID_TEST_VAL_1 5
#define SLOT_ID_TEST_VAL_2 10

static void glo_map_lock(void) {}
static void glo_map_unlock(void) {}

START_TEST(test_glo_map) {
  /* We do not test thread safety here.
     Therefore lock & unlock functions are just stubs. */
  glo_map_init(glo_map_lock, glo_map_unlock);

  for (u16 i = 1; i <= NUM_SATS_GLO; i++) {
    me_gnss_signal_t glo_mesid = construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL);
    gnss_signal_t glo_sid = construct_sid(CODE_GLO_L1OF, i);
    glo_map_set_slot_id(glo_mesid, i);

    fail_if(glo_map_valid(glo_sid) == false, "GLO mapping should be valid");

    u16 fcn = glo_map_get_fcn(glo_sid);
    fail_if(fcn != FCN_TEST_VAL,
            "Incorrect GLO FCN mapping (have, expected): %d, %d",
            fcn,
            FCN_TEST_VAL);

    glo_map_clear_slot_id(i);

    fail_if(glo_map_valid(glo_sid) == true, "GLO mapping should be invalid");
  }

  u16 slot_id1, slot_id2;
  u8 si_num;
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  fail_if(si_num != 0 || slot_id1 != 0 || slot_id2 != 0,
          "Incorrect number of slot ID or slot ID\n"
          "Number of slot ID (have, expected) %d, 0\n"
          "slot_id1 (have, expected): %d, 0\n"
          "slot_id2 (have, expected): %d, 0",
          si_num,
          slot_id1,
          slot_id2);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_1);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  fail_if(si_num != 1 || slot_id1 != SLOT_ID_TEST_VAL_1 || slot_id2 != 0,
          "Incorrect number of slot ID or slot ID\n"
          "Number of slot ID (have, expected) %d, 1\n"
          "slot_id1 (have, expected): %d, %d\n"
          "slot_id2 (have, expected): %d, 0",
          si_num,
          slot_id1,
          SLOT_ID_TEST_VAL_1,
          slot_id2);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_1);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_2);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  fail_if(si_num != 2 || slot_id1 != SLOT_ID_TEST_VAL_1 ||
              slot_id2 != SLOT_ID_TEST_VAL_2,
          "Incorrect number of slot ID or slot ID\n"
          "Number of slot ID (have, expected) %d, 2\n"
          "slot_id1 (have, expected): %d, %d\n,"
          "slot_id2 (have, expected): %d, %d",
          si_num,
          slot_id1,
          SLOT_ID_TEST_VAL_1,
          slot_id2,
          SLOT_ID_TEST_VAL_2);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_1);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_2);
  glo_map_set_slot_id(construct_mesid(CODE_GLO_L1OF, FCN_TEST_VAL),
                      SLOT_ID_TEST_VAL_2 + 1);
  si_num = glo_map_get_slot_id(FCN_TEST_VAL, &slot_id1, &slot_id2);
  fail_if(si_num != 2 || slot_id1 != SLOT_ID_TEST_VAL_1 ||
              slot_id2 != SLOT_ID_TEST_VAL_2,
          "Incorrect number of slot ID or slot ID\n"
          "Number of slot ID (have, expected) %d, 2\n"
          "slot_id1 (have, expected): %d, %d\n,"
          "slot_id2 (have, expected): %d, %d",
          si_num,
          slot_id1,
          SLOT_ID_TEST_VAL_1,
          slot_id2,
          SLOT_ID_TEST_VAL_2);
}
END_TEST

Suite *glo_map_test_suite(void) {
  Suite *s = suite_create("GLO FCN map");
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_glo_map);
  suite_add_tcase(s, tc_core);

  return s;
}
