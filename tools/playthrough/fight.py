#!/usr/bin/env python3
"""Auto-battle from cur.ss: at each action prompt pick FIGHT and move SLOT (0 TL, 1 TR, 2 BL, 3 BR).
usage: fight.py [SLOT] [MAXTURNS]          (as catch.py: throw the ball under the BAG cursor each turn)
Text is advanced with B inside the battle and with A outside it (B would cancel an evolution).
Stops when back in the overworld, after MAXTURNS moves (leaving the action menu open), when a catch
succeeds, or after 400 idle loops ("stuck": usually a "learn a new move?" prompt, answer it by hand).
Env: SWITCH=auto  switch only when the opponent is more than 2 levels above the active Pokemon, or the active one is about
                  to faint (30% HP or less); then to the weakest healthy party member whose level is at or above the opponent's.
     SWITCH=n     force a switch to party slot n (1-5) on the first turn (manual override)
     RESHARE=1    with SWITCH=n, switch again whenever the opponent sends out its next Pokemon (the old swap trick)
     TYPES=1      choose the move by type: use a super effective move if the active POKeMON has one (the best one);
                  in a TRAINER battle, switch out when one of the opponent's moves is super effective against the lead (to a
                  member the opponent's moves don't threaten that has a super effective move, best of all one that resists it)
                  and when the lead has no super effective move (to one that has); max two switches for each opponent
                  POKeMON; otherwise the damaging move with the best power x effectiveness x STAB.
                  `trainer` turns it on by default (TYPES=0 to turn it off). Needs the type data from src/.
     STOPEVO=1  stop as soon as the battle ends (to watch an evolution or a move prompt)"""
import os, sys
from emu import *

slot = int(sys.argv[1]) if len(sys.argv) > 1 else 0
maxturns = int(sys.argv[2]) if len(sys.argv) > 2 else 99
CATCH = sys.argv[0].endswith("catch.py")
AUTO = os.environ.get("SWITCH") == "auto"
SWITCH = 0 if AUTO else int(os.environ.get("SWITCH", "0")); pending = 1 if SWITCH else 0
checked_pair = None
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
    return m[84], int.from_bytes(m[86:88], "little"), int.from_bytes(m[88:90], "little")   # level, HP, max HP


def pick_switch():
    """The party slot to switch to, or 0. A switch happens only when
      1) the opponent is more than 2 levels above the active Pokemon (checked once for each pairing), or
      2) the active Pokemon is about to faint (HP at or under 30% of its maximum, checked at every turn).
    Then the weakest healthy Pokemon whose level is still at or above the opponent's comes in; if none is, the highest-level
    healthy one does (case 2) or nobody (case 1)."""
    global checked_pair
    ai, ei = e.peek(S["gBattlerPartyIndexes"], 1)[0], e.peek(S["gBattlerPartyIndexes"] + 2, 1)[0]
    foe, _, _ = level_hp(S["gEnemyParty"], ei)
    mine, hp, mhp = level_hp(S["gPlayerParty"], ai)
    dying = hp * 100 <= 30 * mhp
    gap = foe - mine > 2 and (ai, ei) != checked_pair
    if gap: checked_pair = (ai, ei)
    if not (gap or dying): return 0
    cands = []
    for i in range(e.peek(S["gPlayerPartyCount"], 1)[0]):
        lv, h, _ = level_hp(S["gPlayerParty"], i)
        if i != ai and h > 0: cands.append((lv, i))
    above = [c for c in cands if c[0] >= foe]
    pick = min(above) if above else (max(cands) if cands and dying else None)
    if pick: log.append(f"auto-switch to slot {pick[1]} (L{pick[0]}) against L{foe}" + (" (about to faint)" if dying else ""))
    return pick[1] if pick else 0


# ---------- type-aware choices (TYPES=1) ----------
TYPES = os.environ.get("TYPES", "0") == "1"
_data = {}
MOVE_FALLBACK_POWER = ("LOW_KICK", "LEVEL_DAMAGE", "FIXED_DAMAGE", "SUPER_FANG", "PSYWAVE", "RETURN", "FRUSTRATION", "ENDEAVOR")


