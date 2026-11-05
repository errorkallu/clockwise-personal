#pragma once

#include <stddef.h>

// Makes an ezTime format string follow the "Use 24h format" setting, so clockfaces and Canvas
// themes that hard-code "H:i" or "h:i:sa" still show the hour the way the user chose.
// Pure C++ so it can be unit tested natively.
namespace cw {

// 12h: H -> h, G -> g. 24h: h -> H, g -> G, and AM/PM (a, A) is dropped with the spaces before it.
// Characters escaped with '~' are copied unchanged. out is always NUL-terminated.
inline void adaptHourFormat(const char *in, bool use24h, char *out, size_t outSize)
{
  size_t o = 0;
  size_t keep = 0; // spaces before this index are not ours to strip
  for (const char *c = in; *c && o + 1 < outSize; c++)
  {
    char ch = *c;
    if (ch == '~' && c[1])
    {
      out[o++] = ch;
      if (o + 1 < outSize) out[o++] = *++c;
      keep = o;
      continue;
    }
    if (use24h && (ch == 'a' || ch == 'A'))
    {
      while (o > keep && out[o - 1] == ' ') o--;
      continue;
    }
    if (use24h)
      ch = (ch == 'h') ? 'H' : (ch == 'g') ? 'G' : ch;
    else
      ch = (ch == 'H') ? 'h' : (ch == 'G') ? 'g' : ch;
    out[o++] = ch;
  }
  out[o] = '\0';
}

} // namespace cw
