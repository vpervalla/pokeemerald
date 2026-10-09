# Builds the island route (MegaIsland), after the user's reference map: from the castle (north), a
# grass plateau leads to two plank bridges over a rock band and a river; below, a meadow with tall
# grass, the memorial pillar, a rock outcrop with a cave, a rest house, terraces with stairs and woods;
# at the bottom, the gate to the port. Also writes the port (the gate's south side, the harbor
# building and the pier from Five Island) and the small cave. Rock and water are autotiled with
# the rules FRLG's own maps use (autotile.py). Tilesets: kanto_general + kanto_sevii_islands_45.
import struct, random, os, json
import autotile
random.seed(11)
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
W,H=30,58
WATER,TALL,FLOWER,BUSH,POST=0x12b,0xd,0x4,0x5,0xde
GRASS=[0x8,0x8,0x9,0x10]
g=[[None]*W for _ in range(H)]; col=[[0]*W for _ in range(H)]
def put(x,y,m,c=0):
    if 0<=x<W and 0<=y<H: g[y][x]=m; col[y][x]=c
# ---- the design: R rock, W water (the river and the sea), = plank bridge, , tall grass, . grass
D=[
"RRRRRR,,,,,,.....,,,,,,.RRRRRR",  # 0  the path from the castle comes down cols 13-16
"RRRRRR,,,,,,.....,,,,,,.RRRRRR",  # 1
"RR.....,,,,......,,,,,..RRRRRR",  # 2
"RR...............,,,,...RRRRRR",  # 3
"RR......RRRR.........RRRRRRRRR",  # 4
"RR......RRRRR........RRRRRRRRR",  # 5
"RR.==...RRRRR.====...RRRRRRRRR",  # 6
"RRR==RRRRRRRRR====RRRRRRRRRRRR",  # 7  the rock band, cut by the bridges
"RRR==RRRRRRRRR====RRRRRRRRRRRR",  # 8
"RRR==RRRRRRRRR====RRRRRRRRRRRR",  # 9
"RRR==RRRRRRRRR====RRRRRRRRRRRR",  # 10
"RRR==RRRRRRRRR====RRRRRRRRRRRR",  # 11
"WWW==WWWWWWWWW====WWWWWWWWWWWW",  # 12 the river
"WWW==WWWWWWWWW====WWWWWWWWWWWW",  # 13
"WWW==WWWWWWWWW====WWWWWWWWWWWW",  # 14
"WWW==WWWWWWWWW====WWWWWWWWWWWW",  # 15
"..............................",  # 16 the south bank
"..............................",  # 17
"RRRRR.........................",  # 18
"RRRRRR...,,,,,,,,...........RR",  # 19
"RRRRRRR..,,,,,,,,,..........RR",  # 20
"RRRRRRR..,,,,,,,,,.........RRR",  # 21
"RRRRRRR..,,,,,,,,,,........RRR",  # 22
"RRRRRR...,,,,,,,,,,........RRR",  # 23
"RRRRRR....,,,,,,,,,,.......RRR",  # 24
"RRRRR.....,,,,,,,,,,.......RRR",  # 25
"RRRRR......,,,,,,,,,.......RRR",  # 26
"RRRR...........,,,,...........",  # 27
"..............................",  # 28
"..............................",  # 29
"..............................",  # 30
"...................,,,,,,,....",  # 31
"..................,,,,,,,,,...",  # 32
"..................,,,,,,,,,...",  # 33
"..................,,,,,,,,,...",  # 34
"...............,,,,,,,,,,.....",  # 35
"...............,,,,,,,,,,.....",  # 36
"..............................",  # 37
"...........,,,,,,.............",  # 38
"...........,,,,,,,,......,,,..",  # 39
"...........,,,,,,,,......,,,,.",  # 40
"............,,,,,,.......,,,,.",  # 41
"..............................",  # 42
"..............................",  # 43
"..............................",  # 44
"..............................",  # 45
"..............................",  # 46
"..............................",  # 47
"..............................",  # 48
"..............................",  # 49
"..............................",  # 50
"..............................",  # 51
"..............................",  # 52
"..............................",  # 53 the gate to the harbor (rows 53-57)
"..............................",  # 54
"..............................",  # 55
"..............................",  # 56
"..............................",  # 57
]
assert len(D)==H and all(len(r)==W for r in D), [len(r) for r in D]
for y in range(H):
    for x in range(W):
        ch=D[y][x]
        if ch=='.': put(x,y,random.choice(GRASS))
        elif ch==',': put(x,y,TALL)
        elif ch=='=': put(x,y,0x1be)
