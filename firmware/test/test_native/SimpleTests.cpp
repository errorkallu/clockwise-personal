#include "unity.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_function_should_be_true(void) {
  TEST_ASSERT_TRUE(true);
}

// Defined in WiFiSetupStateMachineTests.cpp
void run_wifi_setup_state_machine_tests(void);
// Defined in HUB75PinsTests.cpp
void run_hub75_pins_tests(void);
// Defined in AvrStateTests.cpp
void run_avr_state_tests(void);
// Defined in TimeFormatTests.cpp
void run_time_format_tests(void);
// Defined in StaticIpTests.cpp
void run_static_ip_tests(void);
// Defined in ClockfaceCatalogTests.cpp
void run_clockface_catalog_tests(void);

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_function_should_be_true);
  run_wifi_setup_state_machine_tests();
  run_hub75_pins_tests();
  run_avr_state_tests();
  run_time_format_tests();
  run_static_ip_tests();
  run_clockface_catalog_tests();
  return UNITY_END();
}


int main() {
  runUnityTests();  
}

