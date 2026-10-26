"""Shared settings and RAM/ROM addresses for the playthrough tools.

Every address is read from the built ELF (arm-none-eabi-nm), so the tools keep working after the
code changes. Rebuild the ROM first (`make modern`); the table is cached in $PLAY_DIR/syms.json and
refreshed whenever the ELF is newer than the cache.
"""
import json, os, subprocess

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ROM = os.environ.get("PLAY_ROM", os.path.join(REPO, "pokeemerald_modern.gba"))
ELF = ROM[:-4] + ".elf"
PLAY_DIR = os.environ.get("PLAY_DIR", os.path.expanduser("~/.playthrough"))
DRIVER = os.path.join(PLAY_DIR, "mgba_driver")
CUR = os.path.join(PLAY_DIR, "cur.ss")        # the current savestate: every tool loads and replaces it
SAV = os.path.join(PLAY_DIR, "game.sav")      # the cartridge flash (the in-game SAVE)
HIST = os.path.join(PLAY_DIR, "hist")         # one savestate per tool call, for undo and recovering screenshots
SHOTS = os.path.join(PLAY_DIR, "shots")       # named screenshots (240x160 PNG)

GLOBALS = ["gMain", "gSaveBlock1Ptr", "gSaveBlock2Ptr", "gPlayerPartyCount", "gPlayerParty", "gEnemyParty",
           "gBattlerControllerFuncs", "gActionSelectionCursor", "gMoveSelectionCursor", "gBattleOutcome",
           "gBattlerPartyIndexes", "BattleMainCB2", "CB2_Overworld"]


def _load():
    cache = os.path.join(PLAY_DIR, "syms.json")
    if os.path.exists(cache) and os.path.getmtime(cache) > os.path.getmtime(ELF):
        return json.load(open(cache))
    out = subprocess.run(["arm-none-eabi-nm", "-n", ELF], capture_output=True, text=True, check=True).stdout
    rows = [(int(a, 16), name) for a, _, name in (l.split(None, 2) for l in out.splitlines() if len(l.split()) == 3)]
    s = {}
    for addr, name in rows:
        if name in GLOBALS:
            s[name] = addr
    # HandleInputChooseAction/Move are static and exist in several battle controllers. The player's pair is
    # the one where ChooseMove follows ChooseAction closely in the same object file.
    acts = [a for a, n in rows if n == "HandleInputChooseAction"]
    moves = [a for a, n in rows if n == "HandleInputChooseMove"]
    for a in acts:
        near = [m for m in moves if 0 < m - a < 0x2000]
        if near:
            s["HandleInputChooseAction"], s["HandleInputChooseMove"] = a, near[0]
            break
    missing = [n for n in GLOBALS + ["HandleInputChooseAction"] if n not in s]
    if missing:
        raise SystemExit(f"symbols not found in {ELF}: {missing}")
    os.makedirs(PLAY_DIR, exist_ok=True)
    json.dump(s, open(cache, "w"))
    return s


S = _load()
GMAIN_CB2 = S["gMain"] + 4                     # gMain.callback2
CB2_OVERWORLD = S["CB2_Overworld"] | 1         # thumb bit set, as stored in callback2
CB2_BATTLE = S["BattleMainCB2"] | 1
CHOOSE_ACTION = S["HandleInputChooseAction"] | 1
B_OUTCOME_CAUGHT = 7


def le_hex(v):
    """The 4-byte little-endian hex string the driver's `peek ADDR 4` prints for value v."""
    return v.to_bytes(4, "little").hex()
