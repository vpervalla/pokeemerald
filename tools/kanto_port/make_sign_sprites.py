#!/usr/bin/env python3
"""Cut the Poke Ball emblems of the Pokemon Center, Mart and Gyms and their "P.C", "MART" and "GYM" signs
out of kanto_general into 32x32 sprites, and the glass of their doors into 16x16 ones, which src/day_night.c
lays over them at night so they glow. Run from the pokeemerald root.

Pokemon Center and Mart: only the ball's red or blue pixels go in the sprite, so the grey plate and
the ball's white band stay part of the tinted building. They are taken from inside the emblem plate,
row by row between the leftmost and rightmost pixel of its outline (palette colour 6; rows without
outline pixels reuse the previous row's span), since the roof shares the ball's colours.
Gym: only the Poke Ball in the middle of the gold sign above the door glows, so the plate stays
tinted. Its pixels are taken row by row between the leftmost and rightmost pixel of the ball's light
ring (colours 1, 2 and 7, within the ball's columns, as the plate's edges share them), which takes in
the gold between the ring and the centre button; the grey band across the ball stays tinted. The
ball fills the middle of the sprite's top half.
Gym billboards (the signs in front of Gyms): the ball's red, the dots below it and the "GYM" text,
which is dark grey on the plate, so it takes the doors' warm light instead (its darkest strokes the
palest), in two 16x16 sprites, as the billboard's top metatile draws on the top BG layer.
"P.C", "MART" and "GYM" signs: their red letters, which use the Pokemon Center ball's colours (MART's
muted anti-aliasing pixels stay tinted).
Doors: the glass panes of the sliding doors (palette 3's colours 10, 12 and 13), lit like the windows
(the same warm light as LitGlassColor in day_night.c). A door's sprite is a strip of 16x16 frames: the
closed door, then the glass of each frame of its opening animation (graphics/door_anims/kanto), which
day_night.c shows while field_door.c draws that frame.
The sprites only show while the lights are on, so their palettes hold softly brightened colours.

The overworld leaves few sprite palette slots free, so all the sprites share one palette: each
source palette's glowing colours (one group per tileset palette and brightening) get their own range of
it, in the order of SIGNS. Every PNG carries the whole shared palette; day_night.c takes it from the
first one and half tints the door glass's range (printed below)."""
import struct
from PIL import Image

P = "data/tilesets/primary/kanto_general/"
OUTLINE = 6
BALL_COLORS = {11, 12, 13, 14}
GYM_COLORS = {1, 2, 6, 7, 8, 9}
# Colours bounding the glowing pixels in each row, the columns they're looked for in, and whether rows
# without them reuse the previous row's span.
OUTLINE_EDGE = ({OUTLINE}, 0, 63, True)
GYM_BALL_EDGE = ({1, 2, 7}, 16, 31, False)
DOOR_GLASS = {10, 12, 13}
DOOR_GLASS_EDGE = (DOOR_GLASS, 0, 15, False)

def brighten(c):
    return tuple(min(255, round(v * 1.1 + 8)) for v in c)

def brighten_gold(c):
    return tuple(min(255, round(v * 1.04 + 4)) for v in c)

