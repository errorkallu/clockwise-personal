#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Clockfaces (all built in, picked on the settings page)
#include "clockfaces/ClockfaceWrap.h"
#include <ClockfaceCatalog.h>
// Commons
#include <WiFiController.h>
#include <CWDateTime.h>
#include <CWPreferences.h>
#include <CWWebServer.h>
#include <StatusController.h>
#include <HUB75Pins.h>
#include <AvrDisplay.h>
#include <DisplayControl.h>
#include <esp_system.h>

#define MIN_BRIGHT_DISPLAY_ON 4
#define MIN_BRIGHT_DISPLAY_OFF 0

#define ESP32_LED_BUILTIN 2

MatrixPanel_I2S_DMA *dma_display = nullptr;

IClockface *clockface;

WiFiController wifi;
CWDateTime cwDateTime;

bool avrShown = false;
uint32_t avrDrawnUpdateMs = 0;
String avrDrawnTime;

bool autoBrightEnabled;
long autoBrightMillis = 0;
uint8_t currentBrightSlot = -1;

// Sleep (display off) survives software restarts, e.g. the one after the AVR screen, but not power loss.
#define SLEEP_MAGIC 0x5EE9C10C
RTC_NOINIT_ATTR uint32_t sleepMagic;

bool isValidI2SSpeed(uint32_t speed) {
  return speed == 8000000 || speed == 16000000 || speed == 20000000;
}

bool isValidDriver(uint32_t drv) {
  return drv >= 0 && drv <= 5;
}



void displaySetup(bool swapBlueGreen, bool swapBlueRed, uint8_t displayBright, uint8_t displayRotation, uint8_t driver, uint32_t i2cSpeed, uint8_t E_pin)
{
  // Explicit pin map for the 38-pin ESP32 DevKit (see HUB75Pins.h).
  HUB75_I2S_CFG::i2s_pins pins = {
    cw::hub75::R1_PIN, cw::hub75::G1_PIN, cw::hub75::B1_PIN,
    cw::hub75::R2_PIN, cw::hub75::G2_PIN, cw::hub75::B2_PIN,
    cw::hub75::A_PIN, cw::hub75::B_PIN, cw::hub75::C_PIN, cw::hub75::D_PIN,
    (int8_t)cw::hub75::resolveEPin(E_pin),
    cw::hub75::LAT_PIN, cw::hub75::OE_PIN, cw::hub75::CLK_PIN
  };
  HUB75_I2S_CFG mxconfig(64, 64, 1, pins);

  if (swapBlueGreen)
  {
    // Swap Blue and Green pins because the panel is RBG instead of RGB.
    mxconfig.gpio.b1 = cw::hub75::G1_PIN;
    mxconfig.gpio.b2 = cw::hub75::G2_PIN;
    mxconfig.gpio.g1 = cw::hub75::B1_PIN;
    mxconfig.gpio.g2 = cw::hub75::B2_PIN;
  }

  if (swapBlueRed)
  {
    // Swap Blue and Red pins. 
    mxconfig.gpio.b1 = cw::hub75::R1_PIN;
    mxconfig.gpio.b2 = cw::hub75::R2_PIN;
    mxconfig.gpio.r1 = cw::hub75::B1_PIN;
    mxconfig.gpio.r2 = cw::hub75::B2_PIN;
  }

  if (mxconfig.gpio.e != E_pin) {
    Serial.printf("[WARN] E_pin %d conflicts with the HUB75 pin map, using GPIO %d\n", E_pin, mxconfig.gpio.e);
  }
  mxconfig.clkphase = false;

  if (isValidDriver(driver)) {
    mxconfig.driver = static_cast<HUB75_I2S_CFG::shift_driver>(driver);
  } else {
    Serial.printf("[ERROR] Invalid driver from config:%d\n", driver);
  }
  if (isValidI2SSpeed(i2cSpeed)) {
    mxconfig.i2sspeed = static_cast<HUB75_I2S_CFG::clk_speed>(i2cSpeed);
  } else {
    Serial.printf("[ERROR] Invalid I2S speed from config:%d\n", i2cSpeed);
  }

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(displayBright);
  dma_display->clearScreen();
  dma_display->setRotation(displayRotation);
}

