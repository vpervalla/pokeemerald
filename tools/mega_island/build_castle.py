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
MW,MH=60,42
CX=30*16                    # gate axis (between cols 29 and 30)
c=Canvas(MW*16,MH*16)
import castle_facade
castle_facade.draw(c,MW,CX)
ground=[['grass']*MW for _ in range(MH)]
coll=[[0]*MW for _ in range(MH)]
layer=[[1]*MW for _ in range(MH)]         # 1 = COVERED (below player), 0 = NORMAL (above player)
behav=[[0]*MW for _ in range(MH)]         # metatile behaviors (0 = MB_NORMAL)
for x in (29,30):                        # the gate: stepping into it enters the stadium
    behav[11][x]=96                       # MB_NON_ANIMATED_DOOR
for y in range(0,12):                    # the castle
    for x in range(MW): coll[y][x]=1
for x in (23,24,25,34,35,36):            # gargoyles by the gate
    coll[12][x]=1
coll[11][29]=coll[11][30]=0                # the gate is walkable (it warps)
for y in range(12,15):                   # forecourt lawns, between the paths
    for x in list(range(17,24))+list(range(36,43)):
        if y>=13: ground[y][x]='lawn'
for x in (25,26,33,34):                  # obelisks at the top of the staircase
    coll[14][x]=1
for x in range(MW):                      # front parapet of the forecourt
    if not 28<=x<=31: coll[15][x]=1
for y in range(16,20):                   # bastion wall and moat, except the staircase and bridge
    for x in range(MW):
        if not 28<=x<=31: coll[y][x]=1
for y in (18,19):                        # the moat runs the whole width, into the forest on both sides
    for x in range(MW): ground[y][x]='water'
for y in range(13,18):                   # corner towers
    for x in (12,13,14,45,46,47):
        coll[y][x]=1
for x in (27,32):                        # balustrades
    for y in range(14,21): coll[y][x]=1
for x in (25,26,27,32,33,34):            # gargoyles at the end of the bridge
    coll[21][x]=1; coll[22][x]=1; layer[20][x]=0
# courtyard beyond the moat; flagstones stop at the fence (the path beyond is grass from the shared
# primary tileset, so the route shows this map's edge correctly)
for y in range(20,25):
    for x in range(28,32): ground[y][x]='flag'
for y in range(21,24):
    for x in range(22,38): ground[y][x]='flag'
FR=25
for x in range(16,44):
    if 27<=x<=32: continue
    fence(c,x*16,FR*16-8); coll[FR][x]=1; layer[FR-1][x]=0
for px in (27,32):
    gate_pillar(c,px*16,(FR-2)*16+8); coll[FR][px]=1; coll[FR-1][px]=1; layer[FR-2][px]=0
for (lx,ly) in ((23,23),(36,23),(27,29),(32,29)):
    lamp_post(c,lx*16,(ly-1)*16); coll[ly][lx]=1; layer[ly-1][lx]=0
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
# the wings disappear into the forest on both sides; the moat runs between the forests
tree_block(prim,0,8,12,10); tree_block(prim,48,8,12,10)
tree_block(prim,0,22,16,MH-22); tree_block(prim,44,22,16,MH-22)
tree_block(prim,16,32,12,MH-32); tree_block(prim,32,32,12,MH-32)
for y in range(MH):
    for x in range(MW):
        if prim[y][x] is not None:
            coll[y][x]=0 if prim[y][x] in (0xe,0xf) else 1

# ---- tileset building ----
import tilesetgen
B=tilesetgen.Builder()
tiles=B.tiles; metatiles=B.metatiles
cell_tiles=B.cell_tiles; tile_entry=B.tile_entry; metatile=B.metatile
EMPTY_TOP=tilesetgen.EMPTY_TOP
# own flagstone ground metatiles
fc=Canvas(32,16); pale_paving(fc,0,0,32,16)
FLAG=[]
for v in range(2):
    t=cell_tiles(fc,v*16,0)
    FLAG.append([tile_entry(*e) for e in t])
def ground_bottom(kind,x,y):
    if kind=='flag': return FLAG[(x+y)%2]
    if kind=='water': return list(PRIM.meta[0x12b][:4])
    if kind=='lawn': return list(PRIM.meta[random.choice(GRASS)][:4])
    m=random.choice(GRASS)
    return list(PRIM.meta[m][:4])
grid=[[0]*MW for _ in range(MH)]
HIDDEN=6   # rows 0-5 are above anything the camera can show: they repeat row 6
for y in range(HIDDEN,MH):
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
        attr=((layer[y][x]&0xf)<<12)|behav[y][x]
        grid[y][x]=metatile(bottom,top,attr)
for y in range(HIDDEN):
    grid[y]=list(grid[HIDDEN])
if os.environ.get('PREVIEW'):
    from PIL import Image
    img=Image.new('RGB',(MW*16,MH*16))
    for y in range(MH):
        for x in range(MW):
            m=prim[y][x] if prim[y][x] is not None else (0x12b if ground[y][x]=='water' else 8)
            pm=PRIM.meta[m]
            for i in range(4): PRIM.tile(img,pm[i],x*16+(i&1)*8,y*16+(i>>1)*8,False)
            if prim[y][x] is None and ground[y][x]=='flag':
                for i in range(4):
                    e=FLAG[(x+y)%2][i]
                    t=e&0x3ff; pal=PAL_A if (e>>12)==7 else PAL_B
                    T=tiles[t-640]; hf=e>>10&1; vf=e>>11&1
                    for yy in range(8):
                        for xx in range(8):
                            img.putpixel((x*16+(i&1)*8+xx,y*16+(i>>1)*8+yy),pal[T[7-yy if vf else yy][7-xx if hf else xx]])
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
B.check()
B.write_tileset(TSDIR)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle/',grid,coll,
    (0x1e|0xc00|0x3000,0x1f|0xc00|0x3000,0x16|0xc00|0x3000,0x17|0xc00|0x3000))
print('written')
