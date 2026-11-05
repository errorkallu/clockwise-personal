#pragma once

#include "AvrState.h"
#include "AvrIcons.h"
#include "AvrAppLogos.h"

// Bitmap lookups for the AVR screen, kept free of Adafruit_GFX so they can be unit tested.
namespace cw {

inline const uint16_t *sourceLogo(AvrIcon icon)
{
  switch (icon)
  {
  case AvrIcon::AppleTv: return AVR_ICON_APPLETV;
  case AvrIcon::Nvidia: return AVR_ICON_NVIDIA;
  case AvrIcon::BluRay: return AVR_ICON_BLURAY;
  case AvrIcon::Turntable: return AVR_ICON_TURNTABLE;
  case AvrIcon::Xbox: return AVR_ICON_XBOX;
  case AvrIcon::Kodi: return AVR_ICON_KODI;
  case AvrIcon::Laptop: return AVR_ICON_LAPTOP;
  default: return nullptr;
  }
}

// Logo for an app name from the Apple TV / Shield sensors; nullptr means "write the name".
inline const uint16_t *appLogo(const char *app)
{
  if (!app[0]) return nullptr;
  for (size_t i = 0; i < sizeof(AVR_APP_LOGOS) / sizeof(AVR_APP_LOGOS[0]); i++)
  {
    if (sameAppName(app, AVR_APP_LOGOS[i].name)) return AVR_APP_LOGOS[i].bitmap;
  }
  return nullptr;
}

} // namespace cw
