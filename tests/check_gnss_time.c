#include <check.h>
#include <math.h>
#include <time.h>

#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include "check_suites.h"
#include "check_utils.h"

START_TEST(test_gpsdifftime) {
  struct gpsdifftime_testcase {
    gps_time_t a, b;
    double dt;
  } testcases[] = {
      {.a = {567890.0, 1234}, .b = {567890.0, 1234}, .dt = 0},
      {.a = {567890.0, 1234}, .b = {0.0, 1234}, .dt = 567890},
      {.a = {567890.0, WN_UNKNOWN}, .b = {0.0, 1234}, .dt = -36910},
      {.a = {222222.0, 2222}, .b = {2222.0, WN_UNKNOWN}, .dt = 220000},
      {.a = {444444.0, WN_UNKNOWN}, .b = {2222.0, WN_UNKNOWN}, .dt = -162578},
      {.a = {604578.0, 1000}, .b = {222.222, 1001}, .dt = -444.222},
      {.a = {604578.0, 1001}, .b = {222.222, 1000}, .dt = 1209155.778},
  };
  const double tow_tol = 1e-10;
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct gpsdifftime_testcase);
       i++) {
    double dt = gpsdifftime(&testcases[i].a, &testcases[i].b);
    fail_unless(fabs(dt - testcases[i].dt) < tow_tol,
                "gpsdifftime test case %d failed, dt = %.12f",
                i,
                dt);
  }
}
END_TEST

START_TEST(test_normalize_gps_time) {
  gps_time_t testcases[] = {{0, 1234},
                            {3 * DAY_SECS, 1234},
                            {WEEK_SECS + DAY_SECS, 1234},
                            {0 - DAY_SECS, 1234},
                            {DAY_SECS, WN_UNKNOWN},
                            {WEEK_SECS + 1, WN_UNKNOWN}};
  const double tow_tol = 1e-10;
  for (size_t i = 0; i < sizeof(testcases) / sizeof(gps_time_t); i++) {
    double t_original = testcases[i].wn * WEEK_SECS + testcases[i].tow;
    s16 wn = testcases[i].wn;
    normalize_gps_time(&testcases[i]);
    double t_normalized = testcases[i].wn * WEEK_SECS + testcases[i].tow;
    if (testcases[i].wn != WN_UNKNOWN) {
      fail_unless(fabs(t_original - t_normalized) < tow_tol,
                  "normalize_gps_time test case %d failed, t_original = %.12f, "
                  "t_normalized = %.12f",
                  i,
                  t_original,
                  t_normalized);
    }
    /* normalization must not touch unknown week number */
    fail_unless(wn != WN_UNKNOWN || testcases[i].wn == WN_UNKNOWN);
  }
}
END_TEST

/* test that the make_utc_tm conversion matches gmtime */
void gmtime_test(const char *name, time_t start, time_t end, time_t step) {
  time_t t_gps = start;
  while (t_gps < end) {
    time_t t_unix = t_gps + GPS_EPOCH;
    struct tm *date = gmtime(&t_unix);

    gps_time_t t = {.wn = t_gps / WEEK_SECS, .tow = t_gps % WEEK_SECS};

    utc_tm u;
    make_utc_tm(&t, &u);

    fail_unless((date->tm_year + 1900) == u.year,
                "%s, expected year %d, got %d, time = %s",
                name,
                date->tm_year + 1900,
                u.year,
                asctime(date));
    fail_unless((date->tm_mon + 1) == u.month,
                "%s, expected month %d, got %d, time = %s",
                name,
                date->tm_mon + 1,
                u.month,
                asctime(date));
    fail_unless((date->tm_yday + 1) == u.year_day,
                "%s, expected year_day %d, got %d, time = %s",
                name,
                date->tm_yday + 1,
                u.year_day,
                asctime(date));
    fail_unless(date->tm_mday == u.month_day,
                "%s, expected month_day %d, got %d, time = %s",
                name,
                date->tm_mday,
                u.month_day,
                asctime(date));
    fail_unless(date->tm_wday == (u.week_day % 7),
                "%s, expected week_day %d, got %d, time = %s",
                name,
                date->tm_wday,
                u.week_day % 7,
                asctime(date));
    fail_unless(date->tm_hour == u.hour,
                "%s, expected hour %d, got %d, time = %s",
                name,
                date->tm_hour,
                u.hour,
                asctime(date));
    fail_unless(date->tm_min == u.minute,
                "%s, expected minute %d, got %d, time = %s",
                name,
                date->tm_min,
                u.minute,
                asctime(date));
    fail_unless(date->tm_sec == u.second_int,
                "%s, expected second_int %d, got %d, time = %s",
                name,
                date->tm_sec,
                u.second_int,
                asctime(date));
    fail_unless(0.0 == u.second_frac,
                "%s, expected second_frac %f, got %f, time = %s",
                name,
                0.0,
                u.second_frac,
                asctime(date));

    t_gps += step;
  }
}

