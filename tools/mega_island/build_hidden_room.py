# Appends to the arena's tileset (kanto_mega_arena) the hidden door behind MR. M.'s mat and the
# hidden room behind it, and writes the room's layout and the door's animation:
#   - the door: a wooden door in the base of the north wall, under a lintel, drawn here in the wall's
#     own palette. METATILE_MegaArena_HiddenDoor (MB_ANIMATED_DOOR, 0x2C5) and
#     METATILE_MegaArena_HiddenDoorTop (the lintel, 0x2C6) take the wall's place when the door is
#     revealed (MegaIsland_Arena's scripts). Like FRLG's house doors, only the bottom cell animates:
#     graphics/door_anims/kanto/mega_arena.png, its three frames opening (src/field_door.c).
#   - the hidden room (MegaIsland_HiddenRoom): a small room of the castle, after the Pokemon Mansion
#     like the others, with the underground path entrance's small stairwell going down (FRLG's,
#     kanto_generic_building_2) on its floor, and the way back to the arena in its south wall.
# The arena's tileset was edited after build_interiors.py wrote it, so this starts from the tileset
# as it is (its first ARENA_METATILES metatiles and ARENA_TILES tiles) rather than rebuilding it.
# PREVIEW=out.png renders the room and the arena with the door.
import sys, os, struct
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from interiors_lib import *
from PIL import Image

TS=REPO+'data/tilesets/secondary/kanto_mega_arena/'
ARENA_METATILES,ARENA_TILES,ARENA_PALETTES=69,64,(7,8,10,12)
WALL_PAL=12
WALL_ROW1,WALL_ROW2=0x287,0x28d        # the arena's north wall: the plain wall, and its base

# ---- the arena's tileset, as it is ----
T=Tileset()
T.tiles=render.load_tiles(TS)[:ARENA_TILES]
T.tindex={}
for i,t in enumerate(T.tiles): T.tindex.setdefault(T.key(t),i)
T.pal={p:render.load_pal(TS+'palettes/%02d.pal'%p) for p in ARENA_PALETTES}
meta=list(struct.iter_unpack('<8H',open(TS+'metatiles.bin','rb').read()))[:ARENA_METATILES]
attr=[a for (a,) in struct.iter_unpack('<H',open(TS+'metatile_attributes.bin','rb').read())][:ARENA_METATILES]
T.metatiles=[(m,a) for m,a in zip(meta,attr)]
T.mindex={k:NUM_PRIMARY_METATILES+i for i,k in enumerate(T.metatiles)}

def cell_pixels(m):
    # The 16x16 colour indices of one of the arena's metatiles drawn on its top layer, in WALL_PAL.
    g=[[0]*16 for _ in range(16)]
    for i,e in enumerate(meta[m-NUM_PRIMARY_METATILES][4:]):
        t=e&0x3ff
        if t<NUM_PRIMARY_TILES: continue
        px=T.tiles[t-NUM_PRIMARY_TILES]; hf=e>>10&1; vf=e>>11&1
        for y in range(8):
            for x in range(8): g[(i>>1)*8+y][(i&1)*8+x]=px[7-y if vf else y][7-x if hf else x]
    return g

