#include <gtest/gtest.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <swiftnav/bits.h>
#include <time.h>

#include "check_utils.h"

namespace {

TEST(TestBitUtils, Parity) {
  EXPECT_EQ(parity(0x00000000), 0);
  EXPECT_EQ(parity(0xFFFFFFFF), 0);
  EXPECT_EQ(parity(0x01010101), 0);
  EXPECT_EQ(parity(0x10101010), 0);
  EXPECT_EQ(parity(0x10A010A0), 0);

  EXPECT_EQ(parity(0x10000000), 1);
  EXPECT_EQ(parity(0x00000001), 1);
  EXPECT_EQ(parity(0x70707000), 1);
  EXPECT_EQ(parity(0x0B0B0B00), 1);
  EXPECT_EQ(parity(0x00E00000), 1);
}

TEST(TestBitUtils, Getbitu) {
  u8 test_data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

  u32 ret;

  ret = getbitu(test_data, 0, 8);
  EXPECT_EQ(ret, 0x01);

  ret = getbitu(test_data, 4, 8);
  EXPECT_EQ(ret, 0x12);

  ret = getbitu(test_data, 28, 16);
  EXPECT_EQ(ret, 0x789A);

  ret = getbitu(test_data, 12, 32);
  EXPECT_EQ(ret, 0x3456789A);

  ret = getbitu(test_data, 10, 3);
  EXPECT_EQ(ret, 0x4);

  ret = getbitu(test_data, 10, 13);
  EXPECT_EQ(ret, 0x11A2);
}

TEST(TestBitUtils, Getbits) {
  u8 test_data[] = {0x00, 0x03, 0x80, 0xFF, 0xFF, 0xFF, 0xFF};

  s32 ret;

  ret = getbits(test_data, 0, 8);
  EXPECT_EQ(ret, 0);

  ret = getbits(test_data, 13, 3);
  EXPECT_EQ(ret, 3);

  ret = getbits(test_data, 14, 3);
  EXPECT_EQ(ret, -1);

  ret = getbits(test_data, 14, 4);
  EXPECT_EQ(ret, -2);

  ret = getbits(test_data, 24, 32);
  EXPECT_EQ(ret, -1);
}

TEST(TestBitUtils, Setbitu) {
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
      data &= (len == 32u ? ~0u : ((1u << len) - 1u));
      ret = getbitu(test_data, pos, len);
      EXPECT_EQ(ret, data);

      /* Clear data and make sure that no additional bits have changed */
      setbitu(test_data, pos, len, 0);
      EXPECT_EQ(memcmp(test_data, zeroes, sizeof(test_data)), 0)
          << "test case 2 not completely zeroed";
    }
  }
}

TEST(TestBitUtils, Setbits) {
  u8 test_data[10];

  s32 ret;

  setbits(test_data, 14, 3, -1);
  ret = getbits(test_data, 14, 3);
  EXPECT_EQ(ret, -1);

  setbits(test_data, 14, 8, 22);
  ret = getbits(test_data, 14, 8);
  EXPECT_EQ(ret, 22);

  setbits(test_data, 24, 32, -1);
  ret = getbits(test_data, 24, 32);
  EXPECT_EQ(ret, -1);
}

TEST(TestBitUtils, Setbitul) {
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
      EXPECT_EQ(ret, data);

      /* Clear data and make sure that no additional bits have changed */
      setbitul(test_data, pos, len, 0);
      EXPECT_EQ(memcmp(test_data, zeroes, sizeof(test_data)), 0)
          << "test case 2 not completely zeroed";
    }
  }
}

TEST(TestBitUtils, Setbitsl) {
  u8 test_data[64] = {0};
  s64 ret = 0;

  s64 input = INT64_MIN;
  setbitsl(test_data, 0, 64, input);
  ret = getbitsl(test_data, 0, 64);
  EXPECT_EQ(ret, input);

  ret = 0;
  memset(test_data, 0, sizeof(test_data));
  input = 0xABCD;
  setbitsl(test_data, 32, 8, input);
  ret = getbitsl(test_data, 32, 8);
  EXPECT_EQ(ret, (s8)input);

  // This test case should fail due to buffer overflow. setbitsl need fixing.
  ret = 0;
  memset(test_data, 0, sizeof(test_data));
  input = 0xABCD;
  setbitsl(test_data, 56, 32, input);
  ret = getbitsl(test_data, 56, 32);
  EXPECT_EQ(ret, input);
}

