#!/usr/bin/env python3
"""Fill the "trainers" list (and missing item markers) of the walkthrough area files from the game's own data.

usage: gen_area.py [AREA ...]      (AREA = a data/NN_name key from AREAS below; none = all)

For every map of an area it reads data/maps/<Map>/scripts.inc and map.json: each trainerbattle_* constant becomes a card with the
trainer's name and class, the exact team (species, level, custom moves if any), the prize (4 x last level x class factor) and its
state in the current savestate ($PLAY_DIR/cur.ss: the trainer flags live in kantoFlags, bit 256 + id - KANTO_TRAINERS_START).
Earlier hand-written hints (prizes seen in the run) are kept after the generated text.  Re-run after every area."""
import json, os, re, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
R = os.path.abspath(os.path.join(HERE, "..", ".."))
WT = os.path.join(R, "docs", "walkthrough", "data")

# data file -> maps whose trainers belong to it
AREAS = {
    "01_pallet_town": ["PalletTown"],
    "02_route_1": ["Route1"],
    "03_viridian_city": ["ViridianCity", "ViridianCity_Gym"],
    "04_route_22": ["Route22"],
    "05_route_2": ["Route2"],
    "06_viridian_forest": ["ViridianForest"],
    "07_pewter_city": ["PewterCity"],
    "08_pewter_gym": ["PewterCity_Gym"],
    "09_route_3": ["Route3"],
    "10_mt_moon": ["MtMoon_1F", "MtMoon_B1F", "MtMoon_B2F"],
    "11_route_4": ["Route4"],
    "12_cerulean_city": ["CeruleanCity"],
    "13_cerulean_gym": ["CeruleanCity_Gym"],
    "14_route_24_25": ["Route24", "Route25"],
    "15_route_5_underground": ["Route5", "UndergroundPath_NorthEntrance", "UndergroundPath_NorthSouthTunnel", "UndergroundPath_SouthEntrance"],
    "16_route_6": ["Route6"],
    "17_vermilion_city": ["VermilionCity"],
    "18_ss_anne": ["SSAnne_Exterior", "SSAnne_1F_Corridor", "SSAnne_1F_Room1", "SSAnne_1F_Room2", "SSAnne_1F_Room3", "SSAnne_1F_Room4", "SSAnne_1F_Room5",
                   "SSAnne_1F_Room6", "SSAnne_1F_Room7", "SSAnne_Kitchen", "SSAnne_2F_Corridor", "SSAnne_2F_Room1", "SSAnne_2F_Room2", "SSAnne_2F_Room3",
                   "SSAnne_2F_Room4", "SSAnne_2F_Room5", "SSAnne_2F_Room6", "SSAnne_CaptainsOffice", "SSAnne_3F_Corridor", "SSAnne_Deck",
                   "SSAnne_B1F_Corridor", "SSAnne_B1F_Room1", "SSAnne_B1F_Room2", "SSAnne_B1F_Room3", "SSAnne_B1F_Room4", "SSAnne_B1F_Room5"],
    "19_vermilion_gym": ["VermilionCity_Gym"],
    "21_route_9_10_rock_tunnel": ["Route9", "Route10", "RockTunnel_1F", "RockTunnel_B1F"],
    "20_route_11_diglett": ["Route11", "DiglettsCave_SouthEntrance", "DiglettsCave_B1F", "DiglettsCave_NorthEntrance"],
    "25_celadon_city": ["CeladonCity"],
    "26_game_corner_rocket_hideout": ["CeladonCity_GameCorner", "RocketHideout_B1F", "RocketHideout_B2F", "RocketHideout_B3F", "RocketHideout_B4F", "RocketHideout_Elevator"],
    "27_celadon_gym": ["CeladonCity_Gym"],
    "28_saffron_city": ["SaffronCity", "SaffronCity_Dojo", "SaffronCity_Mart", "SaffronCity_PokemonCenter_1F", "SaffronCity_MrPsychicsHouse", "SaffronCity_PokemonTrainerFanClub"],
    "29_silph_co": ['SilphCo_1F', 'SilphCo_2F', 'SilphCo_3F', 'SilphCo_4F', 'SilphCo_5F', 'SilphCo_6F', 'SilphCo_7F', 'SilphCo_8F', 'SilphCo_9F', 'SilphCo_10F', 'SilphCo_11F', 'SilphCo_Elevator'],
    "30_route_13": ["Route13"],
    "31_route_14": ["Route14"],
    "32_route_15": ["Route15", "Route15_WestEntrance_1F", "Route15_WestEntrance_2F"],
    "33_fuchsia_city": ["FuchsiaCity", "FuchsiaCity_PokemonCenter_1F", "FuchsiaCity_Mart", "FuchsiaCity_SafariZone_Office", "FuchsiaCity_WardensHouse", "FuchsiaCity_House1", "FuchsiaCity_House2", "FuchsiaCity_House3", "FuchsiaCity_SafariZone_Entrance"],
    "34_route_16": ["Route16", "Route16_House", "Route16_NorthEntrance_1F"],
    "35_route_17": ["Route17"],
    "36_route_18": ["Route18", "Route18_EastEntrance_1F"],
    "24_route_12": ["Route12"],
    "23_route_8": ["Route8", "UndergroundPath_EastWestTunnel"],
    "22_lavender_town_tower": ["LavenderTown", "PokemonTower_1F", "PokemonTower_2F", "PokemonTower_3F", "PokemonTower_4F",
                               "PokemonTower_5F", "PokemonTower_6F", "PokemonTower_7F"],
}


