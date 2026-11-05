// Clockface cw-cf-0x03, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf03 {
#include "../../clockfaces/cw-cf-0x03/Clockface.cpp"
}

IClockface *createClockface0x03(Adafruit_GFX *display) { return new cf03::Clockface(display); }
