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
    return pretty(i.replace("ITEM_", "").lower()).replace("Tm", "TM").replace("Hm", "HM")


def fill(k, cards_by_map):
    """Items, markers, wild tables and the map(s) of an area whose file has none yet."""
    maps = AREAS[k]
    p = os.path.join(WT, k + ".json")
    d = json.load(open(p, encoding="utf-8"))
    if d.get("items") and d.get("map", {}).get("markers") and d.get("wild"):
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
    if not d.get("map", {}).get("markers"):
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
        known = " ".join(i["text"] for i in items)
        for o in mj.get("object_events", []):
            if o["graphics_id"].endswith("ITEM_BALL") and mp not in have:
                continue
        if mp not in have:
            for o in mj.get("object_events", []):
                if o["graphics_id"].endswith("ITEM_BALL") and o.get("script") and f"({o['x']},{o['y']})" not in known:
                    sc = read(f"data/maps/{mp}/scripts.inc")
                    m = re.search(re.escape(o["script"]) + r"::\s*(?:\w+\s+)*?(?:finditem|giveitem\w*|kanto_giveitem\w*)\s+(?:\w+,\s*)?(ITEM_\w+)", sc)
                    got = flag_set(o["flag"]) if o.get("flag") else None
                    n += 1
                    sp["markers"].append(["item", o["x"], o["y"], str(n)])
                    items.append({"kind": "item", "n": str(n), "text": f"{pretty_item(m.group(1)) if m else 'An item'} at ({o['x']},{o['y']}) ({mp}).",
                                  "chip": "yes" if got else "no", "chipText": "Collected" if got else "Not collected"})
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
        mp0 = d0.get("map")
        if d0.get("items") and mp0 and (isinstance(mp0, list) or mp0.get("markers")):
            extend(k, by_map)
        else:
            fill(k, by_map)
        print(k, len(cards), "trainers,", sum(1 for t in cards if "Beaten" in t["hint"]), "beaten")


if __name__ == "__main__":
    main(sys.argv[1:] or list(AREAS))