# ---- the door, in WALL_PAL: 1-4 browns (mid, light, highlight, dark), 7-11 golds, 12-13 slates ----
# The leaf fills the base's cell (row 2) above its bottom line; the lintel sits at the bottom of the
# plain wall's cell (row 1). Columns 1-2 and 13-14 are the frame.
LEAF=[
    "4444444444",
    "4111111114",
    "4144444414",
    "4143332414",
    "4142222414",
    "4142222414",
    "4144444414",
    "41111111a4",
    "4111111174",
    "4144444414",
    "4143332414",
    "4142222414",
    "4142222414",
    "4144444414",
    "4111111114",
]
DARK='c'
def door_cell(leaf_width):
    # The base's cell with the door, its leaf swung open to leaf_width columns (10: closed).
    g=cell_pixels(WALL_ROW2)
    for y in range(15):
        g[y][1]=4; g[y][2]=2; g[y][13]=2; g[y][14]=4
        for x in range(10):
            if leaf_width==10: v=LEAF[y][x]
            elif x<leaf_width-1: v=LEAF[y][x*10//leaf_width] if leaf_width>2 else '1'
            elif x==leaf_width-1: v='4'
            else: v=DARK if y>0 else '4'
            g[y][3+x]=int(v,16)
    for x in range(1,15): g[15][x]=12
    return g
def lintel_cell():
    g=cell_pixels(WALL_ROW1)
    rows={12:"44444444444444",13:"43333333333334",14:"42222222222224",15:"42444444444424"}
    for y,r in rows.items():
        for i,v in enumerate(r): g[y][1+i]=int(v,16)
    return g
def tiles_of(g):
    return [[g[ty*8+y][tx*8:tx*8+8] for y in range(8)] for ty in (0,1) for tx in (0,1)]

class Art:
    # The door's tiles as a source for the tileset: four 8x8 tiles per cell, in WALL_PAL.
    def __init__(s,cells):
        s.name='door'; s.tiles=[t for g in cells for t in tiles_of(g)]; s.pals={WALL_PAL:T.pal[WALL_PAL]}
    def entries(s,c):
        return [(s,(NUM_PRIMARY_TILES+4*c+i)|WALL_PAL<<12) for i in range(4)]
ART=Art([door_cell(10),lintel_cell()])
NONE=[(PRIM,0)]*4
door=T.metatile(Cell(ART.entries(0),NONE,0),MB_ANIMATED_DOOR)                 # drawn under the player
lintel=T.metatile(Cell(NONE,ART.entries(1),attr[WALL_ROW1-NUM_PRIMARY_METATILES]>>12),MB_NORMAL)
assert (door,lintel)==(0x2c5,0x2c6), (hex(door),hex(lintel))

# The animation: the leaf swinging open, then the open doorway.
frames=[door_cell(w) for w in (6,3,1)]
im=Image.new('P',(16,48)); im.putpalette([v for c in T.pal[WALL_PAL] for v in c])
for f,g in enumerate(frames):
    for y in range(16):
        for x in range(16): im.putpixel((x,f*16+y),g[y][x])
im.save(REPO+'graphics/door_anims/kanto/mega_arena.png')

# ---- the hidden room ----
GB2=Source('kanto_generic_building_2',floors=(0x281,0x283))
R=Room(11,9)
shell(R)
# The small stairwell going down (the underground path entrance's), entered from the east: standing
# on its right column's middle cell and walking west warps.
SX,SY=4,3
for j,row in enumerate(((0x2aa,0x2ab,0x2ac),(0x2b2,0x2b3,0x2b4),(0x2ba,0x2bb,0x2bc))):
    for i,m in enumerate(row):
        R.put(SX+i,SY+j,over(R.cell[SY+j][SX+i],GB2,m),1 if i<2 and j>0 else 0)
R.beh[SY+1][SX+2]=MB_WEST_ARROW_WARP
STAIRS=(SX+2,SY+1)
statue(R,2,3,red=True); statue(R,8,3,red=True)
plant(R,1,6); plant(R,9,6)
exit_mat(R,4)
grid=[[T.metatile(R.cell[y][x],R.beh[y][x]) for x in range(R.w)] for y in range(R.h)]
T.write(TS)
tilesetgen.write_layout(REPO+'data/layouts/MegaIsland_HiddenRoom/',grid,R.coll,(BLACK|0xc00|0x3000,)*4)
print('hidden door %#x, lintel %#x; hidden room %dx%d, stairs at %s, way out at (4-6,%d)'%(door,lintel,R.w,R.h,STAIRS,R.h-2))

if os.environ.get('PREVIEW'):
    a=render.render_layout('MegaIsland_Arena',os.environ['PREVIEW']); arena=Image.open(os.environ['PREVIEW']).copy()
    ts=render.TS('kanto_building','kanto_mega_arena')
    for (x,y,m) in ((11,1,lintel),(11,2,door)):
        ts.metatile(arena,m,x*16,y*16)
    arena=arena.crop((0,0,arena.width,8*16))
    render.render_layout('MegaIsland_HiddenRoom',os.environ['PREVIEW']); room=Image.open(os.environ['PREVIEW']).copy()
    anim=Image.new('RGB',(16*4+12,16))
    for f in range(4):
        cell=Image.new('RGB',(16,16)); ts.metatile(cell,door,0,0)
        if f: cell.paste(im.convert('RGB').crop((0,(f-1)*16,16,f*16)),(0,0))
        anim.paste(cell,(f*20,0))
    out=Image.new('RGB',(arena.width+16+room.width,max(arena.height,room.height)+24))
    out.paste(arena,(0,0)); out.paste(room,(arena.width+16,0)); out.paste(anim,(0,arena.height+4))
    out.resize((out.width*2,out.height*2),Image.NEAREST).save(os.environ['PREVIEW'])
