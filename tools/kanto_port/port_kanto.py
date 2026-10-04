#!/usr/bin/env python3
"""Port Kanto data from pret/pokefirered into this pokeemerald tree.

    python3 tools/kanto_port/port_kanto.py tilesets --frlg ../pokefirered
    python3 tools/kanto_port/port_kanto.py map PalletTown --frlg ../pokefirered --music MUS_RG_PALLET

Run from the pokeemerald root. Both commands are idempotent.

tilesets: copies every FRLG tileset to data/tilesets/<kind>/kanto_<name>, converts the
          32-bit FRLG metatile attributes to Emerald's 16-bit format (remapping behaviours),
          and generates src/data/tilesets/kanto_*.h plus tileset_rules_kanto.mk.
          All ported symbols get a "Kanto" prefix (gTileset_KantoPalletTown) so they
          cannot collide with the Hoenn tilesets.
map:      ports one layout + map. Events, warps and connections are stripped unless the
          matching --keep-* flag is given, because their scripts/targets may not exist yet.
"""
import argparse, json, os, re, shutil, struct, sys
from collections import Counter, defaultdict

ROOT = os.getcwd()
PREFIX_SYM = "Kanto"
PREFIX_DIR = "kanto_"
KANTO_GROUP = "gMapGroup_Kanto"

# FRLG behaviour name -> Emerald behaviour name. None = no Emerald equivalent (becomes MB_NORMAL,
# listed in the report so the behaviour can be re-implemented later).
BEHAVIOR_MAP = {
    "MB_NORMAL": "MB_NORMAL", "MB_UNUSED_01": "MB_NORMAL", "MB_TALL_GRASS": "MB_TALL_GRASS",
    "MB_CAVE": "MB_CAVE", "MB_RUNNING_DISALLOWED": "MB_NO_RUNNING",
    "MB_INDOOR_ENCOUNTER": "MB_INDOOR_ENCOUNTER", "MB_MOUNTAIN_TOP": "MB_MOUNTAIN_TOP",
    "MB_POND_WATER": "MB_POND_WATER", "MB_FAST_WATER": "MB_INTERIOR_DEEP_WATER",
    "MB_DEEP_WATER": "MB_DEEP_WATER", "MB_WATERFALL": "MB_WATERFALL",
    "MB_OCEAN_WATER": "MB_OCEAN_WATER", "MB_PUDDLE": "MB_PUDDLE",
    "MB_SHALLOW_WATER": "MB_SHALLOW_WATER", "MB_UNDERWATER_BLOCKED_ABOVE": "MB_NO_SURFACING",
    "MB_UNUSED_WATER": "MB_OCEAN_WATER", "MB_CYCLING_ROAD_WATER": "MB_OCEAN_WATER",
    "MB_STRENGTH_BUTTON": None, "MB_SAND": "MB_SAND", "MB_SEAWEED": "MB_SEAWEED",
    "MB_ICE": "MB_ICE", "MB_THIN_ICE": "MB_THIN_ICE", "MB_CRACKED_ICE": "MB_CRACKED_ICE",
    "MB_HOT_SPRINGS": "MB_HOT_SPRINGS", "MB_ROCK_STAIRS": None, "MB_SAND_CAVE": "MB_SAND",
    "MB_SPIN_RIGHT": None, "MB_SPIN_LEFT": None, "MB_SPIN_UP": None, "MB_SPIN_DOWN": None,
    "MB_STOP_SPINNING": None,
    "MB_CAVE_DOOR": "MB_NON_ANIMATED_DOOR", "MB_LADDER": "MB_LADDER",
    "MB_FALL_WARP": "MB_CRACKED_FLOOR_HOLE", "MB_REGULAR_WARP": "MB_AQUA_HIDEOUT_WARP",
    "MB_LAVARIDGE_1F_WARP": "MB_LAVARIDGE_GYM_1F_WARP", "MB_WARP_DOOR": "MB_ANIMATED_DOOR",
    "MB_UP_ESCALATOR": "MB_UP_ESCALATOR", "MB_DOWN_ESCALATOR": "MB_DOWN_ESCALATOR",
    # Diagonal stair warps do not exist in Emerald; nearest is a sideways arrow warp.
    "MB_UP_RIGHT_STAIR_WARP": "MB_EAST_ARROW_WARP", "MB_DOWN_RIGHT_STAIR_WARP": "MB_EAST_ARROW_WARP",
    "MB_UP_LEFT_STAIR_WARP": "MB_WEST_ARROW_WARP", "MB_DOWN_LEFT_STAIR_WARP": "MB_WEST_ARROW_WARP",
    "MB_UNION_ROOM_WARP": None,
    "MB_COUNTER": "MB_COUNTER", "MB_BOOKSHELF": "MB_BOOKSHELF", "MB_POKEMART_SHELF": "MB_SHOP_SHELF",
    "MB_PC": "MB_PC", "MB_REGION_MAP": "MB_REGION_MAP", "MB_TELEVISION": "MB_TELEVISION",
    "MB_CABLE_CLUB_WIRELESS_MONITOR": "MB_WIRELESS_BOX_RESULTS",
    "MB_BATTLE_RECORDS": "MB_CABLE_BOX_RESULTS_1", "MB_QUESTIONNAIRE": "MB_QUESTIONNAIRE",
    "MB_BLUEPRINTS": "MB_BLUEPRINT", "MB_TRASH_BIN": "MB_TRASH_CAN",
    "MB_CYCLING_ROAD_PULL_DOWN": None, "MB_CYCLING_ROAD_PULL_DOWN_GRASS": "MB_TALL_GRASS",
}
SAME_NAME = ["MB_IMPASSABLE_EAST", "MB_IMPASSABLE_WEST", "MB_IMPASSABLE_NORTH", "MB_IMPASSABLE_SOUTH",
    "MB_IMPASSABLE_NORTHEAST", "MB_IMPASSABLE_NORTHWEST", "MB_IMPASSABLE_SOUTHEAST", "MB_IMPASSABLE_SOUTHWEST",
    "MB_JUMP_EAST", "MB_JUMP_WEST", "MB_JUMP_NORTH", "MB_JUMP_SOUTH",
    "MB_WALK_EAST", "MB_WALK_WEST", "MB_WALK_NORTH", "MB_WALK_SOUTH",
    "MB_SLIDE_EAST", "MB_SLIDE_WEST", "MB_SLIDE_NORTH", "MB_SLIDE_SOUTH", "MB_TRICK_HOUSE_PUZZLE_8_FLOOR",
    "MB_EASTWARD_CURRENT", "MB_WESTWARD_CURRENT", "MB_NORTHWARD_CURRENT", "MB_SOUTHWARD_CURRENT",
    "MB_EAST_ARROW_WARP", "MB_WEST_ARROW_WARP", "MB_NORTH_ARROW_WARP", "MB_SOUTH_ARROW_WARP"]
