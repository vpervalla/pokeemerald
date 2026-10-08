# Builds the castle tileset (kanto_mega_castle) and the castle grounds map from the art.
import sys, struct, os, random
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from art import *
import render
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
TSDIR=REPO+'data/tilesets/secondary/kanto_mega_castle/'
PRIM=render.TS('kanto_general',None)
PRIM_ATTRS=[a for (a,) in struct.iter_unpack('<H',open(REPO+'data/tilesets/primary/kanto_general/metatile_attributes.bin','rb').read())]
random.seed(7)

# ---- compose the map art (indexed canvas the size of the map) ----
MW,MH=60,36
CX=30*16                    # gate axis (between cols 29 and 30)
c=Canvas(MW*16,MH*16)
import castle_facade
castle_facade.draw(c,MW,CX)
ground=[['grass']*MW for _ in range(MH)]
coll=[[0]*MW for _ in range(MH)]
layer=[[1]*MW for _ in range(MH)]         # 1 = COVERED (below player), 0 = NORMAL (above player)
for y in range(0,10):                    # the castle
    for x in range(MW): coll[y][x]=1
for y in (10,11):                        # terrace on the mound
    for x in range(MW): ground[y][x]='flag'
for y in range(12,16):                   # mound face, except the staircase
    for x in range(MW):
        if not 28<=x<=31: coll[y][x]=1
for x in (25,26,27,32,33,34):            # gargoyle pedestals at the foot of the staircase
    coll[15][x]=1; coll[16][x]=1
for x in (23,24,25,34,35,36):            # gargoyles on the terrace (the row below stays a walkway)
    coll[10][x]=1
for x in (27,32):                        # balustrades
    for y in (11,12,13,14,15): coll[y][x]=1
# courtyard: plaza at the foot of the mound, path south
# flagstones stop at the fence: the path beyond is grass from the shared primary tileset, so the
# route (which draws this map's edge with its own tileset) shows it correctly
for y in range(16,22):
    for x in range(28,32): ground[y][x]='flag'
for y in range(16,20):
    for x in range(22,38): ground[y][x]='flag'
FR=22
for x in range(14,46):
    if 27<=x<=32: continue
    fence(c,x*16,FR*16-8); coll[FR][x]=1; layer[FR-1][x]=0
for px in (27,32):
    gate_pillar(c,px*16,(FR-2)*16+8); coll[FR][px]=1; coll[FR-1][px]=1; layer[FR-2][px]=0
for (lx,ly) in ((26,18),(33,18),(27,26),(32,26)):
    lamp_post(c,lx*16,(ly-1)*16); coll[ly][lx]=1; layer[ly-1][lx]=0
for (tx,ty) in ((17,18),(20,20),(39,20),(42,18),(19,26),(40,26)):
    dead_tree(c,tx*16,(ty-2)*16)
    coll[ty][tx+1]=1
    for yy in (ty-2,ty-1):
        for xx in (tx,tx+1): layer[yy][xx]=0
c.outline()
OVER=c

# ---- primary pieces ----
GRASS=[0x8,0x8,0x8,0x9,0x10]
def tree_block(grid,x0,y0,w,h):
    # Forest of 2x2 trees (canopy 1e/1f, trunk 16/17 or 24/25), tips (e/f) in the row above.
    for ty in range(y0,y0+h,2):
        for tx in range(x0,x0+w,2):
            grid[ty][tx]=0x1e; grid[ty][tx+1]=0x1f
            last=(ty+2>=y0+h)
            grid[ty+1][tx]=0x24 if last else 0x16; grid[ty+1][tx+1]=0x25 if last else 0x17
            if ty==y0 and ty>0: grid[ty-1][tx]=0xe; grid[ty-1][tx+1]=0xf
prim=[[None]*MW for _ in range(MH)]
# the wings disappear into the forest on both sides; the forest closes in around the path
tree_block(prim,0,8,14,MH-8); tree_block(prim,46,8,14,MH-8)
tree_block(prim,14,28,14,MH-28); tree_block(prim,32,28,14,MH-28)
tree_block(prim,14,13,2,14); tree_block(prim,44,13,2,14)
for y in range(MH):
    for x in range(MW):
        if prim[y][x] is not None:
            coll[y][x]=0 if prim[y][x] in (0xe,0xf) else 1
            if prim[y][x] in (0xe,0xf) and y<16: coll[y][x]=1

# ---- tileset building ----
tiles=[]; tindex={}
def add_tile(px):   # px: 8 rows of 8 colour indices; returns (index, hflip, vflip)
    rows=[tuple(r) for r in px]
    cands=[(tuple(rows),0,0),(tuple(r[::-1] for r in rows),1,0),(tuple(rows[::-1]),0,1),(tuple(r[::-1] for r in rows[::-1]),1,1)]
    for k,h,v in cands:
        if k in tindex: return tindex[k],h,v
    tindex[cands[0][0]]=len(tiles); tiles.append(cands[0][0]); return len(tiles)-1,0,0
add_tile([[0]*8]*8)   # secondary tile 0 = transparent
SEC_PAL=7
def tile_entry(i,h,v,pal=SEC_PAL): return (640+i)|(h<<10)|(v<<11)|(pal<<12)
def cell_tiles(canvas,x0,y0):
    out=[]
    for ty in (0,8):
        for tx in (0,8):
            px=[[canvas.get(x0+tx+x,y0+ty+y) for x in range(8)] for y in range(8)]
            if all(p==0 for r in px for p in r): out.append(None)
            else: out.append(add_tile(px))
    return out
