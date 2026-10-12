# Imports the castle grounds' tiles from the user's reference map (reference/castle_grounds.png) and
# writes them as the castle tileset (kanto_mega_castle), with the pieces laid out on an otherwise
# empty grass map (the castle grounds) to be built by hand.
#
# The reference is a smoothed upscale (about 1.4x) of 16 px tile art: it is scaled back down to
# its native size (640x420), and the pieces are cut out of it on its own 16 px grid. Grass around
# them becomes transparent, over Kanto's grass. The pieces:
#   - the castle (the biggest building), 10x14 metatiles, made symmetric from its right half, with
#     the banners and komainu in front of its ground floor taken out;
#   - the gate: the torii over the cobbled path, a statue at the foot of each pillar, the low wall
#     either side (rebuilt on a 16 px period from its clean panels, without the plants in front)
#     and its corners, 16x6 metatiles;
#   - a nobori banner either way round, a patch of cobbles;
#   - the Mewtwo statues (statues.py, drawn by hand after the user's reference), which stand where
#     the reference has komainu: at the gate's pillars, and as pieces of their own (plain, and with
#     red eyes).
# The castle's door is widened to fill its two cells (in the reference it straddles them).
# Colours are reduced to six 15-colour palettes (slots 7-12), each 8x8 tile using one, and the
# tiles merged with their closest neighbours until they fit the 384 secondary tiles.
# The castle grounds are built by hand from these pieces in Porymap, so this doesn't overwrite their
# layout: it keeps the pieces' own layout (which metatile each piece cell got) in
# reference/castle_pieces.bin, and translates the hand-built layout's metatiles from the old pieces
# layout to the new one, cell by cell. --pieces also writes the pieces laid out on grass as the
# layout (the start of a new design). PREVIEW=out.png renders the map.
import sys, os, struct, random
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
import numpy as np
from PIL import Image
import render, tilesetgen, statues
HERE=os.path.dirname(os.path.abspath(__file__))+'/'
REPO=HERE+'../../'
TSDIR=REPO+'data/tilesets/secondary/kanto_mega_castle/'
PRIM=render.TS('kanto_general',None)
random.seed(7)

N=np.asarray(Image.open(HERE+'reference/castle_grounds.png').convert('RGB').resize((640,420),Image.BOX)).astype(int)
AXIS=322    # the castle's and the torii's axis in the native image; the grid is at x = 2 mod 16

def is_bg(p):
    # grass, foliage and the light water around the pieces
    r,g,b=p
    return (g>r+18 and g>b+25) or (g>r+25 and b>r+15)

def rgba(x0,y0,w,h):
    # A piece of the native image as RGBA, transparent outside the image.
    out=np.zeros((h,w,4),int)
    for y in range(h):
        for x in range(w):
            if 0<=x0+x<640 and 0<=y0+y<420:
                out[y,x,:3]=N[y0+y,x0+x]; out[y,x,3]=255
    return out

def clear_bg(a,rows=None):
    # Makes the grass around a piece transparent: flood fill from its edges through grass.
    h,w=a.shape[:2]; seen=np.zeros((h,w),bool)
    stack=[(x,y) for x in range(w) for y in (0,h-1)]+[(x,y) for y in range(h) for x in (0,w-1)]
    while stack:
        x,y=stack.pop()
        if not(0<=x<w and 0<=y<h) or seen[y,x]: continue
        seen[y,x]=True
        if a[y,x,3]==0 or is_bg(a[y,x,:3]):
            a[y,x,3]=0
            stack+=[(x+1,y),(x-1,y),(x,y+1),(x,y-1)]

def mirror_right(a):
    # A symmetric piece from its right half.
    return np.concatenate([a[:,::-1],a],axis=1)

# ---- the castle: right half, x 322-402, y 0-224 ----
cr=rgba(AXIS,0,80,224)
def patch(dst_x0,dst_x1,y0,y1,src_dx=None,src_x=None):
    for y in range(y0,y1):
        for x in range(dst_x0,dst_x1):
            sx=x+src_dx if src_dx is not None else src_x
            cr[y,x-AXIS,:3]=N[y,sx]
patch(372,388,214,224,src_dx=-30)       # the banner: the window and wall from the next bay
patch(356,372,217,224,src_x=340)        # the komainu: plain wall, row by row
for y in range(17):                     # the bank of the island above, behind the roof's top
    for x in range(80):
        r,g,b=cr[y,x,:3]
        if r>b+15: cr[y,x,3]=0
