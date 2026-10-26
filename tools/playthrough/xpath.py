#!/usr/bin/env python3
"""xpath.py MAP X Y GOALMAP[:X,Y] : find the sequence of maps/warps leading from (MAP,X,Y) to GOALMAP, walking inside maps with
path.py's rules (collision, ledges, walls, water, objects, Smash rocks) and taking warps. Prints each leg as
`goto X Y` followed by the warp that is taken."""
import io, json, os, sys, contextlib
from collections import deque
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.dirname(__file__))
import numpy as np

def load(name):
    src = open(os.path.join(os.path.dirname(__file__), "path.py")).read().split("goal_tiles")[0]
    g = {"__file__": os.path.join(os.path.dirname(__file__), "path.py")}
    sys.argv = ["x", name, "0", "0", "0", "0"]
    exec(src, g)
    return g

def reach(g, start):
    blocked, w, h = g["blocked"], g["w"], g["h"]
    seen = {start}; q = deque([start])
    while q:
        c = q.popleft()
        for k, (dx, dy) in {"U": (0, -1), "D": (0, 1), "L": (-1, 0), "R": (1, 0)}.items():
            n = (c[0] + dx, c[1] + dy)
            if not (0 <= n[0] < w and 0 <= n[1] < h) or n in seen: continue
            e = {"U": "N", "D": "S", "L": "W", "R": "E"}[k]; o = {"N": "S", "S": "N", "W": "E", "E": "W"}[e]
            if g["walled"](*c, e) or g["walled"](*n, o): continue
            if g["beh"](*n) == g["JUMP"][k]:
                n = (n[0] + dx, n[1] + dy)
                if 0 <= n[0] < w and 0 <= n[1] < h and n not in seen and not blocked[n[1], n[0]]:
                    seen.add(n); q.append(n)
                continue
            if blocked[n[1], n[0]]: continue
            seen.add(n); q.append(n)
    return seen

def warps(name):
    return json.load(open(f"{R}/data/maps/{name}/map.json"))["warp_events"]

def dest_xy(w):
    d = w["dest_map"][4:]
    names = {m.upper().replace("_", ""): m for m in os.listdir(f"{R}/data/maps")}
    n = names[d.replace("_", "")]
    i = int(w["dest_warp_id"])
    ws = warps(n)
    return n, (ws[i]["x"], ws[i]["y"])

if __name__ == "__main__":
    start_map, sx, sy, goal = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4]
    goal_pos = tuple(map(int, goal.split(":")[1].split(","))) if ":" in goal else None
    goal = goal.split(":")[0]
    gcache = {}
    prev = {}
    q = deque([(start_map, (sx, sy))]); seen = {(start_map, (sx, sy))}
    while q:
        m, pos = q.popleft()
        if m not in gcache: gcache[m] = load(m)
        g = gcache[m]
        rc = reach(g, pos)
        for w in warps(m):
            wp = (w["x"], w["y"])
            # the warp tile itself is blocked in the planner; it is reachable if a neighbour is
            if not any(((wp[0] + dx, wp[1] + dy) in rc) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))) and wp not in rc:
                continue
            try:
                dm, dp = dest_xy(w)
            except KeyError:
                continue
            key = (dm, dp)
            if key in seen: continue
            seen.add(key); prev[key] = ((m, pos), wp, w); q.append(key)
            if dm == goal and (goal_pos is None or dp == goal_pos):
                chain = []
                cur = key
                while cur in prev:
                    p, wp2, w2 = prev[cur]
                    chain.append((p[0], p[1], wp2, cur[0], cur[1]))
                    cur = p
                for a in reversed(chain):
                    print(f"{a[0]}: from {a[1]} goto {a[2]} -> warp to {a[3]} {a[4]}")
                sys.exit(0)
    print("no route")
