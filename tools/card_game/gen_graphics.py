#!/usr/bin/env python3
"""Draws the card game's graphics as indexed PNGs (the build turns them into .4bpp/.gbapal).

    python3 tools/card_game/gen_graphics.py        (from anywhere; needs Pillow)

    graphics/card_game/board.png   BG tiles of the table (see the BOARD_TILE_* in src/card_game.c)
    graphics/card_game/pips.png    8x8 sprites: an Energy of each card type, trophies
    graphics/card_game/coin.png    32x32 sprite frames of the coin: heads, edge, tails
    graphics/card_game/energy_icons.png  9x9 Energy icons for the attack costs in the card texts
"""

import os
from PIL import Image, ImageDraw

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT = os.path.join(ROOT, 'graphics', 'card_game')

# The GBA has 5 bits per channel: keep colors on multiples of 8
def c(r, g, b):
    return (r & 0xF8, g & 0xF8, b & 0xF8)


def save(img, palette, name):
    flat = []
    for color in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += list(color)
    img.putpalette(flat)
    img.save(os.path.join(OUT, name), transparency=0)


# ---------------------------------------------------------------------------------------------
# Board: two table shades (opponent's half, player's half), each with a fill tile and the 9
# tiles of a slot frame (corners, edges, center)

BOARD_PALETTE = [
    c(0, 0, 0),         # 0 transparent
    c(32, 88, 56),      # 1 opponent's table
    c(40, 112, 64),     # 2 player's table
    c(16, 56, 32),      # 3 opponent's slot
    c(24, 72, 40),      # 4 player's slot
    c(144, 200, 152),   # 5 frame
    c(8, 32, 16),       # 6 frame shadow
]


def frame_tile(part, table, slot):
    """part: 0..8 (TL, T, TR, L, C, R, BL, B, BR) of a slot frame."""
    t = [[slot] * 8 for _ in range(8)]
    row, col = divmod(part, 3)
    for y in range(8):
        for x in range(8):
            top, bottom, left, right = row == 0, row == 2, col == 0, col == 2
            # Outside the rounded frame: table
            if (top and y < 1) or (bottom and y > 6) or (left and x < 1) or (right and x > 6):
                t[y][x] = table
            elif (top and y == 1) or (left and x == 1):
                t[y][x] = 5
            elif (bottom and y == 6) or (right and x == 6):
                t[y][x] = 6
    # Round the corners
    if part in (0, 2, 6, 8):
        cy = 1 if row == 0 else 6
        cx = 1 if col == 0 else 6
        t[cy][cx] = table
    return t


def board():
    tiles = []
    for table, slot in ((1, 3), (2, 4)):
        tiles.append([[table] * 8 for _ in range(8)])
        for part in range(9):
            tiles.append(frame_tile(part, table, slot))
    img = Image.new('P', (8 * len(tiles), 8))
    for i, t in enumerate(tiles):
        for y in range(8):
            for x in range(8):
                img.putpixel((i * 8 + x, y), t[y][x])
    save(img, BOARD_PALETTE, 'board.png')


# ---------------------------------------------------------------------------------------------
# Pips: CARD_TYPE_NONE .. CARD_TYPE_COLORLESS, then a trophy won and a trophy to win

PIP_PALETTE = [
    c(0, 0, 0),         # 0 transparent
    c(16, 16, 24),      # 1 outline
    c(248, 248, 248),   # 2 shine
    c(88, 192, 80),     # 3 Grass
    c(232, 72, 48),     # 4 Fire
    c(64, 144, 240),    # 5 Water
    c(248, 208, 48),    # 6 Lightning
    c(176, 96, 208),    # 7 Psychic
    c(200, 120, 56),    # 8 Fighting
    c(80, 72, 104),     # 9 Darkness
    c(168, 176, 184),   # 10 Metal
    c(200, 168, 72),    # 11 Dragon
    c(232, 232, 224),   # 12 Colorless
    c(248, 200, 32),    # 13 trophy
    c(96, 104, 96),     # 14 trophy to win
]

CIRCLE = [
    '..1111..',
    '.122221.',
    '12222221',
    '12222221',
    '12222221',
    '12222221',
    '.122221.',
    '..1111..',
]
STAR = [
    '...11...',
    '..1221..',
    '11122111',
    '12222221',
    '.122221.',
    '.122221.',
    '.12..21.',
    '.1....1.',
]


def pips():
    img = Image.new('P', (8 * 13, 8))
    for i in range(13):
        if i == 0:
            continue  # CARD_TYPE_NONE: empty
        shape, fill = (CIRCLE, 2 + i) if i <= 10 else (STAR, 13 if i == 11 else 14)
        for y in range(8):
            for x in range(8):
                ch = shape[y][x]
                if ch == '1':
                    img.putpixel((i * 8 + x, y), 1)
                elif ch == '2':
                    img.putpixel((i * 8 + x, y), fill)
        if i <= 10:
            img.putpixel((i * 8 + 2, 2), 2)  # shine
    save(img, PIP_PALETTE, 'pips.png')


# ---------------------------------------------------------------------------------------------
# Coin: heads (a Poké Ball), edge (spinning), tails (a gold coin)

COIN_PALETTE = [
    c(0, 0, 0),         # 0 transparent
    c(24, 24, 32),      # 1 outline
    c(232, 56, 48),     # 2 red
    c(248, 248, 248),   # 3 white
    c(200, 200, 208),   # 4 white shade
    c(248, 200, 48),    # 5 gold
    c(184, 128, 24),    # 6 gold shade
    c(160, 32, 32),     # 7 red shade
]