def read(p):
    return open(os.path.join(R, p), encoding="utf-8").read()


def pretty(s):
    return s.replace("_", " ").title().replace("Hp ", "HP ").replace("Pp ", "PP ")


# ---- the trainer database
consts = {m.group(1): int(m.group(2)) for m in re.finditer(r"#define\s+(TRAINER_(?:KANTO|LEAF|KRIS)_\w+)\s+(\d+)\b", read("include/constants/opponents_kanto.h"))}
money = {m.group(1): int(m.group(2)) for m in re.finditer(r"\{(TRAINER_CLASS_\w+),\s*(\d+)\}", read("src/battle_main.c") + read("include/constants/trainers_kanto.h"))}
money["0xFF"] = 5   # "any trainer class not listed above"
parties = {}
for f in ("src/data/kanto_trainer_parties.h", "src/data/lab_trainer_parties.h", "src/data/trainer_parties.h"):
    if not os.path.exists(os.path.join(R, f)):
        continue
    txt = read(f)
    for m in re.finditer(r"sParty_(\w+)\[\]\s*=\s*\{(.*?)\n\};", txt, re.S):
        mons = []
        for b in re.finditer(r"\{(.*?)\n    \}", m.group(2), re.S):
            body = b.group(1)
            sp = re.search(r"\.species\s*=\s*SPECIES_(\w+)", body)
            lv = re.search(r"\.lvl\s*=\s*(\d+)", body)
            mv = re.search(r"\.moves\s*=\s*\{([^}]*)\}", body)
            if sp and lv:
                moves = [pretty(x[5:]) for x in re.findall(r"MOVE_\w+", mv.group(1)) if x != "MOVE_NONE"] if mv else []
                mons.append((sp.group(1).lower(), int(lv.group(1)), ", ".join(moves) or "—"))
        parties[m.group(1)] = mons
trainers = {}
for f in ("src/data/kanto_trainers.h", "src/data/lab_trainers.h"):
    txt = read(f)
    for m in re.finditer(r"\[(TRAINER_(?:KANTO|LEAF|KRIS)_\w+)\]\s*=\s*\{(.*?)\n    \},", txt, re.S):
        b = m.group(2)
        cls = re.search(r"\.trainerClass\s*=\s*(\w+)", b)
        nm = re.search(r'\.trainerName\s*=\s*_\("([^"]*)"\)', b)
        pt = re.search(r"\.party\s*=\s*\w+\(sParty_(\w+)\)", b)
        if cls and nm and pt:
            trainers[m.group(1)] = (cls.group(1), nm.group(1), pt.group(1))


GAP = {k: int(v) for k, v in re.findall(r"#define\s+(KANTO_TRAINERS_GAP_\w+)\s+(\d+)", read("include/constants/opponents_kanto.h"))}


