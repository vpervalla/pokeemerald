#!/usr/bin/env python3
"""Auto-battle from cur.ss: at each action prompt pick FIGHT and move SLOT (0 TL, 1 TR, 2 BL, 3 BR).
usage: fight.py [SLOT] [MAXTURNS]          (as catch.py: throw the ball under the BAG cursor each turn)
Text is advanced with B inside the battle and with A outside it (B would cancel an evolution).
Stops when back in the overworld, after MAXTURNS moves (leaving the action menu open), when a catch
succeeds, or after 400 idle loops ("stuck": usually a "learn a new move?" prompt, answer it by hand).
Env: SWITCH=auto  switch only when the opponent is too strong (its level is above the active Pokemon's), and then
                  to the weakest healthy party member whose level is above the opponent's. If nobody qualifies,
                  stay and fight. Checked once for each opponent that appears (also each next trainer Pokemon).
     SWITCH=n     force a switch to party slot n (1-5) on the first turn (manual override)
     RESHARE=1    with SWITCH=n, switch again whenever the opponent sends out its next Pokemon (the old swap trick)
     STOPEVO=1  stop as soon as the battle ends (to watch an evolution or a move prompt)"""
import os, sys
from emu import *

slot = int(sys.argv[1]) if len(sys.argv) > 1 else 0
maxturns = int(sys.argv[2]) if len(sys.argv) > 2 else 99
CATCH = sys.argv[0].endswith("catch.py")
AUTO = os.environ.get("SWITCH") == "auto"
SWITCH = 0 if AUTO else int(os.environ.get("SWITCH", "0")); pending = 1 if SWITCH else 0
checked_ei = None
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


def level_hp(base, i):
    m = e.peek(base + i * 100, 100)
    return m[84], int.from_bytes(m[86:88], "little")


def pick_switch():
    """The party slot to switch to, or 0. The opponent is too strong when its level is above the active
    Pokemon's; then the weakest (lowest level) healthy Pokemon that is still above the opponent's level comes in."""
    global checked_ei
    ai, ei = e.peek(S["gBattlerPartyIndexes"], 1)[0], e.peek(S["gBattlerPartyIndexes"] + 2, 1)[0]
    if ei == checked_ei: return 0
    checked_ei = ei
    foe, _ = level_hp(S["gEnemyParty"], ei)
    mine, _ = level_hp(S["gPlayerParty"], ai)
    if foe <= mine: return 0
    best = None
    for i in range(e.peek(S["gPlayerPartyCount"], 1)[0]):
        lv, hp = level_hp(S["gPlayerParty"], i)
        if i != ai and hp > 0 and lv > foe and (best is None or lv < best[0]): best = (lv, i)
    if best: log.append(f"auto-switch to slot {best[1]} (L{best[0]}) against L{foe}")
    return best[1] if best else 0


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
        if AUTO and not pending:
            SWITCH = pick_switch()
            pending = 1 if SWITCH else 0
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
