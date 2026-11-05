# Generates AvrAppLogos.h: 16x16 RGB565 app logos for the AVR screen, plus a preview PNG.
# Logos come from the dashboard-icons PNG set, the simple-icons SVG set (white glyph on the
# brand colour) or, where those are unreadable at 16px, the pixel art in AvrIcons.h.
# Apps without an entry here show their name instead of a logo.
# Needs Pillow and cairosvg. Usage: python3 avr_app_logos.py <cache dir> <out .h> <preview .png>
import io, json, os, re, sys, tarfile, urllib.request
from PIL import Image, ImageDraw
import cairosvg

DASH = 'https://raw.githubusercontent.com/homarr-labs/dashboard-icons/main/png/%s.png'
SIMPLE_ICONS_VERSION = '16.35.0'
SIMPLE = 'https://registry.npmjs.org/simple-icons/-/simple-icons-%s.tgz' % SIMPLE_ICONS_VERSION

# (C identifier, source, names the sensors report for it).
# Source is 'dash:<file>', 'si:<slug>' or 'icon:<AVR_ICON_ suffix in AvrIcons.h>'.
LOGOS = [
    ('NETFLIX', 'dash:netflix', ['Netflix']),
    ('YOUTUBE', 'dash:youtube', ['YouTube']),
    ('YOUTUBE_KIDS', 'dash:youtube-kids', ['YouTube Kids']),
    ('YOUTUBE_MUSIC', 'dash:youtube-music', ['YouTube Music']),
    ('DISNEY', 'icon:DISNEY', ['Disney+', 'Disney Plus']),
    ('HOTSTAR', 'dash:jiohotstar', ['Disney+ Hotstar', 'Hotstar', 'JioHotstar']),
    ('PRIME', 'icon:PRIME', ['Prime Video', 'Amazon Prime Video']),
    ('HULU', 'dash:hulu', ['Hulu']),
    ('KODI', 'dash:kodi', ['Kodi']),
    ('PLEX', 'dash:plex', ['Plex', 'Plex Media Server']),
    ('EMBY', 'dash:emby', ['Emby']),
    ('JELLYFIN', 'dash:jellyfin', ['Jellyfin']),
    ('SPOTIFY', 'dash:spotify', ['Spotify']),
    ('TWITCH', 'dash:twitch', ['Twitch']),
    ('CRUNCHYROLL', 'dash:crunchyroll', ['Crunchyroll']),
    ('MAX', 'dash:max', ['Max', 'HBO Max']),
    ('NORDVPN', 'dash:nordvpn', ['NordVPN']),
    ('NVIDIA', 'dash:nvidia', ['NVIDIA Games', 'GeForce NOW']),
    ('ASSISTANT', 'dash:google-assistant', ['Google Assistant']),
    ('CHROMECAST', 'si:googlecast', ['Google Cast', 'Chromecast']),
    ('PLAY_STORE', 'dash:google-play', ['Play Store', 'Google Play Store']),
    ('APPLE_TV', 'icon:APPLETV', ['Apple TV', 'TV', 'Apple TV+']),
    ('APPLE_MUSIC', 'dash:apple-music', ['Apple Music', 'Music']),
    ('APPLE_PODCASTS', 'dash:apple-podcasts', ['Apple Podcasts', 'Podcasts']),
    ('APP_STORE', 'dash:app-store', ['App Store']),
    ('SPEEDTEST', 'dash:ookla-speedtest', ['Speedtest']),
    ('STEAM', 'dash:steam', ['Steam Link', 'Steam']),
    ('APPLE_ARCADE', 'si:applearcade', ['Apple Arcade', 'Arcade']),
    ('CBC', 'si:cbc', ['CBC Gem']),
    ('INFUSE', 'si:infuse', ['Infuse']),
    ('RETROARCH', 'si:retroarch', ['RetroArch']),
    ('TUBI', 'si:tubi', ['Tubi']),
    ('VLC', 'si:vlcmediaplayer', ['VLC']),
]

N = 16

def fetch(url, cache):
    path = os.path.join(cache, re.sub(r'[^A-Za-z0-9._-]', '_', url.split('/')[-1]))
    if not os.path.exists(path):
        with urllib.request.urlopen(url, timeout=60) as r, open(path, 'wb') as f:
            f.write(r.read())
    return path

def on_black(img):
    bg = Image.new('RGBA', img.size, (0, 0, 0, 255))
    bg.alpha_composite(img)
    return bg.convert('RGB')

