#!/usr/bin/env python3
"""Give the Pallet Town houses' window glass a palette of its own, so day/night can light it.
The glass uses colours 9-12 of kanto_general's palette 3, which Pokemon Center and Mart roofs share.
This copies that palette into kanto_pallet_town's unused slot 7 and points the window tiles at it,
so the houses look the same but slot 7's colours 9-12 belong to the windows alone. Run from the
pokeemerald root; running it again changes nothing."""
import shutil, struct
from PIL import Image
P, S = "data/tilesets/primary/kanto_general/", "data/tilesets/secondary/kanto_pallet_town/"
SRC_PAL, DST_PAL, GLASS = 3, 7, {9, 10, 11, 12}

def tiles(path):
    im = Image.open(path); w = im.width // 8
    return [list(im.crop(((i % w) * 8, (i // w) * 8, (i % w) * 8 + 8, (i // w) * 8 + 8)).getdata())
            for i in range(w * (im.height // 8))]

shutil.copyfile(f"{P}palettes/{SRC_PAL:02d}.pal", f"{S}palettes/{DST_PAL:02d}.pal")
pt, st = tiles(P + "tiles.png"), tiles(S + "tiles.png")
mb = bytearray(open(S + "metatiles.bin", "rb").read())
changed = 0
for i in range(len(mb) // 2):
    v = struct.unpack_from("<H", mb, i * 2)[0]
    t = v & 0x3FF
    if v >> 12 == SRC_PAL and set(pt[t] if t < 640 else st[t - 640]) & GLASS:
        struct.pack_into("<H", mb, i * 2, (v & 0x0FFF) | (DST_PAL << 12))
        changed += 1
open(S + "metatiles.bin", "wb").write(mb)
print(f"moved {changed} window tiles to palette {DST_PAL}")