def flag_index(tid):
    """battle_setup.c's GetTrainerFlag for a Kanto trainer: the id minus the unused gap, then the id's offset in the Kanto block."""
    if tid >= GAP["KANTO_TRAINERS_GAP_START"] + GAP["KANTO_TRAINERS_GAP_SIZE"]:
        tid -= GAP["KANTO_TRAINERS_GAP_SIZE"]
    return tid - 864


def class_name(c):
    return pretty(c.replace("TRAINER_CLASS_", "").lower()).replace("Pkmn", "Pokémon")


def flags():
    """The set of beaten Kanto trainer ids in cur.ss (empty when no emulator is available)."""
    try:
        from emu import Emu, S, PLAY_DIR
        e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
        sb1 = e.u32(S["gSaveBlock1Ptr"])
        fl = e.peek(sb1 + 0x3D88, 96)
        e.commit(keep=False)
        return {i for i in range(512) if fl[(256 + i) // 8] >> ((256 + i) % 8) & 1}
    except Exception as ex:   # no savestate: everything shows as unknown
        print("flags unavailable:", ex)
        return None


def map_trainers(mp, beaten):
    sc0 = read(f"data/maps/{mp}/scripts.inc")
    sc = sc0 + "\n" + read("data/scripts/lab_trainers.inc")   # the lab trainers (Leaf/Kris) are scripted there
    mj = json.load(open(os.path.join(R, f"data/maps/{mp}/map.json")))
    pos = {}
    for o in mj.get("object_events", []):
        if o.get("script"):
            pos[o["script"]] = (o["x"], o["y"])
    labels = [(m.start(), m.group(1)) for m in re.finditer(r"^(\w+)::", sc, re.M)]
    out = []
    for m in re.finditer(r"trainerbattle\w*\s+(TRAINER_(?:KANTO|LEAF|KRIS)_\w+)", sc):
        c = m.group(1)
        if c not in trainers:
            continue
        lab = [l for p, l in labels if p < m.start()]
        where = pos.get(lab[-1]) if lab else None
        if where is None and "RIVAL" in trainers[c][0] and m.start() < len(sc0):
            # the rival waits as a script-less person (coord-event battle): his marker is the BLUE object of the map
            for o in mj.get("object_events", []):
                if o["graphics_id"].endswith("_BLUE") or o["graphics_id"].endswith("_RIVAL"):
                    where = (o["x"], o["y"])
        if m.start() >= len(sc0) and where is None:
            continue                       # a lab trainer that belongs to another map
        out.append((c, where))
    seen, res = set(), []
    # the rival has one entry per starter in the same script: keep the fought one, else the one for a PIKACHU start (the first listed)
    rival = [c for c, w in out if "RIVAL" in trainers[c][0]]
    out = [(c, w) for c, w in out if not c.startswith("TRAINER_KRIS") or (beaten is not None and flag_index(consts[c]) in beaten)]
    if beaten is not None:
        won = [c for c in rival if flag_index(consts[c]) in beaten]
        drop = set(rival) - set(won) if won else set()
        out = [(c, w) for c, w in out if c not in drop]
    for c, w in out:
        if c in seen:
            continue
        seen.add(c)
        res.append((c, w, mp))
    return res


def card(c, where, mp, beaten, multi):
    cls, name, pty = trainers[c]
    team = parties.get(pty, [])
    prize = 4 * (team[-1][1] if team else 0) * money.get(cls, money.get('0xFF', 5))
    state = "" if beaten is None else (" <b>Beaten.</b>" if flag_index(consts[c]) in beaten else " <b>Not fought yet.</b>")
    at = f" at ({where[0]},{where[1]})" if where else ""
    loc = f"{mp}{at}. " if multi else (f"({where[0]},{where[1]}). " if where else "")
    title = ("Rival " + name.title()) if "RIVAL" in cls and name.upper() in ("LEAF", "KRIS") else "Rival Blue" if "RIVAL" in cls else f"{class_name(cls)} {name.title()}"
    return {"name": title, "hint": f"{loc}Prize ¥{prize}.{state}",
            "team": [[s, l, m] for s, l, m in team], "_id": c}


# ---- flags by name (for "collected?")
_defs = {}
for m in re.finditer(r"#define\s+(FLAG_\w+)\s+(.+)", read("include/constants/flags.h")):
    _defs[m.group(1)] = re.sub(r"//.*", "", m.group(2)).strip()


def flag_value(name, depth=0):
    e = _defs.get(name)
    if e is None or depth > 8:
        return None
    try:
        return int(eval(re.sub(r"\b(\w+)\b", lambda mm: str(flag_value(mm.group(1), depth + 1)) if mm.group(1) in _defs else mm.group(1), e),
                         {"__builtins__": {}}, {k: v for k, v in _consts().items()}))
    except Exception:
        return None


_cc = {}


def _consts():
    if not _cc:
        for f in ("include/constants/flags.h",):
            for m in re.finditer(r"#define\s+(\w+)\s+(0x[0-9A-Fa-f]+|\d+)\b\s*(?://.*)?$", read(f), re.M):
                _cc[m.group(1)] = int(m.group(2), 0)
        _cc["FLAGS_COUNT"] = 0
    return _cc


_fl = {}


def flag_set(name):
    """Is the named flag set in cur.ss?  None when unknown."""
    v = flag_value(name)
    if v is None:
        return None
    try:
        from emu import Emu, S, PLAY_DIR
        if "sb1" not in _fl:
            e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
            sb1 = e.u32(S["gSaveBlock1Ptr"])
            _fl["main"] = e.peek(sb1 + 0x1270, 0x12C)
            _fl["kanto"] = e.peek(sb1 + 0x3D88, 96)
            e.commit(keep=False)
            _fl["sb1"] = 1
        if v >= 0x1000:
            i = v - 0x1000
            return bool(_fl["kanto"][i // 8] >> (i % 8) & 1)
        return bool(_fl["main"][v // 8] >> (v % 8) & 1)
    except Exception:
        return None


def pretty_item(i):
    if i == "ITEM_NONE":
        return "coins (hidden slot-machine coins)"
    return pretty(i.replace("ITEM_", "").lower()).replace("Tm", "TM").replace("Hm", "HM")


def fill(k, cards_by_map):
    """Items, markers, wild tables and the map(s) of an area whose file has none yet."""
    maps = AREAS[k]
    p = os.path.join(WT, k + ".json")
    d = json.load(open(p, encoding="utf-8"))
    mp0 = d.get("map")
    has_markers = bool(mp0) and (isinstance(mp0, list) and any(x.get("markers") for x in mp0) or isinstance(mp0, dict) and mp0.get("markers"))
    if d.get("items") and has_markers and d.get("wild"):
        return
    items, specs, wild, n = [], [], [], 0
    for mp in maps:
        mj = json.load(open(os.path.join(R, f"data/maps/{mp}/map.json")))
        sc = read(f"data/maps/{mp}/scripts.inc")
        marks = []
        for o in mj.get("object_events", []):
            g = o["graphics_id"]
            if g.endswith("ITEM_BALL") and o.get("script"):
                m = re.search(re.escape(o["script"]) + r"::\s*(?:\w+\s+)*?(?:finditem|giveitem\w*|kanto_giveitem\w*)\s+(?:\w+,\s*)?(ITEM_\w+)", sc)
                nm = pretty_item(m.group(1)) if m else "an item"
                got = flag_set(o["flag"]) if o.get("flag") else None
                n += 1
                marks.append(["item", o["x"], o["y"], str(n)])
                items.append({"kind": "item", "n": str(n), "text": f"{nm} at ({o['x']},{o['y']})" + (f" ({mp})" if len(maps) > 1 else "") + ".",
                              "chip": "yes" if got else "no", "chipText": "Collected" if got else ("Not collected" if got is False else "Not checked")})
            elif g.endswith("CUT_TREE"):
                marks.append(["cut", o["x"], o["y"], "C"])
        for b in mj.get("bg_events", []):
            if b.get("type") == "hidden_item":
                got = flag_set(b["flag"]) if b.get("flag") else None
                n += 1
                marks.append(["hidden", b["x"], b["y"], str(n)])
                items.append({"kind": "hidden", "n": str(n), "text": f"Hidden {pretty_item(b['item'])} at ({b['x']},{b['y']})" + (f" ({mp})" if len(maps) > 1 else "") + ".",
                              "chip": "yes" if got else "no", "chipText": "Found" if got else ("Not found" if got is False else "Not checked")})
        for j, c in enumerate([c for c in cards_by_map.get(mp, [])], 1):
            if c["pos"]:
                marks.append(["trainer", c["pos"][0], c["pos"][1], f"T{c['k']}"])
        specs.append({"name": mp, "markers": marks})
        mid = mj["id"]
        enc = json.load(open(os.path.join(R, "src/data/wild_encounters.json")))["wild_encounter_groups"][0]["encounters"]
        for kind, label in (("land_mons", "Tall grass"), ("water_mons", "Surfing"), ("fishing_mons", "Fishing"), ("rock_smash_mons", "Rock Smash")):
            if any(e.get("map") == mid and kind in e for e in enc):
                wild.append([mid, kind, label + (f" ({mp})" if len(maps) > 1 else "")])
    if not d.get("items"):
        d["items"] = items
    if not has_markers:
        d["map"] = specs[0] if len(specs) == 1 else specs
    if not d.get("wild"):
        d["wild"] = wild
    json.dump(d, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print("  filled", k, len(items), "items,", len(wild), "wild tables")


def extend(k, by_map):
    """For hand-curated areas: renumber the trainer markers to match the cards, add the other maps of the area (with their item
    balls, hidden items and trainers) and the wild tables that are missing.  Existing item entries are kept."""
    maps = AREAS[k]
    p = os.path.join(WT, k + ".json")
    d = json.load(open(p, encoding="utf-8"))
    specs = d["map"] if isinstance(d.get("map"), list) else ([d["map"]] if d.get("map") else [])
    have = {sp["name"]: sp for sp in specs}
    items = d.get("items", [])
    n = max([int(i["n"]) for i in items if str(i.get("n", "")).isdigit()] + [0])
    wild = d.get("wild") or []
    enc = json.load(open(os.path.join(R, "src/data/wild_encounters.json")))["wild_encounter_groups"][0]["encounters"]
    for mp in maps:
        mj = json.load(open(os.path.join(R, f"data/maps/{mp}/map.json")))
        sp = have.get(mp) or {"name": mp, "markers": []}
        sp["markers"] = [m for m in sp["markers"] if m[0] != "trainer"]
        # every item ball, hidden item and Cut tree of the map gets a marker (also on maps the file already lists): an entry of the
        # item list that names the same tile (and floor) keeps its number, otherwise a new entry is added
        floor = mp.rsplit("_", 1)[-1] if re.search(r"_(B?\d+F)$", mp) else ""
        def entry_for(x, y):
            for i in items:
                if f"({x},{y})" in i["text"]:
                    other = re.search(r"\bB?\d+F\b", i["text"])
                    if (other.group(0) if other else (floor if mp == maps[0] else "")) == floor or not other and mp == maps[0]:
                        return i
            return None
        have_pos = {(m[1], m[2]) for m in sp["markers"] if m[0] != "trainer"}
        sc = read(f"data/maps/{mp}/scripts.inc")
        for o in mj.get("object_events", []):
            if o["graphics_id"].endswith("CUT_TREE") and (o["x"], o["y"]) not in have_pos:
                sp["markers"].append(["cut", o["x"], o["y"], "C"]); have_pos.add((o["x"], o["y"]))
            if not (o["graphics_id"].endswith("ITEM_BALL") and o.get("script")) or (o["x"], o["y"]) in have_pos:
                continue
            e = entry_for(o["x"], o["y"])
            if e is None:
                m = re.search(re.escape(o["script"]) + r"::\s*(?:\w+\s+)*?(?:finditem|giveitem\w*|kanto_giveitem\w*)\s+(?:\w+,\s*)?(ITEM_\w+)", sc)
                got = flag_set(o["flag"]) if o.get("flag") else None
                n += 1
                e = {"kind": "item", "n": str(n), "text": f"{pretty_item(m.group(1)) if m else 'An item'} at ({o['x']},{o['y']}) ({mp}).",
                     "chip": "yes" if got else "no", "chipText": "Collected" if got else "Not collected"}
                items.append(e)
            sp["markers"].append(["item", o["x"], o["y"], e["n"]]); have_pos.add((o["x"], o["y"]))
        for bg in mj.get("bg_events", []):
            if bg.get("type") != "hidden_item" or (bg["x"], bg["y"]) in have_pos:
                continue
            e = entry_for(bg["x"], bg["y"])
            if e is None:
                got = flag_set(bg["flag"]) if bg.get("flag") else None
                n += 1
                e = {"kind": "hidden", "n": str(n), "text": f"Hidden {pretty_item(bg['item'])} at ({bg['x']},{bg['y']}) ({mp}).",
                     "chip": "yes" if got else "no", "chipText": "Found" if got else "Not found"}
                items.append(e)
            sp["markers"].append(["hidden", bg["x"], bg["y"], e["n"]]); have_pos.add((bg["x"], bg["y"]))
        for c in by_map.get(mp, []):
            if c["pos"]:
                sp["markers"].append(["trainer", c["pos"][0], c["pos"][1], f"T{c['k']}"])
        have[mp] = sp
        for kind, label in (("land_mons", "Tall grass"), ("water_mons", "Surfing"), ("fishing_mons", "Fishing"), ("rock_smash_mons", "Rock Smash")):
            ent = [x for x in wild if x[0] == mj["id"] and x[1] == kind]
            if not ent and any(e.get("map") == mj["id"] and kind in e for e in enc) and not d.get("wild_done"):
                wild.append([mj["id"], kind, label + (f" ({mp})" if len(maps) > 1 else "")])
    d["map"] = have[maps[0]] if len(maps) == 1 else [have[m] for m in maps]
    d["items"], d["wild"] = items, wild
    json.dump(d, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)


# ---- the Pokémon caught in each area (from the party and PC boxes of the savestate: gen_caught.py) and the ones still missing
AREA_MAPSEC = {
    "01_pallet_town": ["MAPSEC_PALLET_TOWN"], "02_route_1": ["MAPSEC_ROUTE_1"], "03_viridian_city": ["MAPSEC_VIRIDIAN_CITY"],
    "04_route_22": ["MAPSEC_ROUTE_22"], "05_route_2": ["MAPSEC_ROUTE_2"], "06_viridian_forest": ["MAPSEC_VIRIDIAN_FOREST"],
    "07_pewter_city": ["MAPSEC_PEWTER_CITY"], "09_route_3": ["MAPSEC_ROUTE_3"], "10_mt_moon": ["MAPSEC_MT_MOON"],
    "11_route_4": ["MAPSEC_ROUTE_4", "MAPSEC_ROUTE_4_POKECENTER"], "12_cerulean_city": ["MAPSEC_CERULEAN_CITY"],
    "14_route_24_25": ["MAPSEC_ROUTE_24", "MAPSEC_ROUTE_25"], "15_route_5_underground": ["MAPSEC_ROUTE_5", "MAPSEC_UNDERGROUND_PATH", "MAPSEC_UNDERGROUND_PATH_2"],
    "16_route_6": ["MAPSEC_ROUTE_6"], "20_route_11_diglett": ["MAPSEC_ROUTE_11", "MAPSEC_DIGLETTS_CAVE"], "21_route_9_10_rock_tunnel": ["MAPSEC_ROUTE_9", "MAPSEC_ROUTE_10", "MAPSEC_ROCK_TUNNEL", "MAPSEC_ROUTE_10_POKECENTER"], "17_vermilion_city": ["MAPSEC_VERMILION_CITY"], "22_lavender_town_tower": ["MAPSEC_LAVENDER_TOWN", "MAPSEC_POKEMON_TOWER"], "23_route_8": ["MAPSEC_ROUTE_8"], "24_route_12": ["MAPSEC_ROUTE_12"], "25_celadon_city": ["MAPSEC_CELADON_CITY"], "26_game_corner_rocket_hideout": ["MAPSEC_ROCKET_HIDEOUT"], "18_ss_anne": ["MAPSEC_S_S_ANNE"], "30_route_13": ["MAPSEC_ROUTE_13"], "34_route_16": ["MAPSEC_ROUTE_16"], "35_route_17": ["MAPSEC_ROUTE_17"], "36_route_18": ["MAPSEC_ROUTE_18"], "31_route_14": ["MAPSEC_ROUTE_14"], "32_route_15": ["MAPSEC_ROUTE_15"], "33_fuchsia_city": ["MAPSEC_FUCHSIA_CITY", "MAPSEC_KANTO_SAFARI_ZONE"],
}
# the area whose wild tables decide what is "still missing" (several data files share a map section)
HOW = {("pikachu", "MAPSEC_PALLET_TOWN"): "gift from Prof. Oak", ("squirtle", "MAPSEC_CERULEAN_CITY"): "gift (the house at (17,11))",
       ("magikarp", "MAPSEC_ROUTE_4"): "bought for ¥500 from the Center's salesman (by accident)", ("bulbasaur", "MAPSEC_VERMILION_CITY"): "gift from the trainer after the Beedrill rescue (Viridian Forest)", ("charmeleon", "MAPSEC_MT_MOON"): "wild Charmander L10, cornered on B2F (Luna's quest)"}


def caught_for(k):
    try:
        c = json.load(open(os.path.join(WT, "caught.json")))
    except Exception:
        return None
    secs = AREA_MAPSEC.get(k)
    if not secs:
        return None
    mons = [m for m in c["mons"] if m["mapsec"] in secs]
    owned = set(c["dex_owned"])
    got = [{"species": m["species"], "nick": m["nick"], "met": m["metlevel"], "level": m["level"], "where": m["where"],
            "how": HOW.get((m["species"], m["mapsec"]), "wild")} for m in mons]
    # the species of this area's wild tables (every time of day and method) that are not owned yet
    enc = json.load(open(os.path.join(R, "src/data/wild_encounters.json")))["wild_encounter_groups"][0]["encounters"]
    ids = {int(m.group(2)): m.group(1) for m in re.finditer(r"#define SPECIES_(\w+)\s+(\d+)\b", read("include/constants/species.h"))}
    names = {v: k for k, v in ids.items()}
    ground = {}
    for mp in AREAS.get(k, []):
        mid = json.load(open(os.path.join(R, f"data/maps/{mp}/map.json")))["id"]
        for e in enc:
            if e.get("map") == mid:
                for kind in ("land_mons", "water_mons", "fishing_mons", "rock_smash_mons"):
                    for m in e.get(kind, {}).get("mons", []):
                        ground.setdefault(m["species"][8:], set()).add(kind.split("_")[0])
    # evolution lines count: a species is "owned" when the dex says so, which the game sets for the caught form only
    missing = [{"species": sp.lower(), "via": sorted(v)} for sp, v in sorted(ground.items())
               if names.get(sp, 0) not in owned and not any(g["species"] == sp.lower() for g in got)]
    return got, missing, None


def main(keys):
    beaten = flags()
    for k in keys:
        maps = AREAS[k]
        p = os.path.join(WT, k + ".json")
        d = json.load(open(p, encoding="utf-8"))
        cards, by_map = [], {}
        for mp in maps:
            for c, w, m in map_trainers(mp, beaten):
                cards.append(card(c, w, m, beaten, len(maps) > 1))
                by_map.setdefault(mp, []).append({"pos": w, "k": len(cards)})
        # the rival's variants share a script: keep only the one(s) fought or all if none was
        d["trainers"] = [{kk: v for kk, v in t.items() if kk != "_id"} for t in cards]
        json.dump(d, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        d0 = json.load(open(p, encoding="utf-8"))
        cf = caught_for(k)
        if cf:
            d0["caught"], d0["missing"] = cf[0], cf[1]
            json.dump(d0, open(p, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
        mp0 = d0.get("map")
        if d0.get("items") and mp0 and (isinstance(mp0, list) or mp0.get("markers")):
            extend(k, by_map)
        else:
            fill(k, by_map)
        print(k, len(cards), "trainers,", sum(1 for t in cards if "Beaten" in t["hint"]), "beaten")


if __name__ == "__main__":
    main(sys.argv[1:] or list(AREAS))
