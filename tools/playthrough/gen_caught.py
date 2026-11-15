#!/usr/bin/env python3
"""Write docs/walkthrough/data/caught.json from the party and the PC boxes in $PLAY_DIR/cur.ss:
for every Pokémon its species, nickname, level (now and when met), where it was met (the map section stored in the Pokémon), whether it is in the
party or in which box, and the dex "owned" bits.  gen_area.py / the builder use it for the "Pokémon caught here" panel of each area."""
import json, os, re, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from emu import Emu, S, PLAY_DIR
R = os.path.abspath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(R, "docs", "walkthrough", "data", "caught.json")
ORDERS = ["GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA", "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
          "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG", "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG"]


def rd(f):
    return open(os.path.join(R, f), encoding="utf-8", errors="replace").read()


sp_id = {int(m.group(2)): m.group(1) for m in re.finditer(r"#define SPECIES_(\w+)\s+(\d+)\b", rd("include/constants/species.h"))}
mapsecs = re.findall(r"^\s*(MAPSEC_\w+),", rd("include/constants/region_map_sections.h").split("enum {", 1)[1].split("MAPSEC_NONE")[0], re.M)
chars = {}
for line in open(os.path.join(R, "charmap.txt"), encoding="utf-8"):
    m = re.match(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$", line)
    if m:
        chars.setdefault(int(m.group(2), 16), m.group(1))


def txt(b):
    return "".join(chars.get(c, "?") for c in b.split(b"\xff")[0])


def decode(m):
    pers, otid = struct.unpack("<II", m[:8])
    k = pers ^ otid
    raw = b"".join(struct.pack("<I", struct.unpack("<I", m[32 + j * 4:36 + j * 4])[0] ^ k) for j in range(12))
    order = ORDERS[pers % 24]
    sub = {c: raw[order.index(c) * 12:(order.index(c) + 1) * 12] for c in "GAEM"}
    species = struct.unpack("<H", sub["G"][:2])[0]
    if species == 0:
        return None
    exp = struct.unpack("<I", sub["G"][4:8])[0]
    metloc = sub["M"][1]
    origins = struct.unpack("<H", sub["M"][2:4])[0]
    return {"species": sp_id.get(species, str(species)).lower(), "id": species, "nick": txt(m[8:18]), "metloc": metloc,
            "mapsec": mapsecs[metloc] if metloc < len(mapsecs) else f"?{metloc}", "metlevel": origins & 0x7F, "exp": exp}


def main():
    e = Emu(sav=os.path.join(PLAY_DIR, "peek.sav"))
    out = []
    cnt = e.peek(S["gPlayerPartyCount"], 1)[0]
    for i in range(cnt):
        m = e.peek(S["gPlayerParty"] + i * 100, 100)
        d = decode(m)
        if d:
            d["level"] = m[84]; d["where"] = "party"; out.append(d)
    st = e.u32(S["gPokemonStoragePtr"])
    for b in range(14):
        for s in range(30):
            m = e.peek(st + 4 + (b * 30 + s) * 80, 80)
            d = decode(m + b"\0" * 20) if len(m) == 80 else None
            if d:
                d["level"] = None; d["where"] = f"box {b + 1}"; out.append(d)
    sb2 = e.u32(S["gSaveBlock2Ptr"])
    owned = e.peek(sb2 + 0x18 + 0x10, 52)
    e.commit(keep=False)
    own = sorted(i + 1 for i in range(52 * 8) if owned[i // 8] >> (i % 8) & 1)
    json.dump({"mons": out, "dex_owned": own}, open(OUT, "w"), indent=1)
    print(len(out), "mons;", len(own), "species owned")
    for d in out:
        print(f"  {d['species']:12} {d['nick']:10} met {d['mapsec']} L{d['metlevel']} {d['where']}")


if __name__ == "__main__":
    main()
