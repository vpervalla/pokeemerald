#!/usr/bin/env python3
"""Rewrite docs/walkthrough/data/team.json from the party in $PLAY_DIR/cur.ss (the `st` output) and the game's own data.
usage: gen_team.py        (run it whenever the party changed, before build_walkthrough.py)
"met" and "role" are hand-written: edit MET/ROLE below (keyed by species) or keep them from the previous team.json."""
import json, os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
R = os.path.abspath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(R, "docs", "walkthrough", "data", "team.json")

MET = {
    "pikachu": "Gift from Prof. Oak, L5",
    "mankey": "Route 22, east grass, wild L4",
    "butterfree": "Caught in Viridian Forest as a wild Caterpie L4",
    "pidgeotto": "Route 1 or 2, wild Pidgey",
    "charmeleon": "Mt. Moon B2F, the cornered wild Charmander L10 (Luna's quest)",
    "zubat": "Mt. Moon B2F, wild L8",
    "alakazam": "Route 24, wild Abra L10 (raised on Route 6 by wild battles, no Rare Candy; trade-evolved by the Vermilion Center sailor's double exchange)",
    "wartortle": "Gift: the SQUIRTLE of the Cerulean house (Nugget Bridge); raised L12 to L24 on Route 6 by swap-outs, no Rare Candy",
    "ivysaur": "Vermilion house: the BULBASAUR L20 rescued in Viridian Forest (gift); raised to L26 by swap-outs, no Rare Candy",
    "gengar": "Pokémon Tower 5F, wild GASTLY L15 (Modest), renamed DR VENKMAN; HAUNTER at L25 by solo battles, GENGAR by the Lavender Center double exchange",
    "haunter": "Pokémon Tower 5F, wild GASTLY L15 (Modest), renamed DR VENKMAN; evolved at L25 by solo wild battles",
    "gastly": "Pokémon Tower 5F, wild L15 (the seventh encounter of the Modest hunt), renamed DR VENKMAN at Lavender's Name Rater",
    "kadabra": "Route 24, wild Abra L10 (raised to L16 on Route 6 by swapping out on turn 1; no Rare Candy)",
}
ROLE = {
    "pikachu": "The Electric answer to Water types (Misty's STARYU); fast but fragile.",
    "mankey": "The Fighting type: FURY SWIPES replaced SCRATCH; hits Normal and Rock hard.",
    "butterfree": "The status support: SLEEP POWDER decided Misty's STARMIE; CONFUSION is the attack.",
    "pidgeotto": "Flying coverage and the sturdiest body; QUICK ATTACK for priority.",
    "charmeleon": "The Fire starter; goal: Charizard by the end of Kanto.",
    "zubat": "The poison/flying bat, boxed for being far behind in level.",
    "alakazam": "The Psychic special attacker (73 Sp. Atk at L24): CONFUSION and PSYBEAM, DISABLE; REFLECT was declined.",
    "wartortle": "The Water type (BUBBLE, WATER GUN): the Surf candidate and the answer to Fire and Ground; BLASTOISE at L36. Swapped in for PIDGEOTTO.",
    "ivysaur": "The Grass/Poison type: RAZOR LEAF, SLEEP POWDER, POISONPOWDER; the Grass answer to Water. Swapped in for BUTTERFREE (boxed).",
    "gengar": "The Ghost/Poison special attacker (82 Sp. Atk, Modest): the goal of the Kanto run; double-exchanged from HAUNTER at the Lavender Center trader.",
    "haunter": "The GENGAR goal: Modest HAUNTER L25 (raised alone on Tower 5F beside the purified zone); the Vermilion sailor's double exchange makes it GENGAR.",
    "gastly": "The GENGAR goal: Modest (+Sp. Atk, -Atk), needs L25 for HAUNTER and then a trade (the Vermilion sailor's double exchange).",
    "kadabra": "The Psychic special attacker; it evolves into ALAKAZAM by trade. Swapped in for ZUBAT.",
}
NAT = {"Hardy": "", "Lonely": "Atk,Def", "Brave": "Atk,Spe", "Adamant": "Atk,SpA", "Naughty": "Atk,SpD", "Bold": "Def,Atk",
       "Docile": "", "Relaxed": "Def,Spe", "Impish": "Def,SpA", "Lax": "Def,SpD", "Timid": "Spe,Atk", "Hasty": "Spe,Def",
       "Serious": "", "Jolly": "Spe,SpA", "Naive": "Spe,SpD", "Modest": "SpA,Atk", "Mild": "SpA,Def", "Quiet": "SpA,Spe",
       "Bashful": "", "Rash": "SpA,SpD", "Calm": "SpD,Atk", "Gentle": "SpD,Def", "Sassy": "SpD,Spe", "Careful": "SpD,SpA", "Quirky": ""}
