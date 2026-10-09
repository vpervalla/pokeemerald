#!/usr/bin/env python3
"""Adjust which windows src/day_night.c lights at night. Run from the pokeemerald root; running it again
changes nothing.

Pewter Museum: its glass front uses palette 10's pale greys, which other tiles share, so its metatiles are
moved to slot 12, a copy of palette 10 whose greys (2-4) day_night.c lights.

Empty houses: a house nobody is home in keeps its windows dark. Its window metatiles get "unlit twins" in the
town's secondary tileset: the same tiles, with the glass lit through slot 7 (the copy of kanto_general's
palette 3, see split_window_palettes.py) drawn with palette 3 itself. The town's map script swaps them in
while the house is empty."""
import struct

from PIL import Image

PRIMARY = "data/tilesets/primary/kanto_general/"
NUM_PRIMARY = 640
SLOT7_GLASS = set(range(9, 14))
MUSEUM = ("kanto_pewter_city", 10, 12, [0x2A2, 0x2A3, 0x2A4, 0x2AA, 0x2AB, 0x2AC])  # palette, copy slot, glass front
# Lostelle's house on Three Island, empty until she's home (FLAG_HIDE_LOSTELLE_IN_HER_HOME, ThreeIsland's scripts.inc)
EMPTY_HOUSES = [("kanto_sevii_islands_123", [0x2AE, 0x2AD])]


def museum(sec, pal, slot, metatiles):
    d = f"data/tilesets/secondary/{sec}/"
    with open(f"{d}palettes/{pal:02d}.pal", "rb") as f:
        data = f.read()
    with open(f"{d}palettes/{slot:02d}.pal", "wb") as f:
        f.write(data)
    sm = bytearray(open(d + "metatiles.bin", "rb").read())
    for m in metatiles:
        for n in range(8):
            o = ((m - NUM_PRIMARY) * 8 + n) * 2
            v = struct.unpack_from("<H", sm, o)[0]
            if v >> 12 == pal:
                struct.pack_into("<H", sm, o, (v & 0x0FFF) | (slot << 12))
    open(d + "metatiles.bin", "wb").write(sm)
    print(f"Pewter Museum: glass front moved to palette {slot}")


def tiles_of(path):
    im = Image.open(path); w = im.width // 8
    return lambda t: set(im.crop(((t % w) * 8, (t // w) * 8, (t % w) * 8 + 8, (t // w) * 8 + 8)).get_flattened_data())


def unlit_twins(sec, metatiles):
    d = f"data/tilesets/secondary/{sec}/"
    prim_tiles, sec_tiles = tiles_of(PRIMARY + "tiles.png"), tiles_of(d + "tiles.png")
    tile = lambda t: prim_tiles(t) if t < NUM_PRIMARY else sec_tiles(t - NUM_PRIMARY)
    sm, sa = bytearray(open(d + "metatiles.bin", "rb").read()), bytearray(open(d + "metatile_attributes.bin", "rb").read())
    for m in metatiles:
        o = (m - NUM_PRIMARY) * 16
        entries = struct.unpack_from("<8H", sm, o)
        unlit = [(v & 0x0FFF) | (3 << 12) if v >> 12 == 7 and tile(v & 0x3FF) & SLOT7_GLASS else v for v in entries]
        packed, attr = struct.pack("<8H", *unlit), sa[(m - NUM_PRIMARY) * 2:(m - NUM_PRIMARY) * 2 + 2]
        twin = next((i for i in range(len(sm) // 16)
                     if sm[i * 16:i * 16 + 16] == packed and sa[i * 2:i * 2 + 2] == attr and i * 16 != o), None)
        if twin is None:
            sm.extend(packed); sa.extend(attr); twin = len(sm) // 16 - 1
        print(f"{sec}: metatile {m:#05x} unlit twin {NUM_PRIMARY + twin:#05x}")
    assert len(sm) // 16 <= 1024 - NUM_PRIMARY
    open(d + "metatiles.bin", "wb").write(sm); open(d + "metatile_attributes.bin", "wb").write(sa)


museum(*MUSEUM)
for sec, metatiles in EMPTY_HOUSES:
    unlit_twins(sec, metatiles)
