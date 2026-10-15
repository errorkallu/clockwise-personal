// Clockface cw-cf-0x04, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf04 {
#include "../../clockfaces/cw-cf-0x04/Clockface.cpp"
}

IClockface *createClockface0x04(Adafruit_GFX *display) { return new cf04::Clockface(display); }
