#include <check.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <swiftnav/bits.h>
#include <time.h>

#include "check_suites.h"
#include "common/check_utils.h"

START_TEST(test_parity) {
  fail_unless(parity(0x00000000) == 0);
  fail_unless(parity(0xFFFFFFFF) == 0);
  fail_unless(parity(0x01010101) == 0);
  fail_unless(parity(0x10101010) == 0);
  fail_unless(parity(0x10A010A0) == 0);

  fail_unless(parity(0x10000000) == 1);
  fail_unless(parity(0x00000001) == 1);
  fail_unless(parity(0x70707000) == 1);
  fail_unless(parity(0x0B0B0B00) == 1);
  fail_unless(parity(0x00E00000) == 1);
}
END_TEST

START_TEST(test_getbitu) {
  u8 test_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

  u32 ret;

  ret = getbitu(test_data, 0, 8);
  fail_unless(ret == 0x01, "Test case 1 expected 0x01, got 0x%02X", ret);

  ret = getbitu(test_data, 4, 8);
  fail_unless(ret == 0x12, "test case 2 expected 0x12, got 0x%02X", ret);

  ret = getbitu(test_data, 28, 16);
  fail_unless(ret == 0x789A, "test case 3 expected 0x789A, got 0x%04X", ret);

  ret = getbitu(test_data, 12, 32);
  fail_unless(
      ret == 0x3456789A, "test case 4 expected 0x3456789A, got 0x%08X", ret);

  ret = getbitu(test_data, 10, 3);
  fail_unless(ret == 0x4, "test case 5 expected 0x4, got 0x%01X", ret);

  ret = getbitu(test_data, 10, 13);
  fail_unless(ret == 0x11A2, "test case 6 expected 0x11A2, got 0x%04X", ret);
}
END_TEST

START_TEST(test_getbits) {
  u8 test_data[] = {0x00, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 0xFF};

  s32 ret;

  ret = getbits(test_data, 0, 8);
  fail_unless(ret == 0, "Test case 1 expected 0, got %d", ret);

  ret = getbits(test_data, 13, 3);
  fail_unless(ret == 3, "Test case 2 expected 3, got %d", ret);

  ret = getbits(test_data, 14, 3);
  fail_unless(ret == -1, "Test case 3 expected -1, got %d", ret);

  ret = getbits(test_data, 14, 4);
  fail_unless(ret == -2, "Test case 4 expected -2, got %d", ret);

  ret = getbits(test_data, 24, 32);
  fail_unless(ret == -1, "Test case 5 expected -1, got %d", ret);
}
END_TEST

START_TEST(test_setbitu) {
  u8 test_data[10];
  u8 zeroes[10];

  u32 ret;
  unsigned seed = time(NULL);

  srand(seed);

  memset(zeroes, 0, sizeof(zeroes));
  memset(test_data, 0, sizeof(test_data));

  for (unsigned len = 0; len <= 32; len++) {
    for (unsigned pos = 0; pos < 48; pos++) {
      u32 data = rand();

      /* Set 'data' and check that we get the same value when we read it
       * back */
      setbitu(test_data, pos, len, data);
      /* Mask off bits higher than 'len' since they shouldn't be set when we
       * read back */
      data &= (len == 32 ? ~0 : ((1 << len) - 1));
      ret = getbitu(test_data, pos, len);
      fail_unless(
          ret == data,
          "test case 1 expected %04X, got 0x%04X (len %u, pos %u, seed %u)",
          data,
          ret,
          len,
          pos,
          seed);

      /* Clear data and make sure that no additional bits have changed */
      setbitu(test_data, pos, len, 0);
      fail_unless(!memcmp(test_data, zeroes, sizeof(test_data)),
                  "test case 2 not completely zeroed");
    }
  }
}
END_TEST