START_TEST(test_gps2utc_time) {
  /* test make_utc_tm for Jan 6 1980 in 1 s increments */
  gmtime_test("make_utc_tm_day", 0, 1 * DAY_SECS + 1, 1);
}
END_TEST

START_TEST(test_gps2utc_date) {
  /* test make_utc_tm from 1980 to 2080 with (1 day + 1 s) increments*/
  gmtime_test("make_utc_tm_century", 0, 100L * 365 * DAY_SECS, DAY_SECS + 1);
}
END_TEST

START_TEST(test_gps_time_match_weeks) {
  struct gps_time_match_weeks_testcase {
    gps_time_t t, ref, ret;
  } testcases[] = {
      {.t = {0.0, WN_UNKNOWN}, .ref = {0.0, 1234}, .ret = {0.0, 1234}},
      {.t = {WEEK_SECS - 1, WN_UNKNOWN},
       .ref = {0.0, 1234},
       .ret = {WEEK_SECS - 1, 1233}},
      {.t = {0.0, WN_UNKNOWN},
       .ref = {WEEK_SECS - 1, 1234},
       .ret = {0.0, 1235}},
      {.t = {WEEK_SECS - 1, WN_UNKNOWN},
       .ref = {WEEK_SECS - 1, 1234},
       .ret = {WEEK_SECS - 1, 1234}},
      {.t = {2 * DAY_SECS, WN_UNKNOWN},
       .ref = {5 * DAY_SECS, 1234},
       .ret = {2 * DAY_SECS, 1234}},
      {.t = {5 * DAY_SECS, WN_UNKNOWN},
       .ref = {2 * DAY_SECS, 1234},
       .ret = {5 * DAY_SECS, 1234}},
      {.t = {0.0, WN_UNKNOWN},
       .ref = {WEEK_SECS / 2, 1234},
       .ret = {0.0, 1234}},
      {.t = {WEEK_SECS / 2, WN_UNKNOWN},
       .ref = {0.0, 1234},
       .ret = {WEEK_SECS / 2, 1234}},
      {.t = {WEEK_SECS / 2 + 1, WN_UNKNOWN},
       .ref = {0.0, 1234},
       .ret = {WEEK_SECS / 2 + 1, 1233}},
      {.t = {0.0, WN_UNKNOWN},
       .ref = {WEEK_SECS / 2 + 1, 1234},
       .ret = {0.0, 1235}},
      {.t = {DAY_SECS, WN_UNKNOWN},
       .ref = {2 * DAY_SECS, WN_UNKNOWN},
       .ret = {DAY_SECS, WN_UNKNOWN}},
      {.t = {DAY_SECS, WN_UNKNOWN},
       .ref = {6 * DAY_SECS, WN_UNKNOWN},
       .ret = {DAY_SECS, WN_UNKNOWN}},
  };
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct gps_time_match_weeks_testcase);
       i++) {
    gps_time_match_weeks(&testcases[i].t, &testcases[i].ref);
    fail_unless(
        testcases[i].t.wn == testcases[i].ret.wn,
        "gps_time_match_weeks test case %d failed, t.wn = %d, ret.wn = %d",
        i,
        testcases[i].t.wn,
        testcases[i].ret.wn);
    fail_unless(testcases[i].t.tow == testcases[i].ret.tow,
                "gps_time_match_weeks test case %d failed, t.tow = %.12f, "
                "ret.tow = %.12f",
                i,
                testcases[i].t.tow,
                testcases[i].ret.tow);
  }
}
END_TEST

