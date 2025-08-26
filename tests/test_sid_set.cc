#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swiftnav/sid_set.h>

#include "check_utils.h"

namespace {

TEST(TestSidSet, SidSetEmpty) {
  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);
  u32 count = sid_set_get_sat_count(&sid_set);

  EXPECT_EQ(count, 0);
}

TEST(TestSidSet, SidSet) {
  gnss_signal_t sids[] = {
      {.sat = 1, .code = CODE_GPS_L1CA},
      {.sat = 1, .code = CODE_GPS_L2CM},
      {.sat = 2, .code = CODE_GPS_L1CA},
      {.sat = 3, .code = CODE_GPS_L2CM},
      {.sat = 1, .code = CODE_GLO_L1OF},
  };

  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);

  for (u32 i = 0; i < sizeof(sids) / sizeof(sids[0]); i++) {
    const gnss_signal_t sid = sids[i];
    sid_set_add(&sid_set, sid);
    EXPECT_EQ(sid_set_get_sig_count(&sid_set), i + 1);
  }

  u32 count = sid_set_get_sat_count(&sid_set);

  EXPECT_EQ(count, 4);
}

TEST(TestSidSet, SidSetContains) {
  gnss_signal_t sids[] = {
      {.sat = 1, .code = CODE_GPS_L1CA},
      {.sat = 1, .code = CODE_GPS_L2CM},
      {.sat = 2, .code = CODE_GPS_L1CA},
      {.sat = 3, .code = CODE_GPS_L2CM},
      {.sat = 1, .code = CODE_GLO_L1OF},
  };

  gnss_sid_set_t sid_set;
  sid_set_init(&sid_set);

  /* check that add works by adding sids one by one */
  for (auto sid : sids) {
    EXPECT_FALSE(sid_set_contains(&sid_set, sid));
    sid_set_add(&sid_set, sid);
    EXPECT_TRUE(sid_set_contains(&sid_set, sid));
  }

  /* check that remove works by removing sids one by one */
  for (auto sid : sids) {
    EXPECT_TRUE(sid_set_contains(&sid_set, sid));
    sid_set_remove(&sid_set, sid);
    EXPECT_FALSE(sid_set_contains(&sid_set, sid));
  }
}

}  // namespace
