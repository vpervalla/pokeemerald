#!/usr/bin/env python3
"""Makes the card game's pictures from the Pokémon Gold front sprites (Bulbasaur to Celebi), and
the Megas' from tools/card_game/mega_sprites.png (Mega Pokémon drawn in the Gold style).

    git clone https://github.com/pret/pokegold
    python3 tools/card_game/gen_retro_pics.py path/to/pokegold     (needs Pillow)

    graphics/card_game/retro/<name>.png    64x64 indexed PNGs (the build makes .4bpp.lz/.gbapal)
    src/data/card_game/retro_pics.h        the table of them, by species

Each Gold sprite has 4 colors: white, two shades and black. The white around the Pokémon
becomes transparent (a flood fill from the sprite's edges), the white inside stays white. The
sprite keeps its size (40, 48 or 56 pixels) and sits as in Gold: centered, on the bottom of a
56x56 box, itself centered in the 64x64 picture.

mega_sprites.png is a grid of 56x56 cells, already transparent around the Pokémon, in the order
of MEGA_SHEET below (X and Y's Megas, then Omega Ruby and Alpha Sapphire's); None marks the
Megas this game doesn't have. A Mega that isn't in it (Dragonite) keeps the game's front pic.
"""

import os
import sys
from PIL import Image

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT_PICS = os.path.join(ROOT, 'graphics', 'card_game', 'retro')
OUT_TABLE = os.path.join(ROOT, 'src', 'data', 'card_game', 'retro_pics.h')

PIC_SIZE = 64
BOX_SIZE = 56
SPECIES_COUNT = 251

MEGA_SHEET_FILE = os.path.join(ROOT, 'tools', 'card_game', 'mega_sprites.png')
MEGA_SHEET_COLUMNS = 10
MEGA_SHEET = [
    'VENUSAUR_MEGA', 'CHARIZARD_MEGA_X', 'CHARIZARD_MEGA_Y', 'BLASTOISE_MEGA', 'ALAKAZAM_MEGA',
    'GENGAR_MEGA', 'KANGASKHAN_MEGA', 'PINSIR_MEGA', 'GYARADOS_MEGA', 'AERODACTYL_MEGA',
    'MEWTWO_MEGA_X', 'MEWTWO_MEGA_Y', 'AMPHAROS_MEGA', 'SCIZOR_MEGA', 'HERACROSS_MEGA',
    'HOUNDOOM_MEGA', 'TYRANITAR_MEGA', 'BLAZIKEN_MEGA', 'GARDEVOIR_MEGA', 'MAWILE_MEGA',
    'AGGRON_MEGA', 'MEDICHAM_MEGA', 'MANECTRIC_MEGA', 'BANETTE_MEGA', 'ABSOL_MEGA',
    'LATIAS_MEGA', 'LATIOS_MEGA', None, None, None,                 # Garchomp, Lucario, Abomasnow
    'BEEDRILL_MEGA', 'PIDGEOT_MEGA', 'SLOWBRO_MEGA', 'STEELIX_MEGA', 'SCEPTILE_MEGA',
    'SWAMPERT_MEGA', 'SABLEYE_MEGA', 'SHARPEDO_MEGA', 'CAMERUPT_MEGA', 'ALTARIA_MEGA',
    'GLALIE_MEGA', 'SALAMENCE_MEGA', 'METAGROSS_MEGA', None, None,  # Primal Kyogre, Groudon
    'RAYQUAZA_MEGA', None, None, None, None,                        # Lopunny, Gallade, Audino, Diancie
]

# pokegold's names that differ from the SPECIES_* ones
SPECIES_NAMES = {'FARFETCH_D': 'FARFETCHD', 'MR__MIME': 'MR_MIME'}

# Indices in the output: 0 transparent, then the 4 Gold colors
TRANSPARENT = 0


def gba(color):
    # The GBA has 5 bits per channel: keep colors on multiples of 8
    return tuple(v & 0xF8 for v in color)


def read_species(gold):
    """The first 251 names of pokegold's constants/pokemon_constants.asm, in national order."""
    names = []
    with open(os.path.join(gold, 'constants', 'pokemon_constants.asm')) as f:
        for line in f:
            parts = line.split()
            if len(parts) >= 2 and parts[0] == 'const':
                names.append(parts[1])
    return names[:SPECIES_COUNT]


def outside_white(src):
    """The white pixels (index 0) reachable from the sprite's edges."""
    w, h = src.size
    px = src.load()
    seen = set()
    stack = [(x, y) for x in range(w) for y in (0, h - 1)] + [(x, y) for x in (0, w - 1) for y in range(h)]
    while stack:
        x, y = stack.pop()
        if (x, y) in seen or not (0 <= x < w and 0 <= y < h) or px[x, y] != 0:
            continue
        seen.add((x, y))
        stack += [(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)]
    return seen


