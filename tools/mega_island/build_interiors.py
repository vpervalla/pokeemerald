# Builds the castle's interiors from interior.py: the entrance hall with the registration desk (1F),
# the guest floor (2F), the guest room and the arena (3F). The hall, the guest floor and the room
# share kanto_mega_hall; the arena has kanto_mega_arena. PREVIEW=out.png renders the four maps
# instead of writing them.
import sys, os
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from interior import *
import tilesetgen
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
MB_NON_ANIMATED_DOOR,MB_EAST_ARROW_WARP,MB_WEST_ARROW_WARP,MB_NORTH_ARROW_WARP,MB_SOUTH_ARROW_WARP,MB_COUNTER=96,98,99,100,101,128

def shell(m):
    # Collision of a room's shell: the north wall, the side walls and the south wall.
    coll=[[1 if (y<2 or x in (0,m.w-1) or y==m.h-1) else 0 for x in range(m.w)] for y in range(m.h)]
    return coll,[[0]*m.w for _ in range(m.h)]
def block(coll,cells):
    for (x,y) in cells: coll[y][x]=1

# ---- 1F: the entrance hall ----
hall=Map(22,14)
room(hall)
stairs_north(hall.F,16,0,48,5)                     # up to the guest floor, top left
byobu(hall.F,8*16,14*16,0)                         # the gold screen behind the desk
for cx in (6,15): scroll(hall.F,cx*16+8,0)
runner(hall.F,10*16,6*16,32,7*16)
tatami(hall.F,2*16,7*16,5*16,4*16); tatami(hall.F,15*16,7*16,5*16,4*16)   # waiting areas
for x in (10,11):                                  # the entrance: a stone step
    for y in range(13*16,14*16):
        for i in range(16): hall.F.set(x*16+i,y,(ST2,ST1,ST1,ST1,ST0)[min(4,(y-13*16)//3)])
for y in range(5*16):                              # the stairs' balustrade
    for i in range(4,12): hall.O.set(4*16+i,y,(WD1,WD3,WD3,WD2,WD2,WD2,WD1,WD0)[i-4] if y%16 else WD0)
desk(hall,8,14,5)
low_table(hall,4,8); zabuton(hall,3,8); zabuton(hall,5,8)
low_table(hall,17,8); zabuton(hall,16,8); zabuton(hall,18,8)
for (x,y) in ((7,3),(14,3),(7,12),(14,12)): andon(hall,x,y)
for (x,y) in ((2,12),(19,12),(19,2)): bonsai(hall,x,y)
c1,b1=shell(hall)
for x in (1,2,3): c1[1][x]=0; b1[1][x]=MB_NORTH_ARROW_WARP     # top step: up to the guest floor
block(c1,[(4,y) for y in range(5)])
for x in range(8,14): c1[5][x]=1; b1[5][x]=MB_COUNTER         # talk to the receptionist across it
block(c1,[(7,3),(14,3),(7,12),(14,12),(2,12),(19,12),(19,2),(4,8),(17,8)])
for x in (10,11): c1[13][x]=0; b1[13][x]=MB_SOUTH_ARROW_WARP  # out to the castle gate

# ---- 2F: the guest floor ----
GF=Map(24,9)
room(GF)
DOORS=(4,7,10,13,16,19)
for x in DOORS: fusuma(GF.F,x*16,0)
runner(GF.F,3*16,4*16,18*16,32)
stairs_side(GF.F,16,3*16,32,64,up_east=False)        # down to the hall, west
stairs_side(GF.F,21*16,3*16,32,64,up_east=True)      # up to the arena, east
for x in (6,12,17): andon(GF,x,7)
c2,b2=shell(GF)
for x in DOORS: c2[1][x]=0; b2[1][x]=MB_NON_ANIMATED_DOOR
for y in range(3,7): b2[y][1]=MB_WEST_ARROW_WARP; b2[y][22]=MB_EAST_ARROW_WARP
for y in (2,7): block(c2,[(1,y),(2,y),(21,y),(22,y)])         # the stairs' sides
block(c2,[(6,7),(12,7),(17,7)])

# ---- a guest room (one layout for all six) ----
R=Map(11,9)
room(R,floor=tatami)
for x in range(7*16,10*16):                        # the alcove: a dark recess with a scroll
    for j in range(4,32):
        R.F.set(x,j,WD1 if x in (7*16,10*16-1) else (PP1 if j<26 else WD2))
scroll(R.F,8*16+8,0)
for x in range(7*16,10*16):                        # and its raised floor
    for j in range(16): R.F.set(x,32+j,(WD4,WD3,WD3,WD3,WD3,WD3,WD3,WD3,WD3,WD3,WD3,WD3,WD2,WD2,WD1,OUT)[j])
for x in range(5*16,6*16):                         # the threshold to the corridor
    wood_floor(R.F,x,7*16,1,16)
fusuma(R.F,2*16,0,motif=False); fusuma(R.F,3*16,0,motif=False)   # closet doors
futon(R,2,2); andon(R,5,2); vase(R,8,2)
low_table(R,6,5); zabuton(R,5,5); zabuton(R,7,5)
c3,b3=shell(R)
block(c3,[(2,2),(3,2),(2,3),(3,3),(5,2),(7,2),(8,2),(9,2),(6,5)])
b3[7][5]=MB_SOUTH_ARROW_WARP

# ---- 3F: the arena ----
A=Map(23,22)
room(A,wall=plaster_wall)
maku(A.F,16,8*16,0); maku(A.F,15*16,22*16,0)
byobu(A.F,8*16,15*16,0)
tatami(A.F,8*16,2*16,7*16,2*16)                    # MR. M.'s dais: tatami on a wooden platform
for x in range(8*16,15*16):
    for j in range(16):
        col=(WD4,WD3,WD3,WD2,WD2,WD2,WD2,WD2,WD2,WD2,WD2,WD2,WD2,WD1,WD0,OUT)[j]
        if x%16==0 and 3<j<13: col=WD1
        A.F.set(x,4*16+j,col)
battlefield(A.F,5*16,6*16,18*16,19*16,((11*16+8,8*16+8),(11*16+8,16*16+8)),(11*16+8,12*16+8))
stairs_south(A.F,10*16,20*16,48,2)
for (x,y) in ((4,5),(18,5),(4,19),(18,19)): andon(A,x,y)
for (x,y) in ((2,4),(20,4),(2,20),(20,20)): nobori(A,x,y)
SEATS=[(x,y) for y in range(7,18,2) for x in (1,2,3,19,20,21)]
for (x,y) in SEATS: zabuton(A,x,y)
c4,b4=shell(A)
block(c4,[(x,y) for y in (2,3,4) for x in range(8,15)])
block(c4,[(4,5),(18,5),(4,19),(18,19),(2,4),(20,4),(2,20),(20,20)]+SEATS)
for y in range(6,19): c4[y][4]=c4[y][18]=0
for x in (10,11,12): c4[21][x]=0; b4[21][x]=MB_SOUTH_ARROW_WARP   # down to the guest floor
c4[20][9]=c4[20][13]=1

MAPS=((hall,c1,b1,'MegaIsland_Castle_1F'),(GF,c2,b2,'MegaIsland_Castle_2F'),
      (R,c3,b3,'MegaIsland_Castle_GuestRoom'),(A,c4,b4,'MegaIsland_Arena'))
for m,_,_,_ in MAPS: m.O.outline()

if os.environ.get('PREVIEW'):
    from PIL import Image
    def img(m):
        im=Image.new('RGB',(m.w*16,m.h*16))
        for y in range(m.h*16):
            for x in range(m.w*16):
                v=m.O.get(x,y) or m.F.get(x,y)
                im.putpixel((x,y),PAL[v])
        return im
    ims=[img(m) for m,_,_,_ in MAPS]
    out=Image.new('RGB',(sum(i.width for i in ims)+16*len(ims),max(i.height for i in ims)))
    x=0
    for i in ims: out.paste(i,(x,0)); x+=i.width+16
    out.save(os.environ['PREVIEW']); sys.exit(0)

def build(B,m,coll,behav):
    grid=[[0]*m.w for _ in range(m.h)]
    for y in range(m.h):
        for x in range(m.w):
            layer=0 if (x,y) in m.over else 1     # NORMAL: the top layer is above the player
            grid[y][x]=B.metatile(B.entries(m.F,x*16,y*16),B.entries(m.O,x*16,y*16),(layer<<12)|behav[y][x])
    return grid
def void(B):
    blk=Canvas(16,16); blk.rect(0,0,16,16,BLK)
    return B.metatile(B.entries(blk,0,0),[tilesetgen.EMPTY_TOP]*4,0)|0xc00|0x3000
for maps,ts in ((MAPS[:3],'kanto_mega_hall'),(MAPS[3:],'kanto_mega_arena')):
    B=tilesetgen.Builder(PALETTES,PAL)
    grids=[build(B,m,c,b) for m,c,b,_ in maps]
    v=void(B)
    print(ts,end=': '); B.check()
    B.write_tileset(REPO+'data/tilesets/secondary/%s/'%ts)
    for (m,c,b,name),g in zip(maps,grids):
        tilesetgen.write_layout(REPO+'data/layouts/%s/'%name,g,c,(v,)*4)
print('written')
