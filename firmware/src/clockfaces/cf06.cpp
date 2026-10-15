// Clockface cw-cf-0x06, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf06 {
#include "../../clockfaces/cw-cf-0x06/Clockface.cpp"
}

IClockface *createClockface0x06(Adafruit_GFX *display) { return new cf06::Clockface(display); }
