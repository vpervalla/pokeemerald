# The pieces build_interiors.py and build_hidden_room.py build the castle's interiors with: the
# source tilesets (the Pokemon Mansion's and the others listed in build_interiors.py), the cells made
# from their metatiles, the secondary tileset being built, and the mansion's room, door, stairs and
# furniture placed on a map.
import sys, os, struct
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import render, tilesetgen
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
MB_NORMAL,MB_NON_ANIMATED_DOOR,MB_ANIMATED_DOOR,MB_EAST_ARROW_WARP,MB_WEST_ARROW_WARP,MB_SOUTH_ARROW_WARP,MB_COUNTER=0,96,105,98,99,101,128
NUM_PRIMARY_TILES=NUM_PRIMARY_METATILES=640

# ---- the source tilesets ----
class Source:
    def __init__(s,sec,floors=()):
        P=render.R+'primary/kanto_building/'
        s.name=sec
        s.meta=list(struct.iter_unpack('<8H',open(P+'metatiles.bin','rb').read()))
        s.attr=[a for (a,) in struct.iter_unpack('<H',open(P+'metatile_attributes.bin','rb').read())]
        s.meta+=[(0,)*8]*(NUM_PRIMARY_METATILES-len(s.meta)); s.attr+=[0]*(NUM_PRIMARY_METATILES-len(s.attr))
        s.tiles=[]; s.pals={}
        if sec:
            S=render.R+'secondary/'+sec+'/'
            s.meta+=list(struct.iter_unpack('<8H',open(S+'metatiles.bin','rb').read()))
            s.attr+=[a for (a,) in struct.iter_unpack('<H',open(S+'metatile_attributes.bin','rb').read())]
            s.tiles=render.load_tiles(S)
            s.pals={p:render.load_pal(S+'palettes/%02d.pal'%p) for p in range(7,13)}
        # the bottom-layer tiles of the source's plain floor: furniture moved onto the mansion's
        # floor keeps its own bottom layer only where it isn't one of these
        s.floor={e for m in floors for e in s.meta[m][:4]}

M=Source('kanto_pokemon_mansion')
HOTEL=Source('kanto_restaurant_hotel',floors=(0x29e,0x296))
SSANNE=Source('kanto_ss_anne',floors=(0x31c,))
LEAGUE=Source('kanto_pokemon_league')
PRIM=Source(None)

class Cell:
    # A metatile to be: four bottom and four top tile entries, each with the source it comes from,
    # and the layer type.
    def __init__(s,bottom,top,layer): s.bottom,s.top,s.layer=bottom,top,layer
def mt(src,m):
    e=src.meta[m]
    return Cell([(src,x) for x in e[:4]],[(src,x) for x in e[4:]],src.attr[m]>>12)
def over(base,src,m):
    # Source metatile m (a piece of furniture) on the floor of base: its top layer, and its bottom
    # layer where that isn't its own floor.
    e=src.meta[m]
    bottom=[base.bottom[i] if e[i] in src.floor else (src,e[i]) for i in range(4)]
    return Cell(bottom,[(src,x) for x in e[4:]],src.attr[m]>>12)
def quads(cell,top):
    # A cell with its top layer made of (source, metatile, quadrant) pieces (the door mats).
    return Cell(cell.bottom,[(s,s.meta[m][4+q]) if m is not None else (PRIM,0) for s,m,q in top],cell.layer)

