#include <gtest/gtest.h>
#include <math.h>
#include <swiftnav/constants.h>
#include <swiftnav/gnss_time.h>
#include <time.h>

#include "check_utils.h"

namespace {

TEST(TestGnssTime, Gpsdifftime) {
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
      {.a = {0, 5120}, .b = {0, 1024}, .dt = 2477260800},
  };
  const double tow_tol = 1e-10;
  for (auto &testcase : testcases) {
    double dt = gpsdifftime(&testcase.a, &testcase.b);
    EXPECT_NEAR(dt, testcase.dt, tow_tol);
  }
}

TEST(TestGnssTime, GpsdifftimeWeekSecond) {
  struct gpsdifftime_week_second_testcase {
    gps_time_t beginning;
    gps_time_t end;
    gps_time_duration_t dt;
    bool success;
  } testcases[] = {
      {.beginning = {567890.0, 1234},
       .end = {567890.0, 1234},
       .dt = {.seconds = 0, .weeks = 0},
       .success = true},
      {.beginning = {0.0, 1234},
       .end = {567890.0, 1234},
       .dt = {.seconds = 567890, .weeks = 0},
       .success = true},
      {.beginning = {0.0, 1234},
       .end = {567890.0, WN_UNKNOWN},
       .dt = {0, 0},
       .success = false},
      {.beginning = {2222.0, WN_UNKNOWN},
       .end = {222222.0, 2222},
       .dt = {0, 0},
       .success = false},
      {.beginning = {2222.0, WN_UNKNOWN},
       .end = {444444.0, WN_UNKNOWN},
       .dt = {0, 0},
       .success = false},
      {.beginning = {222.222, 1001},
       .end = {604578.0, 1000},
       .dt = {.seconds = -444.222, .weeks = 0},
       .success = true},
      {.beginning = {604578.0, 1000},
       .end = {222.222, 1001},
       .dt = {.seconds = 444.222, .weeks = 0},
       .success = true},
      {.beginning = {222.222, 1000},
       .end = {604578.0, 1001},
       .dt = {.seconds = 604355.778, .weeks = 1},
       .success = true},
      {.beginning = {0, 1024},
       .end = {0, 5120},
       .dt = {.seconds = 0, .weeks = 4096},
       .success = true},
  };
  const double tow_tol = 1e-10;
  for (size_t i = 0;
       i < sizeof(testcases) / sizeof(struct gpsdifftime_week_second_testcase);
       i++) {
    gps_time_duration_t dt = {0, 0};
    bool success = gpsdifftime_week_second(
        &testcases[i].end, &testcases[i].beginning, &dt);
    double absdiff_s = fabs(dt.seconds - testcases[i].dt.seconds) +
                       fabs(static_cast<double>(dt.weeks) -
                            static_cast<double>(testcases[i].dt.weeks)) *
                           WEEK_SECS;
    bool test_passed = absdiff_s < tow_tol && success == testcases[i].success;
    EXPECT_TRUE(test_passed)
        << "gpsdifftime_week_second test case " << i << " failed";
  }
}

TEST(TestGnssTime, LongGpsTimeDiff) {
  gps_time_t t2 = {.tow = 0, .wn = 2345};
  gps_time_t t1 = {.tow = 1.2345678, .wn = 0};
  gps_time_duration_t dt = {0, 0};
  gps_time_duration_t correct_dt = {.seconds = 604798.7654322, .weeks = 2344};
  // Test that less floating-point error with gpsdifftime_week_second than with
  // gpsdifftime(...):
  dt.seconds = gpsdifftime(&t2, &t1);
  dt.weeks = 0;
  normalize_gps_time_duration(&dt);
  EXPECT_NEAR(dt.seconds, correct_dt.seconds, 1e-2);
  EXPECT_TRUE(fabs(dt.seconds - correct_dt.seconds) > 1e-9);

  gpsdifftime_week_second(&t2, &t1, &dt);
  EXPECT_NEAR(dt.seconds, correct_dt.seconds, 1e-12);
}

