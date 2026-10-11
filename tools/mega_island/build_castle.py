# Builds the castle tileset (kanto_mega_castle) and the castle grounds map, after the user's
# reference map: a Japanese castle in a walled compound, a cobbled plaza with banners, and a torii
# as the gate in the front wall. The art is in jcastle.py.
import sys, struct, os, random
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from art import Canvas
from jcastle import *
import render, autotile, tilesetgen
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
TSDIR=REPO+'data/tilesets/secondary/kanto_mega_castle/'
PRIM=render.TS('kanto_general',None)
PRIM_ATTRS=[a for (a,) in struct.iter_unpack('<H',open(REPO+'data/tilesets/primary/kanto_general/metatile_attributes.bin','rb').read())]
random.seed(7)

MW,MH=60,40
CX=30*16              # the gate axis, between cols 29 and 30: the route below lines up with it
CY0=32                # the castle's top (row 2); its bottom is the bottom of row 16
WALL_Y=30             # the front wall (rows 30-31) with the torii; the back wall is rows 6-7
WL,WR=12,47           # the side walls' columns

coll=[[0]*MW for _ in range(MH)]
layer=[[1]*MW for _ in range(MH)]          # 1 = COVERED (below the player), 0 = NORMAL (above)
behav=[[0]*MW for _ in range(MH)]          # metatile behaviors (0 = MB_NORMAL)
def block(x0,y0,x1,y1):
    for y in range(y0,y1+1):
        for x in range(x0,x1+1): coll[y][x]=1
def above(x,y0,y1):                        # rows the player walks behind
    for y in range(y0,y1+1): layer[y][x]=0

# ---- the art ----
GROUND=Canvas(MW*16,MH*16)                 # the plaza: no outline, it lies flat on the grass
cells={(x,y) for y in range(17,25) for x in range(24,36)}
cells|={(x,y) for y in range(25,34) for x in range(28,32)}
plaza(GROUND,cells)

c=Canvas(MW*16,MH*16)
wall_v(c,WL*16+3,6*16,(WALL_Y+1)*16); wall_v(c,WR*16+3,6*16,(WALL_Y+1)*16)
wall_h(c,WL*16+3,WR*16+13,6*16)             # the back wall
block(WL,6,WR,7); block(WL,6,WL,WALL_Y+1); block(WR,6,WR,WALL_Y+1)
castle(c,CX,CY0)
block(24,2,35,16)
for x in (29,30):                          # the door leads into the entrance hall
    behav[16][x]=96                        # MB_NON_ANIMATED_DOOR
    coll[16][x]=0
wall_h(c,WL*16+3,27*16,WALL_Y*16); wall_h(c,33*16,WR*16+13,WALL_Y*16)
block(WL,WALL_Y,27,WALL_Y+1); block(32,WALL_Y,WR,WALL_Y+1)
torii(c,CX,WALL_Y*16-30,(WALL_Y+2)*16)
for x in range(26,34): above(x,WALL_Y-2,WALL_Y-1)
block(27,WALL_Y-2,27,WALL_Y-1); block(32,WALL_Y-2,32,WALL_Y-1)
# komainu by the castle door and outside the gate
for (x,yb) in ((27,18),(32,18),(27,33),(32,33)):
    komainu(c,x*16,(yb+1)*16); block(x,yb,x,yb)
    if yb==33: block(x,32,x,32)
# banners along the approach, in pairs (the pole on the outer side)
for (x,yb) in ((25,19),(25,23),(27,27)):
    for xx,flip in ((x,False),(59-x,True)):
        nobori(c,xx*16,(yb+1)*16,flip); block(xx,yb,xx,yb); above(xx,yb-2,yb-1)
# stone lanterns in the gardens
for (x,yb) in ((17,12),(21,19),(17,26)):
    for xx in (x,59-x):
        toro(c,xx*16,(yb+1)*16); block(xx,yb,xx,yb); above(xx,yb-1,yb-1)
c.outline()
OVER=Canvas(MW*16,MH*16)
OVER.blit(GROUND,0,0); OVER.blit(c,0,0)

# ---- primary pieces: the moat behind the castle, trees, flowers ----
GRASS=[0x8,0x8,0x8,0x9,0x10]
FLOWER,BUSH=0x4,0x5
prim=[[None]*MW for _ in range(MH)]
D=['W'*MW if y<6 else '.'*MW for y in range(MH)]
for (x,y),m in autotile.fill(D,'W',autotile.learn(autotile.WATER,autotile.MAPS)[0]).items():
    prim[y][x]=m; coll[y][x]=1
def tree_block(x0,y0,w,h):
    # 2x2 trees (canopy 1e/1f, trunk 16/17 or 24/25), tips (e/f) in the row above.
    for ty in range(y0,y0+h,2):
        for tx in range(x0,x0+w,2):
            prim[ty][tx]=0x1e; prim[ty][tx+1]=0x1f
            last=ty+2>=y0+h
            prim[ty+1][tx]=0x24 if last else 0x16; prim[ty+1][tx+1]=0x25 if last else 0x17
            block(tx,ty,tx+1,ty+1)
            if ty==y0: prim[ty-1][tx]=0xe; prim[ty-1][tx+1]=0xf
tree_block(0,8,WL,MH-8); tree_block(WR+1,8,MW-WR-1,MH-8)          # the forest around the walls
tree_block(WL,34,10,6); tree_block(38,34,10,6)                    # and along the road south
for (x,y,w,h) in ((13,9,4,4),(20,9,2,2),(13,21,2,4),(21,23,2,2)):  # the gardens
    tree_block(x,y,w,h); tree_block(59-x-w+1,y,w,h)
for (x,y) in ((19,11),(14,15),(22,14),(15,19),(19,25),(22,28),(14,28)):
    prim[y][x]=FLOWER; prim[y][59-x]=FLOWER
for (x,y) in ((16,16),(20,16),(18,22)):
    prim[y][x]=BUSH; prim[y][59-x]=BUSH; block(x,y,x,y); block(59-x,y,59-x,y)

# ---- the tileset and layout ----
B=tilesetgen.Builder(((7,PAL_J),(8,PAL_K)),PAL)
GRASS_BOTTOM=list(PRIM.meta[0x8][:4])
grid=[[0]*MW for _ in range(MH)]
for y in range(MH):
    for x in range(MW):
        art=B.cell_tiles(OVER,x*16,y*16)
        p=prim[y][x]
        if all(e is None for e in art):
            grid[y][x]=p if p is not None else random.choice(GRASS); continue
        top=[B.tile_entry(*e) if e else tilesetgen.EMPTY_TOP for e in art]
        if p is not None:
            pm=PRIM.meta[p]
            assert not any(t&0x3ff for t in pm[4:]), ('art over a primary top layer',x,y)
            bottom=list(pm[:4])                 # the moat's water under the castle's roof
        else:
            bottom=GRASS_BOTTOM
        grid[y][x]=B.metatile(bottom,top,(layer[y][x]<<12)|behav[y][x])
B.check()
B.write_tileset(TSDIR)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle/',grid,coll,
    (0x1e|0xc00|0x3000,0x1f|0xc00|0x3000,0x16|0xc00|0x3000,0x17|0xc00|0x3000))
print('written')
if os.environ.get('PREVIEW'):
    render.render_layout('LAYOUT_MEGA_ISLAND_CASTLE',os.environ['PREVIEW'])
