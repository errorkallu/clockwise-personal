#include "unity.h"
#include <stdint.h>

#include "../../lib/cw-commons/AvrLogos.h"

void test_avr_query_parses_all_fields(void)
{
  cw::AvrState s;
  cw::parseAvrQuery("on=1&vol=-35.5&mode=DOLBY+DIGITAL&deq=On&dvol=off", s, 1000);
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL_STRING("-35.5", s.vol);
  TEST_ASSERT_EQUAL_STRING("DOLBY DIGITAL", s.mode);
  TEST_ASSERT_EQUAL_STRING("On", s.deq);
  TEST_ASSERT_EQUAL_STRING("off", s.dvol);
  TEST_ASSERT_EQUAL_UINT32(1000, s.lastUpdateMs);
}

void test_avr_query_decodes_percent_escapes(void)
{
  cw::AvrState s;
  cw::parseAvrQuery("mode=Multi%20Ch%2FStereo&vol=-40%", s, 0);
  TEST_ASSERT_EQUAL_STRING("Multi Ch/Stereo", s.mode);
  // A trailing '%' without two hex digits is kept literally
  TEST_ASSERT_EQUAL_STRING("-40%", s.vol);
}

void test_avr_query_keeps_absent_keys_and_ignores_unknown(void)
{
  cw::AvrState s;
  cw::parseAvrQuery("on=on&vol=-30&mode=STEREO", s, 0);
  cw::parseAvrQuery("vol=-25&foo=bar&novalue", s, 0);
  TEST_ASSERT_TRUE(s.on);
  TEST_ASSERT_EQUAL_STRING("-25", s.vol);
  TEST_ASSERT_EQUAL_STRING("STEREO", s.mode);
}

void test_avr_query_off_and_truncation(void)
{
  cw::AvrState s;
  cw::parseAvrQuery("on=1&mode=AN+EXTREMELY+LONG+SOUND+MODE+NAME+THAT+OVERFLOWS", s, 0);
  TEST_ASSERT_EQUAL_UINT(sizeof(s.mode) - 1, strlen(s.mode));
  cw::parseAvrQuery("on=0", s, 0);
  TEST_ASSERT_FALSE(s.on);
  cw::parseAvrQuery("on=off", s, 0);
  TEST_ASSERT_FALSE(s.on);
}

void test_avr_screen_times_out_and_survives_wraparound(void)
{
  cw::AvrState s;
  TEST_ASSERT_FALSE(cw::avrScreenActive(s, 0));
  cw::parseAvrQuery("on=1", s, 0xFFFFFF00UL);
  TEST_ASSERT_TRUE(cw::avrScreenActive(s, 0xFFFFFF00UL));
  TEST_ASSERT_TRUE(cw::avrScreenActive(s, 0x00001000UL)); // millis() wrapped
  TEST_ASSERT_FALSE(cw::avrScreenActive(s, (uint32_t)(0xFFFFFF00UL + cw::AVR_TIMEOUT_MS)));
}

void test_wrap_two_lines(void)
{
  char l1[11], l2[11];
  cw::wrapTwoLines("STEREO", 10, l1, l2, sizeof(l1));
  TEST_ASSERT_EQUAL_STRING("STEREO", l1);
  TEST_ASSERT_EQUAL_STRING("", l2);

  cw::wrapTwoLines("DOLBY DIGITAL", 10, l1, l2, sizeof(l1));
  TEST_ASSERT_EQUAL_STRING("DOLBY", l1);
  TEST_ASSERT_EQUAL_STRING("DIGITAL", l2);

  cw::wrapTwoLines("MULTICHANNELSTEREO", 10, l1, l2, sizeof(l1));
  TEST_ASSERT_EQUAL_STRING("MULTICHANN", l1);
  TEST_ASSERT_EQUAL_STRING("ELSTEREO", l2);
}

void test_avr_query_parses_source_and_app(void)
{
  cw::AvrState s;
  cw::parseAvrQuery("on=1&src=Watch%20Apple%20TV&app=YouTube", s, 0);
  TEST_ASSERT_EQUAL_STRING("Watch Apple TV", s.src);
  TEST_ASSERT_EQUAL_STRING("YouTube", s.app);
  cw::parseAvrQuery("src=Turn%20Table&app=", s, 0);
  TEST_ASSERT_EQUAL_STRING("Turn Table", s.src);
  TEST_ASSERT_EQUAL_STRING("", s.app);
}

void test_source_icons_match_harmony_activities(void)
{
  TEST_ASSERT_TRUE(cw::sourceIcon("Watch Apple TV") == cw::AvrIcon::AppleTv);
  TEST_ASSERT_TRUE(cw::sourceIcon("Shield Media player") == cw::AvrIcon::Nvidia);
  TEST_ASSERT_TRUE(cw::sourceIcon("Blu-ray") == cw::AvrIcon::BluRay);
  TEST_ASSERT_TRUE(cw::sourceIcon("Turn Table") == cw::AvrIcon::Turntable);
  TEST_ASSERT_TRUE(cw::sourceIcon("XBOX") == cw::AvrIcon::Xbox);
  TEST_ASSERT_TRUE(cw::sourceIcon("") == cw::AvrIcon::None);
}