void automaticBrightControl()
{
  if (autoBrightEnabled && !ClockwiseWebServer::getInstance()->displaySleeping) {
    if (millis() - autoBrightMillis > 3000)
    {
      int16_t currentValue = analogRead(ClockwiseParams::getInstance()->ldrPin);

      uint16_t ldrMin = ClockwiseParams::getInstance()->autoBrightMin;
      uint16_t ldrMax = ClockwiseParams::getInstance()->autoBrightMax;

      const uint8_t minBright = (currentValue < ldrMin ? MIN_BRIGHT_DISPLAY_OFF : MIN_BRIGHT_DISPLAY_ON);
      uint8_t maxBright = ClockwiseParams::getInstance()->displayBright;

      uint8_t slots = 10; //10 slots
      uint8_t mapLDR = map(currentValue > ldrMax ? ldrMax : currentValue, ldrMin, ldrMax, 1, slots);
      uint8_t mapBright = map(mapLDR, 1, slots, minBright, maxBright);

      // Serial.printf("LDR: %d, mapLDR: %d, Bright: %d\n", currentValue, mapLDR, mapBright);
      if(abs(currentBrightSlot - mapLDR ) >= 2 || mapBright == 0){
           dma_display->setBrightness8(mapBright);
           currentBrightSlot=mapLDR;
          //  Serial.printf("setBrightness: %d , Update currentBrightSlot to %d\n", mapBright, mapLDR);
      }
      autoBrightMillis = millis();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(ESP32_LED_BUILTIN, OUTPUT);

  StatusController::getInstance()->blink_led(5, 100);

  ClockwiseParams::getInstance()->load();

  pinMode(ClockwiseParams::getInstance()->ldrPin, INPUT);

  uint8_t driver = ClockwiseParams::getInstance()->driver;
  uint32_t i2cSpeed = ClockwiseParams::getInstance()->i2cSpeed;
  uint8_t E_pin = ClockwiseParams::getInstance()->E_pin;
  
  displaySetup(ClockwiseParams::getInstance()->swapBlueGreen, ClockwiseParams::getInstance()->swapBlueRed, ClockwiseParams::getInstance()->displayBright, ClockwiseParams::getInstance()->displayRotation, driver, i2cSpeed, E_pin);
  uint8_t clockfaceNumber = cw::validClockface(ClockwiseParams::getInstance()->clockface);
  Serial.printf("[Clockface] %s (%s)\n", cw::clockfaceName(clockfaceNumber), cw::clockfaceId(clockfaceNumber));
  ClockwiseWebServer::getInstance()->runningClockface = clockfaceNumber;
  clockface = cw::createClockface(clockfaceNumber, dma_display);

  autoBrightEnabled = (ClockwiseParams::getInstance()->autoBrightMax > 0);
  ClockwiseWebServer::getInstance()->autoBrightOn = autoBrightEnabled;
  if (esp_reset_reason() == ESP_RST_SW && sleepMagic == SLEEP_MAGIC) {
    Serial.println("[Display] Still asleep after restart");
    ClockwiseWebServer::getInstance()->displaySleeping = true;
    ClockwiseWebServer::getInstance()->displayDirty = true;
  }

  StatusController::getInstance()->clockwiseLogo();
  delay(1000);

  StatusController::getInstance()->wifiConnecting();
  wifi.begin(); // non-blocking; provisioning continues in loop()
}

// Applies brightness or sleep changes pushed to /display or /set without a restart.
void applyDisplayControl()
{
  ClockwiseWebServer *ws = ClockwiseWebServer::getInstance();
  if (!ws->displayDirty)
    return;
  ws->displayDirty = false;
  sleepMagic = ws->displaySleeping ? SLEEP_MAGIC : 0;
  currentBrightSlot = -1; // auto-brightness re-applies on its next pass
  if (ws->displaySleeping || !autoBrightEnabled)
    dma_display->setBrightness8(cw::outputBrightness(ws->displaySleeping, ClockwiseParams::getInstance()->displayBright));
}

// Runs once, as soon as Wi-Fi is up (stored credentials, Improv or AP portal).
void onNetworkReady()
{
  StatusController::getInstance()->ntpConnecting();
  cwDateTime.begin(ClockwiseParams::getInstance()->timeZone.c_str(), 
      ClockwiseParams::getInstance()->use24hFormat, 
      ClockwiseParams::getInstance()->ntpServer.c_str(),
      ClockwiseParams::getInstance()->manualPosix.c_str());
  clockface->setup(&cwDateTime);
}

// Draws the AVR page pushed by Home Assistant; only redraws when data or the minute changes.
void updateAvrScreen(const cw::AvrState &avr)
{
  bool use24h = ClockwiseParams::getInstance()->use24hFormat;
  String timeText = cwDateTime.getFormattedTime(use24h ? "H:i" : "g:i");
  String ampm = use24h ? String("") : cwDateTime.getFormattedTime("A");

  if (!avrShown || avr.lastUpdateMs != avrDrawnUpdateMs || timeText != avrDrawnTime)
  {
    cw::renderAvrScreen(dma_display, avr, timeText.c_str(), ampm.c_str());
    avrShown = true;
    avrDrawnUpdateMs = avr.lastUpdateMs;
    avrDrawnTime = timeText;
  }
}

void loop()
{
  if (wifi.update()) // also services Improv Serial on every pass
  {
    onNetworkReady();
  }

  if (wifi.isConnected())
  {
    ClockwiseWebServer::getInstance()->handleHttpRequest();
    ezt::events();
  }

  if (wifi.connectionSucessfulOnce)
  {
    const cw::AvrState &avr = ClockwiseWebServer::getInstance()->avrState;

    if (cw::avrScreenActive(avr, millis()))
    {
      updateAvrScreen(avr);
    }
    else if (avrShown)
    {
      // Clockfaces draw their background once in setup(), so restart to bring the clockface back cleanly.
      Serial.println("[AVR] Off, restoring clockface");
      StatusController::getInstance()->forceRestart();
    }
    else
    {
      clockface->update();
    }
  }

  ClockwiseWebServer::getInstance()->avrShownNow = avrShown;
  applyDisplayControl();
  automaticBrightControl();
}