TEST(TestGnssTime, NormalizeGpsTime) {
  gps_time_t testcases[] = {{0, 1234},
                            {3 * DAY_SECS, 1234},
                            {WEEK_SECS + DAY_SECS, 1234},
                            {0 - DAY_SECS, 1234},
                            {DAY_SECS, WN_UNKNOWN},
                            {WEEK_SECS + 1, WN_UNKNOWN}};
  const double tow_tol = 1e-10;
  for (auto &testcase : testcases) {
    double t_original = testcase.wn * WEEK_SECS + testcase.tow;
    s16 wn = testcase.wn;
    normalize_gps_time(&testcase);
    double t_normalized = testcase.wn * WEEK_SECS + testcase.tow;
    if (testcase.wn != WN_UNKNOWN) {
      EXPECT_NEAR(t_original, t_normalized, tow_tol);
    }
    /* normalization must not touch unknown week number */
    EXPECT_TRUE(wn != WN_UNKNOWN || testcase.wn == WN_UNKNOWN);
  }
}

TEST(TestGnssTime, NormalizeGpsTimeDuration) {
  gps_time_duration_t testcases[] = {{0, 1234},
                                     {3 * DAY_SECS, 1234},
                                     {WEEK_SECS + DAY_SECS, 1234},
                                     {-DAY_SECS, 1234},
                                     {0, -1234},
                                     {-3 * DAY_SECS, -1234},
                                     {-(WEEK_SECS + DAY_SECS), -1234},
                                     {DAY_SECS, -1234},
                                     {1e-9, 1234}};
  gps_time_duration_t expected_results[] = {
      {0, 1234},
      {3 * DAY_SECS, 1234},
      {DAY_SECS, 1234 + 1},
      {WEEK_SECS - DAY_SECS, 1234 - 1},
      {0, -1234},
      {-3 * DAY_SECS, -1234},
      {-DAY_SECS, -(1234 + 1)},
      {-(WEEK_SECS - DAY_SECS), -1234 + 1},
      {1e-9, 1234}};
  const double tow_tol = 1e-10;
  for (size_t i = 0; i < sizeof(testcases) / sizeof(gps_time_duration_t); i++) {
    normalize_gps_time_duration(&testcases[i]);
    bool test_passed =
        testcases[i].weeks == expected_results[i].weeks &&
        fabs(testcases[i].seconds - expected_results[i].seconds) < tow_tol;
    EXPECT_TRUE(test_passed)
        << "normalize_gps_time_duration test case " << i << " failed";
  }
}

/* test that the make_utc_tm conversion matches gmtime */
void gmtime_test(const char *name, time_t start, time_t end, time_t step) {
  time_t t_gps = start;
  while (t_gps < end) {
    time_t t_unix = t_gps + GPS_EPOCH;
    struct tm *date = gmtime(&t_unix);

    gps_time_t t = {.tow = static_cast<double>(t_gps % WEEK_SECS),
                    .wn = static_cast<int16_t>(t_gps / WEEK_SECS)};

    utc_tm u;
    make_utc_tm(&t, &u);

    EXPECT_EQ((date->tm_year + 1900), u.year);
    EXPECT_EQ((date->tm_mon + 1), u.month);
    EXPECT_EQ((date->tm_yday + 1), u.year_day);
    EXPECT_EQ(date->tm_mday, u.month_day);
    EXPECT_EQ(date->tm_wday, (u.week_day % 7));
    EXPECT_EQ(date->tm_hour, u.hour);
    EXPECT_EQ(date->tm_min, u.minute);
    EXPECT_EQ(date->tm_sec, u.second_int);
    EXPECT_EQ(0.0, u.second_frac);

    t_gps += step;
  }
}

TEST(TestGnssTime, Gps2utcTime) {
  /* test make_utc_tm for Jan 6 1980 in 1 s increments */
  gmtime_test("make_utc_tm_day", 0, 1 * DAY_SECS + 1, 1);
}