def coin():
    img = Image.new('P', (32, 96))
    d = ImageDraw.Draw(img)
    # Heads
    d.ellipse((4, 4, 27, 27), fill=3, outline=1)
    d.pieslice((4, 4, 27, 27), 180, 360, fill=2, outline=1)
    d.rectangle((5, 15, 26, 16), fill=1)
    d.ellipse((12, 12, 19, 19), fill=3, outline=1)
    d.point([(9, 8), (10, 7), (11, 7)], fill=3)
    # Edge
    d.ellipse((13, 32 + 4, 18, 32 + 27), fill=6, outline=1)
    # Tails
    d.ellipse((4, 68, 27, 91), fill=5, outline=1)
    d.ellipse((8, 72, 23, 87), fill=5, outline=6)
    d.polygon([(16, 74), (18, 79), (23, 79), (19, 82), (21, 87), (16, 84), (11, 87), (13, 82), (9, 79), (14, 79)], fill=6)
    save(img, COIN_PALETTE, 'coin.png')


# ---------------------------------------------------------------------------------------------
# Energy icons: drawn in the text windows, in place of the cost letters. One per CARD_TYPE_*,
# each in a 16x16 cell (CARD_TYPE_NONE and DRAGON empty), from the Energy cards of the Game Boy
# TCG: a colored circle with a dark symbol, outlined in black. Colors 0-3 are the text
# palette's (graphics/interface/std_menu.pal), only 4-15 are loaded over it.

ICON_PALETTE = [
    c(0, 0, 0),         # 0 transparent
    c(255, 255, 255),   # 1 white (text background)
    c(98, 98, 98),      # 2 dark gray (text)
    c(213, 213, 205),   # 3 light gray (text shadow)
    c(32, 24, 16),      # 4 outline, dark symbols
    c(48, 192, 48),     # 5 Grass
    c(0, 80, 0),        # 6 leaf
    c(248, 80, 120),    # 7 Fire
    c(184, 0, 0),       # 8 flame
    c(32, 168, 248),    # 9 Water
    c(0, 24, 112),      # 10 drop
    c(248, 216, 0),     # 11 Lightning
    c(240, 88, 16),     # 12 Fighting
    c(216, 104, 216),   # 13 Psychic
    c(96, 24, 120),     # 14 eye
    c(168, 168, 144),   # 15 Metal
]

# o outline, h highlight, b body, s symbol; by CARD_TYPE_*: (body, symbol, rows)
ICONS = {
    1: (5, 6, [         # Grass: a leaf
        "  ooooo  ",
        " obbbbso ",
        "obbbsssso",
        "obbssbsbo",
        "obssbssbo",
        "obsbssbbo",
        "obsbbbbbo",
        " osbbbbo ",
        "  ooooo  "]),
    2: (7, 8, [         # Fire: a flame
        "  ooooo  ",
        " obbsbbo ",
        "obbbssbbo",
        "obsbsssbo",
        "obssssbbo",
        "obsshssbo",
        "obbsssbbo",
        " obbbbbo ",
        "  ooooo  "]),
    3: (9, 10, [        # Water: a drop
        "  ooooo  ",
        " obbsbbo ",
        "obbbsbbbo",
        "obbsssbbo",
        "obbsssbbo",
        "obssshsbo",
        "obbsssbbo",
        " obbbbbo ",
        "  ooooo  "]),
    4: (11, 4, [        # Lightning: a bolt
        "  ooooo  ",
        " obbbsso ",
        "obbbssbbo",
        "obbssbbbo",
        "obssssbbo",
        "obbbssbbo",
        "obbssbbbo",
        " obsbbbo ",
        "  ooooo  "]),
    5: (13, 14, [       # Psychic: an eye
        "  ooooo  ",
        " obbbbbo ",
        "obbbbbbbo",
        "obsssssbo",
        "ossbhbsso",
        "obsssssbo",
        "obbbbbbbo",
        " obbbbbo ",
        "  ooooo  "]),
    6: (12, 4, [        # Fighting: a fist
        "  ooooo  ",
        " obbbbbo ",
        "obsbsbsbo",
        "obsssssbo",
        "obsssssbo",
        "obssssbbo",
        "obbsssbbo",
        " obbbbbo ",
        "  ooooo  "]),
    7: (4, 3, [         # Darkness: a crescent moon
        "  ooooo  ",
        " obbbbbo ",
        "obbbssbbo",
        "obbbbssbo",
        "obbbbssbo",
        "obbbbssbo",
        "obbbssbbo",
        " obbbbbo ",
        "  ooooo  "]),
    8: (15, 2, [        # Metal: a triangle
        "  ooooo  ",
        " ohbbbbo ",
        "obbbsbbbo",
        "obbsssbbo",
        "obbsbsbbo",
        "obsssssbo",
        "obbbbbbbo",
        " obbbbbo ",
        "  ooooo  "]),
    10: (1, 4, [        # Colorless: a star
        "  ooooo  ",
        " obbsbbo ",
        "obsbsbsbo",
        "obbsssbbo",
        "obsssssbo",
        "obbsssbbo",
        "obsbsbsbo",
        " obbsbbo ",
        "  ooooo  "]),
}
ICON_TYPES = 11  # CARD_TYPE_COUNT


def energy_icons():
    img = Image.new('P', (16 * ICON_TYPES, 16))
    for cardType, (body, symbol, rows) in ICONS.items():
        colors = {'o': 4, 'h': 1, 'b': body, 's': symbol}
        for y, row in enumerate(rows):
            for x, ch in enumerate(row):
                if ch != ' ':
                    img.putpixel((cardType * 16 + x, y), colors[ch])
    save(img, ICON_PALETTE, 'energy_icons.png')


def main():
    os.makedirs(OUT, exist_ok=True)
    board()
    pips()
    coin()
    energy_icons()


if __name__ == '__main__':
    main()
