#!/usr/bin/env python3
"""Add items to the bag (or Pokemon to the party) of a Pokemon Emerald save file.

    python3 tools/edit_save.py pokeemerald.sav ITEM_POKE_BALL=10 ITEM_POTION=10
    python3 tools/edit_save.py pokeemerald.sav --add-mon=SPECIES_HAUNTER:25   # species[:level], level defaults to 5
    python3 tools/edit_save.py pokeemerald.sav --running-shoes
    python3 tools/edit_save.py pokeemerald.sav --layout-id=442   # see include/constants/layouts.h

Run from the pokeemerald root (item ids, pockets and Pokemon data are read from the source).
--add-mon adds a Pokemon caught by the player in a Poke Ball (random personality and IVs, no EVs,
its latest level-up moves) to the first free party slot; it can be repeated.
--layout-id repairs a save whose current map looks garbled after layouts were added or removed
(the save stores the numeric id of the layout the player is standing on).
The original file is copied to <file>.bak the first time. Close the emulator first.
"""
import random, re, shutil, struct, sys, os

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

PARTY_COUNT_OFFSET, PARTY_OFFSET, MON_SIZE, PARTY_SIZE = 0x234, 0x238, 100, 6   # SaveBlock1, all in section 1
# Position of the growth/attacks/EVs/misc substructs in BoxPokemon.secure, by personality % 24 (GetSubstruct).
SUBSTRUCT_ORDER = [(0,1,2,3),(0,1,3,2),(0,2,1,3),(0,3,1,2),(0,2,3,1),(0,3,2,1),(1,0,2,3),(1,0,3,2),
                   (2,0,1,3),(3,0,1,2),(2,0,3,1),(3,0,2,1),(1,2,0,3),(1,3,0,2),(2,1,0,3),(3,1,0,2),
                   (2,3,0,1),(3,2,0,1),(1,2,3,0),(1,3,2,0),(2,1,3,0),(3,1,2,0),(2,3,1,0),(3,2,1,0)]
