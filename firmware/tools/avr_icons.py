# Generates the 16x16 RGB565 pixel-art logos for the AVR screen (AvrIcons.h) plus a preview PNG.
# Needs Pillow.
import math, sys
from PIL import Image
N=16
def rgb(h): return ((h>>16)&255,(h>>8)&255,h&255)
BLACK=(0,0,0); WHITE=(255,255,255)
class C:
    def __init__(s,bg=None,round_=True):
        s.p=[[BLACK]*N for _ in range(N)]
        if bg is not None:
            for y in range(N):
                for x in range(N): s.p[y][x]=bg
            if round_:
                for x,y in [(0,0),(15,0),(0,15),(15,15)]: s.p[y][x]=BLACK
    def set(s,x,y,c):
        if 0<=x<N and 0<=y<N: s.p[y][x]=c
    def rect(s,x0,y0,x1,y1,c):
        for y in range(y0,y1+1):
            for x in range(x0,x1+1): s.set(x,y,c)
    def disc(s,cx,cy,r,c):
        for y in range(N):
            for x in range(N):
                if (x-cx)**2+(y-cy)**2<=r*r: s.set(x,y,c)
    def ring(s,cx,cy,r0,r1,c):
        for y in range(N):
            for x in range(N):
                d=(x-cx)**2+(y-cy)**2
                if r0*r0<=d<=r1*r1: s.set(x,y,c)
    def art(s,rows,pal,ox=0,oy=0):
        for y,row in enumerate(rows):
            for x,ch in enumerate(row):
                if ch in pal: s.set(ox+x,oy+y,pal[ch])

ICONS={}
# ---- sources ----
def appletv():
    c=C(rgb(0x202020))
    c.art([
"..........W.....",
".........W......",
".....WW.W.WW....",
"....WWWWWWWWW...",
"...WWWWWWWWW....",
"...WWWWWWWW.....",
"...WWWWWWWW.....",
"...WWWWWWWWW....",
"....WWWWWWWWW...",
".....WWW.WWW....",
"................",
"....W...........",
"...WWW.W...W....",
"....W...W.W.....",
"....W...W.W.....",
".....W...W......"],{'W':WHITE})
    return c
ICONS['appletv']=appletv()
def nvidia():
    G=rgb(0x76B900); c=C(G)
    c.art([
"................",
"................",
"................",
".....WWWWWW.....",
"...WWW....WWW...",
"..WW...WW...WW..",
".WW..WWWWWW..WW.",
".W..WW....WW..W.",
".W..WW....WW..W.",
".WW..WWWWWW..WW.",
"..WW...WW...WW..",
"...WWW....WWW...",
".....WWWWWW.....",
],{'W':WHITE})
    return c
ICONS['nvidia']=nvidia()
def bluray():
    c=C(rgb(0x001A66))
    c.disc(7.5,7.5,7.2,rgb(0x0A64FF))
    c.ring(7.5,7.5,4.6,5.4,rgb(0x6FB4FF))
    c.disc(7.5,7.5,2.2,rgb(0xC0C0C0)); c.disc(7.5,7.5,1.0,BLACK)
    c.art(["WW","W."],{'W':WHITE},3,3)
    return c
ICONS['bluray']=bluray()
def turntable():
    c=C(rgb(0x303030))
    c.disc(6.5,8.5,6.3,rgb(0x080808))
    c.ring(6.5,8.5,3.5,4.2,rgb(0x404040))
    c.disc(6.5,8.5,1.6,rgb(0xE02020)); c.set(6,8,BLACK); c.set(7,8,BLACK); c.set(6,9,BLACK); c.set(7,9,BLACK)
    # tonearm
    for x,y in [(14,1),(14,2),(14,3),(14,4),(14,5),(13,6),(12,7),(11,8),(10,9)]: c.set(x,y,rgb(0xD0D0D0))
    c.rect(13,0,15,1,rgb(0xD0D0D0))
    return c
ICONS['turntable']=turntable()
def xbox():
    c=C(None); c.disc(7.5,7.5,7.6,rgb(0x107C10))
    for i in range(3,13):
        c.set(i,i,WHITE); c.set(i+1,i,WHITE); c.set(15-i,i,WHITE); c.set(14-i,i,WHITE)
    return c
ICONS['xbox']=xbox()
def kodi():
    c=C(rgb(0x17B2E7))
    for y in range(N):
        for x in range(N):
            if abs(x-7.5)+abs(y-7.5)<=5.5: c.set(x,y,WHITE)
    for y in range(N):
        for x in range(N):
            if abs(x-7.5)+abs(y-7.5)<=2.5: c.set(x,y,rgb(0x17B2E7))
    return c
ICONS['kodi']=kodi()
def laptop():
    c=C(None); k=rgb(0xB0B0B0)
    c.rect(2,3,13,10,k); c.rect(3,4,12,9,rgb(0x2060C0)); c.rect(0,11,15,12,k)
    return c
