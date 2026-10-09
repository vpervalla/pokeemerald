# Builds the castle interior tileset (kanto_mega_hall) and the entrance hall (1F), the guest
# corridor (2F) and the guest room layouts from hall.py.
import sys, os
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from art import *
import tilesetgen, hall
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
MB_NON_ANIMATED_DOOR,MB_EAST_ARROW_WARP,MB_WEST_ARROW_WARP,MB_NORTH_ARROW_WARP,MB_SOUTH_ARROW_WARP=96,98,99,100,101
B=tilesetgen.Builder()

def build(canvas,MW,MH,coll,behav):
    grid=[[0]*MW for _ in range(MH)]
    for y in range(MH):
        for x in range(MW):
            grid[y][x]=B.metatile(B.entries(canvas,x*16,y*16),[tilesetgen.EMPTY_TOP]*4,behav[y][x])
    return grid
def blocked(W,H): return [[0]*W for _ in range(H)],[[0]*W for _ in range(H)]

# ---- 1F: the entrance hall ----
W1,H1=hall.H1W,hall.H1H
c1=Canvas(W1*16,H1*16); hall.draw_hall(c1)
coll1,beh1=blocked(W1,H1)
for y in range(H1):
    for x in range(W1):
        if y<2 or x in (0,W1-1): coll1[y][x]=1                # north wall, side walls
for x in (12,13): coll1[1][x]=0; beh1[1][x]=MB_NON_ANIMATED_DOOR   # the door to the court
for x in (1,2,3,22,23,24): beh1[2][x]=MB_NORTH_ARROW_WARP    # top steps: up to the 2F
for y in range(2,7):
    for x in (4,21): coll1[y][x]=1                           # balustrades
for y in (5,6):
    for x in range(9,17): coll1[y][x]=1                      # reception desk
for (x,y) in ((6,4),(19,4),(6,10),(19,10)):
    coll1[y][x]=coll1[y+1][x]=1                              # pillars
for x in (12,13): beh1[H1-1][x]=MB_SOUTH_ARROW_WARP          # doormat: out to the castle gate
g1=build(c1,W1,H1,coll1,beh1)

# ---- 2F: the guest corridor ----
W2,H2=hall.H2W,hall.H2H
c2=Canvas(W2*16,H2*16); hall.draw_corridor(c2)
coll2,beh2=blocked(W2,H2)
for y in range(H2):
    for x in range(W2):
        if y<2 or x in (0,W2-1) or y==H2-1: coll2[y][x]=1
DOORS=(4,7,10,15,18,21)
for x in DOORS: coll2[1][x]=0; beh2[1][x]=MB_NON_ANIMATED_DOOR   # guest room doors
for y in (3,4,5,6):                                           # stairs down at both ends
    beh2[y][1]=MB_WEST_ARROW_WARP; beh2[y][W2-2]=MB_EAST_ARROW_WARP
g2=build(c2,W2,H2,coll2,beh2)

# ---- a guest room (one layout for all the rooms) ----
W3,H3=hall.RW,hall.RH
c3=Canvas(W3*16,H3*16); hall.draw_room(c3)
coll3,beh3=blocked(W3,H3)
for y in range(H3):
    for x in range(W3):
        if y<2 or x in (0,W3-1) or y==H3-1: coll3[y][x]=1
for (x,y) in ((2,2),(2,3),(3,2),(8,1),(9,1),(8,2),(9,2),(7,6),(8,6),(9,6)):
    coll3[y][x]=1                                             # bed, nightstand, wardrobe, table
beh3[H3-2][5]=MB_SOUTH_ARROW_WARP                            # doormat: back to the corridor
g3=build(c3,W3,H3,coll3,beh3)

blk=Canvas(16,16); blk.rect(0,0,16,16,OUT)        # black void outside the maps
void=B.metatile(B.entries(blk,0,0),[tilesetgen.EMPTY_TOP]*4,0)|0xc00|0x3000
if os.environ.get('PREVIEW'):
    from PIL import Image
    ims=[cv.img() for cv in (c1,c2,c3)]
    img=Image.new('RGBA',(sum(i.width for i in ims)+32,max(i.height for i in ims)),(0,0,0,255)); x=0
    for i in ims: img.paste(i,(x,0),i); x+=i.width+16
    img.save(os.environ['PREVIEW']); sys.exit(0)
B.check()
B.write_tileset(REPO+'data/tilesets/secondary/kanto_mega_hall/')
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle_1F/',g1,coll1,(void,)*4)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle_2F/',g2,coll2,(void,)*4)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Castle_GuestRoom/',g3,coll3,(void,)*4)
print('written')