void test_same_app_name_ignores_case_and_punctuation(void)
{
  TEST_ASSERT_TRUE(cw::sameAppName("Disney+", "disney +"));
  TEST_ASSERT_TRUE(cw::sameAppName("Prime Video", "PRIME-VIDEO"));
  TEST_ASSERT_FALSE(cw::sameAppName("YouTube", "YouTube Kids"));
  TEST_ASSERT_FALSE(cw::sameAppName("Disney+ Hotstar", "Disney+"));
  TEST_ASSERT_FALSE(cw::sameAppName("", "Netflix"));
}

void test_app_logos(void)
{
  TEST_ASSERT_NOT_NULL(cw::appLogo("Netflix"));
  TEST_ASSERT_NOT_NULL(cw::appLogo("YouTube Kids"));
  TEST_ASSERT_TRUE(cw::appLogo("YouTube") != cw::appLogo("YouTube Kids"));
  TEST_ASSERT_TRUE(cw::appLogo("Disney+") == cw::AVR_ICON_DISNEY);
  TEST_ASSERT_TRUE(cw::appLogo("TV") == cw::AVR_ICON_APPLETV);
  TEST_ASSERT_TRUE(cw::appLogo("Apple TV") == cw::AVR_ICON_APPLETV);
  // No logo: the screen writes the name instead
  TEST_ASSERT_NULL(cw::appLogo("ETV Win"));
  TEST_ASSERT_NULL(cw::appLogo("Sneaky Sasquatch"));
  TEST_ASSERT_NULL(cw::appLogo(""));
}

void test_source_logos(void)
{
  TEST_ASSERT_TRUE(cw::sourceLogo(cw::AvrIcon::Nvidia) == cw::AVR_ICON_NVIDIA);
  TEST_ASSERT_NULL(cw::sourceLogo(cw::AvrIcon::None));
}

void test_split_volume(void)
{
  char whole[8], frac[8];
  cw::splitVolume("-30.0", whole, frac, sizeof(whole));
  TEST_ASSERT_EQUAL_STRING("-30", whole);
  TEST_ASSERT_EQUAL_STRING("", frac);
  cw::splitVolume("-76.5", whole, frac, sizeof(whole));
  TEST_ASSERT_EQUAL_STRING("-76", whole);
  TEST_ASSERT_EQUAL_STRING(".5", frac);
  cw::splitVolume("-8", whole, frac, sizeof(whole));
  TEST_ASSERT_EQUAL_STRING("-8", whole);
  TEST_ASSERT_EQUAL_STRING("", frac);
}

void run_avr_state_tests(void)
{
  RUN_TEST(test_avr_query_parses_all_fields);
  RUN_TEST(test_avr_query_decodes_percent_escapes);
  RUN_TEST(test_avr_query_keeps_absent_keys_and_ignores_unknown);
  RUN_TEST(test_avr_query_off_and_truncation);
  RUN_TEST(test_avr_screen_times_out_and_survives_wraparound);
  RUN_TEST(test_wrap_two_lines);
  RUN_TEST(test_avr_query_parses_source_and_app);
  RUN_TEST(test_source_icons_match_harmony_activities);
  RUN_TEST(test_same_app_name_ignores_case_and_punctuation);
  RUN_TEST(test_app_logos);
  RUN_TEST(test_source_logos);
  RUN_TEST(test_split_volume);
}

static void test_avr_query_reads_vpn(void)
{
  cw::AvrState s;
  TEST_ASSERT_EQUAL_INT(-1, s.vpn);
  cw::parseAvrQuery("on=1&vpn=1&vpnc=India", s, 0);
  TEST_ASSERT_EQUAL_INT(1, s.vpn);
  TEST_ASSERT_EQUAL_STRING("India", s.vpnc);
  cw::parseAvrQuery("vpn=0&vpnc=", s, 0);
  TEST_ASSERT_EQUAL_INT(0, s.vpn);
  cw::parseAvrQuery("vpn=", s, 0);
  TEST_ASSERT_EQUAL_INT(-1, s.vpn);
}

static void test_vpn_line_fits_width(void)
{
  char label[8], status[8], code[8];
  cw::fitVpnLine(true, "IND", 47, label, status, code);
  TEST_ASSERT_EQUAL_STRING("VPN:", label);
  TEST_ASSERT_EQUAL_STRING("ON", status);
  TEST_ASSERT_EQUAL_STRING("IND", code);
  TEST_ASSERT_EQUAL_INT(35, cw::tomThumbWidth("VPN:ON IND"));
  // Longer text is cut to 3 letters
  cw::fitVpnLine(true, "Canada", 47, label, status, code);
  TEST_ASSERT_EQUAL_STRING("CAN", code);
  // Two logos leave 31px: the label goes first
  cw::fitVpnLine(true, "IND", 31, label, status, code);
  TEST_ASSERT_EQUAL_STRING("", label);
  TEST_ASSERT_EQUAL_STRING("IND", code);
  cw::fitVpnLine(false, "IND", 31, label, status, code);
  TEST_ASSERT_EQUAL_STRING("VPN:", label);
  TEST_ASSERT_EQUAL_STRING("OFF", status);
  TEST_ASSERT_EQUAL_STRING("", code);
}

void run_avr_vpn_tests(void)
{
  RUN_TEST(test_avr_query_reads_vpn);
  RUN_TEST(test_vpn_line_fits_width);
}