clear_bg(cr)
CASTLE=mirror_right(cr)
def widen_door(c):
    # The door unit (4 px posts round a 16 px doorway) is 24 px wide, centred on the cell boundary at
    # x 80: stretch it to fill the two cells (x 64-96), the posts at their outer edges and the
    # doorway 24 px wide, with the transom's three lit panels redrawn even (rows 207-214).
    old=c[:,64:96].copy()
    def src(x): return 4+x if x<4 else 8+int((x-4)*8/12)
    for y in range(200,224):
        for x in range(16):
            c[y,64+x]=old[y,src(x)]; c[y,95-x]=old[y,src(x)]
    for y in range(207,215):
        G,P,Sd,D,Tt=(old[y,k] for k in (8,9,12,13,14))
        for i,v in enumerate((G,P,P,P,P,Sd,D,Tt,P,P,P,P)):
            c[y,68+i]=v; c[y,91-i]=v
widen_door(CASTLE)

# ---- the gate: right half, x 322-450, y 330-426 ----
GY=330
gr=rgba(AXIS,GY,128,96)
def wall_column(x):
    # The straight wall, y 380-407, on a 16 px period stretched from the clean bay at x 386-400.
    return 386+int(((x-2)%16)*14.7/16)
for x in range(370,432):                 # the wall behind the plants, rebuilt
    for y in range(380,406):
        gr[y-GY,x-AXIS,:3]=N[y,wall_column(x)]; gr[y-GY,x-AXIS,3]=255
for x in range(358,450):                 # the plants in front of the wall
    for y in range(406,426):
        gr[y-GY,x-AXIS,3]=0
for y in range(GY,376):                 # fence posts and banners around the gate
    for x in range(AXIS,450):
        if (x>=358 and not (x>=430 and y>=362)) or (x>340 and y<344): gr[y-GY,x-AXIS,3]=0
gr[:,447-AXIS:,3]=0                      # a tree beyond the corner
clear_bg(gr)
gr[np.array([[is_bg(p) for p in row] for row in gr[:,:,:3]]),3]=0   # grass between the beams too
def put_statue(a,x0,y0,red=False):
    for y,row in enumerate(statues.statue(red)):
        for x,p in enumerate(row):
            if p[3]: a[y0+y,x0+x]=p
STATUE=np.zeros((32,16,4),int); put_statue(STATUE,0,0)
STATUE_RED=np.zeros((32,16,4),int); put_statue(STATUE_RED,0,0,red=True)
gr[62:96,20:38,3]=0                      # the komainu at the pillar's foot (x 342-359, y 392-425)
put_statue(gr,16,64)                     # a statue under the pillar instead, in one cell (x 338-353)
GATE=mirror_right(gr)

# ---- small pieces ----
ban=rgba(372,298,16,64)
for y in range(64):                      # the plaza's cobbles behind its top
    for x in range(16):
        r,g,b=ban[y,x,:3]
        if max(r,g,b)-min(r,g,b)<14 and 150<r<215: ban[y,x,3]=0
clear_bg(ban)
cob=rgba(290,290,48,48)
PIECES={'castle':CASTLE,'gate':GATE,'banner_r':ban,'banner_l':ban[:,::-1].copy(),
        'statue_r':STATUE,'statue_l':STATUE,'statue_red':STATUE_RED,'cobbles':cob}

# ---- the map: grass, with the pieces laid out ----
MW,MH=60,40
PLACE={'castle':(25,2),'gate':(22,30),'banner_l':(16,18),'banner_r':(19,18),
       'statue_l':(16,24),'statue_r':(19,24),'statue_red':(22,24),'cobbles':(39,18)}
art=np.zeros((MH*16,MW*16,4),int)
for k,(cx,cy) in PLACE.items():
    a=PIECES[k]; h,w=a.shape[:2]
    reg=art[cy*16:cy*16+h,cx*16:cx*16+w]
    m=a[:,:,3]>0
    reg[m]=a[m]

