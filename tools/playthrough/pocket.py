#!/usr/bin/env python3
"""Print the bag's open pocket index (0 ITEMS, 1 POKe BALLS, 2 TMs & HMs, 3 BERRIES, 4 KEY ITEMS), read from gBagPosition."""
from emu import *
e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
print(e.peek(S["gBagPosition"] + 5, 1)[0])
e.p.stdin.close()
