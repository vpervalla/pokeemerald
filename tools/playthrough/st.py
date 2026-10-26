#!/usr/bin/env python3
"""Print the map, position, money and party in cur.ss (and the wild/trainer Pokemon when in a battle).
Read-only: the state is not saved back."""
import json, re, signal, struct
from emu import *
signal.signal(signal.SIGPIPE, signal.SIG_DFL)


def table(path, pfx, hdr):
    consts = {m.group(1): int(m.group(2)) for m in re.finditer(r"#define\s+(" + pfx + r"\w+)\s+(\d+)\b", open(hdr).read())}
    return {consts[m.group(1)]: m.group(2) for m in re.finditer(r"\[(" + pfx + r"\w+)\]\s*=\s*_\(\"([^\"]*)\"\)", open(path).read())
            if m.group(1) in consts}


SPECIES = table(REPO + "/src/data/text/species_names.h", "SPECIES_", REPO + "/include/constants/species.h")
MOVES = table(REPO + "/src/data/text/move_names.h", "MOVE_", REPO + "/include/constants/moves.h")
GROUPS = json.load(open(REPO + "/data/maps/map_groups.json"))
CHARS = {}
for line in open(REPO + "/charmap.txt", encoding="utf-8"):
    m = re.match(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$", line)
    if m:
        CHARS.setdefault(int(m.group(2), 16), m.group(1))
ORDERS = ["GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA", "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
          "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG", "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"]
NATURES = ["Hardy", "Lonely", "Brave", "Adamant", "Naughty", "Bold", "Docile", "Relaxed", "Impish", "Lax", "Timid", "Hasty",
           "Serious", "Jolly", "Naive", "Modest", "Mild", "Quiet", "Bashful", "Rash", "Calm", "Gentle", "Sassy", "Careful", "Quirky"]


def txt(b):
    return "".join(CHARS.get(c, "?") for c in b.split(b"\xff")[0])


def mon(m):
    """Decode a 100-byte party Pokemon: the 48-byte substructs are XORed with personality^otId and shuffled."""
    pers, otid = struct.unpack("<II", m[:8])
    k = pers ^ otid
    raw = b"".join(struct.pack("<I", struct.unpack("<I", m[32 + j * 4:36 + j * 4])[0] ^ k) for j in range(12))
    order = ORDERS[pers % 24]
    sub = {c: raw[order.index(c) * 12:(order.index(c) + 1) * 12] for c in "GAEM"}
    species, item, exp = struct.unpack("<HHI", sub["G"][:8])
    moves, pp = struct.unpack("<4H", sub["A"][:8]), sub["A"][8:12]
    status, lvl, _, hp, mhp, atk, df, spe, spa, spd = struct.unpack("<IBBHHHHHHH", m[80:100])
    iv = struct.unpack("<I", sub["M"][4:8])[0]
    mv = ", ".join(f"{MOVES.get(x, x)} {pp[j]}" for j, x in enumerate(moves) if x)
    return (f"{txt(m[8:18])} ({SPECIES.get(species, species)}) L{lvl} HP {hp}/{mhp} exp {exp} {NATURES[pers % 25]} "
            f"ability{(iv >> 31) & 1} stats {atk}/{df}/{spa}/{spd}/{spe} status {status} item {item} | {mv}")


e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))   # a throwaway flash, so reading never touches game.sav
sb1, sb2 = e.u32(S["gSaveBlock1Ptr"]), e.u32(S["gSaveBlock2Ptr"])
hdr = e.peek(sb1, 8)
x, y = struct.unpack("<hh", hdr[:4]); g, n = hdr[4], hdr[5]
try:
    mname = GROUPS[GROUPS["group_order"][g]][n]
except (IndexError, KeyError):
    mname = f"{g}.{n}"
money = e.u32(sb1 + 0x490) ^ e.u32(sb2 + 0xAC)   # money is XORed with SaveBlock2's encryptionKey
print(f"map {mname} ({g}.{n}) pos ({x},{y})  money {money}")
cnt = e.peek(S["gPlayerPartyCount"], 1)[0]
for i in range(cnt):
    print("  " + mon(e.peek(S["gPlayerParty"] + i * 100, 100)))
if e.cb2() == CB2_BATTLE:
    print("  ENEMY: " + mon(e.peek(S["gEnemyParty"], 100)))
e.commit(keep=False)
