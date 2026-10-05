#!/usr/bin/env python3
"""Give the Kanto towns' window glass a palette of its own, so day/night can light it (src/day_night.c).

Most window glass uses colours 9-13 of kanto_general's palette 3, which roofs, doors and water share.
This copies palette 3 into slot 7 (unused) of every Kanto secondary tileset and points the tiles of
the window metatiles listed below at slot 7. The maps look the same, but slot 7's glass colours belong
to the windows alone. Windows in a town's own palettes whose glass colours nothing else uses are lit
in place instead; those are listed in src/day_night.c.

Run from the pokeemerald root; running it again changes nothing."""
import glob, json, os, re, shutil, struct
from PIL import Image

SRC_PAL, DST_PAL, GLASS = 3, 7, {9, 10, 11, 12, 13}
PRIMARY = "data/tilesets/primary/kanto_general/"
# Pokemon Center, Mart and house windows in kanto_general. Doors are left out: door animations
# draw with their own palette, so a door moved to slot 7 would change colour as it opens.
PRIMARY_WINDOWS = [0x006, 0x007, 0x020, 0x021, 0x042, 0x043, 0x047, 0x04F, 0x057, 0x058, 0x059, 0x05B,
                   0x150, 0x151, 0x154, 0x155, 0x156, 0x180, 0x181, 0x182, 0x18B, 0x18C, 0x197, 0x19B,
                   0x19C, 0x1B5, 0x1B6, 0x1B7]
# Window metatiles of each town's own tileset that use palette 3's glass.
SECONDARY_WINDOWS = {
    "kanto_pallet_town": [0x299, 0x29A, 0x29B, 0x2A1, 0x2A2],
    "kanto_viridian_city": [0x29A, 0x29B, 0x2A5, 0x2A6, 0x2C6],
    "kanto_cerulean_city": [0x2B6, 0x2B7, 0x2BD, 0x2BE, 0x2BF, 0x2C0, 0x2F4, 0x2F5, 0x2F6, 0x2F7],
    "kanto_celadon_city": [0x295, 0x296, 0x297, 0x29D, 0x29E, 0x29F, 0x2E7, 0x2E8, 0x2E9, 0x2EA, 0x2EB,
                           0x2EC, 0x2F0, 0x2F1, 0x2F2, 0x2F3, 0x2F4, 0x2F8, 0x2F9, 0x2FA, 0x2FB, 0x2FC, 0x2FD,
                           0x2FE, 0x2FF, 0x300, 0x303, 0x308, 0x30B, 0x310, 0x311, 0x312, 0x313, 0x314, 0x315, 0x316],
    "kanto_fuchsia_city": [0x2B9, 0x2BA, 0x2BB, 0x2D9],
    "kanto_saffron_city": [0x336, 0x337],
}

def tiles(path):
    im = Image.open(path); w = im.width // 8
    return [set(im.crop(((i % w) * 8, (i // w) * 8, (i % w) * 8 + 8, (i // w) * 8 + 8)).getdata())
            for i in range(w * (im.height // 8))]

def kanto_secondaries():
    """Secondary tileset dirs used with kanto_general, from the layouts and tileset headers."""
    headers = open("src/data/tilesets/headers.h").read(); graphics = open("src/data/tilesets/graphics.h").read()
    dirs = {}
    for m in re.finditer(r"const struct Tileset (\w+) =\s*\{.*?\.tiles = (\w+)", headers, re.S):
        g = re.search(m.group(2) + r'\[\] = INCBIN_U32\("(data/tilesets/[^"]+/)', graphics)
        if g:
            dirs[m.group(1)] = g.group(1)
    layouts = [l for l in json.load(open("data/layouts/layouts.json"))["layouts"] if l]
    return sorted({dirs[l["secondary_tileset"]] for l in layouts if l.get("primary_tileset") == "gTileset_KantoGeneral"})

def remap(metatiles_path, ids, base, tile_px):
    """Moves the glass tiles of the listed metatiles to slot 7, and any other metatile's slot 7 tiles
    back to palette 3 (nothing in these tilesets used slot 7 before), so the lists are the whole truth."""
    mb = bytearray(open(metatiles_path, "rb").read())
    changed = 0
    for m in range(base, base + len(mb) // 16):
        for k in range(8):
            off = ((m - base) * 8 + k) * 2
            v = struct.unpack_from("<H", mb, off)[0]
            if m in ids and v >> 12 == SRC_PAL and tile_px(v & 0x3FF) & GLASS:
                new = (v & 0x0FFF) | (DST_PAL << 12)
            elif m not in ids and v >> 12 == DST_PAL:
                new = (v & 0x0FFF) | (SRC_PAL << 12)
            else:
                continue
            struct.pack_into("<H", mb, off, new)
            changed += 1
    open(metatiles_path, "wb").write(mb)
    return changed

primary_tiles = tiles(PRIMARY + "tiles.png")
copied = []
for d in kanto_secondaries():
    mb = open(d + "metatiles.bin", "rb").read()
    slots = {struct.unpack_from("<H", mb, i * 2)[0] >> 12 for i in range(len(mb) // 2)}
    name = os.path.basename(d.rstrip("/"))
    if DST_PAL in slots and name not in SECONDARY_WINDOWS:
        print(f"skipping {name}: slot {DST_PAL} is in use")
        continue
    shutil.copyfile(f"{PRIMARY}palettes/{SRC_PAL:02d}.pal", f"{d}palettes/{DST_PAL:02d}.pal")
    copied.append(name)
    if name in SECONDARY_WINDOWS:
        sec_tiles = tiles(d + "tiles.png")
        px = lambda t: primary_tiles[t] if t < 640 else sec_tiles[t - 640]
        print(f"{name}: {remap(d + 'metatiles.bin', SECONDARY_WINDOWS[name], 0x280, px)} tiles changed")
print(f"kanto_general: {remap(PRIMARY + 'metatiles.bin', PRIMARY_WINDOWS, 0, lambda t: primary_tiles[t])} tiles changed")
print("palette 3 copied to slot 7 of:", " ".join(copied))
