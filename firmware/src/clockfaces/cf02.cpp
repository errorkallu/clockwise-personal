// Clockface cw-cf-0x02, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf02 {
#include "../../clockfaces/cw-cf-0x02/Clockface.cpp"
}

IClockface *createClockface0x02(Adafruit_GFX *display) { return new cf02::Clockface(display); }