START_TEST(test_gps_adjust_week_cycle) {
  struct gps_adjust_week_cycle_testcase {
    u16 wn_raw, ret;
  } testcases[] = {
      {.wn_raw = 0, .ret = 2048},
      {.wn_raw = 1023, .ret = 2047},
      {.wn_raw = GPS_WEEK_REFERENCE % 1024, .ret = GPS_WEEK_REFERENCE},
      {.wn_raw = GPS_WEEK_REFERENCE % 1024 + 1, .ret = GPS_WEEK_REFERENCE + 1},
      {.wn_raw = GPS_WEEK_REFERENCE % 1024 - 1,
       .ret = GPS_WEEK_REFERENCE + 1023},
      {.wn_raw = GPS_WEEK_REFERENCE, .ret = GPS_WEEK_REFERENCE},
      {.wn_raw = GPS_WEEK_REFERENCE + 1, .ret = GPS_WEEK_REFERENCE + 1},
  };
  const u16 wn_ref = GPS_WEEK_REFERENCE;
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct gps_adjust_week_cycle_testcase);
       i++) {
    u16 wn = gps_adjust_week_cycle(testcases[i].wn_raw, wn_ref);
    fail_unless(wn == testcases[i].ret,
                "gps_adjust_week_cycle test case %d failed, wn = %d, ret = %d",
                i,
                wn,
                testcases[i].ret);
  }
}
END_TEST

START_TEST(test_is_leap_year) {
  struct is_leap_year_testcase {
    u16 year;
    bool ret;
  } testcases[] = {
      {.year = 1900, .ret = false}, {.year = 1901, .ret = false},
      {.year = 1904, .ret = true},  {.year = 1980, .ret = true},
      {.year = 1981, .ret = false}, {.year = 1982, .ret = false},
      {.year = 1983, .ret = false}, {.year = 1984, .ret = true},
      {.year = 1985, .ret = false}, {.year = 1986, .ret = false},
      {.year = 1987, .ret = false}, {.year = 1988, .ret = true},
      {.year = 1989, .ret = false}, {.year = 1990, .ret = false},
      {.year = 1991, .ret = false}, {.year = 1992, .ret = true},
      {.year = 1993, .ret = false}, {.year = 1994, .ret = false},
      {.year = 1995, .ret = false}, {.year = 1996, .ret = true},
      {.year = 1997, .ret = false}, {.year = 1998, .ret = false},
      {.year = 1999, .ret = false}, {.year = 2000, .ret = true},
      {.year = 2001, .ret = false}, {.year = 2002, .ret = false},
      {.year = 2003, .ret = false}, {.year = 2004, .ret = true},
      {.year = 2005, .ret = false}, {.year = 2006, .ret = false},
      {.year = 2007, .ret = false}, {.year = 2008, .ret = true},
      {.year = 2009, .ret = false}, {.year = 2010, .ret = false},
      {.year = 2011, .ret = false}, {.year = 2012, .ret = true},
      {.year = 2013, .ret = false}, {.year = 2014, .ret = false},
      {.year = 2015, .ret = false}, {.year = 2016, .ret = true},
      {.year = 2017, .ret = false}, {.year = 2018, .ret = false},
      {.year = 2019, .ret = false}, {.year = 2020, .ret = true},
  };

  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct is_leap_year_testcase);
       i++) {
    fail_unless(is_leap_year(testcases[i].year) == testcases[i].ret,
                "is_leap_year test case %d failed, year = %d",
                testcases[i].year);
  }
}
END_TEST