BEHAVIOR_MAP.update({n: n for n in SAME_NAME})
# Emerald behaviours that can trigger wild encounters (TILE_FLAG_HAS_ENCOUNTERS).
EM_ENCOUNTER = {"MB_TALL_GRASS", "MB_LONG_GRASS", "MB_DEEP_SAND", "MB_CAVE", "MB_INDOOR_ENCOUNTER",
    "MB_POND_WATER", "MB_INTERIOR_DEEP_WATER", "MB_DEEP_WATER", "MB_OCEAN_WATER", "MB_SEAWEED",
    "MB_ASHGRASS", "MB_FOOTPRINTS", "MB_SEAWEED_NO_SURFACING"}


def read(p):
    with open(p, encoding="utf-8") as f:
        return f.read()

def write(p, s):
    if os.path.dirname(p):
        os.makedirs(os.path.dirname(p), exist_ok=True)
    with open(p, "w", encoding="utf-8", newline="\n") as f:
        f.write(s)

def emerald_behaviors():
    body = re.search(r"enum\s*\{(.*?)\}", read("include/constants/metatile_behaviors.h"), re.S).group(1)
    body = re.sub(r"//.*", "", body)
    names = [n.strip() for n in body.split(",") if n.strip()]
    return {n: i for i, n in enumerate(names)}

def frlg_behaviors(frlg):
    out = {}
    for m in re.finditer(r"#define\s+(MB_\w+)\s+(0x[0-9A-Fa-f]+)", read(f"{frlg}/include/constants/metatile_behaviors.h")):
        out[int(m.group(2), 16)] = m.group(1)
    return out

def convert_attributes(data, em, fr, tileset, report):
    out = bytearray()
    for i, (v,) in enumerate(struct.iter_unpack("<I", data)):
        beh, enc, layer = v & 0x1FF, (v >> 24) & 7, (v >> 29) & 3
        name = fr.get(beh)
        target = BEHAVIOR_MAP.get(name) if name else None
        if target is None:
            target = "MB_NORMAL"
        final = target
        # FRLG stores "has land encounters" as a separate attribute; Emerald derives it from the behaviour.
        lost = ""
        if enc == 1 and target not in EM_ENCOUNTER:
            if target in ("MB_NORMAL", "MB_SAND"):
                final = "MB_CAVE" if target == "MB_NORMAL" else "MB_DEEP_SAND"
            else:
                lost = " (land encounters LOST)"
        src = name or ("MB_NORMAL" if beh == 0 else f"unknown 0x{beh:02X}")
        if BEHAVIOR_MAP.get(name) is None and beh != 0 and final == "MB_NORMAL":
            lost += " (no Emerald equivalent)"
        if final != src or lost:
            report[(src + (" + land encounters" if enc == 1 else ""), final + lost)].append(f"{tileset}:{i}")
        target = final
        out += struct.pack("<H", em[target] | (layer << 12))
    return bytes(out)

