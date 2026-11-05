#include "unity.h"

#include "../../lib/cw-commons/TimeFormat.h"

static char out[32];

void test_12h_setting_turns_24h_hours_into_12h(void)
{
  cw::adaptHourFormat("H:i", false, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("h:i", out);
  cw::adaptHourFormat("G:i:s", false, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("g:i:s", out);
  cw::adaptHourFormat("h:i:sa", false, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("h:i:sa", out);
}

void test_24h_setting_turns_12h_hours_into_24h_and_drops_am_pm(void)
{
  cw::adaptHourFormat("h:i:sa", true, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("H:i:s", out);
  cw::adaptHourFormat("g:i A", true, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("G:i", out);
  cw::adaptHourFormat("H:i", true, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("H:i", out);
}

void test_hour_format_leaves_other_fields_and_escapes_alone(void)
{
  cw::adaptHourFormat("M d", false, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("M d", out);
  // ezTime prints the character after '~' literally
  cw::adaptHourFormat("~H~a H", false, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("~H~a h", out);
  cw::adaptHourFormat("~a a", true, out, sizeof(out));
  TEST_ASSERT_EQUAL_STRING("~a", out);
}

void test_hour_format_truncates_to_buffer(void)
{
  char small[4];
  cw::adaptHourFormat("H:i:s", false, small, sizeof(small));
  TEST_ASSERT_EQUAL_STRING("h:i", small);
}

void run_time_format_tests(void)
{
  RUN_TEST(test_12h_setting_turns_24h_hours_into_12h);
  RUN_TEST(test_24h_setting_turns_12h_hours_into_24h_and_drops_am_pm);
  RUN_TEST(test_hour_format_leaves_other_fields_and_escapes_alone);
  RUN_TEST(test_hour_format_truncates_to_buffer);
}