START_TEST(test_glo2gps) {
  struct glo2gps_testcase {
    glo_time_t glot;
    gps_time_t ret;
  } testcases[] = {
      {.glot = {.nt = 0, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = GPS_TIME_UNKNOWN},
      {.glot = {.nt = 1462, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = GPS_TIME_UNKNOWN},
      {.glot = {.nt = 1461, .n4 = 0, .h = 12, .m = 12, .s = 12},
       .ret = GPS_TIME_UNKNOWN},
      {.glot = {.nt = 1461, .n4 = 32, .h = 12, .m = 12, .s = 12},
       .ret = GPS_TIME_UNKNOWN},
      /* GLO time 29th Dec 2000 01:00:00 */
      {.glot = {.nt = 364, .n4 = 2, .h = 1, .m = 0, .s = 0},
       .ret = {.wn = 1094, .tow = 424813}},
      /* GLO time 30th Dec 2000 01:00:00 */
      {.glot = {.nt = 365, .n4 = 2, .h = 1, .m = 0, .s = 0},
       .ret = {.wn = 1094, .tow = 511213}},
      /* GLO time 31st Dec 2000 02:00:00 */
      {.glot = {.nt = 366, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.wn = 1094, .tow = 601213}},
      /* GLO time 1st Jan  2001 02:00:00 */
      {.glot = {.nt = 367, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.wn = 1095, .tow = 82813}},
      /* GLO time 2nd Jan  2001 02:00:00 */
      {.glot = {.nt = 368, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.wn = 1095, .tow = 169213}},
      /* GLO time 31st Dec 2009 12:12:12 */
      {.glot = {.nt = 731, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.wn = 1564, .tow = 378747}},
      /* GLO time 31st Dec 2010 12:12:12 */
      {.glot = {.nt = 1096, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.wn = 1616, .tow = 465147}},
      /* GLO time 31st Dec 2011 12:12:12 */
      {.glot = {.nt = 1461, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.wn = 1668, .tow = 551547}},
      /* GLO time 1st Jan 2017 02:59:59 */
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 59},
       .ret = {.wn = 1930, .tow = 16}},
      /* GLO time 1st Jan 2017 02:59:59.5 */
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 59.5},
       .ret = {.wn = 1930, .tow = 16.5}},
      /* GLO time 1st Jan 2017 02:59:60  (leap second)*/
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 60},
       .ret = {.wn = 1930, .tow = 17}},
      /* GLO time 1st Jan 2017 02:59:60.5  (leap second)*/
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 60.5},
       .ret = {.wn = 1930, .tow = 17.5}},
      /* GLO time 1st Jan 2017 03:00:00 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 0},
       .ret = {.wn = 1930, .tow = 18}},
      /* GLO time 1st Jan 2017 03:00:01 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 1},
       .ret = {.wn = 1930, .tow = 19}},
      /* GLO time 1st Jan 2017 03:00:02 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 2},
       .ret = {.wn = 1930, .tow = 20}},
      /* GLO time 1st Jan 2017 03:01:00 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 1, .s = 0},
       .ret = {.wn = 1930, .tow = 78}},
  };
  for (size_t i = 0; i < sizeof(testcases) / sizeof(struct glo2gps_testcase);
       i++) {
    gps_time_t ret = glo2gps(&testcases[i].glot, /* utc_params = */ NULL);
    fail_unless(
        ret.wn == testcases[i].ret.wn && ret.tow == testcases[i].ret.tow,
        "glo2gps test case %d failed, got (%d, %f), expected (%d, %f)",
        i,
        ret.wn,
        ret.tow,
        testcases[i].ret.wn,
        testcases[i].ret.tow);
    if (gps_time_valid(&ret)) {
      /* convert back to GLO time */
      glo_time_t glo = gps2glo(&ret, NULL);
      fail_unless(
          glo.n4 == testcases[i].glot.n4 && glo.nt == testcases[i].glot.nt &&
              glo.h == testcases[i].glot.h && glo.m == testcases[i].glot.m &&
              within_epsilon(glo.s, testcases[i].glot.s),
          "gps2glo test case %d failed, got (%d,%d,%d,%d,%f), expected "
          "(%d,%d,%d,%d,%f)",
          i,
          glo.n4,
          glo.nt,
          glo.h,
          glo.m,
          glo.s,
          testcases[i].glot.n4,
          testcases[i].glot.nt,
          testcases[i].glot.h,
          testcases[i].glot.m,
          testcases[i].glot.s);
    }
  }
}
END_TEST

