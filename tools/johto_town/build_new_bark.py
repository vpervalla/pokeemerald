# Imports NEW BARK TOWN, Johto's starting town, from the user's reference map
# (reference/new_bark_town.jpg) as the kanto_new_bark_town tileset and the NewBarkTown layout.
#
# The reference is a JPEG of 16 px tile art scaled up about 1.24x: the map is x 0-438, y 46.5-385
# of it, 22x17 metatiles. It is scaled back down to its native 352x272 and cut on that 16 px grid.
# The black void around its top (outside the original map) is filled with the forest below it.
# Colours are reduced to six 15-colour palettes (slots 7-12), each 8x8 tile using one, and the
# tiles merged with their closest neighbours until they fit the 384 secondary tiles (as in
# tools/mega_island/build_castle.py). Every cell is its own metatile, drawn below the player.
# What blocks, the doors and the pond are set by hand in MAP below.
# PREVIEW=out.png renders the map.
import sys, os, struct
sys.path.insert(0,os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','mega_island'))
import numpy as np
from PIL import Image
import render, tilesetgen
HERE=os.path.dirname(os.path.abspath(__file__))+'/'
REPO=HERE+'../../'
TSDIR=REPO+'data/tilesets/secondary/kanto_new_bark_town/'
LAYOUT=REPO+'data/layouts/NewBarkTown/'
MW,MH=22,17

src=Image.open(HERE+'reference/new_bark_town.jpg').convert('RGB')
N=np.asarray(src.crop((0,46.5,438,384.8)).resize((MW*16,MH*16),Image.LANCZOS)).astype(int)

# ---- the void: the forest's rows 5-6 (two rows of trees, one per cell) repeated ----
FOREST=N[80:112,0:96].copy()
void=N.sum(2)<90
ys,xs=np.nonzero(void)
N[ys,xs]=FOREST[ys%32,xs%96]

# What each cell is: # blocks, . walkable, D door (MB_NON_ANIMATED_DOOR), W pond (MB_POND_WATER,
# surfable), S sign (blocks; its text is a bg event). Stairs between the levels are walkable, the
# ledges' edges block.
MAP='''\
######################
######################
######################
######################
######################
######################
######################
##################.###
#############D##D##.##
S..########...S#......
..........#........#WW
...######..##.#....#.W
..#######.#D#.S#######
#########......#D#####
###########.......####
######################
######################'''.split('\n')
assert len(MAP)==MH and all(len(r)==MW for r in MAP)

BUILDINGS=[(11,6,15,9),(15,5,19,9),(10,11,13,13),(15,11,18,14)]   # x0,y0,x1,y1 in cells
def in_building(tx,ty): return any(x0<=tx//2<x1 and y0<=ty//2<y1 for x0,y0,x1,y1 in BUILDINGS)

# ---- palettes: GBA colours, then six 15-colour palettes, one per 8x8 tile ----
cells=[(tx,ty) for ty in range(MH*2) for tx in range(MW*2)]
PX=np.array([N[ty*8:ty*8+8,tx*8:tx*8+8].reshape(64,3) for tx,ty in cells],float)
BLD=np.array([in_building(*c) for c in cells])
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
def tile_err(t,P): return ((PX[t][:,None]-P[None])**2).sum(2).min(1).sum()
# Start from tiles grouped by their mean colour, then alternately fit each palette to its tiles
# and move each tile to the palette that draws it best.
means=PX.mean(1)
grp=np.argmin(((means[:,None]-kmeans(means,6)[None])**2).sum(2),1)
for _ in range(6):
    # the buildings' pixels count six times, so their shades aren't lost among the forest's
    P=[gba(kmeans(np.concatenate([PX[(grp==g)&~BLD].reshape(-1,3)]+[PX[(grp==g)&BLD].reshape(-1,3)]*6),15))
       if (grp==g).any() else np.zeros((1,3)) for g in range(6)]
    grp=np.array([min(range(6),key=lambda g:tile_err(t,P[g])) for t in range(len(cells))])
P=[np.unique(p,axis=0) for p in P]
print('palettes',[len(p) for p in P])
PAL_RGB=[[(255,0,255)]+[tuple(int(v) for v in c) for c in p] for p in P]
for p in PAL_RGB: p+=[(0,0,0)]*(16-len(p))
tiles={}
for t,c in enumerate(cells):
    g=grp[t]
    tiles[c]=(g,(1+np.argmin(((PX[t][:,None]-P[g][None])**2).sum(2),1)).reshape(8,8))

# ---- merge near-identical tiles until they fit ----
def key(pi,t):
    return min((pi,t.tobytes()),(pi,t[:,::-1].tobytes()),(pi,t[::-1].tobytes()),(pi,t[::-1,::-1].tobytes()))
LIMIT=int(os.environ.get("LIMIT",383))                                 # secondary tile 0 is the transparent one
uniq={}
for c,(pi,t) in tiles.items(): uniq.setdefault(key(pi,t),[]).append(c)
print('unique tiles',len(uniq))
if len(uniq)>LIMIT:
    keys=list(uniq)
    rgb=np.array([[PAL_RGB[k[0]][v] for v in np.frombuffer(k[1],dtype=int)] for k in keys],float)
    pal_of=np.array([k[0] for k in keys])
    n=len(keys); D=np.full((n,n),np.inf)
    for i in range(n):
        d=((rgb[i]-rgb)**2).sum(2).sum(1)
        d[pal_of!=pal_of[i]]=np.inf; d[i]=np.inf; D[i]=d
    # Tiles of the buildings (whose details the forest would otherwise outbid) cost more to merge.
    for i,k in enumerate(keys):
        if any(in_building(*c) for c in uniq[k]):
            D[i,:]*=6; D[:,i]*=6
    alive=np.ones(n,bool); rep=np.arange(n); weight=np.array([len(uniq[k]) for k in keys],float)
    while alive.sum()>LIMIT:
        # merge the pair whose merging costs least (distance times the uses of the tile dropped)
        W=D*np.minimum(weight[:,None],weight[None,:])
        i,j=np.unravel_index(np.argmin(W),W.shape)
        keep,drop=(i,j) if weight[i]>=weight[j] else (j,i)
        alive[drop]=False; D[drop,:]=np.inf; D[:,drop]=np.inf
        weight[keep]+=weight[drop]
        rep[rep==drop]=keep
    for r,k in enumerate(keys):
        if rep[r]!=r:
            pi,tb=keys[rep[r]]
            t=np.frombuffer(tb,dtype=int).reshape(8,8)
            for c in uniq[k]:
                # the representative, flipped to match this tile's orientation as well as it can
                orig=tiles[c][1]
                tiles[c]=(pi,min((t,t[:,::-1],t[::-1],t[::-1,::-1]),key=lambda v:np.abs(v-orig).sum()))

# ---- the tileset and layout ----
ART_PAL=[c for p in PAL_RGB for c in p]
B=tilesetgen.Builder(tuple((7+i,p) for i,p in enumerate(PAL_RGB)),ART_PAL)
class Cv:
    # the art as palette-indexed pixels (index = palette*16 + colour), for the Builder
    def get(s,x,y):
        pi,t=tiles[(x//8,y//8)]
        return pi*16+t[y%8,x%8]
CV=Cv()
BEHAVIOR={'D':96,'W':16}                  # MB_NON_ANIMATED_DOOR, MB_POND_WATER
grid=[[0]*MW for _ in range(MH)]
coll=[[0]*MW for _ in range(MH)]
for y in range(MH):
    for x in range(MW):
        k=MAP[y][x]
        attr=(1<<12)|BEHAVIOR.get(k,0)    # COVERED: the art is below the player
        grid[y][x]=B.metatile(B.entries(CV,x*16,y*16),[tilesetgen.EMPTY_TOP]*4,attr)
        coll[y][x]=1 if k in '#S' else 0
B.check()
B.write_tileset(TSDIR)
# the border: a block of the forest (rows 5-6, two trees wide), solid
tilesetgen.write_layout(LAYOUT,grid,coll,tuple(grid[y][x]|0xc00|0x3000 for y,x in ((5,2),(5,3),(6,2),(6,3))))
print('written')
if os.environ.get('PREVIEW'):
    render.render_layout('LAYOUT_NEW_BARK_TOWN',os.environ['PREVIEW'])
