#include "unity.h"

#include "../../lib/cw-commons/StaticIp.h"

void test_static_ip_parses_all_four_addresses(void)
{
  cw::StaticIpConfig c;
  TEST_ASSERT_TRUE(cw::parseStaticIp("192.168.10.54,192.168.10.1,255.255.255.0,1.1.1.1", c));
  uint8_t ip[4] = {192, 168, 10, 54}, gw[4] = {192, 168, 10, 1}, mask[4] = {255, 255, 255, 0}, dns[4] = {1, 1, 1, 1};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(ip, c.ip, 4);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(gw, c.gateway, 4);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(mask, c.subnet, 4);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(dns, c.dns, 4);
}

void test_static_ip_dns_defaults_to_gateway(void)
{
  cw::StaticIpConfig c;
  TEST_ASSERT_TRUE(cw::parseStaticIp(" 192.168.10.54 , 192.168.10.1 , 255.255.255.0 ", c));
  uint8_t gw[4] = {192, 168, 10, 1};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(gw, c.dns, 4);
  TEST_ASSERT_TRUE(cw::parseStaticIp("192.168.10.54,192.168.10.1,255.255.255.0,", c));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(gw, c.dns, 4);
}

void test_static_ip_empty_means_dhcp(void)
{
  cw::StaticIpConfig c;
  TEST_ASSERT_FALSE(cw::parseStaticIp("", c));
  TEST_ASSERT_FALSE(cw::parseStaticIp(",,,", c));
}

void test_static_ip_rejects_settings_that_would_lock_you_out(void)
{
  cw::StaticIpConfig c;
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.256,192.168.10.1,255.255.255.0", c)); // octet > 255
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10,192.168.10.1,255.255.255.0", c));     // 3 octets
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.54,192.168.10.1", c));                // no subnet
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.11.54,192.168.10.1,255.255.255.0", c));  // gateway on another subnet
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.54,192.168.10.1,255.0.255.0", c));    // mask not contiguous
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.255,192.168.10.1,255.255.255.0", c)); // broadcast address
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.0,192.168.10.1,255.255.255.0", c));   // network address
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.1,192.168.10.1,255.255.255.0", c));   // same as gateway
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.54,192.168.10.1,255.255.255.0,8.8.8", c));
  TEST_ASSERT_FALSE(cw::parseStaticIp("192.168.10.a4,192.168.10.1,255.255.255.0", c));
}

void run_static_ip_tests(void)
{
  RUN_TEST(test_static_ip_parses_all_four_addresses);
  RUN_TEST(test_static_ip_dns_defaults_to_gateway);
  RUN_TEST(test_static_ip_empty_means_dhcp);
  RUN_TEST(test_static_ip_rejects_settings_that_would_lock_you_out);
}
