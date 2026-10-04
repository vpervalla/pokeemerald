#!/usr/bin/env python3
"""Add items to the bag of a Pokemon Emerald save file.

    python3 tools/edit_save.py pokeemerald.sav ITEM_POKE_BALL=10 ITEM_POTION=10
    python3 tools/edit_save.py pokeemerald.sav --running-shoes
    python3 tools/edit_save.py pokeemerald.sav --layout-id=442   # see include/constants/layouts.h

Run from the pokeemerald root (item ids and pockets are read from the source).
--layout-id repairs a save whose current map looks garbled after layouts were added or removed
(the save stores the numeric id of the layout the player is standing on).
The original file is copied to <file>.bak the first time. Close the emulator first.
"""
import re, shutil, struct, sys, os

SECTOR, FOOTER, SIGNATURE = 4096, 0xFF4, 0x08012025
SECTION1_SIZE = 3968                 # bytes of SaveBlock1 held in section 1 (covers the whole bag)
KEY_OFFSET = 0xAC                    # SaveBlock2.encryptionKey, in section 0
FLAGS_OFFSET = 0x1270                # SaveBlock1.flags
FLAG_SYS_B_DASH = 0x8C0              # "received Running Shoes" (SYSTEM_FLAGS 0x860 + 0x60)
POCKETS = {                          # pocket -> (offset in SaveBlock1, capacity)
    "POCKET_ITEMS": (0x560, 30), "POCKET_KEY_ITEMS": (0x5D8, 30), "POCKET_POKE_BALLS": (0x650, 16),
    "POCKET_TM_HM": (0x690, 64), "POCKET_BERRIES": (0x790, 46),
}

def checksum(data):
    total = sum(struct.unpack(f"<{len(data) // 4}I", data)) & 0xFFFFFFFF
    return ((total >> 16) + total) & 0xFFFF

def item_table():
    # Item ids are the first enum in items.h, numbered from 0 with no explicit values.
    body = re.search(r"enum\s*\{(.*?)\};", open("include/constants/items.h").read(), re.S).group(1)
    names = [n.strip() for n in re.sub(r"//.*", "", body).split(",") if n.strip()]
    if any("=" in n for n in names):
        sys.exit("items.h enum has explicit values; update item_table()")
    ids = {n: i for i, n in enumerate(names)}
    pockets = {m.group(1): m.group(2) for m in re.finditer(r"\[(ITEM_\w+)\] =\s*\{.*?\.pocket = (POCKET_\w+)", open("src/data/items.h").read(), re.S)}
    return ids, pockets

def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    args = sys.argv[2:]
    shoes = "--running-shoes" in args
    layout = next((int(a.split("=")[1]) for a in args if a.startswith("--layout-id=")), None)
    path, wanted = sys.argv[1], [a.split("=") for a in args if not a.startswith("--")]
    ids, pockets = item_table()
    d = bytearray(open(path, "rb").read())
    # Two save slots of 14 sectors each; the one with the higher counter is the current save.
    best = None
    for slot in range(2):
        secs = {}
        for i in range(14):
            o = (slot * 14 + i) * SECTOR
            sid, _, sig, counter = struct.unpack_from("<HHII", d, o + FOOTER)
            if sig == SIGNATURE:
                secs[sid] = (o, counter)
        if len(secs) == 14 and (best is None or secs[1][1] > best[1][1]):
            best = secs
    if best is None:
        sys.exit("no valid save found in this file (save in-game first)")
    o0, o1 = best[0][0], best[1][0]
    if checksum(d[o1:o1 + SECTION1_SIZE]) != struct.unpack_from("<H", d, o1 + FOOTER + 2)[0]:
        sys.exit("existing checksum does not match; refusing to edit (unexpected save layout)")
    key = struct.unpack_from("<I", d, o0 + KEY_OFFSET)[0] & 0xFFFF   # quantities are XORed with the low 16 bits

    for name, qty in wanted:
        qty = int(qty)
        if name not in ids or name not in pockets:
            sys.exit(f"unknown item {name}")
        limit = 1 if pockets[name] == "POCKET_KEY_ITEMS" else (999 if pockets[name] == "POCKET_BERRIES" else 99)
        off, cap = POCKETS[pockets[name]]
        slots = [struct.unpack_from("<HH", d, o1 + off + i * 4) for i in range(cap)]
        idx = next((i for i, (it, _) in enumerate(slots) if it == ids[name]), None)
        have = (slots[idx][1] ^ key) if idx is not None else 0
        if idx is None:
            idx = next((i for i, (it, _) in enumerate(slots) if it == 0), None)
            if idx is None:
                sys.exit(f"{pockets[name]} is full")
        total = min(have + qty, limit)
        struct.pack_into("<HH", d, o1 + off + idx * 4, ids[name], total ^ key)
        print(f"{name}: {have} -> {total}")

    if layout is not None:
        old = struct.unpack_from("<H", d, o1 + 0x32)[0]          # SaveBlock1.mapLayoutId
        struct.pack_into("<H", d, o1 + 0x32, layout)
        print(f"map layout id: {old} -> {layout}")
    struct.pack_into("<H", d, o1 + FOOTER + 2, checksum(d[o1:o1 + SECTION1_SIZE]))

    if shoes:
        # SaveBlock1 is split across sections 1-4 in 3968-byte chunks; the flags start in section 2.
        byte = FLAGS_OFFSET + FLAG_SYS_B_DASH // 8
        sec, rel = 1 + byte // SECTION1_SIZE, byte % SECTION1_SIZE
        o = best[sec][0]
        if sec == 4 or checksum(d[o:o + SECTION1_SIZE]) != struct.unpack_from("<H", d, o + FOOTER + 2)[0]:
            sys.exit("unexpected save layout; refusing to set the flag")
        had = bool(d[o + rel] & (1 << (FLAG_SYS_B_DASH % 8)))
        d[o + rel] |= 1 << (FLAG_SYS_B_DASH % 8)
        struct.pack_into("<H", d, o + FOOTER + 2, checksum(d[o:o + SECTION1_SIZE]))
        print("running shoes: " + ("already owned" if had else "added"))
    if not os.path.exists(path + ".bak"):
        shutil.copyfile(path, path + ".bak")
    open(path, "wb").write(d)
    print(f"wrote {path} (backup: {path}.bak)")

if __name__ == "__main__":
    main()
