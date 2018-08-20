#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "check_suites.h"
#include "check_utils.h"

#include <swiftnav/sid_set.h>

START_TEST(test_sid_set_empty) {
  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);
  u32 count = sid_set_get_sat_count(&sid_set);

  fail_unless(count == 0, "count was not 0. Saw %u", count);
}
END_TEST

START_TEST(test_sid_set) {
  gnss_signal_t sids[] = {
      {.code = CODE_GPS_L1CA, .sat = 1},
      {.code = CODE_GPS_L2CM, .sat = 1},
      {.code = CODE_GPS_L1CA, .sat = 2},
      {.code = CODE_GPS_L2CM, .sat = 3},
      {.code = CODE_GLO_L1OF, .sat = 1},
  };

  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);

  for (u32 i = 0; i < sizeof(sids) / sizeof(sids[0]); i++) {
    const gnss_signal_t sid = sids[i];
    sid_set_add(&sid_set, sid);
    fail_unless(sid_set_get_sig_count(&sid_set) == i + 1);
  }

  u32 count = sid_set_get_sat_count(&sid_set);

  fail_unless(count == 4, "count was not 4. Saw %u", count);
}
END_TEST

START_TEST(test_sid_set_contains) {
  gnss_signal_t sids[] = {
      {.code = CODE_GPS_L1CA, .sat = 1},
      {.code = CODE_GPS_L2CM, .sat = 1},
      {.code = CODE_GPS_L1CA, .sat = 2},
      {.code = CODE_GPS_L2CM, .sat = 3},
      {.code = CODE_GLO_L1OF, .sat = 1},
  };

  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);

  /* check that add works by adding sids one by one */
  for (u32 i = 0; i < sizeof(sids) / sizeof(sids[0]); i++) {
    const gnss_signal_t sid = sids[i];
    fail_unless(sid_set_contains(&sid_set, sid) == false,
                "%d: sid_set should not contain (code %d, sat %d)",
                i,
                sid.code,
                sid.sat);
    sid_set_add(&sid_set, sid);
    fail_unless(sid_set_contains(&sid_set, sid) == true,
                "%d: sid_set should contain (code %d, sat %d)",
                i,
                sid.code,
                sid.sat);
  }

  /* check that remove works by removing sids one by one */
  for (u32 i = 0; i < sizeof(sids) / sizeof(sids[0]); i++) {
    const gnss_signal_t sid = sids[i];
    fail_unless(sid_set_contains(&sid_set, sid) == true,
                "%d: sid_set should contain (code %d, sat %d)",
                i,
                sid.code,
                sid.sat);
    sid_set_remove(&sid_set, sid);
    fail_unless(sid_set_contains(&sid_set, sid) == false,
                "%d: sid_set should not contain (code %d, sat %d)",
                i,
                sid.code,
                sid.sat);
  }
}
END_TEST

Suite *sid_set_test_suite(void) {
  Suite *s = suite_create("SID Set");
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_sid_set_empty);
  tcase_add_test(tc_core, test_sid_set);
  tcase_add_test(tc_core, test_sid_set_contains);
  suite_add_tcase(s, tc_core);
  return s;
}