def rename_symbols(text):
    text = re.sub(r"\b(gTileset|gTilesetTiles|gTilesetPalettes|gMetatiles|gMetatileAttributes)_(\w+)",
                  lambda m: f"{m.group(1)}_{PREFIX_SYM}{m.group(2)}", text)
    return re.sub(r"data/tilesets/(primary|secondary)/", lambda m: f"data/tilesets/{m.group(1)}/{PREFIX_DIR}", text)

def insert_once(path, anchor, line, after=True):
    s = read(path)
    if line in s:
        return
    if anchor not in s:
        sys.exit(f"anchor {anchor!r} not found in {path}")
    s = s.replace(anchor, anchor + "\n" + line if after else line + "\n" + anchor, 1)
    write(path, s)

def cmd_tilesets(args):
    frlg = args.frlg
    em, fr = emerald_behaviors(), frlg_behaviors(frlg)
    report = defaultdict(list)
    count = 0
    for kind in ("primary", "secondary"):
        src_root = f"{frlg}/data/tilesets/{kind}"
        for name in sorted(os.listdir(src_root)):
            src, dst = f"{src_root}/{name}", f"data/tilesets/{kind}/{PREFIX_DIR}{name}"
            if not os.path.isdir(src):
                continue
            os.makedirs(dst, exist_ok=True)
            for dirpath, _, files in os.walk(src):
                rel = os.path.relpath(dirpath, src)
                for f in files:
                    if not f.endswith((".png", ".pal", ".bin")):
                        continue  # skip build products
                    os.makedirs(os.path.join(dst, rel), exist_ok=True)
                    s, d = os.path.join(dirpath, f), os.path.join(dst, rel, f)
                    if f == "metatile_attributes.bin" and rel == ".":
                        with open(s, "rb") as fh:
                            conv = convert_attributes(fh.read(), em, fr, f"{kind}/{name}", report)
                        with open(d, "wb") as fh:
                            fh.write(conv)
                    else:
                        shutil.copyfile(s, d)
            count += 1

    hdr = "// Generated by tools/kanto_port/port_kanto.py from pokefirered. Do not edit by hand.\n\n"
    gfx = read(f"{frlg}/src/data/tilesets/graphics.h")
    # A few tilesets are also used outside the overworld in FRLG, so their graphics live in src/graphics.c.
    shared = re.findall(r"^const u16 gTilesetPalettes_\w+\[\]\[16\] =\n\{.*?\n\};\n|^const u32 gTilesetTiles_\w+\[\] = INCBIN_U32\(.*?\);\n",
                        read(f"{frlg}/src/graphics.c"), re.S | re.M)
    gfx += "\n// From pokefirered src/graphics.c\n" + "\n".join(shared)
    write("src/data/tilesets/kanto_graphics.h", hdr + rename_symbols(gfx))
    met = rename_symbols(read(f"{frlg}/src/data/tilesets/metatiles.h"))
    met = re.sub(r"const u32 (gMetatileAttributes_\w+\[\]) = INCBIN_U32", r"const u16 \1 = INCBIN_U16", met)
    write("src/data/tilesets/kanto_metatiles.h", hdr + met)
    heads = rename_symbols(read(f"{frlg}/src/data/tilesets/headers.h"))
    # Tile animations are not ported yet.
    heads = re.sub(r"\.callback = (InitTilesetAnim_\w+),", r".callback = NULL, // TODO: port \1 from pokefirered", heads)
    write("src/data/tilesets/kanto_headers.h", hdr + heads)

    rules = rename_symbols(read(f"{frlg}/tileset_rules.mk")).replace(
        "$(TILESETGFXDIR)/primary/", f"$(TILESETGFXDIR)/primary/{PREFIX_DIR}").replace(
        "$(TILESETGFXDIR)/secondary/", f"$(TILESETGFXDIR)/secondary/{PREFIX_DIR}")
    rules = "\n".join(l for l in rules.splitlines() if not l.startswith("TILESETGFXDIR"))
    write("tileset_rules_kanto.mk", "# Generated by tools/kanto_port/port_kanto.py from pokefirered.\n" + rules + "\n")

    insert_once("Makefile", "include graphics_file_rules.mk", "include tileset_rules_kanto.mk")
    for inc, anchor in (("kanto_graphics.h", "graphics.h"), ("kanto_metatiles.h", "metatiles.h"), ("kanto_headers.h", "headers.h")):
        insert_once("src/tilesets.c", f'#include "data/tilesets/{anchor}"', f'#include "data/tilesets/{inc}"')

    lines = ["Metatile behaviour conversions (FRLG -> Emerald). Entries are tileset:metatile index.", ""]
    for (a, b), where in sorted(report.items(), key=lambda kv: -len(kv[1])):
        by_ts = Counter(w.split(":")[0] for w in where)
        lines.append(f"{a} -> {b}: {len(where)} metatiles")
        lines.append("    " + ", ".join(f"{t} ({n})" for t, n in sorted(by_ts.items())))
    write("tools/kanto_port/behavior_report.txt", "\n".join(lines) + "\n")
    print(f"ported {count} tilesets; see tools/kanto_port/behavior_report.txt")