def save(dst, palette, name):
    flat = []
    for color in palette + [(0, 0, 0)] * (16 - len(palette)):
        flat += list(color)
    dst.putpalette(flat)
    dst.save(os.path.join(OUT_PICS, name.lower() + '.png'), transparency=0)


def convert(gold, name):
    folder = os.path.join(gold, 'gfx', 'pokemon', 'unown_a' if name == 'UNOWN' else name.lower())
    # front_gold.png where Gold and Silver differ, front.png otherwise
    path = os.path.join(folder, 'front_gold.png')
    if not os.path.exists(path):
        path = os.path.join(folder, 'front.png')
    src = Image.open(path)
    if src.mode != 'P':
        raise SystemExit(f'{path}: expected an indexed PNG')
    w, h = src.size
    colors = src.getpalette()[:12]
    palette = [(0, 0, 0)] + [gba(colors[i * 3:i * 3 + 3]) for i in range(4)]

    outside = outside_white(src)
    dst = Image.new('P', (PIC_SIZE, PIC_SIZE), TRANSPARENT)
    box = (PIC_SIZE - BOX_SIZE) // 2
    left = box + (BOX_SIZE - w) // 2
    top = box + BOX_SIZE - h
    spx, dpx = src.load(), dst.load()
    for y in range(h):
        for x in range(w):
            if (x, y) not in outside:
                dpx[left + x, top + y] = spx[x, y] + 1
    save(dst, palette, name)


def convert_megas():
    """The Megas of mega_sprites.png. Returns their names."""
    sheet = Image.open(MEGA_SHEET_FILE).convert('RGBA')
    names = []
    for i, name in enumerate(MEGA_SHEET):
        if name is None:
            continue
        row, col = divmod(i, MEGA_SHEET_COLUMNS)
        cell = sheet.crop((col * BOX_SIZE, row * BOX_SIZE, (col + 1) * BOX_SIZE, (row + 1) * BOX_SIZE))
        cpx = cell.load()
        # Its colors from the lightest to the darkest, as in Gold (white, 2 shades, black)
        colors = sorted({cpx[x, y][:3] for y in range(BOX_SIZE) for x in range(BOX_SIZE) if cpx[x, y][3] >= 128},
                        key=lambda c: -(c[0] * 299 + c[1] * 587 + c[2] * 114))
        if len(colors) > 15:
            raise SystemExit(f'{name}: {len(colors)} colors, 15 at most')
        dst = Image.new('P', (PIC_SIZE, PIC_SIZE), TRANSPARENT)
        dpx = dst.load()
        box = (PIC_SIZE - BOX_SIZE) // 2
        for y in range(BOX_SIZE):
            for x in range(BOX_SIZE):
                if cpx[x, y][3] >= 128:
                    dpx[box + x, box + y] = colors.index(cpx[x, y][:3]) + 1
        save(dst, [(0, 0, 0)] + [gba(c) for c in colors], name)
        names.append(name)
    return names


def main():
    if len(sys.argv) != 2:
        raise SystemExit(__doc__)
    gold = sys.argv[1]
    names = read_species(gold)
    os.makedirs(OUT_PICS, exist_ok=True)
    for name in names:
        convert(gold, name)
    names += convert_megas()

    with open(OUT_TABLE, 'w') as f:
        f.write('// Generated by tools/card_game/gen_retro_pics.py from the Pokémon Gold front sprites\n')
        f.write('// (pret/pokegold). Do not edit by hand: rerun the script.\n\n')
        for name in names:
            ident = ''.join(part.capitalize() for part in name.lower().split('_'))
            path = f'graphics/card_game/retro/{name.lower()}.png'
            f.write(f'static const u32 sRetroPic_{ident}[] = INCGFX_U32("{path}", ".4bpp.lz");\n')
            f.write(f'static const u16 sRetroPal_{ident}[] = INCGFX_U16("{path}", ".gbapal");\n')
        f.write('\nstatic const struct RetroPic sRetroPics[] =\n{\n')
        for name in names:
            ident = ''.join(part.capitalize() for part in name.lower().split('_'))
            species = 'SPECIES_' + SPECIES_NAMES.get(name, name)
            f.write(f'    [{species}] = {{sRetroPic_{ident}, sRetroPal_{ident}}},\n')
        f.write('};\n')
    print(f'{len(names)} pictures in {os.path.relpath(OUT_PICS, ROOT)}')


if __name__ == '__main__':
    main()
