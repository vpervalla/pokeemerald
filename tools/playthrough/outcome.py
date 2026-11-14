#!/usr/bin/env python3
"""Print gBattleOutcome (0 still going, 1 won, 2 lost, 3 draw, 4 ran, 5 foe ran, 7 caught, ...)."""
from emu import *
e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
print(e.peek(S["gBattleOutcome"], 1)[0])
e.p.stdin.close()
