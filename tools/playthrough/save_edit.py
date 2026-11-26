"""save_edit.py MODE ... : edit the running game (cur.ss) and commit; then save in the game. Modes: nature SLOT NATURE BASESTATS, rename ..., move SLOT OLD NEW PP (replace a move, e.g. 1 325 247 15 = Shadow Punch -> Shadow Ball)."""
import sys, struct, re, random
sys.path.insert(0, '/home/user/pokeemerald/tools/playthrough')
from emu import Emu, S
R='/home/user/pokeemerald/'
ORDERS = ["GAEM","GAME","GEAM","GEMA","GMAE","GMEA","AGEM","AGME","AEGM","AEMG","AMGE","AMEG",
          "EGAM","EGMA","EAGM","EAMG","EMGA","EMAG","MGAE","MGEA","MAGE","MAEG","MEGA","MEAG"]
CH={}; ENC={}
for l in open(R+'charmap.txt',encoding='utf-8'):
    m=re.match(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$",l)
    if m: CH.setdefault(int(m.group(2),16),m.group(1)); ENC.setdefault(m.group(1),int(m.group(2),16))
def txt(b): return ''.join(CH.get(c,'?') for c in b.split(b'\xff')[0])
def enc(s): b=bytes(ENC[c] for c in s); return (b+b'\xff'*10)[:10]
e=Emu()
def decrypt(m):
    pid,otid=struct.unpack('<II',m[:8]); k=pid^otid
    raw=bytearray(b''.join(struct.pack('<I',struct.unpack('<I',m[32+j*4:36+j*4])[0]^k) for j in range(12)))
    o=ORDERS[pid%24]; return {c:raw[o.index(c)*12:o.index(c)*12+12] for c in 'GAEM'}
def encrypt(pid,otid,sub):
    o=ORDERS[pid%24]; raw=b''.join(bytes(sub[c]) for c in o); k=pid^otid
    return b''.join(struct.pack('<I',struct.unpack('<I',raw[j*4:j*4+4])[0]^k) for j in range(12))
mode=sys.argv[1]
if mode=='nature':
    slot=int(sys.argv[2]); nat=int(sys.argv[3]); base=list(map(int,sys.argv[4].split(',')))  # hp,atk,def,spe,spa,spd
    a=S['gPlayerParty']+slot*100; m=bytearray(e.peek(a,100))
    pid,otid=struct.unpack('<II',m[:8]); sub=decrypt(m)
    cs=sum(struct.unpack('<24H',b''.join(bytes(sub[c]) for c in 'GAEM')))&0xffff
    assert cs==struct.unpack('<H',m[0x1C:0x1E])[0],'checksum mismatch'
    tid=(otid&0xffff)^(otid>>16)
    rng=random.Random(1)
    while True:
        np_=(rng.getrandbits(24)<<8)|(pid&0xff)
        if np_%25==nat and (tid^(np_&0xffff)^(np_>>16))>=8: break
    ivw=struct.unpack('<I',sub['M'][4:8])[0]
    iv=[(ivw>>(5*i))&31 for i in range(6)]  # hp atk def spe spa spd
    ev=list(sub['E'][:6])                   # hp atk def spe spa spd
    lv=m[0x54]
    def st(i):
        v=(2*base[i]+iv[i]+ev[i]//4)*lv//100
        if i==0: return v+lv+10
        v+=5
        up,dn=nat//5+1,nat%5+1   # stat order atk def spe spa spd = 1..5
        if up!=dn:
            if i==up: v=v*110//100
            if i==dn: v=v*90//100
        return v
    stats=[st(i) for i in range(6)]
    m[0:4]=struct.pack('<I',np_); m[32:80]=encrypt(np_,otid,sub)
    m[0x56:0x58]=struct.pack('<H',stats[0])
    m[0x58:0x64]=struct.pack('<6H',stats[0],stats[1],stats[2],stats[3],stats[4],stats[5])
    e.send(f"poke {a:x}:{bytes(m).hex()}"); e.commit()
    print('pid',hex(pid),'->',hex(np_),'nature',np_%25,'ivs',iv,'evs',ev,'lv',lv,'stats',stats)
elif mode=='rename':
    stp=e.u32(S['gPokemonStoragePtr'])
    for b in range(14):
        for s in range(30):
            a=stp+4+(b*30+s)*80; m=e.peek(a,80)
            if struct.unpack('<I',m[:4])[0]==0: continue
            if txt(m[8:18])=='AAAAAAAAAA':
                sub=decrypt(m); sp=struct.unpack('<H',sub['G'][:2])[0]
                name=sys.argv[2+[x.split('=')[0] for x in sys.argv[2:]].index(str(sp))].split('=')[1]
                e.send(f"poke {a+8:x}:{enc(name).hex()}"); print('box',b+1,'slot',s,sp,'->',name)
    e.commit()
elif mode=='move':
    # move SLOT OLD NEW PP : replace move OLD by NEW (full PP, its PP-Up bits cleared) in party slot SLOT
    slot,old,new,pp=map(int,sys.argv[2:6])
    a=S['gPlayerParty']+slot*100; m=bytearray(e.peek(a,100))
    pid,otid=struct.unpack('<II',m[:8]); sub=decrypt(m)
    cs=sum(struct.unpack('<24H',b''.join(bytes(sub[c]) for c in 'GAEM')))&0xffff
    assert cs==struct.unpack('<H',m[0x1C:0x1E])[0],'checksum mismatch'
    mv=list(struct.unpack('<4H',bytes(sub['A'][0:8])))
    i=mv.index(old); mv[i]=new
    sub['A'][0:8]=struct.pack('<4H',*mv); sub['A'][8+i]=pp
    sub['G'][8]&=~(3<<(2*i)) & 0xff
    cs=sum(struct.unpack('<24H',b''.join(bytes(sub[c]) for c in 'GAEM')))&0xffff
    m[0x1C:0x1E]=struct.pack('<H',cs); m[32:80]=encrypt(pid,otid,sub)
    e.send(f"poke {a:x}:{bytes(m).hex()}"); e.commit(); print('slot',slot,'move',i,old,'->',new)
