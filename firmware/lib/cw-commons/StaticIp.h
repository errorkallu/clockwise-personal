#pragma once

#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

// Static IP setting, stored as "ip,gateway,subnet[,dns]" (empty = DHCP).
// Pure C++ so it can be unit tested natively.
namespace cw {

struct StaticIpConfig
{
  uint8_t ip[4];
  uint8_t gateway[4];
  uint8_t subnet[4];
  uint8_t dns[4];
};

// Parses a dotted IPv4 address from s[0..len), ignoring surrounding spaces.
inline bool parseIpv4(const char *s, size_t len, uint8_t out[4])
{
  while (len && *s == ' ') { s++; len--; }
  while (len && s[len - 1] == ' ') len--;
  int part = 0;
  size_t i = 0;
  while (part < 4)
  {
    if (i >= len || !isdigit((unsigned char)s[i])) return false;
    int value = 0, digits = 0;
    while (i < len && isdigit((unsigned char)s[i]))
    {
      value = value * 10 + (s[i++] - '0');
      if (++digits > 3 || value > 255) return false;
    }
    out[part++] = (uint8_t)value;
    if (part < 4)
    {
      if (i >= len || s[i] != '.') return false;
      i++;
    }
  }
  return i == len;
}

inline uint32_t ipv4ToU32(const uint8_t a[4])
{
  return ((uint32_t)a[0] << 24) | ((uint32_t)a[1] << 16) | ((uint32_t)a[2] << 8) | a[3];
}

// True only for a complete, self-consistent static setup; anything else means "use DHCP",
// so a typo can't leave the clock unreachable on the wrong subnet.
inline bool parseStaticIp(const char *value, StaticIpConfig &cfg)
{
  const char *fields[4] = {0, 0, 0, 0};
  size_t lens[4] = {0, 0, 0, 0};
  int n = 0;
  const char *start = value;
  for (const char *p = value;; p++)
  {
    if (*p == ',' || *p == '\0')
    {
      if (n == 4) return false;
      fields[n] = start;
      lens[n] = (size_t)(p - start);
      n++;
      if (*p == '\0') break;
      start = p + 1;
    }
  }
  if (n < 3) return false;
  if (!parseIpv4(fields[0], lens[0], cfg.ip) || !parseIpv4(fields[1], lens[1], cfg.gateway) ||
      !parseIpv4(fields[2], lens[2], cfg.subnet))
    return false;

  bool dnsGiven = false;
  if (n == 4)
  {
    size_t l = lens[3];
    const char *d = fields[3];
    while (l && *d == ' ') { d++; l--; }
    if (l)
    {
      if (!parseIpv4(d, l, cfg.dns)) return false;
      dnsGiven = true;
    }
  }
  if (!dnsGiven)
    for (int i = 0; i < 4; i++) cfg.dns[i] = cfg.gateway[i];

  uint32_t ip = ipv4ToU32(cfg.ip), gw = ipv4ToU32(cfg.gateway), mask = ipv4ToU32(cfg.subnet);
  if (mask == 0 || (~mask & (~mask + 1)) != 0) return false; // ones must be contiguous
  if ((ip & mask) != (gw & mask)) return false;               // gateway must be reachable
  uint32_t host = ip & ~mask;
  if (host == 0 || host == ~mask) return false;               // network / broadcast address
  if (ip == gw) return false;
  return true;
}

} // namespace cw
