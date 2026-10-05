import json, os, struct, shutil
NAME, W, H = "TestArea", 20, 16
GROUND, GRASS = 0x3001, 0x300D   # kanto_general metatiles: plain grass ground, tall grass (passable, elevation 3)
if os.path.exists(f"data/layouts/{NAME}/map.bin"):
    raise SystemExit(f"{NAME} already exists; not overwriting it (it may have been edited in Porymap)")
os.makedirs(f"data/layouts/{NAME}", exist_ok=True); os.makedirs(f"data/maps/{NAME}", exist_ok=True)
cells = [GRASS if 8 <= x < 12 and 6 <= y < 10 else GROUND for y in range(H) for x in range(W)]
open(f"data/layouts/{NAME}/map.bin", "wb").write(struct.pack(f"<{W*H}H", *cells))
shutil.copyfile("data/layouts/PalletTown/border.bin", f"data/layouts/{NAME}/border.bin")
lj = json.load(open("data/layouts/layouts.json"))
if not any(l and l["id"] == "LAYOUT_TEST_AREA" for l in lj["layouts"]):
    lj["layouts"].append({"id": "LAYOUT_TEST_AREA", "name": f"{NAME}_Layout", "width": W, "height": H,
        "primary_tileset": "gTileset_KantoGeneral", "secondary_tileset": "gTileset_KantoPalletTown",
        "border_filepath": f"data/layouts/{NAME}/border.bin", "blockdata_filepath": f"data/layouts/{NAME}/map.bin"})
    open("data/layouts/layouts.json", "w", newline="\n").write(json.dumps(lj, indent=2) + "\n")
m = {"id": "MAP_TEST_AREA", "name": NAME, "layout": "LAYOUT_TEST_AREA", "music": "MUS_RG_ROUTE1",
     "region_map_section": "MAPSEC_TEST_AREA", "requires_flash": False, "weather": "WEATHER_SUNNY",
     "map_type": "MAP_TYPE_ROUTE", "allow_cycling": True, "allow_escaping": False, "allow_running": True,
     "show_map_name": False, "battle_scene": "MAP_BATTLE_SCENE_NORMAL", "connections": None,
     "object_events": [], "warp_events": [], "coord_events": [], "bg_events": []}
open(f"data/maps/{NAME}/map.json", "w", newline="\n").write(json.dumps(m, indent=2) + "\n")
open(f"data/maps/{NAME}/scripts.inc", "w", newline="\n").write(f"{NAME}_MapScripts::\n\t.byte 0\n")
g = json.load(open("data/maps/map_groups.json"))
if "gMapGroup_Custom" not in g:
    g["group_order"].append("gMapGroup_Custom"); g["gMapGroup_Custom"] = []
if NAME not in g["gMapGroup_Custom"]:
    g["gMapGroup_Custom"].append(NAME)
    open("data/maps/map_groups.json", "w", newline="\n").write(json.dumps(g, indent=2) + "\n")
es = open("data/event_scripts.s", newline="").read()
inc = f'\t.include "data/maps/{NAME}/scripts.inc"'
if inc not in es:
    last = [l for l in es.splitlines() if l.startswith('\t.include "data/maps/') and l.endswith('scripts.inc"')][-1]
    open("data/event_scripts.s", "w", newline="").write(es.replace(last, last + "\n" + inc, 1))
print("created", NAME)
