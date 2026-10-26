#!/usr/bin/env python3
"""lead.py A B : swap party slots A and B (0-5) by rewriting gPlayerParty. Same result as START > POKeMON > SWITCH,
without the menu timing. Only use it in the overworld: a battle keeps its own copy of the active Pokemon."""
import sys
from emu import *

a, b = int(sys.argv[1]), int(sys.argv[2])
e = Emu()
base = S["gPlayerParty"]
pa, pb = e.peek(base + a * 100, 100), e.peek(base + b * 100, 100)
e.send(f"poke {base + a * 100:x}:{pb.hex()}"); e.send(f"poke {base + b * 100:x}:{pa.hex()}")
e.commit()
print("swapped", a, b)
