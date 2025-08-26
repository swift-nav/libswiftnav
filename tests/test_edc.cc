#include <gtest/gtest.h>
#include <swiftnav/edc.h>

namespace {

const u8 *test_data = (const u8 *)"123456789";

TEST(TestEdc, Crc24q) {
  u32 crc;

  crc = crc24q(test_data, 0, 0);
  EXPECT_EQ(crc, 0)
      << "CRC of empty buffer with starting value 0 should be 0, not " << crc;

  crc = crc24q(test_data, 0, 22);
  EXPECT_EQ(crc, 22)
      << "CRC of empty buffer with starting value 22 should be 22, not " << crc;

  /* Test value taken from python crcmod package tests, see:
   * http://crcmod.sourceforge.net/crcmod.predefined.html */
  crc = crc24q(test_data, 9, 0xB704CE);
  EXPECT_EQ(crc, 0x21CF02)
      << "CRC of \"123456789\" with init value 0xB704CE should be 0x21CF02, "
         "not "
      << crc;
}

}  // namespace
