#include <check.h>
#include <stdlib.h>

#include "check_suites.h"

int main(void) {
  int number_failed;

  Suite *s = edc_suite();

  SRunner *sr = srunner_create(s);
  srunner_set_xml(sr, "test_results.xml");

  srunner_add_suite(sr, almanac_suite());
  srunner_add_suite(sr, bits_suite());
  srunner_add_suite(sr, edc_suite());
  srunner_add_suite(sr, ionosphere_suite());
  srunner_add_suite(sr, coord_system_suite());
  srunner_add_suite(sr, linear_algebra_suite());
  srunner_add_suite(sr, troposphere_suite());
  srunner_add_suite(sr, ephemeris_suite());
  srunner_add_suite(sr, set_suite());
  srunner_add_suite(sr, gnss_time_test_suite());
  srunner_add_suite(sr, signal_test_suite());
  srunner_add_suite(sr, glo_map_test_suite());
  srunner_add_suite(sr, shm_suite());
  srunner_add_suite(sr, pvt_test_suite());
  srunner_add_suite(sr, nav_meas_test_suite());
  srunner_add_suite(sr, sid_set_test_suite());

  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