TEST(TestGnssTime, Gps2utcDate) {
  /* test make_utc_tm from 1980 to 2048 with (1 day + 1 s) increments
   * (68 years is 2144448000, which is just below the maximum value of a
   * signed 32-bit number, i.e. 2147483648) */
  gmtime_test("make_utc_tm_rollover", 0, 68L * 365 * DAY_SECS, DAY_SECS + 1);
}

TEST(TestGnssTime, GpsTimeMatchWeeks) {
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
  for (auto &testcase : testcases) {
    gps_time_match_weeks(&testcase.t, &testcase.ref);
    EXPECT_EQ(testcase.t.wn, testcase.ret.wn);
    EXPECT_EQ(testcase.t.tow, testcase.ret.tow);
  }
}

TEST(TestGnssTime, GpsAdjustWeekCycle) {
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
  for (auto &testcase : testcases) {
    u16 wn = gps_adjust_week_cycle(testcase.wn_raw, wn_ref);
    EXPECT_EQ(wn, testcase.ret);
  }
}

TEST(TestGnssTime, IsLeapYear) {
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

  for (auto &testcase : testcases) {
    EXPECT_EQ(is_leap_year(testcase.year), testcase.ret);
  }
}

TEST(TestGnssTime, Glo2gps) {
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
       .ret = {.tow = 424813, .wn = 1094}},
      /* GLO time 30th Dec 2000 01:00:00 */
      {.glot = {.nt = 365, .n4 = 2, .h = 1, .m = 0, .s = 0},
       .ret = {.tow = 511213, .wn = 1094}},
      /* GLO time 31st Dec 2000 02:00:00 */
      {.glot = {.nt = 366, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.tow = 601213, .wn = 1094}},
      /* GLO time 1st Jan  2001 02:00:00 */
      {.glot = {.nt = 367, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.tow = 82813, .wn = 1095}},
      /* GLO time 2nd Jan  2001 02:00:00 */
      {.glot = {.nt = 368, .n4 = 2, .h = 2, .m = 0, .s = 0},
       .ret = {.tow = 169213, .wn = 1095}},
      /* GLO time 31st Dec 2009 12:12:12 */
      {.glot = {.nt = 731, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.tow = 378747, .wn = 1564}},
      /* GLO time 31st Dec 2010 12:12:12 */
      {.glot = {.nt = 1096, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.tow = 465147, .wn = 1616}},
      /* GLO time 31st Dec 2011 12:12:12 */
      {.glot = {.nt = 1461, .n4 = 4, .h = 12, .m = 12, .s = 12},
       .ret = {.tow = 551547, .wn = 1668}},
      /* GLO time 1st Jan 2017 02:59:59 */
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 59},
       .ret = {.tow = 16, .wn = 1930}},
      /* GLO time 1st Jan 2017 02:59:59.5 */
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 59.5},
       .ret = {.tow = 16.5, .wn = 1930}},
      /* GLO time 1st Jan 2017 02:59:60  (leap second)*/
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 60},
       .ret = {.tow = 17, .wn = 1930}},
      /* GLO time 1st Jan 2017 02:59:60.5  (leap second)*/
      {.glot = {.nt = 367, .n4 = 6, .h = 2, .m = 59, .s = 60.5},
       .ret = {.tow = 17.5, .wn = 1930}},
      /* GLO time 1st Jan 2017 03:00:00 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 0},
       .ret = {.tow = 18, .wn = 1930}},
      /* GLO time 1st Jan 2017 03:00:01 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 1},
       .ret = {.tow = 19, .wn = 1930}},
      /* GLO time 1st Jan 2017 03:00:02 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 0, .s = 2},
       .ret = {.tow = 20, .wn = 1930}},
      /* GLO time 1st Jan 2017 03:01:00 */
      {.glot = {.nt = 367, .n4 = 6, .h = 3, .m = 1, .s = 0},
       .ret = {.tow = 78, .wn = 1930}},
  };
  for (auto &testcase : testcases) {
    gps_time_t ret = glo2gps(&testcase.glot, /* utc_params = */ nullptr);
    EXPECT_EQ(ret.wn, testcase.ret.wn);
    EXPECT_EQ(ret.tow, testcase.ret.tow);
    if (gps_time_valid(&ret)) {
      /* convert back to GLO time */
      glo_time_t glo = gps2glo(&ret, nullptr);
      EXPECT_EQ(glo.n4, testcase.glot.n4);
      EXPECT_EQ(glo.nt, testcase.glot.nt);
      EXPECT_EQ(glo.h, testcase.glot.h);
      EXPECT_EQ(glo.m, testcase.glot.m);
      EXPECT_NEAR(glo.s, testcase.glot.s, 1e-5);
    }
  }
}