def dash_logo(name, cache):
    img = Image.open(fetch(DASH % name, cache)).convert('RGBA')
    img = img.crop(img.getbbox())
    side = max(img.size)
    sq = Image.new('RGBA', (side, side), (0, 0, 0, 0))
    sq.paste(img, ((side - img.width) // 2, (side - img.height) // 2))
    return on_black(sq.resize((N, N), Image.LANCZOS))

def simple_logo(slug, cache, tar):
    svg = tar.extractfile('package/icons/%s.svg' % slug).read().decode()
    meta = next(i for i in json.load(tar.extractfile('package/data/simple-icons.json'))
                if slugify(i) == slug)
    hexc = meta['hex']
    color = tuple(int(hexc[i:i + 2], 16) for i in (0, 2, 4))
    if sum(color) < 120:  # a black brand colour would vanish on the panel
        color = (0x33, 0x33, 0x33)
    glyph = Image.open(io.BytesIO(cairosvg.svg2png(
        bytestring=svg.replace('<path ', '<path fill="#fff" ').encode(), output_width=11 * 8,
        output_height=11 * 8))).convert('RGBA').resize((11, 11), Image.LANCZOS)
    tile = Image.new('RGBA', (N, N), (0, 0, 0, 0))
    ImageDraw.Draw(tile).rounded_rectangle([0, 0, N - 1, N - 1], radius=3, fill=color + (255,))
    tile.alpha_composite(glyph, (3, 3))
    return on_black(tile)

def slugify(icon):
    if 'slug' in icon:
        return icon['slug']
    t = icon['title'].lower().replace('+', 'plus').replace('.', 'dot').replace('&', 'and')
    return re.sub(r'[^a-z0-9]', '', t)

def to565(c):
    r, g, b = c
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

def pixel_art(name, out):
    # Reads an AVR_ICON_<name> array back out of AvrIcons.h (next to the output file).
    text = open(os.path.join(os.path.dirname(out), 'AvrIcons.h')).read()
    body = re.search(r'AVR_ICON_%s\[256\] = \{([^}]*)\}' % name, text).group(1)
    px = [int(v, 16) for v in re.findall(r'0x[0-9A-F]{4}', body)]
    img = Image.new('RGB', (N, N))
    img.putdata([((p >> 11) << 3, ((p >> 5) & 63) << 2, (p & 31) << 3) for p in px])
    return img

def main(cache, out, preview):
    os.makedirs(cache, exist_ok=True)
    tar = tarfile.open(fetch(SIMPLE, cache))
    images = []
    for ident, src, _ in LOGOS:
        kind, name = src.split(':')
        if kind == 'dash':
            images.append(dash_logo(name, cache))
        elif kind == 'si':
            images.append(simple_logo(name, cache, tar))
        else:
            images.append(pixel_art(name, out))

    with open(out, 'w') as f:
        f.write('#pragma once\n\n#include <stdint.h>\n#include "AvrIcons.h"\n\n')
        f.write('// 16x16 RGB565 app logos for the AVR screen. Generated by firmware/tools/avr_app_logos.py.\n')
        f.write('// Brand logos: dashboard-icons (homarr-labs) and simple-icons %s.\n\n' % SIMPLE_ICONS_VERSION)
        f.write('namespace cw {\n\n')
        for (ident, src, _), img in zip(LOGOS, images):
            if src.startswith('icon:'):
                continue
            px = [to565(img.getpixel((x, y))) for y in range(N) for x in range(N)]
            f.write('static const uint16_t AVR_LOGO_%s[256] = {\n' % ident)
            for i in range(0, 256, 16):
                f.write('  ' + ', '.join('0x%04X' % p for p in px[i:i + 16]) + ',\n')
            f.write('};\n\n')
        f.write('struct AvrAppLogo\n{\n  const char *name;\n  const uint16_t *bitmap;\n};\n\n')
        f.write('// Matched against the app name with case, spaces and punctuation ignored.\n')
        f.write('static const AvrAppLogo AVR_APP_LOGOS[] = {\n')
        for ident, src, names in LOGOS:
            array = 'AVR_ICON_' + src[5:] if src.startswith('icon:') else 'AVR_LOGO_' + ident
            for n in names:
                f.write('  {"%s", %s},\n' % (n, array))
        f.write('};\n\n} // namespace cw\n')

    scale, cols = 6, 8
    rows = (len(images) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * (N * scale + 8), rows * (N * scale + 8)), (40, 40, 40))
    for i, img in enumerate(images):
        sheet.paste(img.resize((N * scale, N * scale), Image.NEAREST),
                    ((i % cols) * (N * scale + 8) + 4, (i // cols) * (N * scale + 8) + 4))
    sheet.save(preview)

if __name__ == '__main__':
    main(*sys.argv[1:4])