rock_tab,_=autotile.learn(autotile.ROCK,autotile.MAPS)
water_tab,_=autotile.learn(autotile.WATER,autotile.MAPS)
for (x,y),m in autotile.fill(D,'R',rock_tab).items(): put(x,y,m,1)
for (x,y),m in autotile.fill(D,'W',water_tab).items(): put(x,y,m,0)
# plank bridges: a lit edge on the planks' sides
for y in range(H):
    for x in range(W):
        if D[y][x]=='=':
            l=D[y][x-1]!='='; r=D[y][x+1]!='='
            put(x,y,0x1bd if l else (0x1bf if r else 0x1be))
# wooden posts along the river's south bank, open at the bridges
for x in range(W):
    if D[15][x]=='W' and x not in (2,5,12,18): put(x,16,POST,1)
# trees: canopy 1e/1f, trunks 16/17 (with a tree below) or 24/25, tips e/f in the row above
def tree(x,y,below=False):
    put(x,y,0x1e,1); put(x+1,y,0x1f,1)
    put(x,y+1,0x16 if below else 0x24,1); put(x+1,y+1,0x17 if below else 0x25,1)
def lone_tree(x,y):
    put(x,y,0xe); put(x+1,y,0xf); tree(x,y+1)
def forest(x0,y0,w,h):
    for ty in range(y0,y0+h,2):
        for tx in range(x0,x0+w,2): tree(tx,ty,below=(ty+2<y0+h))
    for tx in range(x0,x0+w,2): put(tx,y0-1,0xe); put(tx+1,y0-1,0xf)
forest(0,40,10,18); forest(26,35,4,6); forest(20,49,10,9)
for (tx,ty) in ((22,0),(19,4),(8,13+5),(19,26),(25,21),(15,30),(6,33),(21,40)):
    lone_tree(tx,ty)
# flowers and bushes
for (x,y) in ((7,3),(16,3),(10,17),(20,17),(25,18),(8,27),(20,28),(4,30),(9,35),(14,38),(25,42),(19,44),(11,47),(18,50)):
    put(x,y,FLOWER)
for (x,y) in ((6,2),(12,2),(22,16),(17,27),(2,35),(23,38)):
    put(x,y,BUSH,1)
# ---- terraces: the meadow steps down from the river (level 3) to the gate (level 0). A cell on
# the high side of a drop becomes a rock wall one tile thick, as in Five Isle Meadow; stairs cut
# through the south faces.
LV={}
def levels(y0,y1,row):
    for y in range(y0,y1+1):
        for x in range(W): LV[(x,y)]=int(row[x])
levels(16,28,"333333333333333333333333333333")
levels(29,31,"333333333322222222222333333333")
levels(32,33,"222222222222222222222333333333")
levels(34,37,"222222222222222222222222222222")
levels(38,42,"111111111111222222222222222222")
levels(43,47,"111111111111111111111111111111")
levels(48,57,"111111111111000000000000000000")
def lv(x,y): return LV.get((x,y),LV.get((min(max(x,0),W-1),min(max(y,16),57)),3))
for (x,y),h in list(LV.items()):
    lowS=lv(x,y+1)<h; lowW=lv(x-1,y)<h; lowE=lv(x+1,y)<h
    m=None
    if lowS and lowW: m=0x78
    elif lowS and lowE: m=0x7a
    elif lowS: m=0x79
    elif lowW: m=0x70
    elif lowE: m=0x72
    elif lv(x+1,y+1)<h: m=0xb2
    elif lv(x-1,y+1)<h: m=0xb3
    if m is not None: put(x,y,m,1)
def stairs(x,y): put(x,y,0x91,0); put(x+1,y,0x89,0)
for (x,y) in ((14,28),(4,31),(24,33),(6,37),(14,42),(14,47),(22,42)):
    stairs(x,y)