TEST(TestGnssTime, UtcOffset) {
  struct utc_offset_testcase {
    gps_time_t t;
    double dUTC;
    bool is_lse;
  } testcases[] = {
      /* July 1 1981 */
      {.t = {.tow = 259199.0, .wn = 77}, .dUTC = 0.0, .is_lse = false},
      {.t = {.tow = 259199.5, .wn = 77}, .dUTC = 0.0, .is_lse = false},
      {.t = {.tow = 259200.0, .wn = 77}, .dUTC = 0.0, .is_lse = true},
      {.t = {.tow = 259200.5, .wn = 77}, .dUTC = 0.0, .is_lse = true},
      {.t = {.tow = 259201.0, .wn = 77}, .dUTC = 1.0, .is_lse = false},
      {.t = {.tow = 259202.0, .wn = 77}, .dUTC = 1.0, .is_lse = false},
      /* Jan 1 2017 */
      {.t = {.tow = 16.0, .wn = 1930}, .dUTC = 17.0, .is_lse = false},
      {.t = {.tow = 16.5, .wn = 1930}, .dUTC = 17.0, .is_lse = false},
      {.t = {.tow = 17.0, .wn = 1930}, .dUTC = 17.0, .is_lse = true},
      {.t = {.tow = 17.5, .wn = 1930}, .dUTC = 17.0, .is_lse = true},
      {.t = {.tow = 18.0, .wn = 1930}, .dUTC = 18.0, .is_lse = false},
      {.t = {.tow = 18.5, .wn = 1930}, .dUTC = 18.0, .is_lse = false},
      {.t = {.tow = 19.0, .wn = 1930}, .dUTC = 18.0, .is_lse = false},
  };
  for (auto &testcase : testcases) {
    double dUTC = get_gps_utc_offset(&testcase.t, nullptr);
    bool is_lse = is_leap_second_event(&testcase.t, nullptr);

    EXPECT_EQ(dUTC, testcase.dUTC);
    EXPECT_EQ(is_lse, testcase.is_lse);

    /* check that offset from the resulting UTC time back to GPS time matches,
     * except during the leap second event when the UTC time is ambiguous */
    if (!is_lse) {
      gps_time_t utc_time = {.tow = testcase.t.tow - dUTC, .wn = testcase.t.wn};
      double dGPS = get_utc_gps_offset(&utc_time, nullptr);
      EXPECT_EQ(dGPS, -testcase.dUTC);
    }
  }
}

/* test a fictional leap second on 1st Jan 2020 */
/* note also the polynomial correction which shifts the time of effectivity */
utc_params_t p_neg_offset = {.a0 = -0.125,
                             .a1 = 0.0,
                             .tot = {.tow = 0, .wn = 2080},
                             .t_lse = {.tow = 259218.0 - 0.125, .wn = 2086},
                             .dt_ls = 18,
                             .dt_lsf = 19};
utc_params_t p_pos_offset = {.a0 = +0.125,
                             .a1 = 0.0,
                             .tot = {.tow = 0, .wn = 2080},
                             .t_lse = {.tow = 259218.0 + 0.125, .wn = 2086},
                             .dt_ls = 18,
                             .dt_lsf = 19};
