#!/usr/bin/env python3
"""autolead.py : the balance rule. Put the lowest-level party member that is fit to fight (HP above ${AUTOLEAD_MINHP:-40}%)
in slot 0, so it leads the next fights and takes their experience; re-checked after every battle, so the trainee changes as
soon as someone levels up past another member. Overworld only (like lead.py). Prints the change, or nothing."""
import os, struct
from emu import *

e = Emu()
base = S["gPlayerParty"]
n = e.peek(S["gPlayerPartyCount"], 1)[0]
mons = []
for i in range(n):
    m = e.peek(base + i * 100, 100)
    lvl = m[84]; hp, maxhp = struct.unpack("<HH", m[86:90])
    if maxhp and hp * 100 > maxhp * int(os.environ.get("AUTOLEAD_MINHP", "40")):
        mons.append((lvl, i))
if mons:
    lvl, best = min(mons)
    cur = e.peek(base + 84, 1)[0]
    if best != 0 and lvl < cur:
        pa, pb = e.peek(base, 100), e.peek(base + best * 100, 100)
        e.send(f"poke {base:x}:{pb.hex()}"); e.send(f"poke {base + best * 100:x}:{pa.hex()}")
        e.commit()
        print(f"autolead: slot {best} (L{lvl}) to the front")
