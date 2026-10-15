// Clockface cw-cf-0x05, compiled in its own namespace (see ClockfaceWrap.h).
#include "ClockfaceWrap.h"

namespace cf05 {
#include "../../clockfaces/cw-cf-0x05/Clockface.cpp"
#include "../../clockfaces/cw-cf-0x05/pacman.cpp"
}

IClockface *createClockface0x05(Adafruit_GFX *display) { return new cf05::Clockface(display); }