def load_data():
    """Species types, move type/power/effect and the type chart, read from the repository's source."""
    import re
    if _data: return _data
    R = REPO
    rd = lambda f: open(os.path.join(R, f), encoding="utf-8", errors="replace").read()
    types = {m.group(1): int(m.group(2)) for m in re.finditer(r"#define TYPE_(\w+)\s+(\d+)\b", rd("include/constants/pokemon.h"))}
    sp_id = {m.group(1): int(m.group(2)) for m in re.finditer(r"#define SPECIES_(\w+)\s+(\d+)\b", rd("include/constants/species.h"))}
    mv_id = {m.group(1): int(m.group(2)) for m in re.finditer(r"#define MOVE_(\w+)\s+(\d+)\b", rd("include/constants/moves.h"))}
    sp_types = {}
    txt = rd("src/data/pokemon/species_info.h")
    txt = txt[txt.index("[SPECIES_NONE]"):].split("\n", 1)[1]     # "[SPECIES_NONE] = {0}," has no closing brace line
    for m in re.finditer(r"\[SPECIES_(\w+)\]\s*=\s*\{(.*?)\n    \},", txt, re.S):
        t = re.search(r"\.types\s*=\s*\{\s*TYPE_(\w+),\s*TYPE_(\w+)", m.group(2))
        if t and m.group(1) in sp_id: sp_types[sp_id[m.group(1)]] = (types[t.group(1)], types[t.group(2)])
    mv = {}
    txt = rd("src/data/battle_moves.h")
    for m in re.finditer(r"\[MOVE_(\w+)\]\s*=\s*\{(.*?)\n    \},", txt, re.S):
        b = m.group(2)
        eff = re.search(r"\.effect\s*=\s*EFFECT_(\w+)", b); pw = re.search(r"\.power\s*=\s*(\d+)", b); ty = re.search(r"\.type\s*=\s*TYPE_(\w+)", b)
        if m.group(1) in mv_id and pw and ty:
            power = int(pw.group(1))
            if power == 0 and eff and any(k in eff.group(1) for k in MOVE_FALLBACK_POWER): power = 40
            mv[mv_id[m.group(1)]] = (m.group(1), types[ty.group(1)], power)
    chart = {}
    mul = {"NO_EFFECT": 0, "NOT_EFFECTIVE": 5, "NORMAL": 10, "SUPER_EFFECTIVE": 20}
    body = rd("src/battle_main.c")
    body = body[body.index("gTypeEffectiveness[366]"):]
    for line in body.split("\n")[2:]:
        if "TYPE_FORESIGHT" in line or "};" in line: break
        t = re.findall(r"TYPE_(\w+)", line)
        if len(t) == 3 and t[0] in types and t[1] in types: chart[(types[t[0]], types[t[1]])] = mul[t[2][4:]] if t[2].startswith("MUL_") else 10
    _data.update(types=types, sp_types=sp_types, mv=mv, chart=chart, tname={v: k for k, v in types.items()})
    return _data


def decode(base, i):
    """(species, [move ids], [pp], level, hp, max hp) of a party member."""
    m = e.peek(base + i * 100, 100)
    pers, otid = int.from_bytes(m[:4], "little"), int.from_bytes(m[4:8], "little")
    k = pers ^ otid
    raw = b"".join((int.from_bytes(m[32 + j * 4:36 + j * 4], "little") ^ k).to_bytes(4, "little") for j in range(12))
    order = ORDERS[pers % 24]
    G, A = raw[order.index("G") * 12:][:12], raw[order.index("A") * 12:][:12]
    moves = [int.from_bytes(A[2 * j:2 * j + 2], "little") for j in range(4)]
    return (int.from_bytes(G[:2], "little"), moves, list(A[8:12]), m[84], int.from_bytes(m[86:88], "little"), int.from_bytes(m[88:90], "little"))


def effectiveness(move_type, def_types):
    d = load_data(); f = 1.0
    for t in set(def_types):
        f *= d["chart"].get((move_type, t), 10) / 10
    return f


def foe_asleep():
    """Whether the opponent's first battler is asleep (gBattleMons[1].status1 & 7)."""
    try:
        return bool(int.from_bytes(e.peek(S["gBattleMons"] + 0x58 + 0x4C, 4), "little") & 7)
    except Exception:
        return False


def best_move(mon, foe_types, banned_slots=()):
    """(score, slot, effectiveness, name) of the member's best damaging move against foe_types, or None."""
    d = load_data(); sp, moves, pp, *_ = mon
    own = d["sp_types"].get(sp, ())
    best = None
    for slot, mid in enumerate(moves):
        if not mid or pp[slot] <= 0 or slot in banned_slots or mid not in d["mv"]: continue
        name, ty, power = d["mv"][mid]
        if power <= 0: continue
        if name == "DREAM_EATER" and not foe_asleep(): continue   # fails unless the target sleeps
        eff = effectiveness(ty, foe_types)
        score = power * eff * (1.5 if ty in own else 1.0)
        if best is None or score > best[0]: best = (score, slot, eff, name)
    return best


switched_for = {}


