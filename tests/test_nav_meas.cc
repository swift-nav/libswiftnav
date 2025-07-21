#include <float.h>
#include <gtest/gtest.h>
#include <inttypes.h>
#include <stdio.h>
#include <swiftnav/nav_meas.h>

namespace {

TEST(TestNavMeas, EncodeLockTime) {
  u8 ret;

  ret = encode_lock_time(0.0);
  EXPECT_EQ(ret, 0);

  ret = encode_lock_time(0.05);
  EXPECT_EQ(ret, 1);

  ret = encode_lock_time(0.1);
  EXPECT_EQ(ret, 2);

  ret = encode_lock_time(0.2);
  EXPECT_EQ(ret, 3);

  ret = encode_lock_time(0.5);
  EXPECT_EQ(ret, 4);

  ret = encode_lock_time(1.0);
  EXPECT_EQ(ret, 5);

  ret = encode_lock_time(2.0);
  EXPECT_EQ(ret, 6);

  ret = encode_lock_time(4.0);
  EXPECT_EQ(ret, 7);

  ret = encode_lock_time(5.0);
  EXPECT_EQ(ret, 8);

  ret = encode_lock_time(10.0);
  EXPECT_EQ(ret, 9);

  ret = encode_lock_time(20.0);
  EXPECT_EQ(ret, 10);

  ret = encode_lock_time(50.0);
  EXPECT_EQ(ret, 11);

  ret = encode_lock_time(100.0);
  EXPECT_EQ(ret, 12);

  ret = encode_lock_time(200.0);
  EXPECT_EQ(ret, 13);

  ret = encode_lock_time(500.0);
  EXPECT_EQ(ret, 14);

  ret = encode_lock_time(1000.0);
  EXPECT_EQ(ret, 15);

  ret = encode_lock_time(DBL_MAX);
  EXPECT_EQ(ret, 15);
}

TEST(TestNavMeas, DecodeLockTime) {
  double ret;

  ret = decode_lock_time(0);
  EXPECT_EQ(ret, 0.0);

  ret = decode_lock_time(0xF0);
  EXPECT_EQ(ret, 0.0);

  ret = decode_lock_time(1);
  EXPECT_EQ(ret, 0.032);

  ret = decode_lock_time(2);
  EXPECT_EQ(ret, 0.064);

  ret = decode_lock_time(3);
  EXPECT_EQ(ret, 0.128);

  ret = decode_lock_time(4);
  EXPECT_EQ(ret, 0.256);

  ret = decode_lock_time(5);
  EXPECT_EQ(ret, 0.512);

  ret = decode_lock_time(6);
  EXPECT_EQ(ret, 1.024);

  ret = decode_lock_time(7);
  EXPECT_EQ(ret, 2.048);

  ret = decode_lock_time(8);
  EXPECT_EQ(ret, 4.096);

  ret = decode_lock_time(9);
  EXPECT_EQ(ret, 8.192);

  ret = decode_lock_time(10);
  EXPECT_EQ(ret, 16.384);

  ret = decode_lock_time(11);
  EXPECT_EQ(ret, 32.768);

  ret = decode_lock_time(12);
  EXPECT_EQ(ret, 65.536);

  ret = decode_lock_time(13);
  EXPECT_EQ(ret, 131.072);

  ret = decode_lock_time(14);
  EXPECT_EQ(ret, 262.144);

  ret = decode_lock_time(15);
  EXPECT_EQ(ret, 524.288);
}

TEST(TestNavMeas, RoundtripLockTime) {
  const double value_to_encode = 260.0;
  u8 encoded_value;
  double decoded_value;

  encoded_value = encode_lock_time(value_to_encode);
  decoded_value = decode_lock_time(encoded_value);

  EXPECT_EQ(encoded_value, 13);

  EXPECT_EQ(decoded_value, 131.072);

  EXPECT_LT(decoded_value, value_to_encode);
}

}  // namespace
