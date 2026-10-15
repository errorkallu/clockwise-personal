#pragma once

#include <stdint.h>

// The clockfaces built into the firmware, numbered like their repositories (0x01..0x07).
// Pure C++ so it can be unit tested natively.
namespace cw {

static const uint8_t CLOCKFACE_COUNT = 7;
static const uint8_t DEFAULT_CLOCKFACE = 7; // Canvas

inline uint8_t validClockface(uint8_t number)
{
  return (number >= 1 && number <= CLOCKFACE_COUNT) ? number : DEFAULT_CLOCKFACE;
}

inline const char *clockfaceId(uint8_t number)
{
  static const char *const ids[CLOCKFACE_COUNT] = {
    "cw-cf-0x01", "cw-cf-0x02", "cw-cf-0x03", "cw-cf-0x04", "cw-cf-0x05", "cw-cf-0x06", "cw-cf-0x07"};
  return ids[validClockface(number) - 1];
}

inline const char *clockfaceName(uint8_t number)
{
  static const char *const names[CLOCKFACE_COUNT] = {
    "Mario", "Words", "World Map", "Castlevania", "Pac-Man", "Pokedex", "Canvas"};
  return names[validClockface(number) - 1];
}

} // namespace cw
