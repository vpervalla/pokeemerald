#!/usr/bin/env python3
"""Makes the Supporter cards' pictures from tools/card_game/trainer_sprites.jpg (the Pokémon
Crystal trainer sprites, ripped by ShapelyMan) and, for those it doesn't have, from
tools/card_game/trainer_sprites_2.png (Kanto's trainers in the same style, in 56x56 cells).

    python3 tools/card_game/gen_trainer_pics.py     (needs Pillow)

    graphics/card_game/retro/trainer_<name>.png    64x64 indexed PNGs (the build makes .4bpp.lz/.gbapal)
    src/data/card_game/trainer_pics.h              the table of them, by Trainer card

Each Crystal sprite has 4 colors: white, two shades and black. The sheet is a JPEG, so each
sprite's pixels are put back on 4 colors (the 4 means of its pixels' colors). The second sheet's
sprites keep their colors (the closest ones merged if more than 15). Then as in
gen_retro_pics.py: the white around the trainer becomes transparent (a flood fill from the
edges), the white inside stays white, and the trainer sits centered, on the bottom of a 56x56 box,
itself centered in the 64x64 picture.
"""

import os
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
SHEET_FILE = os.path.join(ROOT, 'tools', 'card_game', 'trainer_sprites.jpg')
SHEET_2_FILE = os.path.join(ROOT, 'tools', 'card_game', 'trainer_sprites_2.png')
OUT_PICS = os.path.join(ROOT, 'graphics', 'card_game', 'retro')
OUT_TABLE = os.path.join(ROOT, 'src', 'data', 'card_game', 'trainer_pics.h')

PIC_SIZE = 64
BOX_SIZE = 56
TRANSPARENT = 0
MAX_COLORS = 15

# The card, and the box around its trainer in the sheet (left, top, right, bottom). The Supporters
# in neither sheet (DAISY, CELIO) have no picture.
SPRITES = [
    ('PROF_OAKS_RESEARCH', 'oak',      (7, 0, 50, 66)),
    ('KOGA',               'koga',     (174, 0, 222, 66)),
    ('LANCE',              'lance',    (354, 0, 410, 66)),
    ('BRUNO',              'bruno',    (233, 0, 289, 66)),
    ('BROCK',              'brock',    (20, 135, 46, 203)),
    ('MISTY',              'misty',    (62, 135, 109, 203)),
    ('LT_SURGE',           'lt_surge', (120, 135, 169, 203)),
    ('ERIKA',              'erika',    (178, 135, 225, 203)),
    ('SABRINA',            'sabrina',  (309, 135, 353, 203)),
    ('BLAINE',             'blaine',   (362, 135, 397, 203)),
    ('BLUE',               'blue',     (411, 135, 445, 203)),
]

# The same in the second sheet, by cell (row, column): its 8x8 cells of 56x56 pixels start at
# (17, 17), 64 pixels apart
SPRITES_2 = [
    ('GIOVANNI',           'giovanni', (0, 7)),
    ('BILL',               'bill',     (3, 2)),
    ('LORELEI',            'lorelei',  (5, 0)),
    ('AGATHA',             'agatha',   (5, 2)),
]
SHEET_2_LEFT = 17
SHEET_2_STEP = 64


def gba(color):
    # The GBA has 5 bits per channel: keep colors on multiples of 8
    return tuple(int(round(v)) & 0xF8 for v in color)


def four_colors(pixels):
    """The 4 means of the pixels' colors, from white to black (k-means), and each pixel's."""
    centers = [(255, 255, 255), (180, 180, 180), (90, 90, 90), (0, 0, 0)]
    for _ in range(30):
        labels = [min(range(4), key=lambda k: sum((p[i] - centers[k][i]) ** 2 for i in range(3)))
                  for p in pixels]
        new = []
        for k in range(4):
            members = [p for p, l in zip(pixels, labels) if l == k]
            new.append(tuple(sum(p[i] for p in members) / len(members) for i in range(3)) if members else centers[k])
        if new == centers:
            break
        centers = new
    return centers, labels


def outside_white(whites, w, h):
    """The white pixels (whites[i] true) reachable from the box's edges."""
    seen = set()
    stack = [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for x in (0, w - 1) for y in range(h)]
    while stack:
        x, y = stack.pop()
        if (x, y) in seen or not (0 <= x < w and 0 <= y < h) or not whites[y * w + x]:
            continue
        seen.add((x, y))
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return seen


