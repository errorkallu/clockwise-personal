#include "ClockfaceWrap.h"
#include <ClockfaceCatalog.h>

IClockface *createClockface0x01(Adafruit_GFX *display);
IClockface *createClockface0x02(Adafruit_GFX *display);
IClockface *createClockface0x03(Adafruit_GFX *display);
IClockface *createClockface0x04(Adafruit_GFX *display);
IClockface *createClockface0x05(Adafruit_GFX *display);
IClockface *createClockface0x06(Adafruit_GFX *display);
IClockface *createClockface0x07(Adafruit_GFX *display);

IClockface *cw::createClockface(uint8_t number, Adafruit_GFX *display)
{
  switch (cw::validClockface(number))
  {
  case 1: return createClockface0x01(display);
  case 2: return createClockface0x02(display);
  case 3: return createClockface0x03(display);
  case 4: return createClockface0x04(display);
  case 5: return createClockface0x05(display);
  case 6: return createClockface0x06(display);
  default: return createClockface0x07(display);
  }
}
