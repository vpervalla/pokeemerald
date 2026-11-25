"""newwild.py : prints NEW if the current battle is a wild one against a species not owned in the Pokedex, else OLD/TRAINER."""
import sys, re, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from emu import Emu, S
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
e = Emu()
if e.u32(S["gBattleTypeFlags"]) & 8: print("TRAINER"); sys.exit()
import struct
m = e.peek(S["gEnemyParty"], 100); pid, otid = struct.unpack("<II", m[:8]); k = pid ^ otid
ORD = ["GAEM","GAME","GEAM","GEMA","GMAE","GMEA","AGEM","AGME","AEGM","AEMG","AMGE","AMEG","EGAM","EGMA","EAGM","EAMG","EMGA","EMAG","MGAE","MGEA","MAGE","MAEG","MEGA","MEAG"]
raw = b"".join(struct.pack("<I", struct.unpack("<I", m[32+j*4:36+j*4])[0] ^ k) for j in range(12))
sp = struct.unpack("<H", raw[ORD[pid % 24].index("G")*12:][:2])[0]
sb2 = e.u32(S["gSaveBlock2Ptr"])
owned = e.peek(sb2 + 0x18 + 0x10, 52)      # Pokedex "owned" bits (same as gen_caught.py), index = species - 1 for Gen 1-2
print("NEW" if sp <= 251 and not (owned[(sp - 1) // 8] >> ((sp - 1) % 8) & 1) else "OLD", sp)