def threat(foe_mon, my_types):
    """The strongest effectiveness any damaging move of the foe has against a POKeMON of my_types (1.0 if it has none)."""
    d = load_data(); worst = None
    for mid, pp in zip(foe_mon[1], foe_mon[2]):
        if not mid or pp <= 0 or mid not in d["mv"]: continue
        name, ty, power = d["mv"][mid]
        if power <= 0: continue
        eff = effectiveness(ty, my_types)
        worst = eff if worst is None else max(worst, eff)
    return 1.0 if worst is None else worst


def type_plan():
    """('move', slot, best, foe types) or ('switch', party index, best, foe types) or None, by the type rules:
      1. the foe has a move that is super effective against the active POKeMON (trainer battles): switch in a healthy member that
         the foe's moves do not threaten and that has a super effective move; best of all one that resists the foe;
         failing that one that resists it; failing that stay;
      2. otherwise a super effective move if the active one has it (the best one);
      3. a trainer battle and the active one has none: switch in a member that has one (and prefers resisting the foe);
      4. else the damaging move with the best power x effectiveness x STAB.
    At most two switches for each opponent POKeMON, so it can't ping-pong."""
    d = load_data()
    ai, ei = e.peek(S["gBattlerPartyIndexes"], 1)[0], e.peek(S["gBattlerPartyIndexes"] + 2, 1)[0]
    foe = decode(S["gEnemyParty"], ei); ft = d["sp_types"].get(foe[0])
    if not ft: return None
    me = decode(S["gPlayerParty"], ai)
    mine = best_move(me, ft, banned)
    trainer = bool(e.u32(S["gBattleTypeFlags"]) & 8)
    my_t = d["sp_types"].get(me[0], ())
    threatened = threat(foe, my_t) >= 2
    can_switch = trainer and switched_for.get(ei, 0) < 2
    if can_switch and (threatened or not (mine and mine[2] >= 2)):
        cands = []
        for i in range(e.peek(S["gPlayerPartyCount"], 1)[0]):
            if i == ai: continue
            m = decode(S["gPlayerParty"], i)
            if m[4] <= 0 or m[4] * 100 < 30 * m[5]: continue            # fainted or nearly
            mt = d["sp_types"].get(m[0], ())
            dmg = threat(foe, mt)                                       # what the foe's moves do to it
            resist = max(effectiveness(t, mt) for t in set(ft))         # what the foe's own types (its STAB) do to it
            bm = best_move(m, ft)
            offense = bm[2] >= 2 if bm else False
            if threatened:
                if dmg >= 2: continue                                   # no better off
                if not offense and dmg > 0.5: continue                  # nothing to gain
            elif not offense:
                continue
            frac = m[4] / m[5]
            cands.append((offense, -resist, -dmg, (bm[0] if bm else 0) * frac, i, bm, dmg))
        if cands:
            _, _, _, _, i, bm, dmg = max(cands, key=lambda c: c[:4])
            switched_for[ei] = switched_for.get(ei, 0) + 1
            return ("switch", i, bm if bm else (0, 0, 1.0, "-"), ft, dmg, threatened)
    if mine: return ("move", mine[1], mine, ft)
    return None


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


def go_party(cur, target):
    """Move the party menu's cursor from slot cur to slot target (slot 0 is the big box on the left, 1-5 stack on the right)."""
    if cur == target: return
    if target == 0:
        e.keys(BIT["LEFT"], 4); e.wait(10); return
    if cur == 0:
        e.keys(BIT["RIGHT"], 4); e.wait(10); cur = 1
    for _ in range(abs(target - cur)): e.keys(BIT["DOWN"] if target > cur else BIT["UP"], 4); e.wait(10)


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
        if TYPES and not pending:
            plan = type_plan()
            if plan and plan[0] == "switch":
                tn = d_tn = load_data()["tname"]
                log.append(f"type-switch to slot {plan[1]}: {plan[2][3]} (x{plan[2][2]:g}), the foe's moves do x{plan[4]:g} to it" + (" [the foe threatened the lead]" if plan[5] else "") + f" vs {'/'.join(tn[t] for t in set(plan[3]))}")
                SWITCH = plan[1]; pending = 1
            elif plan:
                slot = plan[1]
                if plan[2][2] >= 2 or plan[2][2] < 1: log.append(f"type-move {plan[2][3]} (x{plan[2][2]:g})")
        if AUTO and not pending:
            SWITCH = pick_switch()
            pending = 1 if SWITCH else 0
        if pending:
            pending -= 1
            cursor(ACTION_CUR, 2)                             # POKeMON
            e.keys(BIT["A"], 4); e.wait(90)
            go_party(e.peek(S["gBattlerPartyIndexes"], 1)[0], SWITCH)
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
