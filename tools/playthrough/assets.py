"""Images for walkthrough pages, as data: URIs to inline in a single HTML file.
  map_img("Route3", [("trainer", 10, 5, "T1"), ("item", 3, 4, "1")], crop=(0, 20))   rows 0-20 only
      marker kinds: item (red dot), hidden (hollow red), gift (blue diamond), trainer (orange box), cut (grey)
  front("pikachu"), icon("pikachu")   the game's own sprites with their palettes
  b64(im, quant=True)                 PNG data: URI (quant shrinks maps a lot)"""
import base64, io, os, re
from PIL import Image, ImageDraw, ImageFont
from render import render, R

def b64(im, quant=False):
    if quant:
        im = im.convert("RGB").quantize(colors=128, method=Image.MEDIANCUT, dither=Image.NONE)
    buf = io.BytesIO()
    im.save(buf, "PNG", optimize=True)
    return "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode()

try:
    FONT = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 15)
except Exception:
    FONT = ImageFont.load_default()

COL = {"item": (214, 52, 40), "gift": (40, 120, 200), "trainer": (240, 150, 20), "cut": (120, 120, 120)}

def marker(dr, kind, cx, cy, label):
    r = 13
    if kind == "item":
        dr.ellipse([cx-r, cy-r, cx+r, cy+r], fill=COL["item"], outline=(255, 255, 255), width=2)
        fg = (255, 255, 255)
    elif kind == "gift":
        dr.polygon([(cx, cy-r-3), (cx+r+3, cy), (cx, cy+r+3), (cx-r-3, cy)], fill=COL["gift"], outline=(255, 255, 255))
        fg = (255, 255, 255)
    elif kind == "trainer":
        dr.rectangle([cx-r-2, cy-r, cx+r+2, cy+r], fill=COL["trainer"], outline=(255, 255, 255), width=2)
        fg = (42, 24, 0)
    elif kind == "hidden":
        dr.ellipse([cx-r, cy-r, cx+r, cy+r], fill=(248, 248, 248), outline=COL["item"], width=4)
        fg = COL["item"]
    else:
        dr.ellipse([cx-r, cy-r, cx+r, cy+r], fill=(248, 248, 248), outline=COL["cut"], width=4)
        fg = COL["cut"]
    w = dr.textlength(label, font=FONT)
    dr.text((cx - w/2, cy - 9), label, fill=fg, font=FONT)

def map_img(name, markers, crop=None):
    im = render(name, scale=2).convert("RGB")
    if crop:
        y0, y1 = crop
        im = im.crop((0, y0*32, im.width, y1*32))
    else:
        y0 = 0
    dr = ImageDraw.Draw(im)
    for kind, x, y, label in markers:
        marker(dr, kind, x*32+16, (y-y0)*32+16, label)
    return im

def jasc(path):
    lines = open(path).read().split("\n")[3:19]
    return [tuple(int(v) for v in l.split()) for l in lines if l.strip()]

def front(species):
    p = f"{R}/graphics/pokemon/{species}/front.png"
    im = Image.open(p)
    pal = jasc(f"{R}/graphics/pokemon/{species}/normal.pal")
    out = Image.new("RGBA", im.size)
    px = im.load(); op = out.load()
    for y in range(im.height):
        for x in range(im.width):
            i = px[x, y]
            if i:
                op[x, y] = pal[i] + (255,)
    return out

ICON_PAL = {}
src = open(R + "/src/pokemon_icon.c").read()
for m in re.finditer(r"\[SPECIES_(\w+)\] = (\d),", src):
    ICON_PAL.setdefault(m.group(1), int(m.group(2)))

def icon(species):
    im = Image.open(f"{R}/graphics/pokemon/{species}/icon.png").crop((0, 0, 32, 32))
    pal = jasc(f"{R}/graphics/pokemon/icon_palettes/icon_palette_{ICON_PAL[species.upper()]}.pal")
    out = Image.new("RGBA", im.size)
    px = im.load(); op = out.load()
    for y in range(32):
        for x in range(32):
            i = px[x, y]
            if i:
                op[x, y] = pal[i] + (255,)
    return out

