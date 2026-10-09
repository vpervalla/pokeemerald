# Builds the stadium tileset (kanto_mega_arena) and the inner courtyard map from arena.py.
import sys, os, random
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from art import *
import render, tilesetgen, arena
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
PRIM=render.TS('kanto_general',None)
random.seed(5)
MW,MH=arena.MW,arena.MH
c=Canvas(MW*16,MH*16)
arena.draw(c)
c.outline()
coll=[[1]*MW for _ in range(MH)]
layer=[[1]*MW for _ in range(MH)]
behav=[[0]*MW for _ in range(MH)]
for y in range(12,25):                  # the walkway and the field
    for x in range(5,27): coll[y][x]=0
for (x,y) in ((5,12),(26,12),(5,24),(26,24)): coll[y][x]=1    # braziers
for (x,y) in ((5,11),(26,11),(5,23),(26,23)): layer[y][x]=0   # their flames are above the player
for y in range(25,30):                  # the entrance passage
    for x in (15,16): coll[y][x]=0
for x in (15,16): behav[29][x]=101      # MB_SOUTH_ARROW_WARP: walk south to leave
B=tilesetgen.Builder()
HIDDEN=5   # the camera never shows rows 0-4 (the walkway starts at row 12)
grid=[[0]*MW for _ in range(MH)]
for y in range(HIDDEN,MH):
    for x in range(MW):
        top=B.entries(c,x*16,y*16)
        bottom=list(PRIM.meta[0x8][:4])            # grass under anything transparent
        grid[y][x]=B.metatile(bottom,top,((layer[y][x]&0xf)<<12)|behav[y][x])
for y in range(HIDDEN): grid[y]=list(grid[HIDDEN])
B.check()
if os.environ.get('PREVIEW'):
    from PIL import Image
    img=Image.new('RGB',(MW*16,MH*16))
    for y in range(MH):
        for x in range(MW):
            for i in range(4): PRIM.tile(img,PRIM.meta[8][i],x*16+(i&1)*8,y*16+(i>>1)*8,False)
    art=c.img(); img.paste(art,(0,0),art); img.save(os.environ['PREVIEW']); sys.exit(0)
B.write_tileset(REPO+'data/tilesets/secondary/kanto_mega_arena/')
wall=grid[HIDDEN][0]
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_Arena/',grid,coll,(wall|0xc00|0x3000,)*4)
print('written')
