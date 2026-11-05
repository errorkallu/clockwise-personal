#pragma once

// All seven clockfaces are built into one firmware. Each one names its class "Clockface" and
// declares globals with common names, so every clockface is compiled inside its own namespace
// (see cf01.cpp ... cf07.cpp). The shared headers are included here first, outside any
// namespace, so their include guards keep them from being re-declared inside one.
#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <ArduinoJson.h>
#include <PNGdec.h>
#include "mbedtls/base64.h"

#include <Locator.h>
#include <Game.h>
#include <Tile.h>
#include <Object.h>
#include <Sprite.h>
#include <EventBus.h>
#include <EventTask.h>
#include <ImageUtils.h>
#include <ColorUtil.h>

#include <IClockface.h>
#include <CWDateTime.h>
#include <CWPreferences.h>
#include <CWHttpClient.h>
#include <StatusController.h>
#include <Icons.h>
#include <picopixel.h>

namespace cw {
IClockface *createClockface(uint8_t number, Adafruit_GFX *display);
}
