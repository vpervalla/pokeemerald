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
ORDERS = ["GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA", "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
          "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG", "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"]


def move_pp():
    """(moves, pp) of the Pokemon in the active slot: the PP shown in the move menu is the party copy."""
    idx = e.peek(S["gBattlerPartyIndexes"], 1)[0]
    m = e.peek(S["gPlayerParty"] + idx * 100, 100)
    pers, otid = int.from_bytes(m[:4], "little"), int.from_bytes(m[4:8], "little")
    k = pers ^ otid
    raw = b"".join((int.from_bytes(m[32 + j * 4:36 + j * 4], "little") ^ k).to_bytes(4, "little") for j in range(12))
    o = ORDERS[pers % 24].index("A") * 12
    return raw[o:o + 8], raw[o + 8:o + 12]


def hp_state():
    """(my active HP, foe active HP): if neither changes for a turn the chosen move did nothing (Disable, no PP left)"""
    mi, ei = e.peek(S["gBattlerPartyIndexes"], 1)[0], e.peek(S["gBattlerPartyIndexes"] + 2, 1)[0]
    mine = int.from_bytes(e.peek(S["gPlayerParty"] + mi * 100 + 86, 2), "little")
    foe = int.from_bytes(e.peek(S["gEnemyParty"] + ei * 100 + 86, 2), "little")
    return mine, foe


stalled = 0
last_hp = None
banned = set()


def usable(want):
    """The wanted move slot if it has PP, else the nearest slot that does (a move at 0 PP just loops forever)."""
    moves, pp = move_pp()
    ok = [i for i in range(4) if pp[i] > 0 and moves[2 * i:2 * i + 2] != b"\0\0" and i not in banned]
    return want if want in ok else (ok[0] if ok else want)


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
            e.wait(10); cursor(ACTION_CUR, 0)                 # the menu remembers the last choice
            e.keys(BIT["A"], 4); e.wait(20)                  # FIGHT
            hs = hp_state()
            stalled = stalled + 1 if hs == last_hp else 0
            last_hp = hs
            if stalled >= 2:          # two turns with no effect on either side: the move is disabled or useless, rotate
                banned.add(usable(slot)); stalled = 0
            cursor(MOVE_CUR, usable(slot))
            e.keys(BIT["A"], 4); e.wait(30)
            turns += 1
        idle = 0
        continue
    if CATCH and e.peek(S["gBattleOutcome"], 1)[0] == B_OUTCOME_CAUGHT:
        log.append("CAUGHT"); break
    if cb2 != CB2_BATTLE and os.environ.get("STOPEVO") and turns > 0:
        log.append("left battle (evolution?)"); break
    # inside the battle (or one of its sub-screens: party, bag) B backs out and advances text; once the battle has
    # an outcome (evolution, level-up scenes) A is needed because B cancels an evolution
    in_battle = cb2 == CB2_BATTLE or e.peek(S["gBattleOutcome"], 1)[0] == 0
    e.keys(BIT["B"] if in_battle else BIT["A"], 4); e.wait(20)
    idle += 1
    if idle > 400:
        log.append("stuck"); break
e.commit()
print(f"turns {turns}:", ", ".join(log))
