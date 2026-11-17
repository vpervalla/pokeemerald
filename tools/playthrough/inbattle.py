#!/usr/bin/env python3
"""Print 'battle' or 'field' from gMain.callback2 (the stale ENEMY line of `st` stays after a battle)."""
import os
from emu import *
e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
print("field" if e.cb2() == CB2_OVERWORLD else "battle")
e.commit(keep=False)
