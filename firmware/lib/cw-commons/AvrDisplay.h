#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/TomThumb.h>
#include "AvrState.h"
#include "AvrIcons.h"

// Full-screen AVR status page (64x64), shown instead of the clockface while the AVR is on.
//
//  [src][app]  12:30   <- source + app logos, time
//                 PM
//    -30 dB            <- volume, large
//  DOLBY               <- sound mode, up to two lines
//  DIGITAL
//  DYN VOL:ON EQ:ON
namespace cw {

static const uint16_t AVR_COLOR_TIME = 0x07E0;  // green
static const uint16_t AVR_COLOR_TEXT = 0xFFFF;  // white
static const uint16_t AVR_COLOR_MODE = 0xF81F;  // magenta
static const uint16_t AVR_COLOR_LABEL = 0xFC60; // orange
static const uint16_t AVR_COLOR_OFF = 0x6B4D;   // grey
static const uint16_t AVR_COLOR_TILE = 0x4208;  // dark grey

inline const uint16_t *avrIconBitmap(AvrIcon icon)
{
  switch (icon)
  {
  case AvrIcon::AppleTv: return AVR_ICON_APPLETV;
  case AvrIcon::Nvidia: return AVR_ICON_NVIDIA;
  case AvrIcon::BluRay: return AVR_ICON_BLURAY;
  case AvrIcon::Turntable: return AVR_ICON_TURNTABLE;
  case AvrIcon::Xbox: return AVR_ICON_XBOX;
  case AvrIcon::Kodi: return AVR_ICON_KODI;
  case AvrIcon::Laptop: return AVR_ICON_LAPTOP;
  case AvrIcon::Netflix: return AVR_ICON_NETFLIX;
  case AvrIcon::YouTube: return AVR_ICON_YOUTUBE;
  case AvrIcon::Plex: return AVR_ICON_PLEX;
  case AvrIcon::Disney: return AVR_ICON_DISNEY;
  case AvrIcon::Prime: return AVR_ICON_PRIME;
  case AvrIcon::Spotify: return AVR_ICON_SPOTIFY;
  case AvrIcon::Emby: return AVR_ICON_EMBY;
  case AvrIcon::Hulu: return AVR_ICON_HULU;
  case AvrIcon::Twitch: return AVR_ICON_TWITCH;
  default: return nullptr;
  }
}

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

// Logo for a known source/app; unknown apps get a grey tile with their first two letters.
inline void avrDrawIcon(Adafruit_GFX *d, AvrIcon icon, const char *name, int16_t x, int16_t y)
{
  const uint16_t *bitmap = avrIconBitmap(icon);
  if (bitmap)
  {
    d->drawRGBBitmap(x, y, bitmap, 16, 16);
    return;
  }
  if (!name[0]) return;

  char initials[3] = {(char)toupper((unsigned char)name[0]), 0, 0};
  if (name[1] && name[1] != ' ') initials[1] = (char)toupper((unsigned char)name[1]);
  d->fillRoundRect(x, y, 16, 16, 2, AVR_COLOR_TILE);
  d->setFont(&TomThumb);
  int16_t w = avrTextWidth(d, initials);
  avrPrintAt(d, initials, x + (16 - w) / 2, y + 5, AVR_COLOR_TEXT);
}

inline void renderAvrScreen(Adafruit_GFX *d, const AvrState &s, const char *timeText, const char *ampm)
{
  d->setTextWrap(false);
  d->setTextSize(1);
  d->fillScreen(0);

  // Row 1: source logo, app logo, time (with AM/PM underneath)
  avrDrawIcon(d, sourceIcon(s.src), "", 0, 0);
  avrDrawIcon(d, appIcon(s.app), s.app, 17, 0);

  d->setFont();
  avrPrintAt(d, timeText, 64 - avrTextWidth(d, timeText), 0, AVR_COLOR_TIME);
  if (ampm[0])
  {
    d->setFont(&TomThumb);
    avrPrintAt(d, ampm, 64 - avrTextWidth(d, ampm), 10, AVR_COLOR_TIME);
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
  if (numW + 2 + unitW > 64)
  {
    d->setFont(&FreeSansBold9pt7b);
    numW = avrTextWidth(d, number);
    numTop = 21;
  }
  int16_t x = (64 - (numW + 2 + unitW)) / 2;
  if (x < 0) x = 0;
  avrPrintAt(d, number, x, numTop, AVR_COLOR_TEXT);
  d->setFont();
  avrPrintAt(d, "dB", x + numW + 2, 29, AVR_COLOR_TEXT);

  // Row 3: sound mode, two lines of 10 chars
  char line1[11], line2[11];
  wrapTwoLines(s.mode, 10, line1, line2, sizeof(line1));
  avrPrintAt(d, line1, 0, 39, AVR_COLOR_MODE);
  avrPrintAt(d, line2, 0, 48, AVR_COLOR_MODE);

  // Row 4: DYN VOL:ON EQ:ON (TomThumb capitals sit 5px above the baseline)
  d->setFont(&TomThumb);
  d->setCursor(0, 64);
  const char *labels[2] = {"DYN VOL:", " EQ:"};
  const char *values[2] = {s.dvol, s.deq};
  for (int i = 0; i < 2; i++)
  {
    d->setTextColor(AVR_COLOR_LABEL);
    d->print(labels[i]);
    const char *v = values[i][0] ? values[i] : "--";
    d->setTextColor(isTruthy(values[i]) ? AVR_COLOR_TEXT : AVR_COLOR_OFF);
    for (; *v; v++) d->print((char)toupper((unsigned char)*v));
  }

  d->setFont();
}

} // namespace cw