def cmd_map(args):
    frlg, name = args.frlg, args.name
    fmap = json.loads(read(f"{frlg}/data/maps/{name}/map.json"))
    flayouts = json.loads(read(f"{frlg}/data/layouts/layouts.json"))["layouts"]
    flay = next(l for l in flayouts if l and l["id"] == fmap["layout"])

    groups = json.loads(read("data/maps/map_groups.json"))
    existing = {m for g in groups["group_order"] for m in groups[g]}
    if name in existing and name not in groups.get(KANTO_GROUP, []):
        sys.exit(f"map name {name} already exists in Emerald (Hoenn); resolve the collision first")

    # layout
    if (flay.get("border_width"), flay.get("border_height")) != (2, 2):
        sys.exit(f"{flay['id']}: border is {flay.get('border_width')}x{flay.get('border_height')}; Emerald only supports 2x2")
    lj = json.loads(read("data/layouts/layouts.json"))
    clash = next((l for l in lj["layouts"] if l and l["id"] == flay["id"]), None)
    ldir = os.path.dirname(flay["blockdata_filepath"])
    new_layout = {
        "id": flay["id"], "name": flay["name"], "width": flay["width"], "height": flay["height"],
        "primary_tileset": rename_symbols(flay["primary_tileset"]),
        "secondary_tileset": rename_symbols(flay["secondary_tileset"]),
        "border_filepath": flay["border_filepath"], "blockdata_filepath": flay["blockdata_filepath"],
    }
    if clash and clash != new_layout:
        sys.exit(f"layout id {flay['id']} already exists in Emerald; resolve the collision first")
    if not clash:
        if os.path.exists(ldir):
            sys.exit(f"{ldir} already exists in Emerald; resolve the collision first")
        lj["layouts"].append(new_layout)
        write("data/layouts/layouts.json", json.dumps(lj, indent=2) + "\n")
    os.makedirs(ldir, exist_ok=True)
    for key in ("border_filepath", "blockdata_filepath"):
        shutil.copyfile(f"{frlg}/{flay[key]}", flay[key])

    # map.json
    m = dict(fmap)
    m.pop("floor_number", None)
    if args.music:
        m["music"] = args.music
    if not args.keep_connections:
        m["connections"] = None
    if not args.keep_events:
        m["object_events"], m["coord_events"], m["bg_events"] = [], [], []
    if not args.keep_warps:
        m["warp_events"] = []
    write(f"data/maps/{name}/map.json", json.dumps(m, indent=2) + "\n")
    scripts = f"data/maps/{name}/scripts.inc"
    if not os.path.exists(scripts):
        write(scripts, f"{name}_MapScripts::\n\t.byte 0\n")

    if KANTO_GROUP not in groups:
        groups["group_order"].append(KANTO_GROUP)
        groups[KANTO_GROUP] = []
    if name not in groups[KANTO_GROUP]:
        groups[KANTO_GROUP].append(name)
        write("data/maps/map_groups.json", json.dumps(groups, indent=2) + "\n")

    inc = f'\t.include "data/maps/{name}/scripts.inc"'
    es = read("data/event_scripts.s")
    if inc not in es:
        last = [l for l in es.splitlines() if re.match(r'\t\.include "data/maps/.*/scripts\.inc"', l)][-1]
        write("data/event_scripts.s", es.replace(last, last + "\n" + inc, 1))
    print(f"ported map {name} ({flay['width']}x{flay['height']}) into {KANTO_GROUP}")

def main():
    if not os.path.exists("include/constants/metatile_behaviors.h"):
        sys.exit("run from the pokeemerald root")
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    t = sub.add_parser("tilesets"); t.add_argument("--frlg", default="../pokefirered"); t.set_defaults(fn=cmd_tilesets)
    m = sub.add_parser("map"); m.add_argument("name"); m.add_argument("--frlg", default="../pokefirered")
    m.add_argument("--music"); m.add_argument("--keep-events", action="store_true")
    m.add_argument("--keep-warps", action="store_true"); m.add_argument("--keep-connections", action="store_true")
    m.set_defaults(fn=cmd_map)
    args = ap.parse_args(); args.fn(args)

if __name__ == "__main__":
    main()
