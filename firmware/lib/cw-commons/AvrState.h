#pragma once

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
  char src[24] = "";  // Harmony activity, e.g. "Watch Apple TV"
  char app[24] = "";  // app running on that source, e.g. "YouTube"
  int8_t vpn = -1;    // Shield VPN: -1 hidden (vpn= empty), 0 off, 1 on
  char vpnc[20] = ""; // VPN country, e.g. "India"
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
      else if (keyLen == 3 && strncmp(p, "src", 3) == 0)
        urlDecode(val, valLen, state.src, sizeof(state.src));
      else if (keyLen == 3 && strncmp(p, "app", 3) == 0)
        urlDecode(val, valLen, state.app, sizeof(state.app));
      else if (keyLen == 3 && strncmp(p, "vpn", 3) == 0)
      {
        char tmp[8];
        urlDecode(val, valLen, tmp, sizeof(tmp));
        state.vpn = tmp[0] ? (isTruthy(tmp) ? 1 : 0) : -1;
      }
      else if (keyLen == 4 && strncmp(p, "vpnc", 4) == 0)
        urlDecode(val, valLen, state.vpnc, sizeof(state.vpnc));
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

enum class AvrIcon : uint8_t
{
  None, AppleTv, Nvidia, BluRay, Turntable, Xbox, Kodi, Laptop
};

// Case-insensitive substring search.
inline bool containsNoCase(const char *text, const char *needle)
{
  size_t n = strlen(needle);
  for (const char *t = text; *t; t++)
  {
    size_t i = 0;
    while (i < n && t[i] && tolower((unsigned char)t[i]) == tolower((unsigned char)needle[i])) i++;
    if (i == n) return true;
  }
  return false;
}

// Maps a Harmony activity name ("Watch Apple TV", "Shield Media player", "Blu-ray", "Turn Table") to a logo.
inline AvrIcon sourceIcon(const char *activity)
{
  if (containsNoCase(activity, "apple")) return AvrIcon::AppleTv;
  if (containsNoCase(activity, "shield") || containsNoCase(activity, "nvidia")) return AvrIcon::Nvidia;
  if (containsNoCase(activity, "blu")) return AvrIcon::BluRay;
  if (containsNoCase(activity, "turn") || containsNoCase(activity, "vinyl")) return AvrIcon::Turntable;
  if (containsNoCase(activity, "xbox")) return AvrIcon::Xbox;
  if (containsNoCase(activity, "kodi")) return AvrIcon::Kodi;
  if (containsNoCase(activity, "laptop") || containsNoCase(activity, "pc")) return AvrIcon::Laptop;
  return AvrIcon::None;
}

// True when two app names match ignoring case, spaces and punctuation ("Disney+" == "disney +",
// but "YouTube" != "YouTube Kids").
inline bool sameAppName(const char *a, const char *b)
{
  for (;;)
  {
    while (*a && !isalnum((unsigned char)*a)) a++;
    while (*b && !isalnum((unsigned char)*b)) b++;
    if (!*a || !*b) return !*a && !*b;
    if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
    a++;
    b++;
  }
}

// Splits "-76.5" into "-76" and ".5"; a ".0" fraction is dropped ("-30.0" -> "-30", "").
inline void splitVolume(const char *vol, char *whole, char *frac, size_t bufSize)
{
  whole[0] = frac[0] = '\0';
  const char *dot = strchr(vol, '.');
  size_t n = dot ? (size_t)(dot - vol) : strlen(vol);
  if (n > bufSize - 1) n = bufSize - 1;
  memcpy(whole, vol, n);
  whole[n] = '\0';
  if (dot && dot[1] && !(dot[1] == '0' && dot[2] == '\0'))
  {
    strncpy(frac, dot, bufSize - 1);
    frac[bufSize - 1] = '\0';
  }
}

// Width in TomThumb pixels: capitals and digits advance 4, space and ':' advance 2;
// the last glyph's trailing gap is not counted.
inline int tomThumbWidth(const char *text)
{
  int w = 0;
  for (const char *c = text; *c; c++)
    w += (*c == ' ' || *c == ':') ? 2 : 4;
  return w > 0 ? w - 1 : 0;
}

// The VPN line under the AVR clock: "VPN: " label, then "ON INDIA" or "OFF".
// Drops the label, then shortens the country, until it fits maxWidth pixels.
inline void fitVpnLine(bool on, const char *country, int maxWidth, char *label, char *status, size_t size)
{
  char upper[20];
  size_t n = 0;
  for (; on && country && country[n] && n < sizeof(upper) - 1; n++)
    upper[n] = (char)toupper((unsigned char)country[n]);
  while (n > 0 && upper[n - 1] == ' ') n--;

  // Full line first; then without the label, shortening the country as needed
  for (int withLabel = 1; withLabel >= 0; withLabel--)
  {
    snprintf(label, size, "%s", withLabel ? "VPN: " : "");
    for (size_t len = n;; len--)
    {
      while (len > 0 && upper[len - 1] == ' ') len--;
      if (!on)
        snprintf(status, size, "OFF");
      else if (len)
        snprintf(status, size, "ON %.*s", (int)len, upper);
      else
        snprintf(status, size, "ON");
      char full[48];
      snprintf(full, sizeof(full), "%s%s", label, status);
      if (tomThumbWidth(full) <= maxWidth)
        return;
      if (len == 0 || withLabel)
        break;
    }
  }
  // Nothing fits: bare status
}

} // namespace cw