ICONS['laptop']=laptop()
# ---- apps ----
def netflix():
    c=C(BLACK); R=rgb(0xE50914); D=rgb(0x8B0000)
    for y in range(1,15):
        c.set(4,y,R); c.set(5,y,R); c.set(10,y,R); c.set(11,y,R)
        x=4+round((y-1)*6/13)
        c.set(x,y,R); c.set(x+1,y,R)
    return c
ICONS['netflix']=netflix()
def youtube():
    c=C(None); R=rgb(0xFF0000)
    c.rect(0,3,15,12,R); c.set(0,3,BLACK); c.set(15,3,BLACK); c.set(0,12,BLACK); c.set(15,12,BLACK)
    tri=["W.....","WWW...","WWWWW.","WWWWWW","WWWWW.","WWW...","W....."]
    c.art(tri,{'W':WHITE},6,4)
    return c
ICONS['youtube']=youtube()
def plex():
    c=C(rgb(0x1F1F1F)); O=rgb(0xE5A00D)
    for y in range(2,14):
        d=abs(y-7.5)
        x=int(round(4+(5.5-d)*0.9))
        for k in range(3): c.set(x+k,y,O)
    return c
ICONS['plex']=plex()
def disney():
    c=C(rgb(0x0E1A5C))
    c.art([
"................",
"................",
"..WWWW..........",
"..W...W.........",
"..W....W....W...",
"..W....W....W...",
"..W....W..WWWWW.",
"..W....W....W...",
"..W...W.....W...",
"..WWWW..........",
"................",
"..WWWWWWWWWWWW..",
],{'W':WHITE},0,1)
    return c
ICONS['disney']=disney()
def prime():
    B=rgb(0x00A8E1); c=C(B)
    c.art([
"................",
"................",
"...WWWW.........",
"...W...W........",
"...W...W........",
"...WWWW.........",
"...W............",
"...W............",
"................",
"..W.........W...",
"...WW.....WW.W..",
".....WWWWW..WW..",
],{'W':WHITE},0,1)
    return c
ICONS['prime']=prime()
def spotify():
    c=C(None); c.disc(7.5,7.5,7.6,rgb(0x1DB954))
    c.art([
"................",
"................",
"................",
"................",
"...KKKKKKKKK....",
"..K.........KK..",
"................",
"...KKKKKKKK.....",
"..K........K....",
"................",
"....KKKKKK......",
"...K......K.....",
],{'K':BLACK})
    return c
ICONS['spotify']=spotify()
def emby():
    G=rgb(0x52B54B); c=C(None)
    for y in range(N):
        for x in range(N):
            if abs(x-7.5)+abs(y-7.5)*0.6<=8.5 and abs(x-7.5)<=7.5: c.set(x,y,G)
    c.art(["W....","WWW..","WWWWW","WWW..","W...."],{'W':WHITE},6,5)
    return c
ICONS['emby']=emby()
def hulu():
    c=C(rgb(0x1CE783)); K=BLACK
    c.art([
"W.......W.......",
"W.......W.......",
"WWW.W.W.W.W.W...",
"W.W.W.W.W.W.W...",
"W.W.WWW.W.WWW...",
],{'W':K},2,5)
    return c
ICONS['hulu']=hulu()
def twitch():
    P=rgb(0x9146FF); c=C(P)
    c.rect(3,3,12,10,WHITE); c.rect(4,4,11,9,P); c.rect(6,5,6,7,WHITE); c.rect(9,5,9,7,WHITE)
    c.rect(5,11,6,12,WHITE); c.set(7,11,WHITE)
    return c
ICONS['twitch']=twitch()

def to565(c):
    return [((r>>3)<<11)|((g>>2)<<5)|(b>>3) for row in c.p for (r,g,b) in row]

# usage: python3 avr_icons.py ../lib/cw-commons/AvrIcons.h preview.png
out=sys.argv[1]
names=list(ICONS)
with open(out,'w') as f:
    f.write("#pragma once\n\n#include <stdint.h>\n\n// 16x16 RGB565 pixel-art logos for the AVR screen.\n// Generated by tools/avr_icons.py - edit that and re-run instead of editing by hand.\nnamespace cw {\n\n")
    for n in names:
        v=to565(ICONS[n])
        f.write(f"static const uint16_t AVR_ICON_{n.upper()}[256] = {{\n")
        for i in range(0,256,16):
            f.write("  "+", ".join(f"0x{x:04X}" for x in v[i:i+16])+",\n")
        f.write("};\n\n")
    f.write("} // namespace cw\n")
img=Image.new('RGB',(len(names)*18*8,16*8+8))
for i,n in enumerate(names):
    for y in range(N):
        for x in range(N):
            r,g,b=ICONS[n].p[y][x]
            for dy in range(7):
                for dx in range(7): img.putpixel((i*18*8+x*8+dx,y*8+dy),(r,g,b))
img.save(sys.argv[2])
print(names)
