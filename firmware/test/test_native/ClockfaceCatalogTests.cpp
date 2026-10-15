#include "unity.h"
#include <string.h>

#include "../../lib/cw-commons/ClockfaceCatalog.h"

void test_catalog_lists_all_seven_clockfaces(void)
{
  TEST_ASSERT_EQUAL(7, cw::CLOCKFACE_COUNT);
  TEST_ASSERT_EQUAL_STRING("cw-cf-0x01", cw::clockfaceId(1));
  TEST_ASSERT_EQUAL_STRING("Mario", cw::clockfaceName(1));
  TEST_ASSERT_EQUAL_STRING("cw-cf-0x07", cw::clockfaceId(7));
  TEST_ASSERT_EQUAL_STRING("Canvas", cw::clockfaceName(7));
}

void test_catalog_falls_back_to_canvas_for_unknown_numbers(void)
{
  TEST_ASSERT_EQUAL(7, cw::validClockface(0));
  TEST_ASSERT_EQUAL(7, cw::validClockface(8));
  TEST_ASSERT_EQUAL(3, cw::validClockface(3));
  TEST_ASSERT_EQUAL_STRING("cw-cf-0x07", cw::clockfaceId(42));
}

void run_clockface_catalog_tests(void)
{
  RUN_TEST(test_catalog_lists_all_seven_clockfaces);
  RUN_TEST(test_catalog_falls_back_to_canvas_for_unknown_numbers);
}
