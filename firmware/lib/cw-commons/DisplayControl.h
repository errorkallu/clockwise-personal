#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace cw
{

// A live display change pushed over HTTP: POST /display?bright=0..255&sleep=0|1
// -1 means the key was absent (or invalid) and that setting stays as it is.
struct DisplayCommand
{
  int bright = -1;
  int sleep = -1;
};

// Parses a whole decimal number in [minValue, maxValue]; anything else is rejected.
inline bool parseBoundedInt(const char *text, size_t len, int minValue, int maxValue, int &out)
{
  if (len == 0 || len > 5)
    return false;
  int value = 0;
  for (size_t i = 0; i < len; i++)
  {
    if (text[i] < '0' || text[i] > '9')
      return false;
    value = value * 10 + (text[i] - '0');
  }
  if (value < minValue || value > maxValue)
    return false;
  out = value;
  return true;
}

// Returns true when the query carries at least one valid setting.
inline bool parseDisplayQuery(const char *query, DisplayCommand &cmd)
{
  cmd = DisplayCommand();
  const char *p = query;
  while (p && *p)
  {
    const char *amp = strchr(p, '&');
    size_t pairLen = amp ? (size_t)(amp - p) : strlen(p);
    const char *eq = (const char *)memchr(p, '=', pairLen);
    if (eq)
    {
      size_t keyLen = (size_t)(eq - p);
      const char *val = eq + 1;
      size_t valLen = pairLen - keyLen - 1;
      int v;
      if (keyLen == 6 && strncmp(p, "bright", 6) == 0 && parseBoundedInt(val, valLen, 0, 255, v))
        cmd.bright = v;
      else if (keyLen == 5 && strncmp(p, "sleep", 5) == 0 && parseBoundedInt(val, valLen, 0, 1, v))
        cmd.sleep = v;
    }
    if (!amp)
      break;
    p = amp + 1;
  }
  return cmd.bright >= 0 || cmd.sleep >= 0;
}

// What the panel should be driven at: dark while asleep, otherwise the chosen level.
inline uint8_t outputBrightness(bool sleeping, uint8_t level)
{
  return sleeping ? 0 : level;
}

// GET /state body, read by a Home Assistant REST sensor.
inline int formatDisplayState(char *out, size_t size, uint8_t bright, bool sleeping, bool autoBright,
                              uint8_t clockface, uint8_t savedClockface, bool avrShown)
{
  return snprintf(out, size,
                  "{\"bright\":%u,\"sleep\":%s,\"autoBright\":%s,\"clockface\":%u,\"savedClockface\":%u,\"avr\":%s}",
                  (unsigned)bright, sleeping ? "true" : "false", autoBright ? "true" : "false",
                  (unsigned)clockface, (unsigned)savedClockface, avrShown ? "true" : "false");
}

} // namespace cw
