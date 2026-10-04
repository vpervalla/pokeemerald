#!/usr/bin/env python3
"""Connect TestArea to the west side of Pallet Town. Run from the pokeemerald root."""
import json, struct
W = 24                      # Pallet Town width
GAP_ROWS = (10, 11)         # one tree's worth of rows on the west edge
GROUND = 0x3000 | 662       # Pallet Town's plain ground, passable, elevation 3
OFFSET = 2                  # TestArea row 0 lines up with Pallet Town row 2

p = "data/layouts/PalletTown/map.bin"
d = bytearray(open(p, "rb").read())
for y in GAP_ROWS:
    for x in (0, 1, 2):     # two tree columns plus the ground-edge column
        struct.pack_into("<H", d, (y * W + x) * 2, GROUND)
open(p, "wb").write(d)

def add(path, conn):
    m = json.load(open(path))
    conns = [c for c in (m["connections"] or []) if c["map"] != conn["map"]] + [conn]
    m["connections"] = conns
    open(path, "w", newline="\n").write(json.dumps(m, indent=2) + "\n")
add("data/maps/PalletTown/map.json", {"map": "MAP_TEST_AREA", "offset": OFFSET, "direction": "left"})
add("data/maps/TestArea/map.json", {"map": "MAP_PALLET_TOWN", "offset": -OFFSET, "direction": "right"})
print("connected PalletTown <-> TestArea")