EXP = {  # experience needed for level n, by growth rate (src/data/pokemon/experience_tables.h)
    "GROWTH_MEDIUM_FAST": lambda n: n ** 3,
    "GROWTH_ERRATIC": lambda n: (n ** 3 * (100 - n)) // 50 if n <= 50 else (n ** 3 * (150 - n)) // 100 if n <= 68
        else (n ** 3 * ((1911 - 10 * n) // 3)) // 500 if n <= 98 else (n ** 3 * (160 - n)) // 100,
    "GROWTH_FLUCTUATING": lambda n: (n ** 3 * (24 + (n + 1) // 3)) // 50 if n <= 15 else (n ** 3 * (14 + n)) // 50 if n <= 36
        else (n ** 3 * (32 + n // 2)) // 50,
    "GROWTH_MEDIUM_SLOW": lambda n: (6 * n ** 3) // 5 - 15 * n ** 2 + 100 * n - 140 if n > 1 else 0,
    "GROWTH_FAST": lambda n: (4 * n ** 3) // 5,
    "GROWTH_SLOW": lambda n: (5 * n ** 3) // 4,
}

def defines(path, prefix):
    return {m.group(1): int(m.group(2), 0) for m in re.finditer(rf"#define ({prefix}\w+)\s+(\w+)", open(path).read())
            if re.fullmatch(r"(0x[0-9A-Fa-f]+|\d+)", m.group(2))}

def encode(text, length):
    chars = {m.group(1): int(m.group(2), 16) for m in re.finditer(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$", open("charmap.txt", encoding="utf-8").read(), re.M)}
    return (bytes(chars[c] for c in text) + b"\xFF" * length)[:length]

def make_mon(species_name, level, ot_name, ot_id, ot_gender, ball):
    """Build a party Pokemon (struct Pokemon, 100 bytes) like CreateMon + GiveMonInitialMoveset."""
    species = defines("include/constants/species.h", "SPECIES_").get(species_name)
    if not species:
        sys.exit(f"unknown species {species_name}")
    key = species_name[len("SPECIES_"):]
    info = re.search(rf"\[{species_name}\] =\s*\{{(.*?)\n    \}}", open("src/data/pokemon/species_info.h").read(), re.S).group(1)
    base = [int(re.search(rf"\.base{s}\s*=\s*(\d+)", info).group(1)) for s in ("HP", "Attack", "Defense", "Speed", "SpAttack", "SpDefense")]
    growth = re.search(r"\.growthRate = (\w+)", info).group(1)
    abilities = re.search(r"\.abilities = \{\s*(\w+),\s*(\w+)\s*\}", info).groups()
    nickname = re.search(rf"\[{species_name}\] = _\(\"(.*?)\"\)", open("src/data/text/species_names.h").read()).group(1)
    moves_c = defines("include/constants/moves.h", "MOVE_")
    learnset_name = re.search(rf"\[{species_name}\] = (\w+)", open("src/data/pokemon/level_up_learnset_pointers.h").read()).group(1)
    learnset = re.search(rf"{learnset_name}\[\] = \{{(.*?)\}};", open("src/data/pokemon/level_up_learnsets.h").read(), re.S).group(1)
    moves = []  # GiveBoxMonInitialMoveset: learn in order, skipping known moves, the oldest is forgotten when full
    for lvl, move in re.findall(r"LEVEL_UP_MOVE\(\s*(\d+),\s*(\w+)\)", learnset):
        if int(lvl) <= level and moves_c[move] not in moves:
            moves = (moves + [moves_c[move]])[-4:]
    battle_moves = open("src/data/battle_moves.h").read()
    pp = [int(re.search(rf"\[{next(n for n, v in moves_c.items() if v == m)}\] =\s*\{{.*?\.pp = (\d+)", battle_moves, re.S).group(1)) for m in moves]
    moves += [0] * (4 - len(moves)); pp += [0] * (4 - len(pp))

    personality = random.getrandbits(32)
    ivs = [random.randrange(32) for _ in range(6)]
    ability_num = personality & 1 if abilities[1] != "ABILITY_NONE" else 0
    nature = personality % 25
    stats = [(2 * base[0] + ivs[0]) * level // 100 + level + 10]
    for i in range(1, 6):  # natures raise stat nature // 5 and lower stat nature % 5 (Attack, Defense, Speed, SpAtk, SpDef)
        v = (2 * base[i] + ivs[i]) * level // 100 + 5
        v = v * 110 // 100 if nature // 5 == i - 1 != nature % 5 else v * 90 // 100 if nature % 5 == i - 1 != nature // 5 else v
        stats.append(v)
    hp_max, atk, dfn, spe, spa, spd = stats
    mapsecs = re.findall(r"^\s*(MAPSEC_\w+)", open("include/constants/region_map_sections.h").read(), re.M)
    met_location = mapsecs.index("MAPSEC_PALLET_TOWN")
    iv_word = sum(iv << (5 * i) for i, iv in enumerate(ivs)) | (ability_num << 31)

    subs = [
        struct.pack("<HHIBBH", species, 0, EXP[growth](level), 0, 70, 0),                    # growth (STANDARD_FRIENDSHIP)
        struct.pack("<4H4B", *moves, *pp),                                                    # attacks
        bytes(12),                                                                            # EVs and condition
        struct.pack("<BBHII", 0, met_location, level | (3 << 7) | (ball << 11) | (ot_gender << 15), iv_word, 0),  # misc (VERSION_EMERALD)
    ]
    secure = bytearray(48)
    for kind, pos in enumerate(SUBSTRUCT_ORDER[personality % 24]):
        secure[pos * 12:pos * 12 + 12] = subs[kind]
    checksum16 = sum(struct.unpack("<24H", secure)) & 0xFFFF
    words = [w ^ personality ^ ot_id for w in struct.unpack("<12I", secure)]
    box = struct.pack("<II", personality, ot_id) + encode(nickname, 10) + bytes([2, 0x02]) + ot_name[:7] \
        + struct.pack("<BHH", 0, checksum16, 0) + struct.pack("<12I", *words)   # language English, hasSpecies
    mon = box + struct.pack("<IBBHHHHHHH", 0, level, 0xFF, hp_max, hp_max, atk, dfn, spe, spa, spd)
    assert len(box) == 80 and len(mon) == MON_SIZE
    return mon, nickname

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
    new_mons = [a.split("=", 1)[1] for a in args if a.startswith("--add-mon=")]
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
    for spec in new_mons:
        species_name, _, level = spec.partition(":")
        level = int(level or 5)
        if not 1 <= level <= 100:
            sys.exit(f"bad level {level}")
        count = d[o1 + PARTY_COUNT_OFFSET]
        if count >= PARTY_SIZE:
            sys.exit("the party is full")
        sb2 = d[o0:o0 + 0x10]   # SaveBlock2: playerName, playerGender, playerTrainerId
        mon, nickname = make_mon(species_name, level, bytes(sb2[0:7]), struct.unpack_from("<I", sb2, 0x0A)[0], sb2[8] & 1, ids["ITEM_POKE_BALL"])
        d[o1 + PARTY_OFFSET + count * MON_SIZE:o1 + PARTY_OFFSET + (count + 1) * MON_SIZE] = mon
        d[o1 + PARTY_COUNT_OFFSET] = count + 1
        print(f"party slot {count + 1}: {nickname} Lv. {level}")
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
