#include <check.h>
#include <stdarg.h>
#include <stdio.h>
#include <swiftnav/logging.h>

#include "check_suites.h"
#define MAX_STR 1024

/*globals for test */
static char out_str[MAX_STR];
char *ptr = out_str;
int last_level = 0;

static void reset_log(void) {
  ptr = out_str;
  memset(out_str, 0, MAX_STR);
}

static void test_log(int level, const char *msg, ...) {
  va_list ap;
  va_start(ap, msg);
  ptr += vsprintf(ptr, msg, ap);
  va_end(ap);
  ptr += sprintf(ptr, "\n");
  last_level = level;
}

static void test_detailed_log(int level,
                              const char *file_path,
                              const int line_number,
                              const char *msg,
                              ...) {
  (void)level;
  (void)file_path;
  (void)line_number;
  (void)msg;
}

START_TEST(test_logging) {
  /* check ptr arithmetic in print and null terminatino */
  int expected_len = 11;
  logging_set_implementation(test_log, test_detailed_log);
  log_info("log_info_1");
  fail_unless((ptr - out_str) == expected_len,
              "log_info macro failed length check. got %zu, should be %i. %s",
              (size_t)(ptr - out_str),
              expected_len,
              out_str);
  fail_unless(
      strnlen(out_str, MAX_STR) == (size_t)expected_len,
      "log_info macro failed length check. strlen was %zu, should be %i. %s",
      strnlen(out_str, MAX_STR),
      expected_len,
      out_str);
  reset_log();

  /* test log with rate limit based upon arbitrary tics. no need to check
   * pointer as it was done above*/
  expected_len = (14 * 4); /*should print 4 times. line length is 14.*/

  for (int i = 0; i < 4000; i++) {
    LOG_RATE_LIMIT(i, log_info("log_rate_%04d", i));
  }
  fail_unless(
      strnlen(out_str, MAX_STR) == (size_t)expected_len,
      "log_rate_limit macro failed length check. got %zu, should be %i. %s",
      strnlen(out_str, MAX_STR),
      expected_len,
      out_str);
  reset_log();
  /* if we somehow go back in time, should print again*/
  expected_len = 2 * 14; /* should print 2 times. String is length 14. */
  for (int j = 2000; j > 0; j -= 1500) {
    LOG_RATE_LIMIT(j, log_info("log_rate_%04d", j));
  }
  fail_unless(
      strnlen(out_str, MAX_STR) == (size_t)expected_len,
      "log_rate_limit macro  back in time failed length check. got %zu, "
      "should be %i. %s",
      strnlen(out_str, MAX_STR),
      expected_len,
      out_str);
  reset_log();
  /* if we wrap, arithmetic rules should just work */
  expected_len = (20 * 4); /* should print 4 times.*/
  for (int i = 0xffffffff - 1998; i < 1999; i++) {
    LOG_RATE_LIMIT(i, log_info("log_rate_%010u", i));
  }
  fail_unless(strnlen(out_str, MAX_STR) == (size_t)expected_len,
              "log_rate_limit macro wrap failed length check. got %zu, should "
              "be %i. %s",
              strnlen(out_str, MAX_STR),
              expected_len,
              out_str);
}
END_TEST

Suite *log_suite(void) {
  Suite *s = suite_create("Logging");

  TCase *tc_log = tcase_create("logging");
  tcase_add_test(tc_log, test_logging);
  suite_add_tcase(s, tc_log);

  return s;
}
