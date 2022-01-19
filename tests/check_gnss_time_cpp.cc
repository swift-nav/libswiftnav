#include <check.h>
#include <math.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <time.h>

#include "check_suites.h"
#include "common/check_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPS_TIME_TOL (1.0e-9)

START_TEST(test_gpstime_comparison_ops) {
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
  fail_unless(a < b, "operator< failed");
  fail_unless(b > a, "operator> failed");
  fail_unless(!(a == b), "operator== failed");
  fail_unless(!(b == a), "operator== failed");
  fail_unless(a == c, "operator== failed");
  fail_unless(a <= b, "operator<= failed");
  fail_unless(b >= a, "operator>= failed");
  fail_unless(a >= c, "operator>= failed");
  fail_unless(a <= c, "operator<= failed");
  fail_unless(GPS_TIME_UNKNOWN == GPS_TIME_UNKNOWN, "operator== failed");
  fail_unless(!(time_before_rollover == GPS_TIME_UNKNOWN), "operator== failed");
  fail_unless(!(GPS_TIME_UNKNOWN == time_before_rollover), "operator== failed");
  fail_unless(!(time_after_rollover == GPS_TIME_UNKNOWN), "operator== failed");
  fail_unless(!(GPS_TIME_UNKNOWN == time_after_rollover), "operator== failed");
  fail_unless(!(time_unknown_wn_only == GPS_TIME_UNKNOWN), "operator== failed");
  fail_unless(!(GPS_TIME_UNKNOWN == time_unknown_wn_only), "operator== failed");
  fail_unless(!(time_unknown_tow_only == GPS_TIME_UNKNOWN),
              "operator== failed");
  fail_unless(!(GPS_TIME_UNKNOWN == time_unknown_tow_only),
              "operator== failed");
  fail_unless(!(time_unknown_wn_a == time_unknown_wn_b), "operator== failed");
  fail_unless(!(time_unknown_wn_b == time_unknown_wn_a), "operator== failed");
  fail_unless(!(time_unknown_tow_a == time_unknown_tow_b), "operator== failed");
  fail_unless(!(time_unknown_tow_b == time_unknown_tow_a), "operator== failed");
}
END_TEST

START_TEST(test_gpstime_operator_minus) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  fail_unless(fabs((b - a) - 0.5) < GPS_TIME_TOL, "operator- failed");
  fail_unless(fabs((b - 0.5) - a) < GPS_TIME_TOL, "operator- failed");
}
END_TEST

START_TEST(test_gpstime_operator_plus) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  fail_unless(fabs((a + 0.5) - b) < GPS_TIME_TOL, "operator+ failed");
  fail_unless(fabs((0.5 + a) - b) < GPS_TIME_TOL, "operator+ failed");
}
END_TEST

START_TEST(test_gpstime_operator_plus_assignment) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  a += 0.5;
  fail_unless(fabs(a - b) < GPS_TIME_TOL, "operator+= failed");
}
END_TEST

START_TEST(test_gpstime_operator_minus_assignment) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234};
  b -= 0.5;
  fail_unless(fabs(a - b) < GPS_TIME_TOL, "operator-= failed");
}
END_TEST

Suite *gnss_time_cpp_test_suite(void) {
  Suite *s = suite_create("GPS Time C++ operators");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_gpstime_comparison_ops);
  tcase_add_test(tc_core, test_gpstime_operator_minus);
  tcase_add_test(tc_core, test_gpstime_operator_plus);
  tcase_add_test(tc_core, test_gpstime_operator_plus_assignment);
  tcase_add_test(tc_core, test_gpstime_operator_minus_assignment);
  suite_add_tcase(s, tc_core);

  return s;
}

#ifdef __cplusplus
} /* extern "C" */
#endif