EMPTY_TOP=0  # primary tile 0 is blank
metatiles=[]; mindex={}; attrs=[]
def metatile(bottom,top,attr):
    key=(tuple(bottom),tuple(top),attr)
    if key in mindex: return mindex[key]
    mindex[key]=640+len(metatiles); metatiles.append(key); attrs.append(attr); return mindex[key]
# own flagstone ground metatiles
fc=Canvas(32,16); flagstone(fc,0,0,0); flagstone(fc,16,0,1)
FLAG=[]
for v in range(2):
    t=cell_tiles(fc,v*16,0)
    FLAG.append([tile_entry(*e) for e in t])
def ground_bottom(kind,x,y):
    if kind=='flag': return FLAG[(x+y)%2]
    m=random.choice(GRASS)
    return list(PRIM.meta[m][:4])
grid=[[0]*MW for _ in range(MH)]
for y in range(MH):
    for x in range(MW):
        has_art=any(OVER.get(x*16+i,y*16+j) for i in range(16) for j in range(16))
        if prim[y][x] is not None and not has_art:
            grid[y][x]=prim[y][x]; continue
        if prim[y][x] is not None:
            pm=PRIM.meta[prim[y][x]]
            if any(t&0x3ff for t in pm[4:]):
                # tree tip or canopy in front of the castle: castle art below, the tree's top layer above
                art=cell_tiles(OVER,x*16,y*16)
                bottom=[tile_entry(*e) if e else EMPTY_TOP for e in art]
                grid[y][x]=metatile(bottom,list(pm[4:]),PRIM_ATTRS[prim[y][x]]); continue
            grid[y][x]=prim[y][x]; continue   # trunk rows are opaque: the forest hides the castle
        bottom=ground_bottom(ground[y][x],x,y)
        ov=cell_tiles(OVER,x*16,y*16)
        if all(e is None for e in ov):
            if ground[y][x]=='grass' and prim[y][x] is None:
                grid[y][x]=random.choice(GRASS); continue
            grid[y][x]=metatile(bottom,[EMPTY_TOP]*4,0); continue
        top=[tile_entry(*e) if e else EMPTY_TOP for e in ov]
        attr=(layer[y][x]&0xf)<<12   # behavior MB_NORMAL
        grid[y][x]=metatile(bottom,top,attr)
print('secondary tiles',len(tiles),'metatiles',len(metatiles))
if os.environ.get('PREVIEW'):
    from PIL import Image
    img=Image.new('RGB',(MW*16,MH*16))
    for y in range(MH):
        for x in range(MW):
            m=prim[y][x] if prim[y][x] is not None else 8
            pm=PRIM.meta[m]
            for i in range(4): PRIM.tile(img,pm[i],x*16+(i&1)*8,y*16+(i>>1)*8,False)
            if prim[y][x] is None and ground[y][x]=='flag':
                for i in range(4):
                    e=FLAG[(x+y)%2][i]
                    t=e&0x3ff; pal=PAL
                    T=tiles[t-640]; hf=e>>10&1; vf=e>>11&1
                    for yy in range(8):
                        for xx in range(8):
                            img.putpixel((x*16+(i&1)*8+xx,y*16+(i>>1)*8+yy),PAL[T[7-yy if vf else yy][7-xx if hf else xx]])
    art=OVER.img(); img.paste(art,(0,0),art)
    for y in range(MH):
        for x in range(MW):
            m=prim[y][x]
            if m is None: continue
            pm=PRIM.meta[m]
            if any(t&0x3ff for t in pm[4:]):
                for i in range(4): PRIM.tile(img,pm[4+i],x*16+(i&1)*8,y*16+(i>>1)*8,True)
            else:
                for i in range(4): PRIM.tile(img,pm[i],x*16+(i&1)*8,y*16+(i>>1)*8,False)
    img.save(os.environ['PREVIEW']); sys.exit(0)
assert len(tiles)<=384 and len(metatiles)<=384

# ---- write tileset ----
os.makedirs(TSDIR+'palettes',exist_ok=True)
from PIL import Image
n=len(tiles); rows_=(n+15)//16
im=Image.new('P',(128,rows_*8),0)
im.putpalette([v for col in PAL for v in col])
for i,t in enumerate(tiles):
    for y in range(8):
        for x in range(8): im.putpixel(((i%16)*8+x,(i//16)*8+y),t[y][x])
im.save(TSDIR+'tiles.png')
def write_pal(path,cols):
    with open(path,'w',newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n16\n'+''.join('%d %d %d\n'%c for c in cols))
for p in range(16):
    write_pal(TSDIR+'palettes/%02d.pal'%p, PAL if p==SEC_PAL else [(0,0,0)]*16)
with open(TSDIR+'metatiles.bin','wb') as f:
    for b,t,a in metatiles: f.write(struct.pack('<8H',*b,*t))
with open(TSDIR+'metatile_attributes.bin','wb') as f:
    for a in attrs: f.write(struct.pack('<H',a))
# ---- write layout ----
LD=REPO+'data/layouts/MegaIsland_Castle/'
os.makedirs(LD,exist_ok=True)
with open(LD+'map.bin','wb') as f:
    for y in range(MH):
        for x in range(MW): f.write(struct.pack('<H',grid[y][x]|(coll[y][x]<<10)|(3<<12)))
with open(LD+'border.bin','wb') as f:
    f.write(struct.pack('<4H',0x1e|0xc00|0x3000,0x1f|0xc00|0x3000,0x16|0xc00|0x3000,0x17|0xc00|0x3000))
print('written')
