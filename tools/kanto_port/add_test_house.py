#!/usr/bin/env python3
"""Add an enterable house with a sign to TestArea. Run from the pokeemerald root, after make_test_area.py.
The exterior is Pallet Town's player's house, the interior is a copy of the rival's house layout."""
import json, os, shutil, struct
NAME, HOUSE = "TestArea", "TestArea_House"
SRC_W = 24                       # Pallet Town width
SRC_X, SRC_Y, HW, HH = 5, 3, 5, 5  # player's house footprint in Pallet Town (door at column 1 of the bottom row)
SIGN_SRC = (5, 14)               # Pallet Town's wooden trainer-tips sign
DST_X, DST_Y = 13, 8             # top-left of the house in TestArea
SIGN = (DST_X - 2, DST_Y + HH - 1)  # on the grass, left of the yard
DOOR = (DST_X + 1, DST_Y + HH - 1)
# The roof's top row has Pallet's path baked in, so the house sits on a path yard with Pallet's edge tiles.
YARD = (DST_X - 1, DST_Y - 1, DST_X + HW, DST_Y + HH + 1)  # x0, y0, x1, y1 (inclusive border)
PATH = 0x3296
EDGE = {"TL": 0x32A6, "T": 0x328E, "TR": 0x328F, "L": 0x32AE, "R": 0x3297, "BL": 0x32B6, "B": 0x329E, "BR": 0x329F}

if os.path.exists(f"data/maps/{HOUSE}/map.json"):
    raise SystemExit(f"{HOUSE} already exists; not overwriting it (it may have been edited in Porymap)")

# Exterior: stamp the house and sign into TestArea's blockdata.
src = open("data/layouts/PalletTown/map.bin", "rb").read()
p = f"data/layouts/{NAME}/map.bin"
dst = bytearray(open(p, "rb").read())
W = next(l["width"] for l in json.load(open("data/layouts/layouts.json"))["layouts"] if l and l["id"] == "LAYOUT_TEST_AREA")
def put(x, y, v): struct.pack_into("<H", dst, (y * W + x) * 2, v)
def get(x, y): return struct.unpack_from("<H", src, (y * SRC_W + x) * 2)[0]
x0, y0, x1, y1 = YARD
for y in range(y0, y1 + 1):
    for x in range(x0, x1 + 1):
        v = ("T" if y == y0 else "B" if y == y1 else "") + ("L" if x == x0 else "R" if x == x1 else "")
        put(x, y, EDGE[v] if v else PATH)
for dy in range(HH):
    for dx in range(HW):
        put(DST_X + dx, DST_Y + dy, get(SRC_X + dx, SRC_Y + dy))
put(*SIGN, get(*SIGN_SRC))
open(p, "wb").write(dst)

def dump(path, obj):
    open(path, "w", newline="\n").write(json.dumps(obj, indent=2) + "\n")

m = json.load(open(f"data/maps/{NAME}/map.json"))
m["warp_events"] = [{"x": DOOR[0], "y": DOOR[1], "elevation": 0, "dest_map": "MAP_TEST_AREA_HOUSE", "dest_warp_id": "0"}]
m["bg_events"] = [{"type": "sign", "x": SIGN[0], "y": SIGN[1], "elevation": 0,
                   "player_facing_dir": "BG_EVENT_PLAYER_FACING_ANY", "script": f"{NAME}_EventScript_HouseSign"}]
dump(f"data/maps/{NAME}/map.json", m)
open(f"data/maps/{NAME}/scripts.inc", "w", newline="\n").write(
    f"{NAME}_MapScripts::\n\t.byte 0\n\n"
    f"{NAME}_EventScript_HouseSign::\n\tmsgbox {NAME}_Text_HouseSign, MSGBOX_SIGN\n\tend\n\n"
    f"{NAME}_Text_HouseSign:\n\t.string \"TEST HOUSE$\"\n")

# Interior: a copy of the rival's house, with its exit mat warping back to the TestArea door.
os.makedirs(f"data/layouts/{HOUSE}", exist_ok=True); os.makedirs(f"data/maps/{HOUSE}", exist_ok=True)
for f in ("map.bin", "border.bin"):
    shutil.copyfile(f"data/layouts/PalletTown_RivalsHouse/{f}", f"data/layouts/{HOUSE}/{f}")
lj = json.load(open("data/layouts/layouts.json"))
if not any(l and l["id"] == "LAYOUT_TEST_AREA_HOUSE" for l in lj["layouts"]):
    base = next(l for l in lj["layouts"] if l and l["id"] == "LAYOUT_PALLET_TOWN_RIVALS_HOUSE")
    lj["layouts"].append(dict(base, id="LAYOUT_TEST_AREA_HOUSE", name=f"{HOUSE}_Layout",
        border_filepath=f"data/layouts/{HOUSE}/border.bin", blockdata_filepath=f"data/layouts/{HOUSE}/map.bin"))
    dump("data/layouts/layouts.json", lj)
h = json.load(open("data/maps/PalletTown_RivalsHouse/map.json"))
h.update(id="MAP_TEST_AREA_HOUSE", name=HOUSE, layout="LAYOUT_TEST_AREA_HOUSE", music="MUS_RG_ROUTE1",
         object_events=[], coord_events=[], bg_events=[])
for w in h["warp_events"]:
    w.update(dest_map="MAP_TEST_AREA", dest_warp_id="0")
dump(f"data/maps/{HOUSE}/map.json", h)
open(f"data/maps/{HOUSE}/scripts.inc", "w", newline="\n").write(f"{HOUSE}_MapScripts::\n\t.byte 0\n")

g = json.load(open("data/maps/map_groups.json"))
if HOUSE not in g["gMapGroup_Custom"]:
    g["gMapGroup_Custom"].append(HOUSE)
    dump("data/maps/map_groups.json", g)
es = open("data/event_scripts.s", newline="").read()
anchor, inc = f'\t.include "data/maps/{NAME}/scripts.inc"', f'\t.include "data/maps/{HOUSE}/scripts.inc"'
if inc not in es:
    open("data/event_scripts.s", "w", newline="").write(es.replace(anchor, anchor + "\n" + inc, 1))
print("added", HOUSE)
