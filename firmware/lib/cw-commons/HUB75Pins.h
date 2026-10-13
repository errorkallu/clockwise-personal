#pragma once

#include <stdint.h>

// HUB75 wiring for a 38-pin ESP32 DevKit (esp32dev).
// Matches the panel wiring proven on this board with ESPHome:
//   R1=25 G1=26 B1=27 R2=14 G2=12 B2=13
//   A=23 B=19 C=5 D=18 E=32 LAT=4 OE=15 CLK=16
namespace cw {
namespace hub75 {

static const int8_t R1_PIN = 25;
static const int8_t G1_PIN = 26;
static const int8_t B1_PIN = 27;
static const int8_t R2_PIN = 14;
static const int8_t G2_PIN = 12;
static const int8_t B2_PIN = 13;

static const int8_t A_PIN = 23;
static const int8_t B_PIN = 19;
static const int8_t C_PIN = 5;
static const int8_t D_PIN = 18;
static const int8_t E_PIN = 32;

static const int8_t LAT_PIN = 4;
static const int8_t OE_PIN = 15;
static const int8_t CLK_PIN = 16;

// The E line pin is user configurable (NVS "E_pin"). Older firmware defaulted
// it to GPIO 18, which is now the D line, so a stale or out-of-range value
// falls back to the board default instead of driving two lines from one pin.
// GPIO 34-39 are input-only on the ESP32, so they cannot drive the E line.
inline uint8_t resolveEPin(uint8_t stored)
{
  if (stored > 33 || stored == (uint8_t)R1_PIN || stored == (uint8_t)G1_PIN || stored == (uint8_t)B1_PIN ||
      stored == (uint8_t)R2_PIN || stored == (uint8_t)G2_PIN || stored == (uint8_t)B2_PIN ||
      stored == (uint8_t)A_PIN || stored == (uint8_t)B_PIN || stored == (uint8_t)C_PIN || stored == (uint8_t)D_PIN ||
      stored == (uint8_t)LAT_PIN || stored == (uint8_t)OE_PIN || stored == (uint8_t)CLK_PIN)
  {
    return (uint8_t)E_PIN;
  }
  return stored;
}

} // namespace hub75
} // namespace cw
