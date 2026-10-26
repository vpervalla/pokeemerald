#!/usr/bin/env python3
"""Step the game from cur.ss and save the result back (the previous state goes to hist/). Writes last.png.
Tokens (space separated, run in order):
  A B ST SE L R      press a button (then wait 10 frames)
  U3 D2 L1 R4        walk N tiles. Each tile waits for the player's position to change, so turns and
                     bumps don't drift. Stops with "STOPPED" if the game leaves the overworld (a battle,
                     a script, a menu), so the rest of the tokens are dropped.
  hU5                hold a direction for 5 frames without walking logic (turn in place, ledges, menus)
  w60                wait 60 frames
  A*5                repeat a token
  shot:NAME          save the screen to shots/NAME.png
  peek:ADDR:N        print N bytes at hex ADDR
  undo               (alone) restore the previous state from hist/
"""
import glob, os, shutil, sys
from PIL import Image
from emu import *

KEYS = {"A": "A", "B": "B", "ST": "START", "SE": "SELECT", "L": "L", "R": "R"}
DIRS = {"U": "UP", "D": "DOWN", "L": "LEFT", "R": "RIGHT"}
OVERWORLD = {CB2_OVERWORLD, CB2_OVERWORLD - 1}

args = " ".join(sys.argv[1:]).split()
if args == ["undo"]:
    hs = sorted(glob.glob(os.path.join(HIST, "*.ss")))
    if hs:
        shutil.copy(hs[-1], CUR); os.remove(hs[-1]); print("restored", hs[-1])
    args = ["w1"]


def walk(e, k, n):
    for _ in range(n):
        if e.cb2() not in OVERWORLD:
            print("STOPPED: not in overworld (battle/menu/script)"); return False
        p0 = e.pos()
        for _ in range(12):
            e.keys(k, 4)
            if e.pos() != p0:
                e.keys(k, 10); break
    e.wait(12)
    return True


def run(e, tok, shots):
    if tok.startswith("shot:"):
        p = os.path.join(PLAY_DIR, f"tmp_{len(shots)}.ppm")
        e.shot(p); shots.append((p, os.path.join(SHOTS, tok[5:] + ".png")))
    elif tok in KEYS:
        e.keys(BIT[KEYS[tok]], 6); e.wait(16)
    elif tok[0] == "w" and tok[1:].isdigit():
        e.wait(int(tok[1:]))
    elif tok[0] == "h" and tok[1:2] in DIRS:
        e.keys(BIT[DIRS[tok[1]]], int(tok[2:]))
    elif tok[0] in DIRS and tok[1:].isdigit():
        return walk(e, BIT[DIRS[tok[0]]], int(tok[1:]))
    elif tok.startswith("peek:"):
        _, a, n = tok.split(":"); print(e.peek(int(a, 16), int(n)).hex())
    else:
        sys.exit(f"bad token {tok}")
    return True


e = Emu()
shots = []
for tok in args:
    tok, rep = (tok.split("*")[0], int(tok.split("*")[1])) if "*" in tok else (tok, 1)
    if not all(run(e, tok, shots) for _ in range(rep)):
        break
e.commit()
for p, out in shots:
    if os.path.exists(p):
        Image.open(p).save(out); os.remove(p)
print("ok")
