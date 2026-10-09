# Builds the castle interior tileset (kanto_mega_hall) and the entrance hall (1F) and guest
# floor (2F) layouts from hall.py.
import sys, os
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from art import *
import render, tilesetgen, hall
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
MB_NON_ANIMATED_DOOR,MB_EAST_ARROW_WARP,MB_WEST_ARROW_WARP,MB_NORTH_ARROW_WARP,MB_SOUTH_ARROW_WARP=96,98,99,100,101
B=tilesetgen.Builder()

def build(canvas,MW,MH,coll,behav):
    grid=[[0]*MW for _ in range(MH)]
    for y in range(MH):
        for x in range(MW):
            grid[y][x]=B.metatile(B.entries(canvas,x*16,y*16),[tilesetgen.EMPTY_TOP]*4,behav[y][x])
    return grid

# ---- 1F: the entrance hall ----
W1,H1=hall.H1W,hall.H1H
c1=Canvas(W1*16,H1*16); hall.draw_hall(c1)
coll1=[[0]*W1 for _ in range(H1)]; beh1=[[0]*W1 for _ in range(H1)]
for y in range(H1):
    for x in range(W1):
        if y<4 or x in (0,W1-1): coll1[y][x]=1                # north wall, side walls
for x in (12,13): coll1[3][x]=0; beh1[3][x]=MB_NON_ANIMATED_DOOR   # the door to the court
for y in range(4,10):
    for x in (4,21): coll1[y][x]=1                           # balustrades
for x in (1,2,3,22,23,24): beh1[4][x]=MB_NORTH_ARROW_WARP    # top steps: up to the 2F
for y in (7,8):
    for x in range(9,17): coll1[y][x]=1                      # reception desk
for (x,y) in ((6,5),(19,5),(6,13),(19,13)):
    coll1[y][x]=coll1[y+1][x]=1                              # pillars
for x in (12,13): beh1[H1-1][x]=MB_SOUTH_ARROW_WARP          # doormat: out to the castle gate
g1=build(c1,W1,H1,coll1,beh1)

# ---- 2F: the guest floor ----
W2,H2=hall.H2W,hall.H2H
c2=Canvas(W2*16,H2*16); hall.draw_guest_floor(c2)
coll2=[[1]*W2 for _ in range(H2)]; beh2=[[0]*W2 for _ in range(H2)]
for (x0,x1) in hall.ROOM_COLS:
    for (y0,dy) in ((2,(7,8)),(14,(12,13))):
        for y in range(y0,y0+5):
            for x in range(x0,x1): coll2[y][x]=0
        for (x,y) in ((x0+1,y0),(x0+1,y0+1),(x0+2,y0),(x1-3,y0),(x1-2,y0),(x1-3,y0+1),(x1-2,y0+1),(x1-2,y0+4)):
            coll2[y][x]=1                                    # bed, nightstand, wardrobe, table
        door=(x0+x1)//2
        for y in dy: coll2[y][door]=0                        # doorway
for y in (9,10,11):
    for x in range(1,W2-1): coll2[y][x]=0                    # corridor and stairwells
    beh2[y][1]=MB_WEST_ARROW_WARP; beh2[y][W2-2]=MB_EAST_ARROW_WARP   # down to the 1F
g2=build(c2,W2,H2,coll2,beh2)

if os.environ.get('PREVIEW'):
    from PIL import Image
    a=c1.img(); b=c2.img()
    img=Image.new('RGBA',(a.width+16+b.width,max(a.height,b.height)),(0,0,0,255))
    img.paste(a,(0,0),a); img.paste(b,(a.width+16,0),b); img.save(os.environ['PREVIEW']); sys.exit(0)
blk=Canvas(16,16); blk.rect(0,0,16,16,OUT)        # black void outside the maps
wall=B.metatile(B.entries(blk,0,0),[tilesetgen.EMPTY_TOP]*4,0)|0xc00|0x3000
B.check()
B.write_tileset(REPO+'data/tilesets/secondary/kanto_mega_hall/')
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle_1F/',g1,coll1,(wall,)*4)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle_2F/',g2,coll2,(wall,)*4)
print('written')
