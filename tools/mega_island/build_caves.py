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
# The sanctum has a tileset of its own, kanto_mega_sanctum: kanto_cave with a glowing rock, a purple
# light around the boulder that pulses (three frames of animation, src/tileset_anims.c), in three
# purples put in the rock's palette's free colours.
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
    def rock(s,x,y,m=ROCK): s.wall(x,y,m)
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
for j,r in enumerate(CIRCLE):                # the ring on the floor, before the spire
    for i,m in enumerate(r): S.floor(9+i,6+j,m)
# ---- the sanctum's tileset: kanto_cave and the glowing rock ----
def build_sanctum_tileset():
    import render
    from PIL import Image
    SRC=REPO+'data/tilesets/secondary/kanto_cave/'; DST=REPO+'data/tilesets/secondary/kanto_mega_sanctum/'
    tiles=render.load_tiles(SRC)
    meta=list(struct.iter_unpack('<8H',open(SRC+'metatiles.bin','rb').read()))
    attr=[a for (a,) in struct.iter_unpack('<H',open(SRC+'metatile_attributes.bin','rb').read())]
    pals=[render.load_pal(SRC+'palettes/%02d.pal'%p) for p in range(16)]
    rock=meta[ROCK-640]
    PAL=rock[4]>>12
    def layer(ents):
        g=[[0]*16 for _ in range(16)]
        for i,e in enumerate(ents):
            t=tiles[(e&0x3ff)-640]; hf=e>>10&1; vf=e>>11&1
            for y in range(8):
                for x in range(8): g[(i>>1)*8+y][(i&1)*8+x]=t[7-y if vf else y][7-x if hf else x]
        return g
    stone=layer(rock[4:])
    # The purples go in the colours the rock's palette leaves unused (magenta).
    free=[i for i,c in enumerate(pals[PAL]) if c==(255,0,255)]
    DARK,MID,BRIGHT=free[:3]
    pals[PAL][DARK],pals[PAL][MID],pals[PAL][BRIGHT]=(112,56,160),(176,96,232),(238,190,255)
    solid=lambda x,y: 0<=x<16 and 0<=y<16 and stone[y][x]!=0
    near=lambda x,y: [(x+dx,y+dy) for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))]
    SHADOW=13                                          # its shadow on the floor, from this row down
    def frame(level):
        # 0: a dark purple rim; 1: brighter, the light catching its top, a halo around it;
        # 2: brightest.
        g=[r[:] for r in stone]
        for y in range(SHADOW):
            for x in range(16):
                if solid(x,y):
                    if any(not solid(a,b) for a,b in near(x,y)): g[y][x]=(DARK,MID,MID)[level]
                    elif stone[y][x]==9 and level: g[y][x]=(MID,BRIGHT)[level-1]
                    elif stone[y][x]==0xa and level==2: g[y][x]=MID
                elif level and any(solid(a,b) for a,b in near(x,y)):
                    g[y][x]=DARK if level==1 else (BRIGHT if (x+y)%2 else MID)
        return g
    frames=[frame(l) for l in (0,1,2)]
    split=lambda g: [[g[ty*8+y][tx*8:tx*8+8] for y in range(8)] for ty in (0,1) for tx in (0,1)]
    first=len(tiles); tiles+=split(frames[0])
    glow=640+len(meta)
    meta.append(rock[:4]+tuple((640+first+i)|PAL<<12 for i in range(4))); attr.append(attr[ROCK-640])
    # The stone spire of the island's route (the Memorial Pillar's, on kanto_sevii_islands_45): its top
    # layer, recoloured from kanto_general's greys into the cave rock's browns (the rock's palette),
    # on the cave's floor, and glowing as the rock does. Its tiles follow the rock's, one block, a
    # metatile's four after another, so its animation is one copy.
    SEVII=REPO+'data/tilesets/secondary/kanto_sevii_islands_45/'
    stiles=render.load_tiles(SEVII)
    smeta=list(struct.iter_unpack('<8H',open(SEVII+'metatiles.bin','rb').read()))
    sattr=[a for (a,) in struct.iter_unpack('<H',open(SEVII+'metatile_attributes.bin','rb').read())]
    W,H=16*len(SPIRE[0]),16*len(SPIRE)
    G=[[0]*W for _ in range(H)]
    for j,row in enumerate(SPIRE):
        for i,m in enumerate(row):
            if m is None: continue
            for q,e in enumerate(smeta[m-640][4:]):
                if e&0x3ff<640: continue
                t=stiles[(e&0x3ff)-640]; hf=e>>10&1; vf=e>>11&1
                for y in range(8):
                    for x in range(8): G[j*16+(q>>1)*8+y][i*16+(q&1)*8+x]=t[7-y if vf else y][7-x if hf else x]
    BASE=H-12                       # from this row down, the shadow at its foot, on the floor
    STONE={2:9,3:0xa,4:0xb,5:0xc,6:0xd,7:0xe}      # its greys, light to dark, to the rock's browns
    SHADE={2:9,3:0xa,4:0xb,5:4,6:0xd,7:0xe}        # in the shadow, the floor's shade
    G=[[(SHADE if y>=BASE else STONE)[v] if v else 0 for v in r] for y,r in enumerate(G)]
    def light(g0,level,base):
        # The rock's light, on any picture of the stone (rows from base down left as they are).
        h,w=len(g0),len(g0[0]); g=[r[:] for r in g0]
        solid=lambda x,y: 0<=x<w and 0<=y<h and g0[y][x]!=0
        for y in range(min(base,h)):
            for x in range(w):
                if solid(x,y):
                    if any(not solid(a,b) for a,b in near(x,y)): g[y][x]=(DARK,MID,MID)[level]
                    elif g0[y][x]==9 and level: g[y][x]=(MID,BRIGHT)[level-1]
                    elif g0[y][x]==0xa and level==2: g[y][x]=MID
                elif level and any(solid(a,b) for a,b in near(x,y)):
                    g[y][x]=DARK if level==1 else (BRIGHT if (x+y)%2 else MID)
        return g
    cells=[(i,j,m) for j,row in enumerate(SPIRE) for i,m in enumerate(row) if m]
    def spire_frame(level):
        # The spire's cells, one under the other, as its tiles are laid out.
        g=light(G,level,BASE)
        return [r[i*16:i*16+16] for i,j,m in cells for r in g[j*16:j*16+16]]
    spire_frames=[spire_frame(l) for l in (0,1,2)]
    spire_first=len(tiles)
    floor=meta[FLOOR-640][:4]
    spire={}
    for k,(i,j,m) in enumerate(cells):
        tiles.extend(split(spire_frames[0][k*16:k*16+16]))
        spire[m]=640+len(meta)
        meta.append(floor+tuple((640+spire_first+4*k+q)|PAL<<12 for q in range(4))); attr.append(sattr[m-640])
    os.makedirs(DST+'palettes',exist_ok=True); os.makedirs(DST+'anim/glow',exist_ok=True)
    src=Image.open(SRC+'tiles.png')
    im=Image.new('P',(128,(len(tiles)+15)//16*8),0); im.putpalette(src.getpalette())
    for i,t in enumerate(tiles):
        for y in range(8):
            for x in range(8): im.putpixel(((i%16)*8+x,(i//16)*8+y),t[y][x])
    im.save(DST+'tiles.png')
    for p in range(16):
        with open(DST+'palettes/%02d.pal'%p,'w',newline='\r\n') as f:
            f.write('JASC-PAL\n0100\n16\n'+''.join('%d %d %d\n'%c for c in pals[p]))
    open(DST+'metatiles.bin','wb').write(b''.join(struct.pack('<8H',*e) for e in meta))
    open(DST+'metatile_attributes.bin','wb').write(b''.join(struct.pack('<H',a) for a in attr))
    def save_frame(g,path):
        fi=Image.new('P',(len(g[0]),len(g))); fi.putpalette([v for c in pals[PAL] for v in c])
        for y,r in enumerate(g):
            for x,v in enumerate(r): fi.putpixel((x,y),v)
        fi.save(path)
    os.makedirs(DST+'anim/spire',exist_ok=True)
    for f,g in enumerate(frames): save_frame(g,DST+'anim/glow/%d.png'%f)
    for f,g in enumerate(spire_frames): save_frame(g,DST+'anim/spire/%d.png'%f)
    print('the spire: %d tiles from %d'%(4*len(cells),spire_first))
    print('kanto_mega_sanctum: %d tiles (the glowing rock\'s at %d), glowing rock metatile %#x'%(len(tiles),first,glow))
    return glow,pals,spire,len(tiles)
# The spire as it stands on the route: its tip (walked behind), the stone (solid), and the shadows at
# its foot on either side.
SPIRE=((None,0x2c0,0x2c1,None),(None,0x2c8,0x2c9,None),(None,0x2d0,0x2d1,None),(0x2c2,0x2d8,0x2d9,0x2ca))
GLOW_ROCK,SANCTUM_PALS,SPIRE_METATILES,SANCTUM_TILES=build_sanctum_tileset()
assert GLOW_ROCK==0x321
SPIRE_AT=(8,2)                               # behind the ring, against the north wall
for j,row in enumerate(SPIRE):
    for i,m in enumerate(row):
        if m is None: continue
        x,y=SPIRE_AT[0]+i,SPIRE_AT[1]+j
        if 0<j and 0<i<3: S.wall(x,y,SPIRE_METATILES[m]|3<<12)
        else: S.floor(x,y,SPIRE_METATILES[m])
for p in ((3,9),(7,3),(11,10),(14,3),(2,5),(13,9)): S.rock(*p,GLOW_ROCK)
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
    ims=[]
    for room in (T,S,L):
        ts=render.TS('kanto_general','kanto_mega_sanctum' if room is S else 'kanto_cave')
        im=Image.new('RGB',(room.w*16,room.h*16))
        for y in range(room.h):
            for x in range(room.w): ts.metatile(im,room.g[y][x]&0x3ff,x*16,y*16)
        ims.append(im)
    out=Image.new('RGB',(sum(i.width for i in ims)+16*len(ims),max(i.height for i in ims)))
    x=0
    for i in ims: out.paste(i,(x,0)); x+=i.width+16
    out.save(os.environ['PREVIEW'])
