#!/usr/bin/env python3
"""Light Pewter Museum's glass front at night. Run from the pokeemerald root; running it again changes nothing.

The glass front uses palette 10's pale greys, which other tiles share, so its metatiles are moved to slot 12,
a copy of palette 10 whose greys (2-4) src/day_night.c lights."""
import struct

NUM_PRIMARY = 640
MUSEUM = ("kanto_pewter_city", 10, 12, [0x2A2, 0x2A3, 0x2A4, 0x2AA, 0x2AB, 0x2AC])  # palette, copy slot, glass front


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


museum(*MUSEUM)