def lit_glass(c):
    """LitGlassColor in src/day_night.c, on the colour's GBA (5-bit) value."""
    v = sum(x >> 3 for x in c) // 3
    return tuple(x * 8 for x in (min(31, 24 + v // 4), 12 + v * 18 // 31, 2 + v * 16 // 31))
# name: (palette, metatile rows, pixel x of the sprite's left edge within those rows, glowing colours,
#        edge of the glowing pixels, brighten function[, sprite size, door animation, secondary tileset])
SIGNS = {
    "pokemon_center_sign": (2, [[0x51, 0x52, 0x53], [0x59, 0x5A, 0x5B]], 8, BALL_COLORS, OUTLINE_EDGE, brighten),
    "mart_sign": (3, [[0x31, 0x32], [0x39, 0x3A]], 0, BALL_COLORS, OUTLINE_EDGE, brighten),
    "pokemon_center_text": (2, [[0x60, 0x61]], 0, BALL_COLORS, (BALL_COLORS, 0, 31, False), brighten),
    "mart_text": (2, [[0x40, 0x41]], 0, BALL_COLORS, (BALL_COLORS, 0, 31, False), brighten),
    "gym_text": (2, [[0x151], [0x159]], 0, BALL_COLORS, (BALL_COLORS, 0, 15, False), brighten),
    "gym_sign": (5, [[0x152, 0x153, 0x154]], 8, GYM_COLORS, GYM_BALL_EDGE, brighten_gold),
    "sliding_door": (3, [[0x062]], 0, DOOR_GLASS, DOOR_GLASS_EDGE, lit_glass, 16, "sliding_single"),  # Pokemon Center, Mart
    "gym_door": (3, [[0x15B]], 0, DOOR_GLASS, DOOR_GLASS_EDGE, lit_glass, 16, "sliding_double"),
    "dept_store_door": (3, [[0x294]], 0, DOOR_GLASS, DOOR_GLASS_EDGE, lit_glass, 16, "dept_store",
                        "data/tilesets/secondary/kanto_celadon_city/"),
}

NUM_PRIMARY = 640  # Tiles and metatiles in kanto_general; secondary ids follow them

def load_tileset(d):
    return Image.open(d + "tiles.png"), open(d + "metatiles.bin", "rb").read()

primary = load_tileset(P)

def tile(t, secondary):
    im = primary[0] if t < NUM_PRIMARY else secondary[0]
    t %= NUM_PRIMARY
    w = im.width // 8
    return im.crop(((t % w) * 8, (t // w) * 8, (t % w) * 8 + 8, (t // w) * 8 + 8))

def compose(m, pal, secondary=None):
    """16x16 palette indices of a metatile's visible pixels; -1 where another palette shows."""
    g = [[-1] * 16 for _ in range(16)]
    metatiles = primary[1] if m < NUM_PRIMARY else secondary[1]
    for n in range(8):
        v = struct.unpack_from("<H", metatiles, (m % NUM_PRIMARY * 8 + n) * 2)[0]
        t = tile(v & 0x3FF, secondary)
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

sprites = {}
for name, (pal, rows, left, glow, (edge, lo, hi, reuse), bright, *extra) in SIGNS.items():
    size = extra[0] if extra else 32
    door_anim = extra[1] if len(extra) > 1 else None
    secondary = load_tileset(extra[2]) if len(extra) > 2 else None
    grid = []
    for row in rows:
        parts = [compose(m, pal, secondary) for m in row]
        for y in range(16):
            grid.append([c for p in parts for c in p[y]])
    out = Image.new("P", (size, size), 0)
    span = None
    for y in range(len(grid)):
        xs = [x for x, c in enumerate(grid[y]) if c in edge and lo <= x <= hi]
        if len(xs) >= 2:
            span = (min(xs), max(xs))
        elif span is None or not reuse:
            continue
        for x in range(span[0], span[1] + 1):
            c = grid[y][x]
            if c in glow and left <= x < left + size:
                out.putpixel((x - left, y), c)
    if door_anim:
        # The animation's frames are drawn with palette 3 as they are, no flips.
        anim = Image.open(f"graphics/door_anims/kanto/{door_anim}.png")
        strip = Image.new("P", (size, size * (1 + anim.height // 16)), 0)
        strip.paste(out, (0, 0))
        for y in range(anim.height):
            for x in range(16):
                if anim.getpixel((x, y)) in glow:
                    strip.putpixel((x, size + y), anim.getpixel((x, y)))
        out = strip
    sprites[name] = (out, (pal, bright))

# Gym billboard: metatile 0x160 over 0x168. Text pixels are recoloured to the door glass's light.
BILLBOARD_RED = {11, 12, 13}
BILLBOARD_TEXT_ROWS = range(20, 23)
BILLBOARD_TEXT = {6: 10, 5: 12, 4: 13}  # Text colour -> glass colour whose light it takes
grid = compose(0x160, 2) + compose(0x168, 2)
for half, rows in (("gym_billboard_top", range(0, 16)), ("gym_billboard_bottom", range(16, 32))):
    out = Image.new("P", (16, 16), 0)
    for y in rows:
        for x in range(16):
            c = grid[y][x]
            if c in BILLBOARD_RED or (c in BILLBOARD_TEXT and y in BILLBOARD_TEXT_ROWS):
                out.putpixel((x, y - rows[0]), c)
    sprites[half] = (out, (2, brighten), {c: ((3, lit_glass), g) for c, g in BILLBOARD_TEXT.items()})
sprites = {name: v if len(v) == 3 else v + ({},) for name, v in sprites.items()}

# The shared palette: index 0 is transparent, then each group's colours that the sprites use.
shared = [(255, 0, 255)]
remap = {}
for name, (out, group, recolor) in sprites.items():
    keys = [recolor.get(c, (group, c)) for c in sorted(set(out.get_flattened_data()) - {0})]
    for key in keys:
        if key not in remap:
            (pal, bright), c = key
            remap[key] = len(shared)
            shared.append(bright(jasc(f"{P}palettes/{pal:02d}.pal")[c]))
    print(f"{name}: shared colours {sorted(remap[k] for k in keys)}")
assert len(shared) <= 16, "the glowing colours don't fit in one palette"
shared += [(0, 0, 0)] * (16 - len(shared))
for name, (out, group, recolor) in sprites.items():
    out = out.point(lambda c: remap.get(recolor.get(c, (group, c)), 0))
    out.putpalette([v for c in shared for v in c])
    out.save(f"graphics/day_night/{name}.png")
