#pragma once

#include <stdint.h>
#include <string.h>
#include <ctype.h>

// AVR status pushed by Home Assistant to POST /avr?on=1&vol=-35.5&mode=...&deq=On&dvol=off
// Pure C++ so it can be unit tested natively.
namespace cw {

struct AvrState
{
  bool on = false;
  char vol[8] = "";
  char mode[32] = "";
  char deq[8] = "";
  char dvol[8] = "";
  uint32_t lastUpdateMs = 0;
};

// Home Assistant re-sends the state periodically while the AVR is on; if it goes
// quiet for this long (HA restarting, network issue) the clock falls back to its clockface.
static const uint32_t AVR_TIMEOUT_MS = 15UL * 60UL * 1000UL;

inline int hexValue(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// URL-decodes src[0..len) into dst (always NUL-terminated, truncated to dstSize).
inline void urlDecode(const char *src, size_t len, char *dst, size_t dstSize)
{
  size_t o = 0;
  for (size_t i = 0; i < len && o + 1 < dstSize; i++)
  {
    char c = src[i];
    if (c == '+')
    {
      c = ' ';
    }
    else if (c == '%' && i + 2 < len)
    {
      int hi = hexValue(src[i + 1]);
      int lo = hexValue(src[i + 2]);
      if (hi >= 0 && lo >= 0)
      {
        c = (char)(hi * 16 + lo);
        i += 2;
      }
    }
    dst[o++] = c;
  }
  dst[o] = '\0';
}

inline bool isTruthy(const char *v)
{
  return strcmp(v, "1") == 0 || strcmp(v, "on") == 0 || strcmp(v, "On") == 0 ||
         strcmp(v, "ON") == 0 || strcmp(v, "true") == 0 || strcmp(v, "True") == 0;
}

// Parses a query string (without the leading '?') into state. Unknown keys are ignored,
// keys that are absent keep their previous value.
inline void parseAvrQuery(const char *query, AvrState &state, uint32_t nowMs)
{
  const char *p = query;
  while (*p)
  {
    const char *amp = strchr(p, '&');
    size_t pairLen = amp ? (size_t)(amp - p) : strlen(p);
    const char *eq = (const char *)memchr(p, '=', pairLen);
    if (eq)
    {
      size_t keyLen = (size_t)(eq - p);
      const char *val = eq + 1;
      size_t valLen = pairLen - keyLen - 1;

      if (keyLen == 2 && strncmp(p, "on", 2) == 0)
      {
        char tmp[8];
        urlDecode(val, valLen, tmp, sizeof(tmp));
        state.on = isTruthy(tmp);
      }
      else if (keyLen == 3 && strncmp(p, "vol", 3) == 0)
        urlDecode(val, valLen, state.vol, sizeof(state.vol));
      else if (keyLen == 4 && strncmp(p, "mode", 4) == 0)
        urlDecode(val, valLen, state.mode, sizeof(state.mode));
      else if (keyLen == 3 && strncmp(p, "deq", 3) == 0)
        urlDecode(val, valLen, state.deq, sizeof(state.deq));
      else if (keyLen == 4 && strncmp(p, "dvol", 4) == 0)
        urlDecode(val, valLen, state.dvol, sizeof(state.dvol));
    }
    if (!amp) break;
    p = amp + 1;
  }
  state.lastUpdateMs = nowMs;
}

// True while the AVR screen should be shown. Unsigned subtraction survives millis() wraparound.
inline bool avrScreenActive(const AvrState &state, uint32_t nowMs)
{
  return state.on && (uint32_t)(nowMs - state.lastUpdateMs) < AVR_TIMEOUT_MS;
}

// Splits text into at most two lines of maxChars, breaking at the last space that fits.
inline void wrapTwoLines(const char *text, size_t maxChars, char *line1, char *line2, size_t bufSize)
{
  line1[0] = line2[0] = '\0';
  size_t len = strlen(text);
  if (len <= maxChars)
  {
    strncpy(line1, text, bufSize - 1);
    line1[bufSize - 1] = '\0';
    return;
  }
  size_t cut = maxChars;
  for (size_t i = maxChars; i > 0; i--)
  {
    if (text[i] == ' ')
    {
      cut = i;
      break;
    }
  }
  size_t n1 = cut < bufSize - 1 ? cut : bufSize - 1;
  memcpy(line1, text, n1);
  line1[n1] = '\0';

  const char *rest = text + cut;
  while (*rest == ' ') rest++;
  size_t n2 = strlen(rest);
  if (n2 > maxChars) n2 = maxChars;
  if (n2 > bufSize - 1) n2 = bufSize - 1;
  memcpy(line2, rest, n2);
  line2[n2] = '\0';
}

} // namespace cw
