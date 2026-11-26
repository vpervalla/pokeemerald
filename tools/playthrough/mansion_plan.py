#!/usr/bin/env python3
"""mansion_plan.py START_FLOOR X Y STATE GOAL_FLOOR GX GY [adj]
Static route planner for the Pokémon Mansion (floors 1F 2F 3F B1F), whose doors depend on the switch flag
(FLAG_POKEMON_MANSION_SWITCH_STATE, STATE 0 = clear, 1 = set). The map data is the flag-clear layout; the
Kanto_PokemonMansion_EventScript_PressSwitch_xF scripts (data/scripts/kanto_npcs.inc) give the flag-set tiles and their
collision. Edges: walking (collision bits, objects block), warps (stairs, holes; the arrival tile is the destination
warp, left by any step), and pressing a statue (stand below it, face up) which flips the state.
Prints the route as legs: 'walk FLOOR X Y' (the last tile of a leg), 'warp -> FLOOR X Y', 'press statue at FLOOR X Y'."""
import json, os, re, sys
from collections import deque
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
from render import tileset_dir
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
FLOORS = ["1F", "2F", "3F", "B1F"]
BEH = [m.group(1) for m in re.finditer(r"^\s*(MB_\w+)", open(R + "/include/constants/metatile_behaviors.h").read(), re.M)]
src = open(R + "/data/scripts/kanto_npcs.inc").read()


def press_tiles(fl):
    m = re.search(r"Kanto_PokemonMansion_EventScript_PressSwitch_%s::(.*?)return" % fl, src, re.S)
    return [(int(x), int(y), int(c)) for x, y, c in re.findall(r"setmetatile (\d+), (\d+), \w+, (\d)", m.group(1))]


maps = {}
for fl in FLOORS:
    name = "PokemonMansion_" + fl
    mj = json.load(open(f"{R}/data/maps/{name}/map.json"))
    L = {l["id"]: l for l in json.load(open(R + "/data/layouts/layouts.json"))["layouts"] if l}[mj["layout"]]
    w, h = L["width"], L["height"]
    b = np.frombuffer(open(R + "/" + L["blockdata_filepath"], "rb").read(), dtype="<u2").reshape(h, w)
    base = ((b >> 10) & 3) != 0
    NM = 640 if "Kanto" in L["primary_tileset"] else 512
    pa, sa = [np.frombuffer(open(tileset_dir(ts) + "/metatile_attributes.bin", "rb").read(), dtype="<u2") & 0xFF
              for ts in (L["primary_tileset"], L["secondary_tileset"])]
    def behn(x, y, b=b, pa=pa, sa=sa, NM=NM):
        m = int(b[y, x]) & 0x3FF
        return BEH[int(pa[m]) if m < NM else (int(sa[m - NM]) if m - NM < len(sa) else 0)]
    s1 = base.copy()
    for x, y, c in press_tiles(fl):
        s1[y, x] = bool(c)
    objs = {(o["x"], o["y"]) for o in mj["object_events"]}
    statues = {(bg["x"], bg["y"]) for bg in mj["bg_events"] if "Statue" in bg.get("script", "")}
    warps = []
    for wp in mj["warp_events"]:
        dm = wp["dest_map"]
        n = behn(wp["x"], wp["y"])
        # only a tile whose behavior warps (stairs, a hole, a door, a ladder) can be used: a hole's landing spot is a warp
        # event on an MB_CAVE tile, so it is arrived at but never taken back up
        usable = "ARROW_WARP" in n or "HOLE" in n or "STAIRS" in n or "LADDER" in n or "DOOR" in n   # MB_CAVE = a landing spot only
        arrow = next((d for k, d in (("EAST", "R"), ("WEST", "L"), ("NORTH", "U"), ("SOUTH", "D")) if k + "_ARROW" in n), "")
        warps.append(((wp["x"], wp["y"]), dm, int(wp["dest_warp_id"]), usable, arrow))
    maps[fl] = dict(name=name, w=w, h=h, grid=[base, s1], objs=objs, statues=statues, warps=warps)


def warp_dest(dm, wid):
    for fl in FLOORS:
        if dm == "MAP_POKEMON_MANSION_" + fl:
            (x, y) = maps[fl]["warps"][wid][0]
            return fl, x, y
    return None   # out of the Mansion


def plan(start, goal_fl, gx, gy, adj):
    goals = {(gx + dx, gy + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))} if adj else {(gx, gy)}
    prev = {start: None}
    q = deque([start])
    while q:
        c = q.popleft()
        fl, x, y, s = c
        if fl == goal_fl and (x, y) in goals:
            out = []
            while c is not None:
                out.append(c); c = prev[c] and prev[c][0]
            # rebuild with actions
            seq = []; c = (fl, x, y, s)
            while prev[c] is not None:
                p, act = prev[c]; seq.append((act, c)); c = p
            return list(reversed(seq))
        M = maps[fl]
        nxt = []
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if not (0 <= nx < M["w"] and 0 <= ny < M["h"]):
                continue
            if M["grid"][s][ny, nx] or ((nx, ny) in M["objs"] and not (fl == goal_fl and (nx, ny) == (gx, gy))):
                continue
            wp = next((wp for wp in M["warps"] if wp[0] == (nx, ny)), None)
            if wp and wp[3]:
                d = warp_dest(wp[1], wp[2])
                if d:
                    nxt.append(((d[0], d[1], d[2], s), ("warp", (nx, ny), wp[4])))
                continue
            nxt.append(((fl, nx, ny, s), ("walk", None)))
        if (x, y - 1) in M["statues"]:
            nxt.append(((fl, x, y, 1 - s), ("press", (x, y - 1))))
        for n, act in nxt:
            if n not in prev:
                prev[n] = (c, act); q.append(n)
    return None


if __name__ == "__main__":
    a = sys.argv
    start = (a[1], int(a[2]), int(a[3]), int(a[4]))
    seq = plan(start, a[5], int(a[6]), int(a[7]), len(a) > 8)
    if seq is None:
        print("no route"); sys.exit(1)
    last = start
    for act, c in seq:
        if act[0] == "walk":
            last = c; continue
        print(f"walk {last[0]} {last[1]} {last[2]}")
        if act[0] == "warp":
            print(f"warp at {act[1]} -> {c[0]} {c[1]} {c[2]} arrow {act[2] or '-'}")
        else:
            print(f"press statue at {c[0]} {act[1][0]} {act[1][1]} (state -> {c[3]})")
        last = c
    print(f"walk {last[0]} {last[1]} {last[2]}  (goal, state {last[3]})")