# copied blocks: the memorial pillar (Memorial Pillar map) and a house (Four Island)
def copy_block(layout,sx,sy,w,h,dx,dy):
    L={l['id']:l for l in json.load(open(REPO+'data/layouts/layouts.json'))['layouts'] if 'id' in l}[layout]
    d=[v for (v,) in struct.iter_unpack('<H',open(REPO+L['blockdata_filepath'],'rb').read())]
    for j in range(h):
        for i in range(w):
            v=d[(sy+j)*L['width']+sx+i]; put(dx+i,dy+j,v&0x3ff,(v>>10)&3)
copy_block('LAYOUT_FIVE_ISLAND_MEMORIAL_PILLAR',7,40,4,5,21,20)
copy_block('LAYOUT_FOUR_ISLAND',11,10,4,4,10,31)
# the cave mouth, in the outcrop's south face (as Mt. Moon's on Route 4)
put(2,27,0xa9,0)
# ---- the gate to the harbor, seen from the north (as on Route 5): its doors at (14,55), (15,55) ----
copy_block('LAYOUT_ROUTE5',22,30,6,5,12,53)
for (tx,ty) in ((10,51),(18,51)):
    lone_tree(tx,ty)
LD=REPO+'data/layouts/MegaIsland/'
os.makedirs(LD,exist_ok=True)
with open(LD+'map.bin','wb') as f:
    for y in range(H):
        for x in range(W): f.write(struct.pack('<H',g[y][x]|(col[y][x]<<10)|(3<<12)))
with open(LD+'border.bin','wb') as f:     # trees all around
    f.write(struct.pack('<4H',0x1e|0xc00|0x3000,0x1f|0xc00|0x3000,0x16|0xc00|0x3000,0x17|0xc00|0x3000))
print('route written',W,H)

# ---- the port: the gate's south side (as on Route 6), the harbor building and the pier ----
PW,PH=30,20
g=[[None]*PW for _ in range(PH)]; col=[[0]*PW for _ in range(PH)]
W,H=PW,PH
for y in range(PH):
    for x in range(PW): put(x,y,random.choice(GRASS))
forest(0,0,12,8); forest(18,0,12,8)
copy_block('LAYOUT_ROUTE6',10,0,6,7,12,0)       # its doors at (14,5), (15,5)
for (x,y) in ((11,8),(19,9),(7,9)): put(x,y,FLOWER)
for x in range(PW):
    put(x,10,0x123,1)
    for y in range(11,PH): put(x,y,WATER if y<17 else 0x1d9)
fd=list(struct.iter_unpack('<H',open(REPO+'data/layouts/FiveIsland/map.bin','rb').read()))
for dy in range(0,7):                              # the harbor building's door at (15,11)
    for dx in range(0,7):
        v=fd[(13+dy)*24+9+dx][0]
        if dy==0 and (v&0x3ff)==0x123: continue
        put(12+dx,10+dy,v&0x3ff,(v>>10)&3)
LD=REPO+'data/layouts/MegaIsland_Port/'
os.makedirs(LD,exist_ok=True)
with open(LD+'map.bin','wb') as f:
    for y in range(PH):
        for x in range(PW): f.write(struct.pack('<H',g[y][x]|(col[y][x]<<10)|(3<<12)))
with open(LD+'border.bin','wb') as f:
    f.write(struct.pack('<4H',*([WATER|0x3000]*4)))
print('port written',PW,PH)

# ---- the small cave: the Lost Cave's first room, without its ladder ----
L={l['id']:l for l in json.load(open(REPO+'data/layouts/layouts.json'))['layouts'] if 'id' in l}['LAYOUT_FIVE_ISLAND_LOST_CAVE_ENTRANCE']
d=[v for (v,) in struct.iter_unpack('<H',open(REPO+L['blockdata_filepath'],'rb').read())]
d[5*11+5]=d[4*11+5]                                 # floor where the ladder was
LD=REPO+'data/layouts/MegaIsland_Cave/'
os.makedirs(LD,exist_ok=True)
open(LD+'map.bin','wb').write(b''.join(struct.pack('<H',v) for v in d))
open(LD+'border.bin','wb').write(open(REPO+L['border_filepath'],'rb').read())
print('cave written')