START_TEST(test_utc_offset) {
  struct utc_offset_testcase {
    gps_time_t t;
    double dUTC;
    bool is_lse;
  } testcases[] = {
      /* July 1 1981 */
      {.t = {.wn = 77, .tow = 259199.0}, .dUTC = 0.0, .is_lse = false},
      {.t = {.wn = 77, .tow = 259199.5}, .dUTC = 0.0, .is_lse = false},
      {.t = {.wn = 77, .tow = 259200.0}, .dUTC = 0.0, .is_lse = true},
      {.t = {.wn = 77, .tow = 259200.5}, .dUTC = 0.0, .is_lse = true},
      {.t = {.wn = 77, .tow = 259201.0}, .dUTC = 1.0, .is_lse = false},
      {.t = {.wn = 77, .tow = 259202.0}, .dUTC = 1.0, .is_lse = false},
      /* Jan 1 2017 */
      {.t = {.wn = 1930, .tow = 16.0}, .dUTC = 17.0, .is_lse = false},
      {.t = {.wn = 1930, .tow = 16.5}, .dUTC = 17.0, .is_lse = false},
      {.t = {.wn = 1930, .tow = 17.0}, .dUTC = 17.0, .is_lse = true},
      {.t = {.wn = 1930, .tow = 17.5}, .dUTC = 17.0, .is_lse = true},
      {.t = {.wn = 1930, .tow = 18.0}, .dUTC = 18.0, .is_lse = false},
      {.t = {.wn = 1930, .tow = 18.5}, .dUTC = 18.0, .is_lse = false},
      {.t = {.wn = 1930, .tow = 19.0}, .dUTC = 18.0, .is_lse = false},
  };
  for (size_t i = 0; i < sizeof(testcases) / sizeof(struct utc_offset_testcase);
       i++) {
    double dUTC = get_gps_utc_offset(&testcases[i].t, NULL);
    double is_lse = is_leap_second_event(&testcases[i].t, NULL);

    fail_unless(dUTC == testcases[i].dUTC && is_lse == testcases[i].is_lse,
                "utc_leap_testcase %d failed, expected (%f,%d) got (%f,%d)",
                i,
                testcases[i].dUTC,
                testcases[i].is_lse,
                dUTC,
                is_lse);

    /* check that offset from the resulting UTC time back to GPS time matches,
     * except during the leap second event when the UTC time is ambiguous */
    if (!is_lse) {
      gps_time_t utc_time = {.wn = testcases[i].t.wn,
                             .tow = testcases[i].t.tow - dUTC};
      double dGPS = get_utc_gps_offset(&utc_time, NULL);
      fail_unless(dGPS == -testcases[i].dUTC,
                  "utc_leap_testcase inverse %d failed, expected %f got %f",
                  i,
                  -testcases[i].dUTC,
                  dGPS);
    }
  }
}
END_TEST

/* test a fictional leap second on 1st Jan 2020 */
/* note also the polynomial correction which shifts the time of effectivity */
static utc_params_t p_neg_offset = {.a0 = -0.125,
                                    .a1 = 0.0,
                                    .tot.wn = 2080,
                                    .tot.tow = 0,
                                    .t_lse.wn = 2086,
                                    .t_lse.tow = 259218.0 - 0.125,
                                    .dt_ls = 18,
                                    .dt_lsf = 19};
static utc_params_t p_pos_offset = {.a0 = +0.125,
                                    .a1 = 0.0,
                                    .tot.wn = 2080,
                                    .tot.tow = 0,
                                    .t_lse.wn = 2086,
                                    .t_lse.tow = 259218.0 + 0.125,
                                    .dt_ls = 18,
                                    .dt_lsf = 19};