utc_params_t p_pos_trend = {
    .a0 = 0,
    .a1 = 1e-12,
    .tot = {.tow = 0, .wn = 2080},
    .t_lse = {.tow = 259218.0 + 1e-12 * (6 * WEEK_SECS + 259218.0), .wn = 2086},
    .dt_ls = 18,
    .dt_lsf = 19};
utc_params_t p_neg_trend = {
    .a0 = 0,
    .a1 = -1e-12,
    .tot = {.tow = 0, .wn = 2080},
    .t_lse = {.tow = 259218.0 - 1e-12 * (6 * WEEK_SECS + 259218.0), .wn = 2086},
    .dt_ls = 18,
    .dt_lsf = 19};

TEST(TestGnssTime, UtcParams) {
  struct utc_params_testcase {
    gps_time_t t;
    double dUTC;
    bool is_lse;
    utc_params_t *p;
  } testcases[] = {
      /* Jan 1 2020 (constant negative UTC offset) */
      {.t = {.tow = 259217.0 - 0.125, .wn = 2086},
       .dUTC = 18.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.tow = 259217.5 - 0.125, .wn = 2086},
       .dUTC = 18.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.tow = 259218.0 - 0.125, .wn = 2086},
       .dUTC = 18.0 - 0.125,
       .is_lse = true,
       .p = &p_neg_offset},
      {.t = {.tow = 259218.5 - 0.125, .wn = 2086},
       .dUTC = 18.0 - 0.125,
       .is_lse = true,
       .p = &p_neg_offset},
      {.t = {.tow = 259219.0 - 0.125, .wn = 2086},
       .dUTC = 19.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      {.t = {.tow = 259219.5 - 0.125, .wn = 2086},
       .dUTC = 19.0 - 0.125,
       .is_lse = false,
       .p = &p_neg_offset},
      /* Jan 1 2020 (constant positive UTC offset) */
      {.t = {.tow = 259217.0 + 0.125, .wn = 2086},
       .dUTC = 18.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.tow = 259217.5 + 0.125, .wn = 2086},
       .dUTC = 18.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.tow = 259218.0 + 0.125, .wn = 2086},
       .dUTC = 18.0 + 0.125,
       .is_lse = true,
       .p = &p_pos_offset},
      {.t = {.tow = 259218.5 + 0.125, .wn = 2086},
       .dUTC = 18.0 + 0.125,
       .is_lse = true,
       .p = &p_pos_offset},
      {.t = {.tow = 259219.0 + 0.125, .wn = 2086},
       .dUTC = 19.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      {.t = {.tow = 259219.5 + 0.125, .wn = 2086},
       .dUTC = 19.0 + 0.125,
       .is_lse = false,
       .p = &p_pos_offset},
      /* Jan 1 2020 (positive UTC linear correction) */
      {.t = {.tow = 259217.0, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.tow = 259217.5, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.tow = 259218.0001, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_pos_trend},
      {.t = {.tow = 259218.5, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_pos_trend},
      {.t = {.tow = 259219.0001, .wn = 2086},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_pos_trend},
      {.t = {.tow = 259219.5, .wn = 2086},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_pos_trend},
      /* Jan 1 2020 (negative UTC linear correction) */
      {.t = {.tow = 259217.0, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.tow = 259217.5, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.tow = 259218.0, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_neg_trend},
      {.t = {.tow = 259218.5, .wn = 2086},
       .dUTC = 18.0,
       .is_lse = true,
       .p = &p_neg_trend},
      {.t = {.tow = 259219.0, .wn = 2086},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_neg_trend},
      {.t = {.tow = 259219.5, .wn = 2086},
       .dUTC = 19.0,
       .is_lse = false,
       .p = &p_neg_trend},
  };
  for (auto &testcase : testcases) {
    bool is_lse = is_leap_second_event(&testcase.t, testcase.p);
    EXPECT_EQ(is_lse, testcase.is_lse);

    double dUTC = get_gps_utc_offset(&testcase.t, testcase.p);

    EXPECT_NEAR(dUTC, testcase.dUTC, 1e-5);

    /* check that offset from the resulting UTC time back to GPS time matches,
     * except during the leap second event when the UTC time is ambiguous */
    if (!is_lse) {
      gps_time_t utc_time = {.tow = testcase.t.tow - dUTC, .wn = testcase.t.wn};
      double dGPS = get_utc_gps_offset(&utc_time, testcase.p);
      EXPECT_NEAR(dGPS, -testcase.dUTC, 1e-5);
    }

    /* check conversion to GLO and back with the UTC parameters */
    glo_time_t glo_time = gps2glo(&testcase.t, testcase.p);
    gps_time_t converted = glo2gps(&glo_time, testcase.p);
    EXPECT_TRUE(fabs(gpsdifftime(&testcase.t, &converted)) < 0.2);
  }
}

TEST(TestGnssTime, Gps2utc) {
  /* test leap second on 1st Jan 2020 */
  /* note also the polynomial correction which shifts the time of effectivity */

  struct gps2utc_testcase {
    gps_time_t t;
    utc_tm u;
    utc_params_t *p;
  } testcases[] = {
      /* July 1 1981 */
      {.t = {.tow = 259199.0, .wn = 77},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 259199.5, .wn = 77},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = nullptr},
      {.t = {.tow = 259200.0, .wn = 77},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 259200.5, .wn = 77},
       .u = {.year = 1981,
             .month = 6,
             .month_day = 30,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = nullptr},
      {.t = {.tow = 259201.0, .wn = 77},
       .u = {.year = 1981,
             .month = 7,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = nullptr},
      /* Jan 1 2017 */
      {.t = {.tow = 16.0, .wn = 1930},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 16.5, .wn = 1930},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = nullptr},
      {.t = {.tow = 17.0, .wn = 1930},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 17.5, .wn = 1930},
       .u = {.year = 2016,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = nullptr},
      {.t = {.tow = 18.0, .wn = 1930},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 01,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = nullptr},
      /* Jan 8 2017 */
      {.t = {.tow = 17.0, .wn = 1931},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 17.5, .wn = 1931},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = nullptr},
      {.t = {.tow = 18 - 6e-11, .wn = 1931},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 7,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 1 - 6e-11},
       .p = nullptr},
      {.t = {.tow = 18 - 5e-11, .wn = 1931},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 8,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = nullptr},
      {.t = {.tow = 18.0, .wn = 1931},
       .u = {.year = 2017,
             .month = 01,
             .month_day = 8,
             .hour = 00,
             .minute = 00,
             .second_int = 00,
             .second_frac = 0.0},
       .p = nullptr},
      /* Jan 1 2020 (leap second announced in utc_params_t above, constant
         negative offset) */
      {.t = {.tow = 259217.0 - 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_neg_offset},
      {.t = {.tow = 259217.5 - 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_neg_offset},
      {.t = {.tow = 259218.0 - 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_neg_offset},
      {.t = {.tow = 259218.5 - 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_neg_offset},
      {.t = {.tow = 259219.0 - 0.125, .wn = 2086},
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
      {.t = {.tow = 259217.0 + 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_pos_offset},
      {.t = {.tow = 259217.5 + 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_pos_offset},
      {.t = {.tow = 259218.0 + 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_pos_offset},
      {.t = {.tow = 259218.5 + 0.125, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_pos_offset},
      {.t = {.tow = 259219.0 + 0.125, .wn = 2086},
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
      {.t = {.tow = 259217.0, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_pos_trend},
      {.t = {.tow = 259217.5, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_pos_trend},
      {.t = {.tow = 259218.0, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_pos_trend},
      {.t = {.tow = 259218.5, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_pos_trend},
      {.t = {.tow = 259219.00001, .wn = 2086},
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
      {.t = {.tow = 259217.0, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.0},
       .p = &p_neg_trend},
      {.t = {.tow = 259217.5, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 59,
             .second_frac = 0.5},
       .p = &p_neg_trend},
      {.t = {.tow = 259218.0, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.0},
       .p = &p_neg_trend},
      {.t = {.tow = 259218.5, .wn = 2086},
       .u = {.year = 2019,
             .month = 12,
             .month_day = 31,
             .hour = 23,
             .minute = 59,
             .second_int = 60,
             .second_frac = 0.5},
       .p = &p_neg_trend},
      {.t = {.tow = 259219.0, .wn = 2086},
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
  for (auto &testcase : testcases) {
    utc_tm expected = testcase.u;
    gps2utc(&testcase.t, &u, testcase.p);

    EXPECT_EQ(u.year, expected.year);
    EXPECT_EQ(u.month, expected.month);
    EXPECT_EQ(u.month_day, expected.month_day);
    EXPECT_EQ(u.hour, expected.hour);
    EXPECT_EQ(u.minute, expected.minute);
    EXPECT_NEAR(u.second_int + u.second_frac,
                expected.second_int + expected.second_frac,
                1e-5);
    EXPECT_LT(u.second_frac, 1);

    const gps_time_t *g = &testcase.t;
    gps_time_t converted_gps;
    utc2gps(&u, &converted_gps, testcase.p);
    EXPECT_EQ(converted_gps.wn, g->wn);
    EXPECT_NEAR(converted_gps.tow, g->tow, 1e-5);
  }
}

TEST(TestGnssTime, TimeConversions) {
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
  for (auto &testcase : testcases) {
    /* test gps -> mjd -> gps */
    double mjd = gps2mjd(&testcase);
    gps_time_t ret = mjd2gps(mjd);
    EXPECT_LT(fabs(gpsdifftime(&testcase, &ret)), tow_tol);

    /* test mjd -> date -> mjd */
    s32 year, month, day, hour, min;
    double sec;
    mjd2date(mjd, &year, &month, &day, &hour, &min, &sec);
    EXPECT_LT(fabs(date2mjd(year, month, day, hour, min, sec) - mjd), tow_tol);

    /* test mjd -> utc -> mjd */
    utc_tm utc = mjd2utc(mjd);
    EXPECT_LT(fabs(utc2mjd(&utc) - mjd), tow_tol);

    /* test gps -> date -> gps */
    gps2date(&testcase, &year, &month, &day, &hour, &min, &sec);
    ret = date2gps(year, month, day, hour, min, sec);
    EXPECT_LT(fabs(gpsdifftime(&testcase, &ret)), tow_tol);

    /* test utc -> date -> utc */
    utc2date(&utc, &year, &month, &day, &hour, &min, &sec);
    utc = date2utc(year, month, day, hour, min, sec);
    EXPECT_LT(fabs(utc2mjd(&utc) - mjd), tow_tol);
  }
}

TEST(TestGnssTime, RoundToEpoch) {
  const double soln_freq = 10.0;

  gps_time_t testcases[] = {
      {567890.01, 1234},
      {567890.0501, 1234},
      {604800.06, 1234},
  };

  gps_time_t expectations[] = {
      {567890.00, 1234},
      {567890.10, 1234},
      {0.1, 1235},
  };

  for (size_t i = 0; i < ARRAY_SIZE(testcases); ++i) {
    gps_time_t rounded = round_to_epoch(&testcases[i], soln_freq);
    EXPECT_NEAR(gpsdifftime(&rounded, &expectations[i]), 0.0, 1e-5);
  }
}

TEST(TestGnssTime, FloorToEpoch) {
  const double soln_freq = 10.0;

  gps_time_t testcases[] = {
      {567890.01, 1234},
      {567890.0501, 1234},
      {604800.06, 1234},
  };

  gps_time_t expectations[] = {
      {567890.00, 1234},
      {567890.00, 1234},
      {0.0, 1235},
  };

  for (size_t i = 0; i < ARRAY_SIZE(testcases); ++i) {
    gps_time_t rounded = floor_to_epoch(&testcases[i], soln_freq);
    EXPECT_NEAR(gpsdifftime(&rounded, &expectations[i]), 0.0, 1e-5);
  }
}

}  // namespace