TEST(TestBitUtils, Bitshl) {
  u8 src0[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res0[] = {0xBE, 0xEF, 0x00, 0x00};

  u8 src1[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res1[] = {0xEA, 0xDB, 0xEE, 0xF0};

  u8 src2[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res2[] = {0xDB, 0xEE, 0xF0, 0x00};

  u8 src3[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res3[] = {0xB6, 0xFB, 0xBC, 0x00};

  bitshl(src0, sizeof(src0), 16);
  EXPECT_EQ(0, memcmp(src0, res0, 4));

  bitshl(src1, sizeof(src1), 4);
  EXPECT_EQ(0, memcmp(src1, res1, 4));

  bitshl(src2, sizeof(src2), 12);
  EXPECT_EQ(0, memcmp(src2, res2, 4));

  bitshl(src3, sizeof(src3), 10);
  EXPECT_EQ(0, memcmp(src3, res3, 4));
}

TEST(TestBitUtils, Bitcopy) {
  u8 src0[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res0[] = {0xBE, 0xEF, 0xBE, 0xEF};

  u8 src1[] = {0xDE, 0xAD, 0xBE, 0xEF};
  u8 res1[] = {0xEA, 0xDB, 0xEE, 0xFF};

  u8 src2[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD};
  // u8 dst2[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD};
  u8 res2[] = {0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xAD};

  bitcopy(src0, 0, src0, 16, 16);
  EXPECT_EQ(0, memcmp(src0, res0, 4));

  bitcopy(src1, 0, src1, 4, 28);
  EXPECT_EQ(0, memcmp(src1, res1, 4));

  bitcopy(src2, 0, src2, 8, 72);
  EXPECT_EQ(0, memcmp(src2, res2, 4));
}

TEST(TestBitUtils, CountBitsX) {
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
    EXPECT_EQ(res8[i], r);
    r = count_bits_u8(src8[i], 0);
    EXPECT_EQ(8 - res8[i], r);
  }

  for (unsigned i = 0; i < sizeof(src16) / sizeof(src16[0]); i++) {
    u8 r = count_bits_u16(src16[i], 1);
    EXPECT_EQ(res16[i], r);
    r = count_bits_u16(src16[i], 0);
    EXPECT_EQ(16 - res16[i], r);
  }

  for (unsigned i = 0; i < sizeof(src32) / sizeof(src32[0]); i++) {
    u8 r = count_bits_u32(src32[i], 1);
    EXPECT_EQ(res32[i], r);
    r = count_bits_u32(src32[i], 0);
    EXPECT_EQ(32 - res32[i], r);
  }

  for (unsigned i = 0; i < sizeof(src64) / sizeof(src64[0]); i++) {
    u8 r = count_bits_u64(src64[i], 1);
    EXPECT_EQ(res64[i], r);
    r = count_bits_u64(src64[i], 0);
    EXPECT_EQ(64 - res64[i], r);
  }
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbitfield-constant-conversion"
#endif

TEST(TestBitUtils, SignExtend32) {
  EXPECT_EQ(BITS_SIGN_EXTEND_32(5, 0), 0);
  EXPECT_EQ(BITS_SIGN_EXTEND_32(5, 1), 1);
  EXPECT_EQ(BITS_SIGN_EXTEND_32(5, 15), 15);
  EXPECT_EQ(BITS_SIGN_EXTEND_32(5, 0x1F), -1);
  EXPECT_EQ(BITS_SIGN_EXTEND_32(5, 0x10), -16);
}

TEST(TestBitUtils, SignExtend64) {
  EXPECT_EQ(BITS_SIGN_EXTEND_64(33, 0), 0);
  EXPECT_EQ(BITS_SIGN_EXTEND_64(33, 1), 1);
  EXPECT_EQ(BITS_SIGN_EXTEND_64(33, INT64_C(0xFFFFFFFF)), INT64_C(0xFFFFFFFF));
  EXPECT_EQ(BITS_SIGN_EXTEND_64(33, INT64_C(0x1FFFFFFFF)), -INT64_C(0x1));
  EXPECT_EQ(BITS_SIGN_EXTEND_64(33, INT64_C(0x100000000)),
            -INT64_C(0x100000000));
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

TEST(TestBitUtils, Endianess) {
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
  EXPECT_EQ(get_endianess(), expected_endianess);
  // Order of elements in value array is big, little, host
  EXPECT_EQ(byte_swap_16(u16_values[0]), u16_values[1]);
  EXPECT_EQ(byte_swap_16(u16_values[1]), u16_values[0]);
  EXPECT_EQ(betoh_16(u16_values[0]), u16_values[2]);
  EXPECT_EQ(letoh_16(u16_values[1]), u16_values[2]);
  EXPECT_EQ(htobe_16(u16_values[2]), u16_values[0]);
  EXPECT_EQ(htole_16(u16_values[2]), u16_values[1]);

  EXPECT_EQ(byte_swap_32(u32_values[0]), u32_values[1]);
  EXPECT_EQ(byte_swap_32(u32_values[1]), u32_values[0]);
  EXPECT_EQ(betoh_32(u32_values[0]), u32_values[2]);
  EXPECT_EQ(letoh_32(u32_values[1]), u32_values[2]);
  EXPECT_EQ(htobe_32(u32_values[2]), u32_values[0]);
  EXPECT_EQ(htole_32(u32_values[2]), u32_values[1]);

  EXPECT_EQ(byte_swap_64(u64_values[0]), u64_values[1]);
  EXPECT_EQ(byte_swap_64(u64_values[1]), u64_values[0]);
  EXPECT_EQ(betoh_64(u64_values[0]), u64_values[2]);
  EXPECT_EQ(letoh_64(u64_values[1]), u64_values[2]);
  EXPECT_EQ(htobe_64(u64_values[2]), u64_values[0]);
  EXPECT_EQ(htole_64(u64_values[2]), u64_values[1]);
#else
// No compiler indication of endianess, tests disabled
#endif
}

}  // namespace
