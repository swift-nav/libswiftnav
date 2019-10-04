#include <check.h>
#include <math.h>
#include <time.h>

#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include "check_suites.h"
#include "common/check_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

START_TEST(test_gpstime_comparison_ops) {
  gps_time_t a = {567890.0, 1234}, b = {567890.5, 1234}, c = {567890.0, 1234};
  fail_unless(a < b, "operator< failed");
  fail_unless(b > a, "operator> failed");
  fail_unless(a == c, "operator== failed");
  fail_unless(a <= b, "operator<= failed");
  fail_unless(b >= a, "operator>= failed");
  fail_unless(a >= c, "operator>= failed");
  fail_unless(a <= c, "operator<= failed");
}
END_TEST

Suite *gnss_time_cpp_test_suite(void) {
  Suite *s = suite_create("GPS Time C++ operators");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_gpstime_comparison_ops);
  suite_add_tcase(s, tc_core);

  return s;
}

#ifdef __cplusplus
} /* extern "C" */
#endif