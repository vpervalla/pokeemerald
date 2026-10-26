#!/usr/bin/env python3
"""Auto-battle from cur.ss: at each action prompt pick FIGHT and move SLOT (0 TL, 1 TR, 2 BL, 3 BR).
usage: fight.py [SLOT] [MAXTURNS]          (as catch.py: throw the ball under the BAG cursor each turn)
Text is advanced with B inside the battle and with A outside it (B would cancel an evolution).
Stops when back in the overworld, after MAXTURNS moves (leaving the action menu open), when a catch
succeeds, or after 400 idle loops ("stuck": usually a "learn a new move?" prompt, answer it by hand).
Env: SWITCH=n   on the first turn switch to party slot n (1-5) - the swap trick for sharing exp
     RESHARE=1  with SWITCH, switch again whenever the opponent sends out its next Pokemon
     STOPEVO=1  stop as soon as the battle ends (to watch an evolution or a move prompt)"""
import os, sys
from emu import *

slot = int(sys.argv[1]) if len(sys.argv) > 1 else 0
maxturns = int(sys.argv[2]) if len(sys.argv) > 2 else 99
CATCH = sys.argv[0].endswith("catch.py")
SWITCH = int(os.environ.get("SWITCH", "0")); pending = 1 if SWITCH else 0
RESHARE = os.environ.get("RESHARE") == "1"
ACTION_CUR, MOVE_CUR = S["gActionSelectionCursor"], S["gMoveSelectionCursor"]
e = Emu()
turns = idle = 0; log = []; last_ei = None


def cursor(addr, want):
    """Move a 2x2 menu cursor (bit 0 = right column, bit 1 = bottom row) to position want."""
    cur = e.peek(addr, 1)[0]
    if (cur & 1) != (want & 1): e.keys(BIT["RIGHT"] if want & 1 else BIT["LEFT"], 4); e.wait(8)
    if (cur & 2) != (want & 2): e.keys(BIT["DOWN"] if want & 2 else BIT["UP"], 4); e.wait(8)


for _ in range(3000):
    cb2 = e.cb2()
    if cb2 == CB2_OVERWORLD:
        log.append("overworld"); break
    if cb2 == CB2_BATTLE and e.u32(S["gBattlerControllerFuncs"]) | 1 == CHOOSE_ACTION:
        if turns >= maxturns:
            log.append("action menu (turn limit)"); break
        if SWITCH and RESHARE:
            ei = e.peek(S["gBattlerPartyIndexes"] + 2, 1)[0]   # the opponent's active party slot
            if last_ei is not None and ei != last_ei:
                pending += 2                                  # new opponent: the weak Pokemon in, then out again
            last_ei = ei
        if pending:
            pending -= 1
            cursor(ACTION_CUR, 2)                             # POKeMON
            e.keys(BIT["A"], 4); e.wait(90)
            e.keys(BIT["RIGHT"], 4); e.wait(10)               # party slot 1
            for _ in range(SWITCH - 1): e.keys(BIT["DOWN"], 4); e.wait(10)
            e.keys(BIT["A"], 4); e.wait(30); e.keys(BIT["A"], 4); e.wait(60)   # SHIFT
        elif CATCH:
            cursor(ACTION_CUR, 1)                             # BAG; it reopens on the last pocket used
            e.keys(BIT["A"], 4); e.wait(90); e.keys(BIT["A"], 4); e.wait(30); e.keys(BIT["A"], 4); e.wait(30)
            turns += 1
        else:
            e.wait(10); e.keys(BIT["A"], 4); e.wait(20)       # FIGHT
            cursor(MOVE_CUR, slot)
            e.keys(BIT["A"], 4); e.wait(30)
            turns += 1
        idle = 0
        continue
    if CATCH and e.peek(S["gBattleOutcome"], 1)[0] == B_OUTCOME_CAUGHT:
        log.append("CAUGHT"); break
    if cb2 != CB2_BATTLE and os.environ.get("STOPEVO") and turns > 0:
        log.append("left battle (evolution?)"); break
    e.keys(BIT["B"] if cb2 == CB2_BATTLE else BIT["A"], 4); e.wait(20)
    idle += 1
    if idle > 400:
        log.append("stuck"); break
e.commit()
print(f"turns {turns}:", ", ".join(log))
