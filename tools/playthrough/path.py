#!/usr/bin/env python3
"""BFS over a map's collision bits. usage: path.py MAPNAME X0 Y0 X1 Y1 [--adj]
Prints walk tokens (e.g. U3 R5 D2). --adj: stop on a tile next to (X1,Y1) (for talking/items).
Object events (people, item balls) block. Warps (doors, ladders, stairs) block unless they are the goal.
Surfable water is a wall. One-way ledges (metatile behavior MB_JUMP_*) are jumped in their own direction and never crossed against it."""
import json, os, re, sys
from collections import deque
import numpy as np
from render import tileset_dir
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
a = sys.argv
name, x0, y0, x1, y1 = a[1], *map(int, a[2:6])
adj = "--adj" in a
mj = json.load(open(f"{R}/data/maps/{name}/map.json"))
L = {l["id"]: l for l in json.load(open(R + "/data/layouts/layouts.json"))["layouts"] if l}[mj["layout"]]
w, h = L["width"], L["height"]
b = np.frombuffer(open(R + "/" + L["blockdata_filepath"], "rb").read(), dtype="<u2").reshape(h, w)
blocked = ((b >> 10) & 3) != 0
# metatile behaviors: one-way ledges (MB_JUMP_*) can be jumped in their own direction, landing 2 tiles on
beh_names = [m.group(1) for m in re.finditer(r"^\s*(MB_\w+)", open(R + "/include/constants/metatile_behaviors.h").read(), re.M)]
JUMP = {"U": beh_names.index("MB_JUMP_NORTH"), "D": beh_names.index("MB_JUMP_SOUTH"),
        "L": beh_names.index("MB_JUMP_WEST"), "R": beh_names.index("MB_JUMP_EAST")}
NM = 640 if "Kanto" in L["primary_tileset"] else 512
def attrs(ts):
    return np.frombuffer(open(tileset_dir(ts) + "/metatile_attributes.bin", "rb").read(), dtype="<u2") & 0xFF
pa, sa = attrs(L["primary_tileset"]), attrs(L["secondary_tileset"])
def beh(x, y):
    m = int(b[y, x]) & 0x3FF
    return int(pa[m]) if m < NM else (int(sa[m - NM]) if m - NM < len(sa) else 0)
# directional walls (MB_IMPASSABLE_<edge>): a tile whose edge is walled can't be left or entered across it
EDGES = {"N": ("NORTH", "NORTHEAST", "NORTHWEST"), "S": ("SOUTH", "SOUTHEAST", "SOUTHWEST"),
         "E": ("EAST", "NORTHEAST", "SOUTHEAST"), "W": ("WEST", "NORTHWEST", "SOUTHWEST")}
def walled(x, y, edge):
    return beh_names[beh(x, y)] in ["MB_IMPASSABLE_" + e for e in EDGES[edge]]
# surfable water has collision 0 but can't be walked without Surf (shallow and semi-deep water are wadeable)
for y in range(h):
    for x in range(w):
        n = beh_names[beh(x, y)]
        if "WATER" in n and not any(k in n for k in ("SHALLOW", "SEMI_DEEP", "UNDERWATER")) or "WATERFALL" in n:
            blocked[y, x] = True
for o in mj.get("object_events", []):
    if (o["x"], o["y"]) != (x1, y1):
        blocked[o["y"], o["x"]] = True
goal_tiles = {(x1, y1)}
for wp in mj.get("warp_events", []):
    blocked[wp["y"], wp["x"]] = (wp["x"], wp["y"]) not in goal_tiles
goal = {(x1, y1)} if not adj else {(x1 + dx, y1 + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))}
prev = {(x0, y0): None}
q = deque([(x0, y0)])
D = {"U": (0, -1), "D": (0, 1), "L": (-1, 0), "R": (1, 0)}
end = None
while q:
    c = q.popleft()
    if c in goal:
        end = c; break
    for k, (dx, dy) in D.items():
        n = (c[0] + dx, c[1] + dy)
        if 0 <= n[0] < w and 0 <= n[1] < h and beh(*n) == JUMP[k]:      # a ledge: jump over it
            n = (n[0] + dx, n[1] + dy)
            if 0 <= n[0] < w and 0 <= n[1] < h and n not in prev and not blocked[n[1], n[0]]:
                prev[n] = (c, k + "J"); q.append(n)
            continue
        e = {"U": "N", "D": "S", "L": "W", "R": "E"}[k]; o = {"N": "S", "S": "N", "W": "E", "E": "W"}[e]
        if walled(*c, e) or (0 <= n[0] < w and 0 <= n[1] < h and walled(*n, o)):
            continue
        if 0 <= n[0] < w and 0 <= n[1] < h and n not in prev and (not blocked[n[1], n[0]] or n in goal):
            prev[n] = (c, k); q.append(n)
if end is None:
    sys.exit("no path")
steps = []
c = end
while prev[c]:
    c, k = prev[c][0], prev[c][1]
    steps.append(k)
steps.reverse()
out = []
for s in steps:
    n = 2 if s.endswith("J") else 1      # a jump covers two tiles
    s = s[0]
    if out and out[-1][0] == s: out[-1][1] += n
    else: out.append([s, n])
print(" ".join(f"{s}{n}" for s, n in out), f"# end {end}")
