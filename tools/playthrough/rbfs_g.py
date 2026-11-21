#!/usr/bin/env python3
"""rbfs_g.py G N X Y [adj] : like rbfs.py but warps are edges too (Silph Co.'s teleport pads and stairs): BFS over (map, tile)
in the REAL game until tile (X,Y) of map G.N (e.g. 34 51 = SilphCo_5F) is reached (or a neighbour with adj), then replays the
key path from cur.ss and commits. Prints the maps passed through. Stairs need a direction press: every tile also tries all four keys."""
import sys, collections
from emu import *
import rbfs
from rbfs import step, save, mapid, D, TMP

G, N, X, Y = int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
ADJ = len(sys.argv) > 5
LIMIT = int(os.environ.get("RB_LIMIT", "2500"))
os.makedirs(TMP, exist_ok=True)
e = Emu()
root = f"{TMP}/g_root.ss"; save(e, root)
start = (bytes(mapid(e)), e.obj_pos())
seen = {start: ("", root)}
q = collections.deque([start]); found = None; n = 0

def goal(k):
    m, p = k
    if m != bytes([G, N]): return False
    return (abs(p[0] - X) + abs(p[1] - Y) == 1) if ADJ else p == (X, Y)

if goal(start): found = start
while q and not found and n < LIMIT:
    k = q.popleft(); path, f = seen[k]
    for d in "UDLR":
        e.send(f"loadstate {f}"); e.wait(2)
        step(e, d)
        if e.cb2() not in rbfs.OVERWORLD_CB:
            continue                       # a fight or a menu: not part of the graph
        nk = (bytes(mapid(e)), e.obj_pos())
        if nk in seen: continue
        n += 1
        nf = f"{TMP}/g{n}.ss"; save(e, nf)
        seen[nk] = (path + d, nf); q.append(nk)
        if goal(nk): found = nk; break
e.p.stdin.close(); e.p.wait()
if not found:
    print("rbfs_g: no path in", n, "states"); sys.exit(1)
path = seen[found][0]
print("rbfs_g path", len(path), "steps ->", found[0].hex(), found[1])
e = Emu()
for d in path: step(e, d)
e.commit()
shutil.rmtree(TMP, ignore_errors=True)