static utc_params_t p_pos_trend = {
    .a0 = 0,
    .a1 = 1e-12,
    .tot.wn = 2080,
    .tot.tow = 0,
    .t_lse.wn = 2086,
    .t_lse.tow = 259218.0 + 1e-12 * (6 * WEEK_SECS + 259218.0),
    .dt_ls = 18,
    .dt_lsf = 19};
static utc_params_t p_neg_trend = {
    .a0 = 0,
    .a1 = -1e-12,
    .tot.wn = 2080,
    .tot.tow = 0,
    .t_lse.wn = 2086,
    .t_lse.tow = 259218.0 - 1e-12 * (6 * WEEK_SECS + 259218.0),
    .dt_ls = 18,
    .dt_lsf = 19};

START_TEST(test_utc_params) {
  struct utc_params_testcase {
    gps_time_t t;
    double dUTC;
    bool is_lse;
    utc_params_t *p;
  } testcases[] = {
      /* Jan 1 2020 (constant negative UTC offset) */
      {.t = {.wn = 2086, .tow = 259217.0 - 0.125},
       .dUTC = 18.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259217.5 - 0.125},
       .dUTC = 18.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259218.0 - 0.125},
       .dUTC = 18.0 - 0.125,
       .is_lse = true,
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259218.5 - 0.125},
       .dUTC = 18.0 - 0.125,
       .is_lse = true,
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259219.0 - 0.125},
       .dUTC = 19.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259219.5 - 0.125},
       .dUTC = 19.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      /* Jan 1 2020 (constant positive UTC offset) */
      {.t = {.wn = 2086, .tow = 259217.0 + 0.125},
       .dUTC = 18.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259217.5 + 0.125},
       .dUTC = 18.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259218.0 + 0.125},
       .dUTC = 18.0 + 0.125,
       .is_lse = true,
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259218.5 + 0.125},
       .dUTC = 18.0 + 0.125,
       .is_lse = true,
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259219.0 + 0.125},
       .dUTC = 19.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259219.5 + 0.125},
       .dUTC = 19.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      /* Jan 1 2020 (positive UTC linear correction) */
      {.t = {.wn = 2086, .tow = 259217.0},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259217.5},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259218.0001},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259218.5},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259219.0001},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259219.5},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_pos_trend},
      /* Jan 1 2020 (negative UTC linear correction) */
      {.t = {.wn = 2086, .tow = 259217.0},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259217.5},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259218.0},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259218.5},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259219.0},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259219.5},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_neg_trend},
  };
  for (size_t i = 0; i < sizeof(testcases) / sizeof(struct utc_params_testcase);
       i++) {
    bool is_lse = is_leap_second_event(&testcases[i].t, testcases[i].p);
    fail_unless(is_lse == testcases[i].is_lse,
                "utc_params_testcase %d failed, expected LSE=%d got %d",
                i,
                testcases[i].is_lse,
                is_lse);

    double dUTC = get_gps_utc_offset(&testcases[i].t, testcases[i].p);

    fail_unless(within_epsilon(dUTC, testcases[i].dUTC),
                "utc_params_testcase %d failed, expected dUTC=%.16f got %.16f",
                i,
                testcases[i].dUTC,
                dUTC);

    /* check that offset from the resulting UTC time back to GPS time matches,
     * except during the leap second event when the UTC time is ambiguous */
    if (!is_lse) {
      gps_time_t utc_time = {.wn = testcases[i].t.wn,
                             .tow = testcases[i].t.tow - dUTC};
      double dGPS = get_utc_gps_offset(&utc_time, testcases[i].p);
      fail_unless(
          within_epsilon(dGPS, -testcases[i].dUTC),
          "utc_params_testcase inverse %d failed, expected %.16f got %.16f",
          i,
          -testcases[i].dUTC,
          dGPS);
    }

    /* check conversion to GLO and back with the UTC parameters */
    glo_time_t glo_time = gps2glo(&testcases[i].t, testcases[i].p);
    gps_time_t converted = glo2gps(&glo_time, testcases[i].p);
    fail_unless(
        fabs(gpsdifftime(&testcases[i].t, &converted)) < 1e-5,
        "utc_params_testcase %d gps2glo2gps failed for (%u/%u %u:%u:%.16f), "
        "expected (%d, %f), got (%d, %f)",
        i,
        glo_time.n4,
        glo_time.nt,
        glo_time.h,
        glo_time.m,
        glo_time.s,
        testcases[i].t.wn,
        testcases[i].t.tow,
        converted.wn,
        converted.tow);
  }
}
END_TEST

