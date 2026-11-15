#!/usr/bin/env python3
"""mapinfo.py [MAP] : the current map's (or MAP's) people, trainers, item balls, hidden items and warps, with the
trainers/items already beaten/collected in cur.ss marked.  Output feeds trainer / pick / hid / talk calls."""
import json, os, re, sys, subprocess
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
import gen_area as G
R = G.R
name = sys.argv[1] if len(sys.argv) > 1 else subprocess.run([sys.executable, HERE + "/st.py"], capture_output=True, text=True,
     env=dict(os.environ, PLAY_DIR=os.environ.get("PLAY_DIR", os.path.expanduser("~/.playthrough")))).stdout.split()[1]
mj = json.load(open(f"{R}/data/maps/{name}/map.json"))
beaten = G.flags()
sc = G.read(f"data/maps/{name}/scripts.inc") + "\n" + G.read("data/scripts/lab_trainers.inc")
labels = {m.group(1): m.start() for m in re.finditer(r"^(\w+)::", sc, re.M)}
print("MAP", name, mj["id"])
for o in mj.get("object_events", []):
    g = o["graphics_id"].replace("OBJ_EVENT_GFX_", "").replace("KANTO_", "")
    s = o.get("script", "")
    tr = None
    if s in labels:
        m = re.search(r"trainerbattle\w*\s+(TRAINER_(?:KANTO|LEAF|KRIS)_\w+)", sc[labels[s]:labels[s] + 600])
        tr = m.group(1) if m else None
    if tr in G.trainers:
        state = "?" if beaten is None else ("beaten" if G.flag_index(G.consts[tr]) in beaten else "TO FIGHT")
        team = ", ".join(f"{a} L{b}" for a, b, _ in G.parties.get(G.trainers[tr][2], []))
        print(f"  TRAINER ({o['x']},{o['y']}) {G.trainers[tr][1].title()} [{G.class_name(G.trainers[tr][0])}] {state}: {team}")
    elif g.endswith("ITEM_BALL"):
        got = G.flag_set(o["flag"]) if o.get("flag") else None
        m = re.search(re.escape(s) + r"::\s*(?:\w+\s+)*?(?:finditem|giveitem\w*|kanto_giveitem\w*)\s+(?:\w+,\s*)?(ITEM_\w+)", sc)
        print(f"  ITEM    ({o['x']},{o['y']}) {G.pretty_item(m.group(1)) if m else '?'} {'taken' if got else 'TO TAKE'}")
    else:
        print(f"  npc     ({o['x']},{o['y']}) {g}")
for b in mj.get("bg_events", []):
    if b.get("type") == "hidden_item":
        got = G.flag_set(b["flag"]) if b.get("flag") else None
        print(f"  HIDDEN  ({b['x']},{b['y']}) {G.pretty_item(b['item'])} {'found' if got else 'TO FIND'}")
for w in mj.get("warp_events", []):
    print(f"  warp    ({w['x']},{w['y']}) -> {w['dest_map'][4:]}")
