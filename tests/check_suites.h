#ifndef CHECK_SUITES_H
#define CHECK_SUITES_H

#ifdef __cplusplus
extern "C" {
#endif

Suite* almanac_suite(void);
Suite* coord_system_suite(void);
Suite* bits_suite(void);
Suite* edc_suite(void);
Suite* linear_algebra_suite(void);
Suite* ephemeris_suite(void);
Suite* decode_glo_suite(void);
Suite* ionosphere_suite(void);
Suite* set_suite(void);
Suite* gnss_time_test_suite(void);
Suite* gnss_time_cpp_test_suite(void);
Suite* signal_test_suite(void);
Suite* glo_map_test_suite(void);
Suite* shm_suite(void);
Suite* troposphere_suite(void);
Suite* pvt_test_suite(void);
Suite* nav_meas_test_suite(void);
Suite* nav_meas_calc_test_suite(void);
Suite* sid_set_test_suite(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CHECK_SUITES_H */
