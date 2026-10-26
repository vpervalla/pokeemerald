#!/usr/bin/env python3
"""BFS over a map's collision bits. usage: path.py MAPNAME X0 Y0 X1 Y1 [--adj]
Prints walk tokens (e.g. U3 R5 D2). --adj: stop on a tile next to (X1,Y1) (for talking/items).
Object events (people, item balls) block. Warps (doors, ladders, stairs) block unless they are the goal."""
import json, os, sys
from collections import deque
import numpy as np
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
a = sys.argv
name, x0, y0, x1, y1 = a[1], *map(int, a[2:6])
adj = "--adj" in a
mj = json.load(open(f"{R}/data/maps/{name}/map.json"))
L = {l["id"]: l for l in json.load(open(R + "/data/layouts/layouts.json"))["layouts"] if l}[mj["layout"]]
w, h = L["width"], L["height"]
b = np.frombuffer(open(R + "/" + L["blockdata_filepath"], "rb").read(), dtype="<u2").reshape(h, w)
blocked = ((b >> 10) & 3) != 0
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
    if out and out[-1][0] == s: out[-1][1] += 1
    else: out.append([s, 1])
print(" ".join(f"{s}{n}" for s, n in out), f"# end {end}")
