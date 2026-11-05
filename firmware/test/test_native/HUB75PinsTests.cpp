#include "unity.h"
#include <stdint.h>

#include "../../lib/cw-commons/HUB75Pins.h"

// Old firmware stored E_pin=18, which is the D line on the 38-pin board.
void test_legacy_e_pin_18_falls_back_to_32(void)
{
  TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin(18));
}

void test_valid_e_pin_is_kept(void)
{
  TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin(32));
  TEST_ASSERT_EQUAL_UINT8(22, cw::hub75::resolveEPin(22));
}

void test_e_pin_colliding_with_any_hub75_line_falls_back(void)
{
  const int8_t used[] = {25, 26, 27, 14, 12, 13, 23, 19, 5, 18, 4, 15, 16};
  for (unsigned i = 0; i < sizeof(used); i++)
  {
    TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin((uint8_t)used[i]));
  }
}

void test_out_of_range_e_pin_falls_back(void)
{
  TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin(34));
  TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin(39));
  TEST_ASSERT_EQUAL_UINT8(32, cw::hub75::resolveEPin(255));
}

void run_hub75_pins_tests(void)
{
  RUN_TEST(test_legacy_e_pin_18_falls_back_to_32);
  RUN_TEST(test_valid_e_pin_is_kept);
  RUN_TEST(test_e_pin_colliding_with_any_hub75_line_falls_back);
  RUN_TEST(test_out_of_range_e_pin_falls_back);
}
