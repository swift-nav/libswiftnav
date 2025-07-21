#include <gtest/gtest.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <swiftnav/set.h>

#include "check_utils.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

namespace {

void test_map_f(void *context, u32 n, const void *a_, const void *b_) {
  const s32 *a = (const s32 *)a_;
  const s32 *b = (const s32 *)b_;
  s32 *c = (s32 *)context;

  EXPECT_NE(context, nullptr);
  EXPECT_EQ(*a, *b);
  c[n] = *a;
}

#define TEST_INTERSECTION(a, b, c)                                            \
  {                                                                           \
    s32 c_result[LEN(c)];                                                     \
                                                                              \
    qsort(a, LEN(a), sizeof(a[0]), cmp_s32_s32);                              \
    qsort(b, LEN(b), sizeof(b[0]), cmp_s32_s32);                              \
    qsort(c, LEN(c), sizeof(c[0]), cmp_s32_s32);                              \
                                                                              \
    s32 ret = intersection_map(LEN(a),                                        \
                               sizeof(a[0]),                                  \
                               a,                                             \
                               LEN(b),                                        \
                               sizeof(b[0]),                                  \
                               b,                                             \
                               cmp_s32_s32,                                   \
                               c_result,                                      \
                               test_map_f);                                   \
                                                                              \
    EXPECT_EQ(ret, LEN(c)) << "Intersection length does not match test data"; \
                                                                              \
    EXPECT_EQ(memcmp(c, c_result, sizeof(c)), 0)                              \
        << "Output of intersection does not match test data";                 \
                                                                              \
    ret = intersection(LEN(a),                                                \
                       sizeof(a[0]),                                          \
                       a,                                                     \
                       c_result,                                              \
                       LEN(b),                                                \
                       sizeof(b[0]),                                          \
                       b,                                                     \
                       nullptr,                                               \
                       cmp_s32_s32);                                          \
                                                                              \
    EXPECT_EQ(ret, LEN(c))                                                    \
        << "Intersection length does not match test data (2)";                \
                                                                              \
    EXPECT_EQ(memcmp(c, c_result, sizeof(c)), 0)                              \
        << "Output of intersection does not match test data (2)";             \
                                                                              \
    ret = intersection(LEN(a),                                                \
                       sizeof(a[0]),                                          \
                       a,                                                     \
                       nullptr,                                               \
                       LEN(b),                                                \
                       sizeof(b[0]),                                          \
                       b,                                                     \
                       c_result,                                              \
                       cmp_s32_s32);                                          \
                                                                              \
    EXPECT_EQ(ret, LEN(c))                                                    \
        << "Intersection length does not match test data (3)";                \
                                                                              \
    EXPECT_EQ(memcmp(c, c_result, sizeof(c)), 0)                              \
        << "Output of intersection does not match test data (3)";             \
  }

TEST(TestSet, IntersectionMap1) {
  /* Empty first set */
  s32 a[] = {};
  s32 b[] = {1, 2, 3};
  s32 c[] = {};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap2) {
  /* Empty second set */
  s32 a[] = {1, 2, 3};
  s32 b[] = {};
  s32 c[] = {};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap3) {
  /* Beginning intersects */
  s32 a[] = {1, 2, 3, 4, 5, 6, 7};
  s32 b[] = {1, 2, 3};
  s32 c[] = {1, 2, 3};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap4) {
  /* End intersects */
  s32 a[] = {1, 2, 3, 4, 5, 6, 7};
  s32 b[] = {5, 6, 7};
  s32 c[] = {5, 6, 7};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap5) {
  /* Same set */
  s32 a[] = {1, 2, 3, 4, 5, 6, 7};
  s32 b[] = {1, 2, 3, 4, 5, 6, 7};
  s32 c[] = {1, 2, 3, 4, 5, 6, 7};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap6) {
  /* Disjoint */
  s32 a[] = {1, 2, 3, 4};
  s32 b[] = {5, 6, 7, 8};
  s32 c[] = {};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap7) {
  /* Middle overlaps */
  s32 a[] = {1, 2, 3, 4, 5, 6, 7};
  s32 b[] = {5, 6};
  s32 c[] = {5, 6};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap8) {
  /* Overlapping but not subset */
  s32 a[] = {1, 2, 3, 4, 5};
  s32 b[] = {4, 5, 6, 7, 8};
  s32 c[] = {4, 5};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap9) {
  /* Alternating disjoint */
  s32 a[] = {2, 4, 6, 8, 10};
  s32 b[] = {1, 3, 7, 9, 11};
  s32 c[] = {};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IntersectionMap10) {
  /* Alternating with overlap */
  s32 a[] = {2, 4, 6, 8, 9, 10};
  s32 b[] = {1, 3, 7, 8, 9, 11};
  s32 c[] = {8, 9};
  TEST_INTERSECTION(a, b, c)
}

TEST(TestSet, IsPrnSet) {
#define TEST_IS_SET(set, result) \
  EXPECT_EQ(is_sid_set(sizeof(set) / sizeof(set[0]), set), result)

  /* Normal set. */
  gnss_signal_t prns1[] = {{.sat = 0},
                           {.sat = 1},
                           {.sat = 2},
                           {.sat = 33},
                           {.sat = 44},
                           {.sat = 200}};
  TEST_IS_SET(prns1, true);

  /* Empty set. */
  EXPECT_TRUE(is_sid_set(0, prns1));

  /* Single element set. */
  gnss_signal_t prns2[] = {{.sat = 22}};
  TEST_IS_SET(prns2, true);

  /* Repeated elements. */

  gnss_signal_t prns3[] = {{.sat = 22}, {.sat = 22}};
  TEST_IS_SET(prns3, false);

  gnss_signal_t prns4[] = {
      {.sat = 0}, {.sat = 1}, {.sat = 2}, {.sat = 3}, {.sat = 3}};
  TEST_IS_SET(prns4, false);

  gnss_signal_t prns5[] = {
      {.sat = 1}, {.sat = 1}, {.sat = 2}, {.sat = 3}, {.sat = 4}};
  TEST_IS_SET(prns5, false);

  /* Incorrectly sorted. */

  gnss_signal_t prns6[] = {
      {.sat = 22}, {.sat = 1}, {.sat = 2}, {.sat = 3}, {.sat = 4}};
  TEST_IS_SET(prns6, false);

  gnss_signal_t prns7[] = {
      {.sat = 0}, {.sat = 1}, {.sat = 2}, {.sat = 3}, {.sat = 1}};
  TEST_IS_SET(prns7, false);

  gnss_signal_t prns8[] = {
      {.sat = 0}, {.sat = 1}, {.sat = 22}, {.sat = 3}, {.sat = 4}};
  TEST_IS_SET(prns8, false);
}

}  // namespace