START_TEST(test_gps2utc) {
  /* test leap second on 1st Jan 2020 */
  /* note also the polynomial correction which shifts the time of effectivity */

  struct gps2utc_testcase {
    gps_time_t t;
    utc_tm u;
    utc_params_t *p;
  } testcases[] = {
      /* July 1 1981 */
      {.t = {.wn = 77, .tow = 259199.0},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 77, .tow = 259199.5},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = NULL},
      {.t = {.wn = 77, .tow = 259200.0},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 77, .tow = 259200.5},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = NULL},
      {.t = {.wn = 77, .tow = 259201.0},
       .u = {.year = 1981,
             .month = 7,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = NULL},
      /* Jan 1 2017 */
      {.t = {.wn = 1930, .tow = 16.0},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 1930, .tow = 16.5},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = NULL},
      {.t = {.wn = 1930, .tow = 17.0},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 1930, .tow = 17.5},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = NULL},
      {.t = {.wn = 1930, .tow = 18.0},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = NULL},
      /* Jan 8 2017 */
      {.t = {.wn = 1931, .tow = 17.0},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 1931, .tow = 17.5},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = NULL},
      {.t = {.wn = 1931, .tow = 18 - 6e-11},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 1 - 6e-11},
       .p = NULL},
      {.t = {.wn = 1931, .tow = 18 - 5e-11},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 8,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = NULL},
      {.t = {.wn = 1931, .tow = 18.0},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 8,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = NULL},
      /* Jan 1 2020 (leap second announced in utc_params_t above, constant
         negative offset) */
      {.t = {.wn = 2086, .tow = 259217.0 - 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259217.5 - 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259218.0 - 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259218.5 - 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_neg_offset},
      {.t = {.wn = 2086, .tow = 259219.0 - 0.125},
       .u = {.year = 2020,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = &p_neg_offset},
      /* Jan 1 2020 (leap second announced in utc_params_t above, constant
         positive offset) */
      {.t = {.wn = 2086, .tow = 259217.0 + 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259217.5 + 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259218.0 + 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259218.5 + 0.125},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_pos_offset},
      {.t = {.wn = 2086, .tow = 259219.0 + 0.125},
       .u = {.year = 2020,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = &p_pos_offset},
      /* Jan 1 2020 (leap second announced in utc_params_t above, positive UTC
         linear correction) */
      {.t = {.wn = 2086, .tow = 259217.0},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259217.5},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259218.0},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259218.5},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_pos_trend},
      {.t = {.wn = 2086, .tow = 259219.00001},
       .u = {.year = 2020,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = &p_pos_trend},
      /* Jan 1 2020 (leap second announced in utc_params_t above, negative UTC
         linear correction) */
      {.t = {.wn = 2086, .tow = 259217.0},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259217.5},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259218.0},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259218.5},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_neg_trend},
      {.t = {.wn = 2086, .tow = 259219.0},
       .u = {.year = 2020,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = &p_neg_trend},
  };

  utc_tm u;
  for (size_t i = 0; i < sizeof(testcases) / sizeof(struct gps2utc_testcase);
       i++) {
    utc_tm expected = testcases[i].u;
    gps2utc(&testcases[i].t, &u, testcases[i].p);

    fail_unless(u.year == expected.year,
                "gps2utc_testcase %d failed, got year %d expected %d",
                i,
                u.year,
                expected.year);
    fail_unless(u.month == expected.month,
                "gps2utc_testcase %d failed, got month %d expected %d",
                i,
                u.month,
                expected.month);
    fail_unless(u.month_day == expected.month_day,
                "gps2utc_testcase %d failed, got day %d expected %d",
                i,
                u.month_day,
                expected.month_day);
    fail_unless(u.hour == expected.hour,
                "gps2utc_testcase %d failed, got hour %d expected %d",
                i,
                u.hour,
                expected.hour);
    fail_unless(u.minute == expected.minute,
                "gps2utc_testcase %d failed, got minute %d expected %d",
                i,
                u.minute,
                expected.minute);
    fail_unless(within_epsilon(u.second_int + u.second_frac,
                               expected.second_int + expected.second_frac),
                "gps2utc_testcase %d failed, got second %.16f expected %.16f",
                i,
                u.second_int + u.second_frac,
                expected.second_int + expected.second_frac);
    fail_unless(u.second_frac < 1,
                "gps2utc_testcase %d failed, got second_frac %g expected <1",
                i,
                u.second_frac);
  }
}
END_TEST

