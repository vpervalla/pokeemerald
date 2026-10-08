#!/usr/bin/env python3
"""Draw the street lamp that src/day_night.c places in towns, and the pool of light it casts at night.
Run from the pokeemerald root.

graphics/day_night/lamp.png       16x32 lantern on an iron post, with its daytime palette (grey glass)
graphics/day_night/lamp_night.pal the palette in the evening and at night: lit glass
graphics/day_night/lamp_pool.png  64x32 ellipse of warm light, blended additively onto the ground

Palette: 1-4 iron (tinted like the map), 5-7 glass, 8-11 the pool's bands, dim to bright (untinted)."""
from PIL import Image

DAY = [(255, 0, 255), (40, 40, 56), (64, 72, 96), (104, 112, 136), (160, 168, 192),
       (208, 224, 240), (168, 192, 216), (128, 152, 184),
       (64, 36, 0), (128, 76, 8), (192, 120, 16), (248, 168, 32)]
NIGHT = DAY[:5] + [(255, 240, 176), (248, 208, 120), (232, 168, 80)] + DAY[8:]
DAY += [(0, 0, 0)] * (16 - len(DAY))
NIGHT += [(0, 0, 0)] * (16 - len(NIGHT))

KEY = {'.': 0, 'o': 1, 'd': 2, 'm': 3, 'l': 4, 'G': 5, 'g': 6, 'h': 7}
LAMP = [
    "................",
    ".......oo.......",
    "......odlo......",
    ".....odmmlo.....",
    "....oddmmllo....",
    "...oooooooooo...",
    "....omGGGGmo....",
    "....oGGGGGgo....",
    "....oGGGGggo....",
    "....ogGGgggo....",
    "....ohgghhho....",
    "...oooooooooo...",
    ".....oddmlo.....",
    "......odmo......",
] + ["......odmo......"] * 13 + [
    ".....oddmlo.....",
    "....oddmmllo....",
    "....oooooooo....",
    "................",
    "................",
]
assert len(LAMP) == 32 and all(len(r) == 16 for r in LAMP)

lamp = Image.new("P", (16, 32), 0)
for y, row in enumerate(LAMP):
    for x, c in enumerate(row):
        lamp.putpixel((x, y), KEY[c])
lamp.putpalette([v for c in DAY for v in c])
lamp.save("graphics/day_night/lamp.png")

# The pool: four bands from the rim (8) to the middle (11), dithered where they meet.
pool = Image.new("P", (64, 32), 0)
for y in range(32):
    for x in range(64):
        dx, dy = (x + 0.5 - 32) / 28, (y + 0.5 - 16) / 13
        r = (dx * dx + dy * dy) ** 0.5
        if r >= 1:
            continue
        level = (1 - r) * 4
        band = int(level)
        if level - band > 0.5 and (x + y) % 2:
            band += 1
        pool.putpixel((x, y), 8 + min(3, band))
pool.putpalette([v for c in NIGHT for v in c])
pool.save("graphics/day_night/lamp_pool.png")

with open("graphics/day_night/lamp_night.pal", "w", newline="\r\n") as f:
    f.write("JASC-PAL\n0100\n16\n" + "".join(f"{r} {g} {b}\n" for r, g, b in NIGHT))
print("wrote lamp.png, lamp_night.pal, lamp_pool.png")
