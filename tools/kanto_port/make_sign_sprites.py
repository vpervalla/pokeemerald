#!/usr/bin/env python3
"""Cut the Poke Ball emblems of the Pokemon Center and Mart out of kanto_general into 32x32 sprites,
which src/day_night.c lays over the emblems at night so they glow. Run from the pokeemerald root.

The emblem plate is taken row by row between the leftmost and rightmost pixel of its outline
(palette colour 6); rows without outline pixels reuse the previous row's span."""
import struct
from PIL import Image

P = "data/tilesets/primary/kanto_general/"
OUTLINE = 6
# name: (palette, metatiles in 2 rows, pixel x of the sprite's left edge within those rows)
SIGNS = {
    "pokemon_center": (2, [[0x51, 0x52, 0x53], [0x59, 0x5A, 0x5B]], 8),
    "mart": (3, [[0x31, 0x32], [0x39, 0x3A]], 0),
}

tiles_im = Image.open(P + "tiles.png")
metatiles = open(P + "metatiles.bin", "rb").read()

def tile(t):
    w = tiles_im.width // 8
    return tiles_im.crop(((t % w) * 8, (t // w) * 8, (t % w) * 8 + 8, (t // w) * 8 + 8))

def compose(m, pal):
    """16x16 palette indices of a metatile's visible pixels; -1 where another palette shows."""
    g = [[-1] * 16 for _ in range(16)]
    for n in range(8):
        v = struct.unpack_from("<H", metatiles, (m * 8 + n) * 2)[0]
        t = tile(v & 0x3FF)
        if v & 0x400: t = t.transpose(Image.FLIP_LEFT_RIGHT)
        if v & 0x800: t = t.transpose(Image.FLIP_TOP_BOTTOM)
        for y in range(8):
            for x in range(8):
                c = t.getpixel((x, y))
                if n >= 4 and c == 0:
                    continue
                g[(n % 4 // 2) * 8 + y][(n % 2) * 8 + x] = c if v >> 12 == pal else -1
    return g

def jasc(path):
    return [tuple(map(int, l.split())) for l in open(path).read().split("\n")[3:19]]

for name, (pal, rows, left) in SIGNS.items():
    grid = []
    for row in rows:
        parts = [compose(m, pal) for m in row]
        for y in range(16):
            grid.append([c for p in parts for c in p[y]])
    out = Image.new("P", (32, 32), 0)
    span = None
    for y in range(32):
        xs = [x for x, c in enumerate(grid[y]) if c == OUTLINE]
        if len(xs) >= 2:
            span = (min(xs), max(xs))
        elif span is None:
            continue
        for x in range(span[0], span[1] + 1):
            c = grid[y][x]
            if c > 0 and left <= x < left + 32:
                out.putpixel((x - left, y), c)
    colors = jasc(f"{P}palettes/{pal:02d}.pal")
    colors[0] = (255, 0, 255)  # transparent
    out.putpalette([v for c in colors for v in c])
    out.save(f"graphics/day_night/{name}_sign.png")
    print("wrote", name)