FULL = {"Atk": "Attack", "Def": "Defense", "SpA": "Sp. Atk", "SpD": "Sp. Def", "Spe": "Speed"}


def rd(f):
    return open(os.path.join(R, f), encoding="utf-8", errors="replace").read()


def pretty(s):
    return s.replace("_", " ").title()


def main():
    env = dict(os.environ, PLAY_DIR=os.environ.get("PLAY_DIR", os.path.expanduser("~/.playthrough")))
    out = subprocess.run([sys.executable, os.path.join(HERE, "st.py")], capture_output=True, text=True, env=env).stdout
    sp_info = rd("src/data/pokemon/species_info.h")
    mv_info = rd("src/data/battle_moves.h")
    old = {}
    try:
        old = {m["species"]: m for m in json.load(open(OUT, encoding="utf-8"))["mons"]}
    except Exception:
        pass
    mons = []
    for line in out.splitlines():
        m = re.match(r"\s+(.*?) \((.*?)\) L(\d+) HP (\d+)/(\d+) exp (\d+) (\w+) ability(\d) stats (\d+)/(\d+)/(\d+)/(\d+)/(\d+) status \d+ item \d+ \| (.*)", line)
        if not m:
            continue
        nick, sp, lv, hp, mhp, exp, nat, ab, atk, df, spa, spd, spe, mvs = m.groups()
        key = re.sub(r"[^a-z]", "", sp.lower().replace("♀", "f").replace("♂", "m"))
        const = re.sub(r"[^A-Z0-9_]", "", sp.upper().replace(" ", "_").replace("♀", "_F").replace("♂", "_M"))
        blk = re.search(r"\[SPECIES_" + const + r"\]\s*=\s*\{(.*?)\n    \},", sp_info, re.S)
        types = re.findall(r"TYPE_(\w+)", (re.search(r"\.types\s*=\s*\{([^}]*)\}", blk.group(1)).group(1)) if blk else "")
        types = list(dict.fromkeys(pretty(t.lower()) for t in types))
        abl = re.findall(r"ABILITY_(\w+)", re.search(r"\.abilities\s*=\s*\{([^}]*)\}", blk.group(1)).group(1)) if blk and re.search(r"\.abilities", blk.group(1)) else []
        ability = pretty(abl[int(ab)].lower()) if len(abl) > int(ab) and abl[int(ab)] != "NONE" else (pretty(abl[0].lower()) if abl else "—")
        up, dn = (NAT[nat].split(",") + ["", ""])[:2] if NAT[nat] else ("", "")
        eff = {"Atk": 0, "Def": 0, "SpA": 0, "SpD": 0, "Spe": 0}
        stats = [["HP", int(mhp), 0]] + [[FULL[k], int(v), 1 if k == up else -1 if k == dn else 0]
                                          for k, v in (("Atk", atk), ("Def", df), ("SpA", spa), ("SpD", spd), ("Spe", spe))]
        natxt = f"{nat} (+{FULL[up]}, −{FULL[dn]})" if up else f"{nat} (neutral)"
        moves = []
        for mm in mvs.split(", "):
            mt = re.match(r"(.*) (\d+)$", mm.strip())
            if not mt:
                continue
            name, pp = mt.groups()
            norm = re.sub(r"[^A-Z0-9]", "", name.upper())
            b = next((x for x in re.finditer(r"\[MOVE_(\w+)\]\s*=\s*\{(.*?)\n    \},", mv_info, re.S) if x.group(1).replace("_", "") == norm), None)
            g = lambda pat: (re.search(pat, b.group(2)).group(1) if b and re.search(pat, b.group(2)) else None)
            pw, ac, ty, mpp = g(r"\.power\s*=\s*(\d+)"), g(r"\.accuracy\s*=\s*(\d+)"), g(r"\.type\s*=\s*TYPE_(\w+)"), g(r"\.pp\s*=\s*(\d+)")
            moves.append([name.title().replace("_", " "), pretty((ty or "normal").lower()), "—" if not pw or pw == "0" else pw,
                          f"{ac}%" if ac and ac != "0" else "—", f"{pp}/{mpp or pp}"])
        o = old.get(key, {})
        mons.append({"species": key, "name": nick.title() if nick.upper() == sp.upper() else nick, "lv": int(lv), "exp": int(exp), "types": types,
                     "nature": natxt, "ability": ability, "met": MET.get(key, o.get("met", "see the playthrough log")),
                     "role": ROLE.get(key, o.get("role", "")), "stats": stats, "moves": moves})
    json.dump({"mons": mons}, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(len(mons), "mons:", ", ".join(f"{m['name']} L{m['lv']}" for m in mons))


if __name__ == "__main__":
    main()
