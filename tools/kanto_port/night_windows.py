#!/usr/bin/env python3
"""Adjust which windows src/day_night.c lights at night. Run from the pokeemerald root; running it again
changes nothing.

Dark buildings: buildings you can't enter (no door) keep their windows dark. Their window metatiles are
replaced, on those buildings only, by "unlit twins" added to the town's secondary tileset: the same tiles,
drawn with a palette that holds the same colours but isn't lit.
  - Glass lit through slot 7 (the copy of kanto_general's palette 3, see split_window_palettes.py) is drawn
    with palette 3 itself.
  - Glass lit in a town palette (sLitPalettes) is drawn with an unlit copy of that palette in a free slot.

Pewter Museum: its glass front uses palette 10's pale greys, which other tiles share, so its metatiles are
moved to slot 12, a copy of palette 10 whose greys (2-4) day_night.c lights."""
import json, struct

PRIMARY = "data/tilesets/primary/kanto_general/"
NUM_PRIMARY = 640
SLOT7_GLASS = set(range(9, 14))

# Town: (secondary tileset dir, layout, glass lighting: ('slot7',) or ('palette', lit palette, unlit copy slot, lit colours),
#        door-less buildings as (x0, x1, y0, y1) tile boxes, inclusive)
DARK = {
    "CeladonCity": ("kanto_celadon_city", ("slot7",),
                    [(4, 8, 7, 11), (18, 24, 7, 11), (34, 45, 7, 11), (44, 51, 17, 21), (4, 7, 26, 29),
                     (16, 19, 26, 29), (20, 23, 26, 34), (32, 35, 26, 29), (44, 47, 26, 29)]),
    "CeruleanCity": ("kanto_cerulean_city", ("slot7",), [(38, 43, 7, 11), (31, 35, 25, 29)]),
    "VermilionCity": ("kanto_vermilion_city", ("palette", 9, 11, {8, 9, 14, 15}), [(18, 21, 3, 7), (25, 29, 3, 7)]),
}
MUSEUM = ("kanto_pewter_city", 10, 12, [0x2A2, 0x2A3, 0x2A4, 0x2AA, 0x2AB, 0x2AC])  # palette, copy slot, glass front


def tiles_of(path):
    from PIL import Image
    im = Image.open(path); w = im.width // 8
    return lambda t: set(im.crop(((t % w) * 8, (t // w) * 8, (t % w) * 8 + 8, (t // w) * 8 + 8)).get_flattened_data())


def copy_palette(d, src, dst):
    with open(f"{d}palettes/{src:02d}.pal", "rb") as f:
        data = f.read()
    with open(f"{d}palettes/{dst:02d}.pal", "wb") as f:
        f.write(data)


def dark_buildings(name, sec, glass, boxes):
    d = f"data/tilesets/secondary/{sec}/"
    layout = json.load(open(f"data/maps/{name}/map.json"))["layout"]
    L = next(l for l in json.load(open("data/layouts/layouts.json"))["layouts"] if l and l.get("id") == layout)
    prim_tiles, sec_tiles = tiles_of(PRIMARY + "tiles.png"), tiles_of(d + "tiles.png")
    tile = lambda t: prim_tiles(t) if t < NUM_PRIMARY else sec_tiles(t - NUM_PRIMARY)
    pm, pa = open(PRIMARY + "metatiles.bin", "rb").read(), open(PRIMARY + "metatile_attributes.bin", "rb").read()
    sm, sa = bytearray(open(d + "metatiles.bin", "rb").read()), bytearray(open(d + "metatile_attributes.bin", "rb").read())
    if glass[0] == "palette":
        copy_palette(d, glass[1], glass[2])

    def entries(m):
        data, mm = (pm, m) if m < NUM_PRIMARY else (sm, m - NUM_PRIMARY)
        return [struct.unpack_from("<H", data, (mm * 8 + n) * 2)[0] for n in range(8)]

    def attr(m):
        data, mm = (pa, m) if m < NUM_PRIMARY else (sa, m - NUM_PRIMARY)
        return data[mm * 2:mm * 2 + 2]

    def unlit(v):
        pal, t = v >> 12, v & 0x3FF
        if glass[0] == "slot7" and pal == 7 and tile(t) & SLOT7_GLASS:
            return (v & 0x0FFF) | (3 << 12)
        if glass[0] == "palette" and pal == glass[1] and tile(t) & glass[3]:
            return (v & 0x0FFF) | (glass[2] << 12)
        return v

    def twin(m):
        e = [unlit(v) for v in entries(m)]
        if e == entries(m):
            return m  # Nothing lit in it
        packed = struct.pack("<8H", *e)
        for i in range(len(sm) // 16):  # Reuse an existing twin
            if bytes(sm[i * 16:i * 16 + 16]) == packed and bytes(sa[i * 2:i * 2 + 2]) == attr(m):
                return NUM_PRIMARY + i
        sm.extend(packed); sa.extend(attr(m))
        return NUM_PRIMARY + len(sm) // 16 - 1

    blocks = bytearray(open(L["blockdata_filepath"], "rb").read())
    changed = 0
    for x0, x1, y0, y1 in boxes:
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                o = (y * L["width"] + x) * 2
                v = struct.unpack_from("<H", blocks, o)[0]
                m = twin(v & 0x3FF)
                if m != v & 0x3FF:
                    struct.pack_into("<H", blocks, o, (v & ~0x3FF) | m); changed += 1
    assert len(sm) // 16 <= 1024 - NUM_PRIMARY
    open(d + "metatiles.bin", "wb").write(sm); open(d + "metatile_attributes.bin", "wb").write(sa)
    open(L["blockdata_filepath"], "wb").write(blocks)
    print(f"{name}: {changed} window tiles darkened, {sec} now has {len(sm) // 16} metatiles")


def museum(sec, pal, slot, metatiles):
    d = f"data/tilesets/secondary/{sec}/"
    copy_palette(d, pal, slot)
    sm = bytearray(open(d + "metatiles.bin", "rb").read())
    for m in metatiles:
        for n in range(8):
            o = ((m - NUM_PRIMARY) * 8 + n) * 2
            v = struct.unpack_from("<H", sm, o)[0]
            if v >> 12 == pal:
                struct.pack_into("<H", sm, o, (v & 0x0FFF) | (slot << 12))
    open(d + "metatiles.bin", "wb").write(sm)
    print(f"Pewter Museum: glass front moved to palette {slot}")


for name, (sec, glass, boxes) in DARK.items():
    dark_buildings(name, sec, glass, boxes)
museum(*MUSEUM)
