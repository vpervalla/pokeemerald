import sys,os,re
sys.path.insert(0,'/home/user/pokeemerald/tools/playthrough')
from emu import Emu
CH={}
for l in open('/home/user/pokeemerald/charmap.txt',encoding='utf-8'):
    m=re.match(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$",l)
    if m: CH.setdefault(int(m.group(2),16),m.group(1))
e=Emu()
for a in (0x0203e138,0x020001e0):
    b=e.peek(a,120); print(''.join(CH.get(c,'?') for c in b.split(b'\xff')[0]))