# ---- palettes: GBA colours, then six 15-colour palettes, one per 8x8 tile ----
opq=art[:,:,3]>0
cells=[(tx,ty) for ty in range(MH*2) for tx in range(MW*2) if opq[ty*8:ty*8+8,tx*8:tx*8+8].any()]
PX=np.array([art[ty*8:ty*8+8,tx*8:tx*8+8,:3].reshape(64,3) for tx,ty in cells],float)
AL=np.array([opq[ty*8:ty*8+8,tx*8:tx*8+8].reshape(64) for tx,ty in cells])
rng=np.random.default_rng(7)
def kmeans(X,k,it=20):
    # k colours for the pixels X (k-means, from a spread-out start)
    X=np.unique(X,axis=0) if len(X)<=k else X
    C=[X[rng.integers(len(X))]]
    for _ in range(1,min(k,len(X))):
        d=np.min([((X-c)**2).sum(1) for c in C],axis=0)
        C.append(X[np.argmax(d)])
    C=np.array(C,float)
    for _ in range(it):
        lab=np.argmin(((X[:,None]-C[None])**2).sum(2),1)
        for j in range(len(C)):
            if (lab==j).any(): C[j]=X[lab==j].mean(0)
    return C
def gba(C): return np.minimum(248,np.round(C/8)*8)
def tile_err(t,P):
    X=PX[t][AL[t]]
    return ((X[:,None]-P[None])**2).sum(2).min(1).sum()
# Six palettes of 15 colours: start from tiles grouped by their mean colour, then alternately fit
# each palette to its tiles and move each tile to the palette that draws it best.
means=np.array([PX[t][AL[t]].mean(0) for t in range(len(cells))])
grp=np.argmin(((means[:,None]-kmeans(means,6)[None])**2).sum(2),1)
for _ in range(6):
    P=[gba(kmeans(PX[grp==g][AL[grp==g]],15)) if (grp==g).any() else np.zeros((1,3)) for g in range(6)]
    grp=np.array([min(range(6),key=lambda g:tile_err(t,P[g])) for t in range(len(cells))])
P=[np.unique(p,axis=0) for p in P]
print('palettes',[len(p) for p in P])
PAL_RGB=[[(0,0,0)]+[tuple(int(v) for v in c) for c in p] for p in P]
for p in PAL_RGB: p+=[(0,0,0)]*(16-len(p))
tiles={}
for t,c in enumerate(cells):
    g=grp[t]
    idx=1+np.argmin(((PX[t][:,None]-P[g][None])**2).sum(2),1)
    tiles[c]=(g,np.where(AL[t],idx,0).reshape(8,8))

# ---- merge near-identical tiles until they fit ----
def key(pi,t):
    return min((pi,t.tobytes()),(pi,t[:,::-1].tobytes()),(pi,t[::-1].tobytes()),(pi,t[::-1,::-1].tobytes()))
LIMIT=383                                 # secondary tile 0 is the transparent one
uniq={}
for c,(pi,t) in tiles.items(): uniq.setdefault(key(pi,t),[]).append(c)
print('unique tiles',len(uniq))
if len(uniq)>LIMIT:
    keys=list(uniq)
    rgb=np.array([[[0,0,0] if v==0 else PAL_RGB[k[0]][v] for v in np.frombuffer(k[1],dtype=int)] for k in keys],float)
    alpha=np.array([[v!=0 for v in np.frombuffer(k[1],dtype=int)] for k in keys])
    pal_of=np.array([k[0] for k in keys])
    n=len(keys); D=np.full((n,n),np.inf)
    for i in range(n):
        d=((rgb[i]-rgb)**2).sum(2).sum(1)+1e6*(alpha[i]!=alpha).sum(1)
        d[pal_of!=pal_of[i]]=np.inf; d[i]=np.inf; D[i]=d
    alive=np.ones(n,bool); rep=list(range(n)); weight=np.array([len(uniq[k]) for k in keys],float)
    while alive.sum()>LIMIT:
        # merge the pair whose merging costs least (distance times the uses of the tile dropped)
        W=D*np.minimum(weight[:,None],weight[None,:])
        i,j=np.unravel_index(np.argmin(W),W.shape)
        keep,drop=(i,j) if weight[i]>=weight[j] else (j,i)
        alive[drop]=False; D[drop,:]=np.inf; D[:,drop]=np.inf
        weight[keep]+=weight[drop]
        for r in range(n):
            if rep[r]==drop: rep[r]=keep
    for r,k in enumerate(keys):
        if rep[r]!=r:
            pi,tb=keys[rep[r]]
            t=np.frombuffer(tb,dtype=int).reshape(8,8)
            for c in uniq[k]:
                # the representative, flipped to match this tile's orientation as well as it can
                orig=tiles[c][1]
                tiles[c]=(pi,min((t,t[:,::-1],t[::-1],t[::-1,::-1]),key=lambda v:((v!=0)!=(orig!=0)).sum()+0.001*np.abs(v-orig).sum()))