def fewer_colors(pixels):
    """The pixels' colors on the GBA, the closest two merged (into the more used one) until
    MAX_COLORS are left: the colors, from the lightest to the darkest, and each pixel's."""
    counts = {}
    for p in pixels:
        counts[gba(p)] = counts.get(gba(p), 0) + 1
    merged = {c: c for c in counts}
    while len(counts) > MAX_COLORS:
        a, b = min(((a, b) for a in counts for b in counts if a < b),
                   key=lambda ab: sum((ab[0][i] - ab[1][i]) ** 2 for i in range(3)))
        keep, drop = (a, b) if counts[a] >= counts[b] else (b, a)
        counts[keep] += counts.pop(drop)
        for c in merged:
            if merged[c] == drop:
                merged[c] = keep
    colors = sorted(counts, key=lambda c: -(c[0] * 299 + c[1] * 587 + c[2] * 114))
    return colors, [colors.index(merged[gba(p)]) for p in pixels]


def convert(sheet, name, box, exact):
    crop = sheet.crop(box)
    w, h = crop.size
    cpx = crop.load()
    pixels = [cpx[x, y] for y in range(h) for x in range(w)]
    if exact:
        colors, labels = fewer_colors(pixels)
        outside = outside_white([min(p) >= 240 for p in pixels], w, h)
    else:
        colors, labels = four_colors(pixels)
        colors = [gba(c) for c in colors]
        outside = outside_white([l == 0 for l in labels], w, h)
    inside = [(x, y) for y in range(h) for x in range(w) if (x, y) not in outside]
    left = min(x for x, _ in inside)
    right = max(x for x, _ in inside) + 1
    top = min(y for _, y in inside)
    bottom = max(y for _, y in inside) + 1
    if right - left > BOX_SIZE or bottom - top > BOX_SIZE:
        raise SystemExit(f'{name}: {right - left}x{bottom - top}, {BOX_SIZE}x{BOX_SIZE} at most')

    dst = Image.new('P', (PIC_SIZE, PIC_SIZE), TRANSPARENT)
    dpx = dst.load()
    margin = (PIC_SIZE - BOX_SIZE) // 2
    dx = margin + (BOX_SIZE - (right - left)) // 2 - left
    dy = margin + BOX_SIZE - bottom
    for x, y in inside:
        dpx[x + dx, y + dy] = labels[y * w + x] + 1
    flat = []
    for color in [(0, 0, 0)] + colors + [(0, 0, 0)] * (15 - len(colors)):
        flat += list(color)
    dst.putpalette(flat)
    dst.save(os.path.join(OUT_PICS, f'trainer_{name}.png'), transparency=0)


def main():
    sheet = Image.open(SHEET_FILE).convert('RGB')
    sheet2 = Image.open(SHEET_2_FILE).convert('RGB')
    os.makedirs(OUT_PICS, exist_ok=True)
    for _, name, box in SPRITES:
        convert(sheet, name, box, False)
    for _, name, (row, col) in SPRITES_2:
        left = SHEET_2_LEFT + col * SHEET_2_STEP
        top = SHEET_2_LEFT + row * SHEET_2_STEP
        convert(sheet2, name, (left, top, left + BOX_SIZE, top + BOX_SIZE), True)
    sprites = SPRITES + SPRITES_2

    with open(OUT_TABLE, 'w') as f:
        f.write('// Generated by tools/card_game/gen_trainer_pics.py from the trainer sprites in\n')
        f.write('// tools/card_game/. Do not edit by hand: rerun the script.\n\n')
        for _, name, _ in sprites:
            ident = ''.join(part.capitalize() for part in name.split('_'))
            path = f'graphics/card_game/retro/trainer_{name}.png'
            f.write(f'static const u32 sTrainerPic_{ident}[] = INCGFX_U32("{path}", ".4bpp.lz");\n')
            f.write(f'static const u16 sTrainerPal_{ident}[] = INCGFX_U16("{path}", ".gbapal");\n')
        f.write('\nstatic const struct RetroPic sTrainerPics[CARD_COUNT - CARD_TRAINER_START] =\n{\n')
        for card, name, _ in sprites:
            ident = ''.join(part.capitalize() for part in name.split('_'))
            f.write(f'    [CARD_{card} - CARD_TRAINER_START] = {{sTrainerPic_{ident}, sTrainerPal_{ident}}},\n')
        f.write('};\n')
    print(f'{len(sprites)} pictures in {os.path.relpath(OUT_PICS, ROOT)}')


if __name__ == '__main__':
    main()
