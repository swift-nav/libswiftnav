#include <gtest/gtest.h>
#include <math.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <time.h>

#include "check_utils.h"

#define GPS_TIME_TOL (1.0e-9)

namespace {

TEST(GnssTimeCppOperators, GpstimeComparisonOps) {
  gps_time_t a = {567890.0, 1234};
  gps_time_t b = {567890.5, 1234};
  gps_time_t c = {567890.0, 1234};
  gps_time_t time_before_rollover = {WEEK_SECS - 1.0, 1234};
  gps_time_t time_after_rollover{1.0, 1234};
  gps_time_t time_unknown_wn_only{1.0, WN_UNKNOWN};
  gps_time_t time_unknown_tow_only{TOW_UNKNOWN, 1234};
  gps_time_t time_unknown_wn_a{0.0, WN_UNKNOWN};
  gps_time_t time_unknown_wn_b{WEEK_SECS - 1.0, WN_UNKNOWN};
  gps_time_t time_unknown_tow_a{TOW_UNKNOWN, 1234};
  gps_time_t time_unknown_tow_b{TOW_UNKNOWN, 1235};
  EXPECT_LE(a, b);
  EXPECT_GT(b, a);
  EXPECT_NE(a, b);
  EXPECT_NE(b, a);
  EXPECT_EQ(a, c);
  EXPECT_LE(a, b);
  EXPECT_GE(b, a);
  EXPECT_GE(a, c);
  EXPECT_LE(a, c);
  EXPECT_EQ(GPS_TIME_UNKNOWN, GPS_TIME_UNKNOWN);
  EXPECT_NE(time_before_rollover, GPS_TIME_UNKNOWN);
  EXPECT_NE(GPS_TIME_UNKNOWN, time_before_rollover);
  EXPECT_NE(time_after_rollover, GPS_TIME_UNKNOWN);
  EXPECT_NE(GPS_TIME_UNKNOWN, time_after_rollover);
  EXPECT_NE(time_unknown_wn_only, GPS_TIME_UNKNOWN);
  EXPECT_NE(GPS_TIME_UNKNOWN, time_unknown_wn_only);
  EXPECT_NE(time_unknown_tow_only, GPS_TIME_UNKNOWN);
  EXPECT_NE(GPS_TIME_UNKNOWN, time_unknown_tow_only);
  EXPECT_NE(time_unknown_wn_a, time_unknown_wn_b);
  EXPECT_NE(time_unknown_wn_b, time_unknown_wn_a);
  EXPECT_NE(time_unknown_tow_a, time_unknown_tow_b);
  EXPECT_NE(time_unknown_tow_b, time_unknown_tow_a);
}

TEST(GnssTimeCppOperators, GpstimeOperatorMinus) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  EXPECT_TRUE(fabs((b - a) - 0.5) < GPS_TIME_TOL);
  EXPECT_TRUE(fabs((b - 0.5) - a) < GPS_TIME_TOL);
}

TEST(GnssTimeCppOperators, GpstimeOperatorPlus) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  EXPECT_TRUE(fabs((a + 0.5) - b) < GPS_TIME_TOL);
  EXPECT_TRUE(fabs((0.5 + a) - b) < GPS_TIME_TOL);
}

TEST(GnssTimeCppOperators, GpstimeOperatorPlusAssignment) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  a += 0.5;
  EXPECT_TRUE(fabs(a - b) < GPS_TIME_TOL);
}

TEST(GnssTimeCppOperators, GpstimeOperatorMinusAssignment) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  b -= 0.5;
  EXPECT_TRUE(fabs(a - b) < GPS_TIME_TOL);
}

}  // namespace
