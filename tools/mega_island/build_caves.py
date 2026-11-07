# Builds the caves on the hidden way from the castle's arena to MEWTWO and back out to the island's
# route, as rooms of the Lost Cave's kind (kanto_general + kanto_cave): their two-layer rock walls,
# the light openings in the south wall, the openings in the side and north walls, ladders and ledges.
# (The room behind the arena's hidden door, with the stairs down to the tunnel, is a room of the
# castle: build_hidden_room.py.)
#   MegaIsland_Tunnel       a tunnel going south, from the ladder up (to the hidden room's stairs) to
#                           an opening in its south end
#   MegaIsland_Sanctum      the big cave where MEWTWO waits, in front of its east wall; after the
#                           battle a passage opens there, behind it
#   MegaIsland_LedgeRoom    past the passage: a ledge that can only be jumped down from the north,
#                           and a ladder up to the island's cave (MegaIsland_Cave): from there, the
#                           room below the ledge is a dead end
# The coordinates the map events and scripts use are the constants below. PREVIEW=out.png renders
# the three maps side by side.
import struct, os
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'

FLOOR,ROCK,FILL=0x281,0x282,0x291
LEDGE_LEFT,LEDGE=0x2bc,0x2bd                 # jump south (MB_JUMP_SOUTH)
LADDER_TOP,LADDER=0x28f,0x297                # ladder up: its top (solid) and its foot (MB_LADDER)
CIRCLE=((0x2d5,0x2d6),(0x2dd,0x2de))         # a ring drawn on the floor
SOUTH_EXIT=(0x2c8,0x2c9,0x2ca)               # the light opening in the south wall, under...
SOUTH_EXIT_WARP=0x287                        # ...the floor cell that leads out (MB_SOUTH_ARROW_WARP)
NORTH_EXIT=(0x28b,0x28c,0x28d)               # the opening's top, in the top wall row, over...
NORTH_EXIT_DOOR,NORTH_EXIT_RIGHT=0x294,0x295 # ...its door (MB_NON_ANIMATED_DOOR) and right side
EAST_EXIT_WARP,EAST_EXIT=0x308,0x309         # the floor cell (MB_EAST_ARROW_WARP) and the wall
WEST_EXIT,WEST_EXIT_WARP=0x30a,0x30b         # the wall and the floor cell (MB_WEST_ARROW_WARP)

class Room:
    # A room of the Lost Cave's kind, w x h: two rows of rock over the floor, two columns each
    # side, the south wall and a row of rock under it. Walls are solid at elevation 0, the floor
    # walkable at elevation 3.
    def __init__(s,w,h):
        s.w,s.h=w,h
        s.g=[[None]*w for _ in range(h)]
        for y in range(h):
            for x in range(w):
                if y==0: m=0x29b if x==0 else 0x29c if x==w-1 else 0x299
                elif y==h-1: m=0x2df
                elif x==0: m=0x2c7
                elif x==w-1: m=0x2c6
                elif y==1: m=0x29b if x==1 else 0x29c if x==w-2 else 0x299
                elif y==h-2: m=0x2a3 if x==1 else 0x2a4 if x>=w-3 else 0x289
                elif x==1: m=0x292
                elif x==w-2: m=0x290
                else: m=None
                if m is None: s.floor(x,y)
                else: s.wall(x,y,m)
    def wall(s,x,y,m): s.g[y][x]=m|1<<10
    def floor(s,x,y,m=FLOOR): s.g[y][x]=m|3<<12
    def south_exit(s,x):
        # The opening centred on x, in the south wall: returns the warp cell.
        for i,m in enumerate(SOUTH_EXIT): s.wall(x-1+i,s.h-2,m)
        s.floor(x,s.h-3,SOUTH_EXIT_WARP)
        return (x,s.h-3)
    def north_exit(s,x):
        for i,m in enumerate(NORTH_EXIT): s.wall(x-1+i,0,m)
        s.floor(x,1,NORTH_EXIT_DOOR); s.wall(x+1,1,NORTH_EXIT_RIGHT)
        return (x,1)
    def west_exit(s,y):
        s.wall(1,y,WEST_EXIT); s.floor(2,y,WEST_EXIT_WARP)
        return (2,y)
    def ladder_up(s,x,y):
        # A ladder standing on the floor: its top at y-1 (in the wall if y is the first floor row).
        s.wall(x,y-1,LADDER_TOP); s.floor(x,y,LADDER)
        return (x,y)
    def ledge(s,y):
        # A ledge across the room, wall to wall, jumped down from the north only.
        for x in range(2,s.w-2): s.wall(x,y,(LEDGE_LEFT if x==2 else LEDGE)|3<<12)
    def rock(s,x,y): s.wall(x,y,ROCK)
    def write(s,name):
        d=REPO+'data/layouts/%s/'%name
        os.makedirs(d,exist_ok=True)
        with open(d+'map.bin','wb') as f:
            for r in s.g: f.write(b''.join(struct.pack('<H',v) for v in r))
        open(d+'border.bin','wb').write(struct.pack('<4H',*([FILL|1<<10]*4)))
        print(name,s.w,s.h)

