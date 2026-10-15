#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/TomThumb.h>
#include "AvrState.h"
#include "AvrLogos.h"

// Full-screen AVR status page (64x64), shown instead of the clockface while the AVR is on.
//
//  [src][app] 12:30PM   <- source + app logos, time with AM/PM beside it
//   (app name)          <- only when the app has no logo
//     -30 dB            <- volume, large
//      Dolby            <- sound mode, up to two centred lines
//    Digital +
//  DYN VOL:ON EQ:ON
namespace cw {

static const uint16_t AVR_COLOR_TIME = 0x07E0;  // green
static const uint16_t AVR_COLOR_TEXT = 0xFFFF;  // white
static const uint16_t AVR_COLOR_APP = 0x07FF;   // cyan
static const uint16_t AVR_COLOR_MODE = 0xF81F;  // magenta
static const uint16_t AVR_COLOR_LABEL = 0xFC60; // orange
static const uint16_t AVR_COLOR_OFF = 0x6B4D;   // grey

inline int16_t avrTextWidth(Adafruit_GFX *d, const char *text)
{
  int16_t x1, y1;
  uint16_t w, h;
  d->getTextBounds(text, 0, 32, &x1, &y1, &w, &h);
  return (int16_t)w;
}

// Draws text with its top-left pixel at (x, top), whatever font is active.
inline void avrPrintAt(Adafruit_GFX *d, const char *text, int16_t x, int16_t top, uint16_t color)
{
  int16_t x1, y1;
  uint16_t w, h;
  d->getTextBounds(text, 0, 32, &x1, &y1, &w, &h);
  d->setTextColor(color);
  d->setCursor(x - x1, top + (32 - y1));
  d->print(text);
}

inline void avrPrintCentered(Adafruit_GFX *d, const char *text, int16_t top, uint16_t color)
{
  int16_t x = (64 - avrTextWidth(d, text)) / 2;
  avrPrintAt(d, text, x < 0 ? 0 : x, top, color);
}

// Width of text in the built-in 5x7 font with 1px between glyphs' inked columns, so the
// time fits beside two logos ("1" and ":" are much narrower than their 6px cell).
inline int16_t avrTightWidth(const char *text)
{
  int16_t w = 0;
  for (const char *c = text; *c; c++)
  {
    w += (*c == '1') ? 3 : (*c == ':') ? 1 : 5;
    if (c[1]) w += 1;
  }
  return w;
}

inline void avrPrintTight(Adafruit_GFX *d, const char *text, int16_t x, int16_t top, uint16_t color)
{
  d->setTextColor(color);
  for (const char *c = text; *c; c++)
  {
    // Inked columns inside the 6px cell: '1' uses 1..3, ':' uses 2, the rest 0..4.
    int16_t skip = (*c == '1') ? 1 : (*c == ':') ? 2 : 0;
    d->drawChar(x - skip, top, *c, color, color, 1); // bg == fg draws transparently
    x += ((*c == '1') ? 3 : (*c == ':') ? 1 : 5) + 1;
  }
}

inline void renderAvrScreen(Adafruit_GFX *d, const AvrState &s, const char *timeText, const char *ampm)
{
  d->setTextWrap(false);
  d->setTextSize(1);
  d->fillScreen(0);

  // Row 1: source logo, app logo, time with AM/PM beside it
  const uint16_t *src = sourceLogo(sourceIcon(s.src));
  const uint16_t *app = appLogo(s.app);
  if (src) d->drawRGBBitmap(0, 0, src, 16, 16);
  if (app) d->drawRGBBitmap(src ? 16 : 0, 0, app, 16, 16);

  d->setFont(&TomThumb);
  int16_t ampmW = ampm[0] ? avrTextWidth(d, ampm) + 1 : 0;
  int16_t timeX = 64 - ampmW - avrTightWidth(timeText);
  d->setFont();
  avrPrintTight(d, timeText, timeX, 4, AVR_COLOR_TIME);
  if (ampm[0])
  {
    d->setFont(&TomThumb);
    avrPrintAt(d, ampm, 64 - ampmW + 1, 6, AVR_COLOR_TIME);
  }

  // Apps without a logo get their name on its own line, and the volume shrinks to make room
  bool nameLine = !app && s.app[0];
  if (nameLine)
  {
    d->setFont(&TomThumb);
    avrPrintCentered(d, s.app, 18, AVR_COLOR_APP);
  }

  // Row 2: volume, large, with a small "dB"
  char whole[8], frac[8];
  splitVolume(s.vol[0] ? s.vol : "--", whole, frac, sizeof(whole));
  char number[16];
  snprintf(number, sizeof(number), "%s%s", whole, frac);

  d->setFont();
  int16_t unitW = avrTextWidth(d, "dB");
  d->setFont(&FreeSansBold12pt7b);
  int16_t numW = avrTextWidth(d, number);
  int16_t numTop = 19;
  if (nameLine || numW + 2 + unitW > 64)
  {
    d->setFont(&FreeSansBold9pt7b);
    numW = avrTextWidth(d, number);
    numTop = nameLine ? 25 : 21;
  }
  int16_t x = (64 - (numW + 2 + unitW)) / 2;
  if (x < 0) x = 0;
  avrPrintAt(d, number, x, numTop, AVR_COLOR_TEXT);
  d->setFont();
  avrPrintAt(d, "dB", x + numW + 2, nameLine ? 31 : 29, AVR_COLOR_TEXT);

  // Row 3: sound mode, two centred lines of 10 chars
  char line1[11], line2[11];
  wrapTwoLines(s.mode, 10, line1, line2, sizeof(line1));
  avrPrintCentered(d, line1, 39, AVR_COLOR_MODE);
  avrPrintCentered(d, line2, 48, AVR_COLOR_MODE);

  // Row 4: DYN VOL:ON EQ:ON, centred (TomThumb capitals sit 5px above the baseline)
  d->setFont(&TomThumb);
  char values[2][8];
  const char *raw[2] = {s.dvol, s.deq};
  for (int i = 0; i < 2; i++)
  {
    const char *v = raw[i][0] ? raw[i] : "--";
    size_t n = 0;
    for (; v[n] && n < sizeof(values[i]) - 1; n++) values[i][n] = (char)toupper((unsigned char)v[n]);
    values[i][n] = '\0';
  }
  // Shorten the label, then the values ("MEDIUM" -> "MED"), until the line fits
  static const char *const dynLabels[] = {"DYN VOL:", "DYNVOL:", "DVOL:"};
  const char *labels[2] = {dynLabels[0], " EQ:"};
  char full[40];
  for (int step = 0; step < 4; step++)
  {
    if (step < 3) labels[0] = dynLabels[step];
    else values[0][3] = values[1][3] = '\0';
    snprintf(full, sizeof(full), "%s%s%s%s", labels[0], values[0], labels[1], values[1]);
    if (avrTextWidth(d, full) <= 64) break;
  }
  int16_t bx = (64 - avrTextWidth(d, full)) / 2;
  d->setCursor(bx < 0 ? 0 : bx, 64);
  for (int i = 0; i < 2; i++)
  {
    d->setTextColor(AVR_COLOR_LABEL);
    d->print(labels[i]);
    d->setTextColor(isTruthy(raw[i]) ? AVR_COLOR_TEXT : AVR_COLOR_OFF);
    d->print(values[i]);
  }

  d->setFont();
}

} // namespace cw