START_TEST(test_setbits) {
  u8 test_data[10];

  s32 ret;

  setbits(test_data, 14, 3, -1);
  ret = getbits(test_data, 14, 3);
  fail_unless(ret == -1, "Test case 1 expected -1, got %d", ret);

  setbits(test_data, 14, 8, 22);
  ret = getbits(test_data, 14, 8);
  fail_unless(ret == 22, "Test case 2 expected 22, got %d", ret);

  setbits(test_data, 24, 32, -1);
  ret = getbits(test_data, 24, 32);
  fail_unless(ret == -1, "Test case 3 expected -1, got %d", ret);
}
END_TEST

START_TEST(test_setbitul) {
  u8 test_data[64] = {0};
  u8 zeroes[64] = {0};

  u64 ret = 0;
  unsigned seed = time(NULL);

  srand(seed);

  for (unsigned len = 0; len <= sizeof(u64); len++) {
    for (unsigned pos = 0; pos <= sizeof(u64); pos++) {
      u64 data = (((u64)rand() << 32) | ((u64)rand()));

      /* Set 'data' and check that we get the same value when we read it
       * back */
      setbitul(test_data, pos, len, data);
      /* Mask off bits higher than 'len' since they shouldn't be set when we
       * read back */
      data &= (len == 64 ? ~(u64)0 : (((u64)1 << len) - 1));
      ret = getbitul(test_data, pos, len);
      fail_unless(
          ret == data,
          "test case 1 expected 0x%04X, got 0x%04X (len %u, pos %u, seed %u)",
          (u32)data,
          (u32)ret,
          len,
          pos,
          seed);

      /* Clear data and make sure that no additional bits have changed */
      setbitul(test_data, pos, len, 0);
      fail_unless(!memcmp(test_data, zeroes, sizeof(test_data)),
                  "test case 2 not completely zeroed");
    }
  }
}
END_TEST

START_TEST(test_setbitsl) {
  u8 test_data[64] = {0};
  s64 ret = 0;

  s64 input = INT64_MIN;
  setbitsl(test_data, 0, 64, input);
  ret = getbitsl(test_data, 0, 64);
  fail_unless(ret == input,
              "Test case 1 expected %04X, got %04X",
              (s32)input,
              (s32)ret);

  ret = 0;
  memset(test_data, 0, sizeof(test_data));
  input = 0xABCD;
  setbitsl(test_data, 32, 8, input);
  ret = getbitsl(test_data, 32, 8);
  fail_unless(ret == (s8)input,
              "Test case 2 expected 0x%04X, got 0x%04X",
              (s32)input,
              (s32)ret);

  // This test case should fail due to buffer overflow. setbitsl need fixing.
  ret = 0;
  memset(test_data, 0, sizeof(test_data));
  input = 0xABCD;
  setbitsl(test_data, 56, 32, input);
  ret = getbitsl(test_data, 56, 32);
  fail_unless(ret == input,
              "Test case 3 expected 0x%04X, got 0x%04X",
              (s32)input,
              (s32)ret);
}
END_TEST