# ---- the tunnel going south, from the ladder up to the south opening ----
T=Room(7,26)
TUNNEL_LADDER=T.ladder_up(3,2)               # (3,2), to the hidden room's stairs
TUNNEL_EXIT=T.south_exit(3)                  # (3,23), to the sanctum's north door
for p in ((2,7),(4,12),(2,17),(4,20)): T.rock(*p)

# ---- the sanctum: in through the north door, MEWTWO in front of the east wall ----
S=Room(17,13)
SANCTUM_ENTRANCE=S.north_exit(4)             # (4,1)
MEWTWO=(13,6)
SANCTUM_EXIT=(14,6)                          # the passage that opens behind MEWTWO (see below)
for j,r in enumerate(CIRCLE):                # the ring on the floor, before MEWTWO
    for i,m in enumerate(r): S.floor(9+i,5+j,m)
for p in ((3,9),(7,3),(11,10),(14,3),(2,5),(13,9)): S.rock(*p)
# The passage, closed and open: the floor cell in front of the east wall and the wall cell.
SANCTUM_CLOSED=(FLOOR,0x290)
SANCTUM_OPEN=(EAST_EXIT_WARP,EAST_EXIT)

# ---- past the passage: in from the west, the ledge, the ladder up to the island's cave ----
L=Room(11,11)
LEDGE_ENTRANCE=L.west_exit(3)                # (2,3), to the sanctum's passage
L.ledge(5)
LEDGE_LADDER=L.ladder_up(6,7)                # (6,7)
for p in ((8,2),(3,8),(8,8)): L.rock(*p)

for room,name in ((T,'MegaIsland_Tunnel'),(S,'MegaIsland_Sanctum'),(L,'MegaIsland_LedgeRoom')):
    room.write(name)
print('tunnel: ladder',TUNNEL_LADDER,'exit',TUNNEL_EXIT)
print('sanctum: entrance',SANCTUM_ENTRANCE,'MEWTWO',MEWTWO,'passage',SANCTUM_EXIT,
      'closed %#x %#x open %#x %#x'%(SANCTUM_CLOSED+SANCTUM_OPEN))
print('ledge room: entrance',LEDGE_ENTRANCE,'ladder',LEDGE_LADDER)

if os.environ.get('PREVIEW'):
    import render
    from PIL import Image
    ts=render.TS('kanto_general','kanto_cave')
    ims=[]
    for room in (T,S,L):
        im=Image.new('RGB',(room.w*16,room.h*16))
        for y in range(room.h):
            for x in range(room.w): ts.metatile(im,room.g[y][x]&0x3ff,x*16,y*16)
        ims.append(im)
    out=Image.new('RGB',(sum(i.width for i in ims)+16*len(ims),max(i.height for i in ims)))
    x=0
    for i in ims: out.paste(i,(x,0)); x+=i.width+16
    out.save(os.environ['PREVIEW'])
