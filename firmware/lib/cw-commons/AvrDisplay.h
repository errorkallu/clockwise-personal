#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "AvrState.h"

// Full-screen AVR status page (64x64), shown instead of the clockface while the AVR is on.
//
//  AVR     10:42   <- header, time in the corner
//  -------------
//    -35.5         <- volume (size 2)
//  DOLBY           <- sound mode, up to two lines
//  DIGITAL
//  DynEQ   ON
//  DynVol OFF
namespace cw {

static const uint16_t AVR_COLOR_ACCENT = 0x05FF; // cyan
static const uint16_t AVR_COLOR_LINE = 0x0010;   // dim blue
static const uint16_t AVR_COLOR_TEXT = 0xFFFF;   // white
static const uint16_t AVR_COLOR_MODE = 0xFD20;   // amber
static const uint16_t AVR_COLOR_ON = 0x07E0;     // green
static const uint16_t AVR_COLOR_OFF = 0x6B4D;    // grey

inline void avrPrintRight(Adafruit_GFX *d, const char *text, int16_t y, uint16_t color)
{
  d->setTextColor(color);
  d->setCursor(64 - (int16_t)strlen(text) * 6, y);
  d->print(text);
}

inline void avrPrintFlag(Adafruit_GFX *d, const char *label, const char *value, int16_t y)
{
  d->setTextSize(1);
  d->setTextColor(AVR_COLOR_TEXT);
  d->setCursor(0, y);
  d->print(label);

  char shown[4];
  strncpy(shown, value[0] ? value : "--", sizeof(shown) - 1);
  shown[sizeof(shown) - 1] = '\0';
  for (char *c = shown; *c; c++) *c = toupper(*c);
  avrPrintRight(d, shown, y, isTruthy(value) ? AVR_COLOR_ON : AVR_COLOR_OFF);
}

inline void renderAvrScreen(Adafruit_GFX *d, const AvrState &s, const char *timeText)
{
  d->setFont();
  d->setTextWrap(false);
  d->fillScreen(0);

  // Header: label + time in the top-right corner
  d->setTextSize(1);
  d->setTextColor(AVR_COLOR_ACCENT);
  d->setCursor(0, 0);
  d->print("AVR");
  avrPrintRight(d, timeText, 0, AVR_COLOR_TEXT);
  d->drawFastHLine(0, 9, 64, AVR_COLOR_LINE);

  // Volume, centred, size 2 (12px per char)
  const char *vol = s.vol[0] ? s.vol : "--";
  d->setTextSize(2);
  d->setTextColor(AVR_COLOR_TEXT);
  int16_t w = (int16_t)strlen(vol) * 12;
  d->setCursor(w < 64 ? (64 - w) / 2 : 0, 13);
  d->print(vol);

  // Sound mode, two lines of 10 chars
  char line1[11], line2[11];
  wrapTwoLines(s.mode, 10, line1, line2, sizeof(line1));
  d->setTextSize(1);
  d->setTextColor(AVR_COLOR_MODE);
  d->setCursor(0, 32);
  d->print(line1);
  d->setCursor(0, 40);
  d->print(line2);

  avrPrintFlag(d, "DynEQ", s.deq, 49);
  avrPrintFlag(d, "DynVol", s.dvol, 57);
}

} // namespace cw