START_TEST(test_bitshl) {
  u8 src0[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res0[] = {0xBE, 0xEF, 0x00, 0x00};

  u8 src1[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res1[] = {0xEA, 0xDB, 0xEE, 0xF0};

  u8 src2[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res2[] = {0xDB, 0xEE, 0xF0, 0x00};

  u8 src3[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res3[] = {0xB6, 0xFB, 0xBC, 0x00};

  bitshl(src0, sizeof(src0), 16);
  fail_unless(0 == memcmp(src0, res0, 4), "Byte shift test");

  bitshl(src1, sizeof(src1), 4);
  fail_unless(0 == memcmp(src1, res1, 4), "4-bit shift");

  bitshl(src2, sizeof(src2), 12);
  fail_unless(0 == memcmp(src2, res2, 4), "12-bit shift");

  bitshl(src3, sizeof(src3), 10);
  fail_unless(0 == memcmp(src3, res3, 4), "10-bit shift");
}
END_TEST

START_TEST(test_bitcopy) {
  u8 src0[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res0[] = {0xBE, 0xEF, 0xBE, 0xEF};

  u8 src1[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res1[] = {0xEA, 0xDB, 0xEE, 0xFF};

  u8 src2[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD};
  // u8 dst2[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD};
  u8 res2[] = {0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xAD};

  bitcopy(src0, 0, src0, 16, 16);
  fail_unless(0 == memcmp(src0, res0, 4), "16-bit copy");

  bitcopy(src1, 0, src1, 4, 28);
  fail_unless(0 == memcmp(src1, res1, 4), "28-bit copy");

  bitcopy(src2, 0, src2, 8, 72);
  fail_unless(0 == memcmp(src2, res2, 4), "72-bit copy");
}
END_TEST

START_TEST(test_count_bits_x) {
  u8 src8[] = {0xDE, 0xAD, 0x12, 0xEF};
  u8 res8[] = {6, 5, 2, 7};

  u16 src16[] = {0xDE05, 0xADF6, 0xBE32, 0xEF45};
  u8 res16[] = {8, 11, 9, 10};

  u32 src32[] = {0xDE051234, 0x00000000, 0x00329300, 0x1F45A6C8};
  u8 res32[] = {13, 0, 7, 15};

  u64 src64[] = {0xDE051234432150ED,
                 0x0000000080000000,
                 0x0032930000392300,
                 0x10F14350A060C080};
  u8 res64[] = {26, 1, 14, 18};

  for (unsigned i = 0; i < sizeof(src8) / sizeof(src8[0]); i++) {
    u8 r = count_bits_u8(src8[i], 1);
    fail_unless(res8[i] == r, "count_bits_u8(0x%x, 1) = %d", src8[i], r);
    r = count_bits_u8(src8[i], 0);
    fail_unless(8 - res8[i] == r, "count_bits_u8(0x%x, 0) = %d", src8[i], r);
  }

  for (unsigned i = 0; i < sizeof(src16) / sizeof(src16[0]); i++) {
    u8 r = count_bits_u16(src16[i], 1);
    fail_unless(res16[i] == r, "count_bits_u16(0x%x, 1) = %d", src16[i], r);
    r = count_bits_u16(src16[i], 0);
    fail_unless(
        16 - res16[i] == r, "count_bits_u16(0x%x, 0) = %d", src16[i], r);
  }

  for (unsigned i = 0; i < sizeof(src32) / sizeof(src32[0]); i++) {
    u8 r = count_bits_u32(src32[i], 1);
    fail_unless(res32[i] == r, "count_bits_u32(0x%x, 1) = %d", src32[i], r);
    r = count_bits_u32(src32[i], 0);
    fail_unless(
        32 - res32[i] == r, "count_bits_u32(0x%x, 0) = %d", src32[i], r);
  }

  for (unsigned i = 0; i < sizeof(src64) / sizeof(src64[0]); i++) {
    u8 r = count_bits_u64(src64[i], 1);
    fail_unless(
        res64[i] == r, "count_bits_u64(0x%" PRIx64 ", 1) = %d", src64[i], r);
    r = count_bits_u64(src64[i], 0);
    fail_unless(64 - res64[i] == r,
                "count_bits_u64(0x%" PRIx64 ", 0) = %d",
                src64[i],
                r);
  }
}
END_TEST

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#endif

START_TEST(test_sign_extend32) {
  fail_unless(BITS_SIGN_EXTEND_32(5, 0) == 0);
  fail_unless(BITS_SIGN_EXTEND_32(5, 1) == 1);
  fail_unless(BITS_SIGN_EXTEND_32(5, 15) == 15);
  fail_unless(BITS_SIGN_EXTEND_32(5, 0x1F) == -1);
  fail_unless(BITS_SIGN_EXTEND_32(5, 0x10) == -16);
}
END_TEST

START_TEST(test_sign_extend64) {
  fail_unless(BITS_SIGN_EXTEND_64(33, 0) == 0);
  fail_unless(BITS_SIGN_EXTEND_64(33, 1) == 1);
  fail_unless(BITS_SIGN_EXTEND_64(33, INT64_C(0xFFFFFFFF)) ==
              INT64_C(0xFFFFFFFF));
  fail_unless(BITS_SIGN_EXTEND_64(33, INT64_C(0x1FFFFFFFF)) == -INT64_C(0x1));
  fail_unless(BITS_SIGN_EXTEND_64(33, INT64_C(0x100000000)) ==
              -INT64_C(0x100000000));
}
END_TEST

#ifdef __clang__
#pragma clang diagnostic pop
#endif

START_TEST(test_endianess) {
#ifdef __BYTE_ORDER__  // Available on gcc and clang, proabbly not other
                       // compilers
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  enum endianess expected_endianess = SWIFT_LITTLE_ENDIAN;
  u16 u16_values[] = {0x1234, 0x3412, 0x3412};
  u32 u32_values[] = {0x12345678, 0x78563412, 0x78563412};
  u64 u64_values[] = {
      0x123456789abcdef0, 0xf0debc9a78563412, 0xf0debc9a78563412};
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  enum endianess expected_endianess = SWIFT_BIG_ENDIAN;
  u16 u16_values[] = {0x1234, 0x3412, 0x1234};
  u32 u32_values[] = {0x12345678, 0x78563412, 0x12345678};
  u64 u64_values[] = {
      0x123456789abcdef0, 0xf0debc9a78563412, 0x123456789abcdef0};
#else
#error "__BYTE_ORDER__ not set properly by compiler"
#endif
  fail_unless(get_endianess() == expected_endianess);
  // Order of elements in value array is big, little, host
  fail_unless(byte_swap_16(u16_values[0]) == u16_values[1]);
  fail_unless(byte_swap_16(u16_values[1]) == u16_values[0]);
  fail_unless(betoh_16(u16_values[0]) == u16_values[2]);
  fail_unless(letoh_16(u16_values[1]) == u16_values[2]);
  fail_unless(htobe_16(u16_values[2]) == u16_values[0]);
  fail_unless(htole_16(u16_values[2]) == u16_values[1]);

  fail_unless(byte_swap_32(u32_values[0]) == u32_values[1]);
  fail_unless(byte_swap_32(u32_values[1]) == u32_values[0]);
  fail_unless(betoh_32(u32_values[0]) == u32_values[2]);
  fail_unless(letoh_32(u32_values[1]) == u32_values[2]);
  fail_unless(htobe_32(u32_values[2]) == u32_values[0]);
  fail_unless(htole_32(u32_values[2]) == u32_values[1]);

  fail_unless(byte_swap_64(u64_values[0]) == u64_values[1]);
  fail_unless(byte_swap_64(u64_values[1]) == u64_values[0]);
  fail_unless(betoh_64(u64_values[0]) == u64_values[2]);
  fail_unless(letoh_64(u64_values[1]) == u64_values[2]);
  fail_unless(htobe_64(u64_values[2]) == u64_values[0]);
  fail_unless(htole_64(u64_values[2]) == u64_values[1]);
#else
// No compiler indication of endianess, tests disabled
#endif
}
END_TEST

Suite *bits_suite(void) {
  Suite *s = suite_create("Bit Utils");

  TCase *tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_parity);
  tcase_add_test(tc_core, test_getbitu);
  tcase_add_test(tc_core, test_getbits);
  tcase_add_test(tc_core, test_setbitu);
  tcase_add_test(tc_core, test_setbitul);
  tcase_add_test(tc_core, test_setbits);
  tcase_add_test(tc_core, test_setbitsl);
  tcase_add_test(tc_core, test_bitshl);
  tcase_add_test(tc_core, test_bitcopy);
  tcase_add_test(tc_core, test_count_bits_x);
  tcase_add_test(tc_core, test_sign_extend32);
  tcase_add_test(tc_core, test_sign_extend64);
  tcase_add_test(tc_core, test_endianess);
  suite_add_tcase(s, tc_core);

  return s;
}
