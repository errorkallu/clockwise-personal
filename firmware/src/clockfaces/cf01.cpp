// Clockface cw-cf-0x01, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf01 {
#include "../../clockfaces/cw-cf-0x01/Clockface.cpp"
#include "../../clockfaces/cw-cf-0x01/gfx/block.cpp"
#include "../../clockfaces/cw-cf-0x01/gfx/mario.cpp"
}

IClockface *createClockface0x01(Adafruit_GFX *display) { return new cf01::Clockface(display); }
