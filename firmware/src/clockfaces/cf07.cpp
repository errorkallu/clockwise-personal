// Clockface cw-cf-0x07, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf07 {
#include "../../clockfaces/cw-cf-0x07/Clockface.cpp"
#include "../../clockfaces/cw-cf-0x07/CustomSprite.cpp"
}

IClockface *createClockface0x07(Adafruit_GFX *display) { return new cf07::Clockface(display); }
