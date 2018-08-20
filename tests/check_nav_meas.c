#include <check.h>
#include <float.h>
#include <inttypes.h>
#include <stdio.h>

#include <swiftnav/nav_meas.h>
#include "check_suites.h"

START_TEST(test_encode_lock_time) {
  u8 ret;

  ret = encode_lock_time(0.0);
  fail_unless(ret == 0, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 0);

  ret = encode_lock_time(0.05);
  fail_unless(ret == 1, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 1);

  ret = encode_lock_time(0.1);
  fail_unless(ret == 2, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 2);

  ret = encode_lock_time(0.2);
  fail_unless(ret == 3, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 3);

  ret = encode_lock_time(0.5);
  fail_unless(ret == 4, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 4);

  ret = encode_lock_time(1.0);
  fail_unless(ret == 5, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 5);

  ret = encode_lock_time(2.0);
  fail_unless(ret == 6, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 6);

  ret = encode_lock_time(4.0);
  fail_unless(ret == 7, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 7);

  ret = encode_lock_time(5.0);
  fail_unless(ret == 8, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 8);

  ret = encode_lock_time(10.0);
  fail_unless(ret == 9, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 9);

  ret = encode_lock_time(20.0);
  fail_unless(
      ret == 10, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 10);

  ret = encode_lock_time(50.0);
  fail_unless(
      ret == 11, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 11);

  ret = encode_lock_time(100.0);
  fail_unless(
      ret == 12, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 12);

  ret = encode_lock_time(200.0);
  fail_unless(
      ret == 13, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 13);

  ret = encode_lock_time(500.0);
  fail_unless(
      ret == 14, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 14);

  ret = encode_lock_time(1000.0);
  fail_unless(
      ret == 15, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 15);

  ret = encode_lock_time(DBL_MAX);
  fail_unless(
      ret == 15, "Incorrect return (%" PRIu8 " vs %" PRIu8 ")", ret, 15);
}
END_TEST

START_TEST(test_decode_lock_time) {
  double ret;

  ret = decode_lock_time(0);
  fail_unless(ret == 0.0, "Incorrect return (%f vs %f)", ret, 0.0);

  ret = decode_lock_time(0xF0);
  fail_unless(ret == 0.0, "Incorrect return (%f vs %f)", ret, 0.0);

  ret = decode_lock_time(1);
  fail_unless(ret == 0.032, "Incorrect return (%f vs %f)", ret, 0.032);

  ret = decode_lock_time(2);
  fail_unless(ret == 0.064, "Incorrect return (%f vs %f)", ret, 0.064);

  ret = decode_lock_time(3);
  fail_unless(ret == 0.128, "Incorrect return (%f vs %f)", ret, 0.128);

  ret = decode_lock_time(4);
  fail_unless(ret == 0.256, "Incorrect return (%f vs %f)", ret, 0.256);

  ret = decode_lock_time(5);
  fail_unless(ret == 0.512, "Incorrect return (%f vs %f)", ret, 0.512);

  ret = decode_lock_time(6);
  fail_unless(ret == 1.024, "Incorrect return (%f vs %f)", ret, 1.024);

  ret = decode_lock_time(7);
  fail_unless(ret == 2.048, "Incorrect return (%f vs %f)", ret, 2.048);

  ret = decode_lock_time(8);
  fail_unless(ret == 4.096, "Incorrect return (%f vs %f)", ret, 4.096);

  ret = decode_lock_time(9);
  fail_unless(ret == 8.192, "Incorrect return (%f vs %f)", ret, 8.192);

  ret = decode_lock_time(10);
  fail_unless(ret == 16.384, "Incorrect return (%f vs %f)", ret, 16.384);

  ret = decode_lock_time(11);
  fail_unless(ret == 32.768, "Incorrect return (%f vs %f)", ret, 32.768);

  ret = decode_lock_time(12);
  fail_unless(ret == 65.536, "Incorrect return (%f vs %f)", ret, 65.536);

  ret = decode_lock_time(13);
  fail_unless(ret == 131.072, "Incorrect return (%f vs %f)", ret, 131.072);

  ret = decode_lock_time(14);
  fail_unless(ret == 262.144, "Incorrect return (%f vs %f)", ret, 262.144);

  ret = decode_lock_time(15);
  fail_unless(ret == 524.288, "Incorrect return (%f vs %f)", ret, 524.288);
}
END_TEST

START_TEST(test_roundtrip_lock_time) {
  const double value_to_encode = 260.0;
  u8 encoded_value;
  double decoded_value;

  encoded_value = encode_lock_time(value_to_encode);
  decoded_value = decode_lock_time(encoded_value);

  fail_unless(encoded_value == 13,
              "Incorrect return (%" PRIu8 " vs %" PRIu8 ")",
              encoded_value,
              13);

  fail_unless(decoded_value == 131.072,
              "Incorrect return (%f vs %f)",
              decoded_value,
              131.072);

  fail_unless(decoded_value < value_to_encode,
              "Minimum lock time not less than original lock time (%f < %f)",
              decoded_value,
              value_to_encode);
}
END_TEST

Suite *nav_meas_test_suite(void) {
  Suite *s = suite_create("Navigation Measurement");
  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_encode_lock_time);
  tcase_add_test(tc_core, test_decode_lock_time);
  tcase_add_test(tc_core, test_roundtrip_lock_time);
  suite_add_tcase(s, tc_core);
  return s;
}
