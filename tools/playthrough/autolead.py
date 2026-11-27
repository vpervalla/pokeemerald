#!/usr/bin/env python3
"""autolead.py : the balance rule, run after every battle (fight.py) unless AUTOLEAD=0. Overworld only.
1. The lowest-level party member fit to fight (HP above ${AUTOLEAD_MINHP:-40}%) goes to slot 0: it leads the next fights.
2. The EXP. SHARE goes to the second-lowest member if that one has the same level as the lowest, otherwise to the lowest.
   It is taken from the BAG (or from the member holding it); an item the new holder carried goes back to the BAG.
Re-checked after every battle, so both follow every level-up. Prints what changed."""
import os, re, struct
from emu import *

ORDERS = ["GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA", "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
          "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG", "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"]
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ITEMS = re.findall(r"^\s+(ITEM_[A-Z0-9_]+),", open(R + "/include/constants/items.h").read(), re.M)
EXP_SHARE = ITEMS.index("ITEM_EXP_SHARE")

e = Emu()
base = S["gPlayerParty"]
n = e.peek(S["gPlayerPartyCount"], 1)[0]


def mon(i):
    return bytearray(e.peek(base + i * 100, 100))


def subs(m):
    pid, otid = struct.unpack("<II", m[:8]); k = pid ^ otid
    raw = bytearray(b"".join(struct.pack("<I", struct.unpack("<I", m[32 + j * 4:36 + j * 4])[0] ^ k) for j in range(12)))
    o = ORDERS[pid % 24]
    return {c: raw[o.index(c) * 12:o.index(c) * 12 + 12] for c in "GAEM"}


def held(m):
    return struct.unpack("<H", bytes(subs(m)["G"][2:4]))[0]


def set_held(i, item):
    m = mon(i); pid, otid = struct.unpack("<II", m[:8]); s = subs(m)
    s["G"][2:4] = struct.pack("<H", item)
    m[0x1C:0x1E] = struct.pack("<H", sum(struct.unpack("<24H", b"".join(bytes(s[c]) for c in "GAEM"))) & 0xffff)
    o = ORDERS[pid % 24]; raw = b"".join(bytes(s[c]) for c in o); k = pid ^ otid
    m[32:80] = b"".join(struct.pack("<I", struct.unpack("<I", raw[j * 4:j * 4 + 4])[0] ^ k) for j in range(12))
    e.send(f"poke {base + i * 100:x}:{bytes(m).hex()}")


def bag_add(item, delta):
    """Add delta (+1/-1) of item to the BAG's ITEMS pocket (SaveBlock1+0x560, 30 slots, quantities XOR the key)."""
    sb1 = e.u32(S["gSaveBlock1Ptr"]); key = e.u32(e.u32(S["gSaveBlock2Ptr"]) + 0xAC) & 0xffff
    slots = [struct.unpack("<HH", e.peek(sb1 + 0x560 + j * 4, 4)) for j in range(30)]
    for j, (it, q) in enumerate(slots):
        if it == item:
            q = (q ^ key) + delta
            e.send(f"poke {sb1 + 0x560 + j * 4:x}:{struct.pack('<HH', item if q > 0 else 0, (q ^ key) if q > 0 else key).hex()}")
            return True
    if delta > 0:
        for j, (it, q) in enumerate(slots):
            if it == 0:
                e.send(f"poke {sb1 + 0x560 + j * 4:x}:{struct.pack('<HH', item, 1 ^ key).hex()}"); return True
    return False


# 1. the lead
fit = []
for i in range(n):
    m = mon(i); hp, maxhp = struct.unpack("<HH", m[86:90])
    if maxhp and hp * 100 > maxhp * int(os.environ.get("AUTOLEAD_MINHP", "40")):
        fit.append((m[84], i))
if fit:
    lvl, best = min(fit)
    lead_fit = any(i == 0 for _, i in fit)
    if best != 0 and (lvl < mon(0)[84] or not lead_fit):     # a lower level, or the lead itself is too weak to fight
        pa, pb = e.peek(base, 100), e.peek(base + best * 100, 100)
        e.send(f"poke {base:x}:{pb.hex()}"); e.send(f"poke {base + best * 100:x}:{pa.hex()}")
        print(f"autolead: slot {best} (L{lvl}) to the front")

# 2. the EXP. SHARE: the second-lowest if it ties with the lowest, else the lowest
ranked = sorted((mon(i)[84], i) for i in range(n) if struct.unpack("<H", mon(i)[86:88])[0] > 0)
if ranked:
    target = ranked[1][1] if len(ranked) > 1 and ranked[1][0] == ranked[0][0] else ranked[0][1]
    holder = next((i for i in range(n) if held(mon(i)) == EXP_SHARE), None)
    if holder != target:
        if holder is not None:
            set_held(holder, 0)
        elif not bag_add(EXP_SHARE, -1):
            target = None                      # no EXP. SHARE anywhere
        if target is not None:
            old = held(mon(target))
            if old:
                bag_add(old, +1)
            set_held(target, EXP_SHARE)
            print(f"autolead: EXP. SHARE to slot {target} (L{mon(target)[84]})" + (f", its {ITEMS[old]} to the BAG" if old else ""))
e.commit()