# ---- the tileset and layout ----
ART_PAL=[c for p in PAL_RGB for c in p]
B=tilesetgen.Builder(tuple((7+i,p) for i,p in enumerate(PAL_RGB)),ART_PAL)
class Cv:
    # the art as palette-indexed pixels (index = palette*16 + colour), for the Builder
    def get(s,x,y):
        tx,ty=x//8,y//8
        if (tx,ty) not in tiles: return 0
        pi,t=tiles[(tx,ty)]; v=t[y%8,x%8]
        return pi*16+v if v else 0
CV=Cv()
GRASS=[0x8,0x8,0x8,0x9,0x10]
GRASS_BOTTOM=list(PRIM.meta[0x8][:4])
DOOR=[(29,15),(30,15)]                    # the castle door's two halves
grid=[[0]*MW for _ in range(MH)]
coll=[[0]*MW for _ in range(MH)]
for y in range(MH):
    for x in range(MW):
        e=B.cell_tiles(CV,x*16,y*16)
        if all(v is None for v in e):
            grid[y][x]=random.choice(GRASS); continue
        top=[B.tile_entry(*v) if v else tilesetgen.EMPTY_TOP for v in e]
        # COVERED (below the player), but the torii's beams are above it; the door is
        # MB_NON_ANIMATED_DOOR
        over=26<=x<=33 and 30<=y<=32
        attr=((0 if over else 1)<<12)|(96 if (x,y) in DOOR else 0)
        grid[y][x]=B.metatile(GRASS_BOTTOM,top,attr)
        # the pieces block, except the door, the path through the torii and the cobbles
        solid=opq[y*16:y*16+16,x*16:x*16+16].mean()>0.4
        if (x,y) in DOOR or (x in (29,30) and 30<=y<=35) or (39<=x<42 and 18<=y<21): solid=False
        coll[y][x]=1 if solid else 0
B.check()
B.write_tileset(TSDIR)
LAYOUT=REPO+'data/layouts/MegaIsland_Castle/'
PIECES_BIN=HERE+'reference/castle_pieces.bin'
if os.path.exists(PIECES_BIN) and '--pieces' not in sys.argv:
    # translate the hand-built layout: the old pieces layout says which piece cell each old secondary
    # metatile was, the new one which metatile that cell is now
    old=[v for (v,) in struct.iter_unpack('<H',open(PIECES_BIN,'rb').read())]
    votes={}
    for i,o in enumerate(old):
        if o>=640: votes.setdefault(o,{}).setdefault(grid[i//MW][i%MW],0); votes[o][grid[i//MW][i%MW]]+=1
    remap={o:max(v,key=v.get) for o,v in votes.items()}
    split={hex(o):sorted(hex(n) for n in v) for o,v in votes.items() if len(v)>1}
    if split: print('old metatiles whose cells now differ (the commonest wins):',split)
    hand=[v for (v,) in struct.iter_unpack('<H',open(LAYOUT+'map.bin','rb').read())]
    missing=sorted({hex(v&0x3ff) for v in hand if (v&0x3ff)>=640 and (v&0x3ff) not in remap})
    if missing: sys.exit('the castle grounds use metatiles that are not in the pieces layout: %s' % missing)
    hand=[(v&~0x3ff)|remap[v&0x3ff] if (v&0x3ff)>=640 else v for v in hand]
    open(LAYOUT+'map.bin','wb').write(b''.join(struct.pack('<H',v) for v in hand))
    print('castle grounds layout translated to the new metatiles')
else:
    tilesetgen.write_layout(LAYOUT,grid,coll,
        (0x1e|0xc00|0x3000,0x1f|0xc00|0x3000,0x16|0xc00|0x3000,0x17|0xc00|0x3000))
    print('pieces written as the castle grounds layout')
open(PIECES_BIN,'wb').write(b''.join(struct.pack('<H',grid[y][x]) for y in range(MH) for x in range(MW)))
print('written')
if os.environ.get('PREVIEW'):
    render.render_layout('LAYOUT_MEGA_ISLAND_CASTLE',os.environ['PREVIEW'])