# ---- the tileset being built ----
class Tileset:
    def __init__(s):
        s.tiles=[[[0]*8 for _ in range(8)]]; s.tindex={s.key(s.tiles[0]):0}
        s.slots={}           # (source, palette) -> slot
        s.pal={}             # slot -> colours
        s.metatiles=[]; s.mindex={}
    @staticmethod
    def key(px): return tuple(tuple(r) for r in px)
    def slot(s,src,p):
        # The slot of a source palette: the one already holding the same colours, the source's own
        # slot if it's free, or the first free one.
        if (src.name,p) not in s.slots:
            pal=src.pals[p]
            same=[q for q,c in s.pal.items() if c==pal]
            free=[q for q in [p]+list(range(7,13)) if q not in s.pal]
            assert same or free, 'more than 6 secondary palettes'
            q=same[0] if same else free[0]
            s.pal[q]=pal; s.slots[(src.name,p)]=q
        return s.slots[(src.name,p)]
    def entry(s,src,e):
        t,h,v,p=e&0x3ff,e>>10&1,e>>11&1,e>>12
        if p>=7: p=s.slot(src,p)
        if t>=NUM_PRIMARY_TILES:
            px=src.tiles[t-NUM_PRIMARY_TILES]
            rows=[list(r) for r in px]
            cands=[(rows,0,0),([r[::-1] for r in rows],1,0),(rows[::-1],0,1),([r[::-1] for r in rows[::-1]],1,1)]
            for k,fh,fv in cands:
                if s.key(k) in s.tindex: t=NUM_PRIMARY_TILES+s.tindex[s.key(k)]; h^=fh; v^=fv; break
            else:
                s.tindex[s.key(rows)]=len(s.tiles); s.tiles.append(rows); t=NUM_PRIMARY_TILES+len(s.tiles)-1
        return t|h<<10|v<<11|p<<12
    def metatile(s,cell,behavior):
        ents=tuple(s.entry(src,e) for src,e in cell.bottom+cell.top)
        key=(ents,cell.layer<<12|behavior)
        if key not in s.mindex:
            s.mindex[key]=NUM_PRIMARY_METATILES+len(s.metatiles); s.metatiles.append(key)
        return s.mindex[key]
    def write(s,tsdir):
        print(os.path.basename(tsdir.rstrip('/')),': secondary tiles',len(s.tiles),'metatiles',len(s.metatiles),
              'palettes',sorted(s.pal))
        assert len(s.tiles)<=384 and len(s.metatiles)<=384
        from PIL import Image
        os.makedirs(tsdir+'palettes',exist_ok=True)
        im=Image.new('P',(128,(len(s.tiles)+15)//16*8),0)
        im.putpalette([v for c in s.pal[min(s.pal)] for v in c])
        for i,t in enumerate(s.tiles):
            for y in range(8):
                for x in range(8): im.putpixel(((i%16)*8+x,(i//16)*8+y),t[y][x])
        im.save(tsdir+'tiles.png')
        for p in range(16):
            cols=s.pal.get(p,[(0,0,0)]*16)
            with open(tsdir+'palettes/%02d.pal'%p,'w',newline='\r\n') as f:
                f.write('JASC-PAL\n0100\n16\n'+''.join('%d %d %d\n'%c for c in cols))
        with open(tsdir+'metatiles.bin','wb') as f:
            for ents,a in s.metatiles: f.write(struct.pack('<8H',*ents))
        with open(tsdir+'metatile_attributes.bin','wb') as f:
            for ents,a in s.metatiles: f.write(struct.pack('<H',a))

# ---- the maps ----
BLACK=8   # kanto_building's black edge (the border, and the south edge of each room)
class Room:
    def __init__(s,w,h):
        s.w,s.h=w,h
        s.cell=[[None]*w for _ in range(h)]; s.coll=[[0]*w for _ in range(h)]; s.beh=[[MB_NORMAL]*w for _ in range(h)]
    def put(s,x,y,cell,coll=0,beh=MB_NORMAL):
        if isinstance(cell,int): cell=mt(M,cell)
        s.cell[y][x]=cell; s.coll[y][x]=coll; s.beh[y][x]=beh
    def over(s,x,y,src,m,coll=1):
        s.put(x,y,over(s.cell[y][x],src,m),coll)
    def rows(s,x,y,rows,colls=None):
        # Mansion metatiles, a row of the list per map row; colls: '#' where it's solid.
        for j,r in enumerate(rows):
            for i,m in enumerate(r):
                if m is not None: s.put(x+i,y+j,m,1 if colls and colls[j][i]=='#' else 0)

def shell(R,windows=()):
    # The mansion's room: a north wall three cells high with windows, the side walls, the floor
    # with the walls' shadow, and the black south edge.
    w,h=R.w,R.h
    for x in range(w):
        i=0 if x==0 else 1 if x==1 else 3 if x==w-2 else 4 if x==w-1 else 2
        for y,row in enumerate(((0x2a0,0x2a1,0x2a2,0x2a3,0x2a4),(0x2a8,0x2a9,0x2aa,0x2ab,0x2ac),(0x2a8,0x2b1,0x2b2,0x2b3,0x2ac))):
            R.put(x,y,row[i],1)
        for y in range(3,h-1):
            if x in (0,w-1): R.put(x,y,0x2a8 if x==0 else 0x2ac,1)
            elif y==3: R.put(x,y,0x288 if x==1 else 0x289 if x==w-2 else 0x285)
            else: R.put(x,y,0x285 if x==1 else 0x284)
        R.put(x,h-1,mt(PRIM,BLACK),1)
    for x in windows: R.rows(x,1,((0x378,0x379),(0x380,0x381)),('##','##'))

def door(R,x,n,warp=True):
    # A door mat in the south edge, n (1-3) cells wide, from kanto_building's (0x1a-0x1c): its
    # pieces are the left end, two middle halves and the right end; the cells above are the warps.
    y=R.h-1
    pieces={1:(((0x1a,1),(0x1c,0)),),2:(((0x1a,1),(0x1b,0)),((0x1b,1),(0x1c,0))),
            3:(((None,0),(0x1a,1)),((0x1b,0),(0x1b,1)),((0x1c,0),(None,0)))}[n]
    for i,(a,b) in enumerate(pieces):
        R.put(x+i,y,quads(mt(PRIM,BLACK),[(PRIM,a[0],a[1]),(PRIM,b[0],b[1]),(PRIM,None,0),(PRIM,None,0)]),1)
        if warp: R.beh[y-1][x+i]=MB_SOUTH_ARROW_WARP

def stairs_up(R,x,y=2):
    # The mansion's stairs up (as on its 1F and 2F): the top step in the wall's base, entered from
    # the west; standing on the lower left step and walking east warps.
    R.put(x+2,y-1,0x374,1)
    R.rows(x,y,((0x2d5,0x2d6,0x2d7),(0x2dd,0x2de,0x2df),(0x2e5,0x2e6,0x2e7),(0x37a,0x28d,0x28d)),('###','.##','.##','...'))
    R.beh[y+1][x]=MB_EAST_ARROW_WARP
    return (x,y+1)
def stairs_down(R,x,y=3):
    # The mansion's stairs down (as on its 2F), under the north wall, entered from the east;
    # standing on the top right step and walking west warps.
    R.put(x,y-1,0x2d3,1)
    R.rows(x,y,((0x2d0,0x2d1,0x2d2),(0x2d8,0x2d9,0x2da),(0x2e0,0x2e1,0x2e2)),('##.','##.','...'))
    R.beh[y][x+2]=MB_WEST_ARROW_WARP
    return (x+2,y)
def wall_plant(R,x):
    R.rows(x,2,((0x324,),(0x323,)),('#','#'))
def plant(R,x,y):
    R.rows(x,y,((0x30b,),(0x313,)),('.','#'))
def statue(R,x,y,red=False):
    # The mansion's Mewtwo statue; red: with its eyes lit (the statue after the switch).
    R.rows(x,y,((0x34d if red else 0x314,),(0x31c,)),('.','#'))
def pillar(R,x,y):
    R.rows(x,y,((0x2b5,None),(0x2bd,None),(0x2c5,0x28c),(0x2cd,0x28c),(0x28d,0x295)),('..','#.','#.','#.','..'))
def table(R,x,y,book=False,chairs=(True,True)):
    R.rows(x,y,((0x308,0x309,0x30a),(0x375 if book else 0x310,0x311,0x312),(0x318,0x319,0x31a),(0x320,0x321,0x322)),
           ('...','###','###','...'))
    if chairs[0]: R.put(x-1,y+1,0x2f0)
    R.rows(x+3,y+1,((0x2f1 if chairs[1] else 0x28c,),(0x28c,),(0x28c,)))
def carpet(R,x,y0,y1):
    # The mansion's pink carpet, ending on a red mat at y1.
    for y in range(y0,y1+1):
        R.rows(x,y,(((0x2e8,0x2e9,0x2ea) if y==y1 else (0x2b6,0x2c6,0x2b7)),))
def exit_mat(R,x):
    # The mansion's way out: a red mat on the floor and a door mat in the south edge, three wide.
    R.rows(x,R.h-2,((0x348,0x349,0x34a),))
    door(R,x,3)

