#!/usr/bin/env python3
"""rbfs.py : breadth-first search over the REAL game (savestate cloning) for places the static planner gets wrong (spinner mazes).
usage:  rbfs.py go X Y [adj]   walk from cur.ss to tile (X,Y) (or next to it) by exploring real moves, then replay them and commit
        rbfs.py map           print the reachable tiles (does not change cur.ss)
Player tile = the ObjectEvent's coordinates (the save-block position lags after spinner slides)."""
import json, os, sys, shutil, collections
from emu import *

D = {"U": BIT["UP"], "D": BIT["DOWN"], "L": BIT["LEFT"], "R": BIT["RIGHT"]}
TMP = "/dev/shm/rbfs"; os.makedirs(TMP, exist_ok=True)


def step(e, d):
    """one real move: hold the key until the tile changes (or 24 frames), then wait for the slide to finish. Returns state tag."""
    p0 = e.obj_pos(); m0 = e.u32(S["gSaveBlock1Ptr"] ) and e.peek(e.u32(S["gSaveBlock1Ptr"]) + 4, 2)
    for _ in range(6):
        e.keys(D[d], 4)
        if e.obj_pos() != p0 or e.cb2() not in OVERWORLD_CB: break
    last = None
    for _ in range(60):
        e.wait(8)
        cur = (e.obj_pos(), e.cb2() in OVERWORLD_CB)
        if cur == last: break
        last = cur
    e.wait(4)
    e.fix_pos(); e.wait(2)    # spinner slides leave the camera position behind (ROM bug): re-sync it


OVERWORLD_CB = {CB2_OVERWORLD, CB2_OVERWORLD - 1}


def mapid(e):
    return bytes(e.peek(e.u32(S["gSaveBlock1Ptr"]) + 4, 2))


def save(e, f):
    e.send(f"savestate {f}"); e.send("sync"); e.p.stdout.readline()


def explore(targets, adj, limit=400):
    e = Emu()
    root = f"{TMP}/root.ss"; save(e, root)
    m0 = mapid(e); p0 = e.obj_pos()
    seen = {p0: ("", root)}
    q = collections.deque([p0])
    found = None
    events = []               # moves that start a battle (a trainer's line of sight, wild grass): a fight is a valid way to arrive
    def goal(p):
        return any((abs(p[0] - tx) + abs(p[1] - ty) == 1) if adj else p == (tx, ty) for tx, ty in targets)
    if goal(p0): found = p0
    n = 0
    while q and not found and n < limit:
        p = q.popleft(); path, f = seen[p]
        for d in "UDLR":
            e.send(f"loadstate {f}"); e.wait(2)
            step(e, d)
            if mapid(e) == m0 and e.cb2() not in OVERWORLD_CB:
                events.append(path + d)
            if mapid(e) != m0 or e.cb2() not in OVERWORLD_CB:
                continue          # a warp or a fight: not part of the walkable graph
            np_ = e.obj_pos()
            if np_ in seen: continue
            n += 1
            nf = f"{TMP}/n{n}.ss"; save(e, nf)
            seen[np_] = (path + d, nf); q.append(np_)
            if goal(np_): found = np_; break
    e.send("quit") if False else None
    e.p.stdin.close(); e.p.wait()
    seen["events"] = events
    return seen, found


if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "map":
        seen, _ = explore([(-99, -99)], False)
        print(sorted(k for k in seen if k != "events"))
    else:
        x, y, adj = int(sys.argv[2]), int(sys.argv[3]), len(sys.argv) > 4
        seen, found = explore([(x, y)], adj)
        if not found:
            ev = seen.get("events") or []
            if not ev:
                print("rbfs: no real path"); sys.exit(1)
            path = min(ev, key=len)
            print("rbfs path", path, "-> BATTLE")
            e = Emu()
            for d in path[:-1]: step(e, d)
            e.keys(D[path[-1]], 20)
            e.commit(); sys.exit(0)
        path = seen[found][0]
        print("rbfs path", path, "->", found)
        e = Emu()
        for d in path: step(e, d)
        e.commit()
    shutil.rmtree(TMP, ignore_errors=True)