START_TEST(test_time_conversions) {
  gps_time_t testcases[] = {
      {567890.0, 1234},
      {567890.5, 1234},
      {567890.0, 1234},
      {0.0, 1234},
      {604578.0, 1000},
      {222.222, 1001},
      {604578.0, 1001},
      {222.222, 1939},
      {16, 1930},
      {18, 1930} /* around Jan 2017 leap second */
  };
  const double tow_tol = 1e-6;
  for (size_t i = 0; i < ARRAY_SIZE(testcases); i++) {
    /* test gps -> mjd -> gps */
    double mjd = gps2mjd(&testcases[i]);
    gps_time_t ret = mjd2gps(mjd);
    fail_unless(fabs(gpsdifftime(&testcases[i], &ret)) < tow_tol,
                "gps2mjd2gps test case %d failed",
                i);

    /* test mjd -> date -> mjd */
    s32 year, month, day, hour, min;
    double sec;
    mjd2date(mjd, &year, &month, &day, &hour, &min, &sec);
    fail_unless(
        fabs(date2mjd(year, month, day, hour, min, sec) - mjd) < tow_tol,
        "mjd2date2mjd test case %d failed",
        i);

    /* test mjd -> utc -> mjd */
    utc_tm utc = mjd2utc(mjd);
    fail_unless(
        fabs(utc2mjd(&utc) - mjd) < tow_tol, "utc2mjd test case %d failed", i);

    /* test gps -> date -> gps */
    gps2date(&testcases[i], &year, &month, &day, &hour, &min, &sec);
    ret = date2gps(year, month, day, hour, min, sec);
    fail_unless(fabs(gpsdifftime(&testcases[i], &ret)) < tow_tol,
                "gps2date2gps test case %d failed",
                i);

    /* test utc -> date -> utc */
    utc2date(&utc, &year, &month, &day, &hour, &min, &sec);
    utc = date2utc(year, month, day, hour, min, sec);
    fail_unless(fabs(utc2mjd(&utc) - mjd) < tow_tol,
                "utc2date2utc test case %d failed",
                i);
  }
}
END_TEST

Suite *gnss_time_test_suite(void) {
  Suite *s = suite_create("Time handling");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_gpsdifftime);
  tcase_add_test(tc_core, test_normalize_gps_time);
  tcase_add_test(tc_core, test_gps_time_match_weeks);
  tcase_add_test(tc_core, test_gps_adjust_week_cycle);
  tcase_add_test(tc_core, test_is_leap_year);
  tcase_add_test(tc_core, test_utc_offset);
  tcase_add_test(tc_core, test_utc_params);
  tcase_add_test(tc_core, test_gps2utc);
  tcase_add_test(tc_core, test_glo2gps);
  tcase_add_test(tc_core, test_gps2utc_time);
  tcase_add_test(tc_core, test_gps2utc_date);
  tcase_add_test(tc_core, test_time_conversions);
  suite_add_tcase(s, tc_core);

  return s;
}
