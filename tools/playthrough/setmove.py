#!/usr/bin/env python3
"""setmove.py SLOT OLD_MOVE NEW_MOVE : replace a party Pokemon's move in cur.ss (re-encrypts the substructs and the checksum, PP full)."""
import sys, struct
sys.argv[0] = "st.py"
from emu import *
import importlib.util, io, contextlib
# reuse st.py's tables without running its main part
src = open(os.path.join(os.path.dirname(__file__), "st.py")).read().split("e = Emu(sav=")[0]
g = {"__name__": "st_tables"}; exec(compile(src, "st.py", "exec"), g)
MOVES, ORDERS, SPECIES = g["MOVES"], g["ORDERS"], g["SPECIES"]
PP = {"Flamethrower": 15, "Water Gun": 25, "Hypnosis": 20}
slot, old, new = int(sys.argv[1]), sys.argv[2].lower(), sys.argv[3].lower()
ids = {v.lower(): k for k, v in MOVES.items()}
e = Emu()
a = S["gPlayerParty"] + slot * 100
m = bytearray(e.peek(a, 100))
pers, otid = struct.unpack("<II", m[:8]); k = pers ^ otid
raw = bytearray(b"".join(struct.pack("<I", struct.unpack("<I", m[32 + j * 4:36 + j * 4])[0] ^ k) for j in range(12)))
i = ORDERS[pers % 24].index("A") * 12
mv = list(struct.unpack("<4H", raw[i:i + 8]))
assert ids[old] in mv, f"{old} not known: {[MOVES[x] for x in mv]}"
p = mv.index(ids[old]); mv[p] = ids[new]
raw[i:i + 8] = struct.pack("<4H", *mv)
raw[i + 8 + p] = 5 if False else raw[i + 8 + p]
# PP: full base PP of the new move (from the table below, else keep)
BASE = {"flamethrower": 15, "water gun": 25, "hypnosis": 20, "recover": 20}
raw[i + 8 + p] = BASE.get(new, raw[i + 8 + p])
cs = sum(struct.unpack("<24H", raw)) & 0xFFFF
m[0x1C:0x1E] = struct.pack("<H", cs)
enc = b"".join(struct.pack("<I", struct.unpack("<I", raw[j * 4:j * 4 + 4])[0] ^ k) for j in range(12))
m[32:80] = enc
e.send(f"poke {a:x}:{bytes(m[:0x20]).hex()}"); e.send(f"poke {a+32:x}:{enc.hex()}")
e.commit()
print(SPECIES.get(struct.unpack("<H", raw[ORDERS[pers % 24].index('G') * 12:][:2])[0]), [MOVES[x] for x in mv])
