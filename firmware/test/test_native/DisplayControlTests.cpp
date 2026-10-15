#include "unity.h"
#include "../../lib/cw-commons/DisplayControl.h"

static void test_display_query_reads_brightness(void)
{
  cw::DisplayCommand cmd;
  TEST_ASSERT_TRUE(cw::parseDisplayQuery("bright=128", cmd));
  TEST_ASSERT_EQUAL_INT(128, cmd.bright);
  TEST_ASSERT_EQUAL_INT(-1, cmd.sleep);
}

static void test_display_query_reads_sleep_and_brightness(void)
{
  cw::DisplayCommand cmd;
  TEST_ASSERT_TRUE(cw::parseDisplayQuery("sleep=1&bright=0", cmd));
  TEST_ASSERT_EQUAL_INT(1, cmd.sleep);
  TEST_ASSERT_EQUAL_INT(0, cmd.bright);
  TEST_ASSERT_TRUE(cw::parseDisplayQuery("sleep=0", cmd));
  TEST_ASSERT_EQUAL_INT(0, cmd.sleep);
}

static void test_display_query_rejects_bad_values(void)
{
  cw::DisplayCommand cmd;
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("bright=256", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("bright=-1", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("bright=12.5", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("bright=", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("sleep=2", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("brightness=10", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery("", cmd));
  TEST_ASSERT_FALSE(cw::parseDisplayQuery(nullptr, cmd));
}

static void test_display_query_keeps_valid_part(void)
{
  cw::DisplayCommand cmd;
  TEST_ASSERT_TRUE(cw::parseDisplayQuery("bright=abc&sleep=1", cmd));
  TEST_ASSERT_EQUAL_INT(-1, cmd.bright);
  TEST_ASSERT_EQUAL_INT(1, cmd.sleep);
}

static void test_output_brightness_is_dark_while_asleep(void)
{
  TEST_ASSERT_EQUAL_UINT8(0, cw::outputBrightness(true, 200));
  TEST_ASSERT_EQUAL_UINT8(200, cw::outputBrightness(false, 200));
}

static void test_display_state_json(void)
{
  char buf[160];
  cw::formatDisplayState(buf, sizeof(buf), 64, true, false, 7, 1, false);
  TEST_ASSERT_EQUAL_STRING(
      "{\"bright\":64,\"sleep\":true,\"autoBright\":false,\"clockface\":7,\"savedClockface\":1,\"avr\":false}", buf);
}

void run_display_control_tests(void)
{
  RUN_TEST(test_display_query_reads_brightness);
  RUN_TEST(test_display_query_reads_sleep_and_brightness);
  RUN_TEST(test_display_query_rejects_bad_values);
  RUN_TEST(test_display_query_keeps_valid_part);
  RUN_TEST(test_output_brightness_is_dark_while_asleep);
  RUN_TEST(test_display_state_json);
}
