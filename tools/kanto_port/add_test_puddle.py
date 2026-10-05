#!/usr/bin/env python3
"""Add a walkable puddle to TestArea, for checking the splash/ripple animations when running through water.
Run from the pokeemerald root, after make_test_area.py. Stamps over plain ground on the left of the map."""
import json, struct
NAME = "TestArea"
X, Y, PW, PH = 2, 10, 6, 3   # top-left and size of the puddle, in metatiles
# kanto_general's 3x3 puddle block (MB_PUDDLE, as used on Four Island); the middle row/column repeats.
ROWS = ((0x1A0, 0x1A1, 0x1A2), (0x1A8, 0x1A9, 0x1AA), (0x1B0, 0x1B1, 0x1B2))
ELEVATION = 3 << 12          # same elevation as the surrounding ground, no collision

W = next(l["width"] for l in json.load(open("data/layouts/layouts.json"))["layouts"] if l and l["id"] == "LAYOUT_TEST_AREA")
p = f"data/layouts/{NAME}/map.bin"
dst = bytearray(open(p, "rb").read())
for dy in range(PH):
    row = ROWS[0 if dy == 0 else 2 if dy == PH - 1 else 1]
    for dx in range(PW):
        struct.pack_into("<H", dst, ((Y + dy) * W + X + dx) * 2, ELEVATION | row[0 if dx == 0 else 2 if dx == PW - 1 else 1])
open(p, "wb").write(dst)
print("added puddle to", NAME)
