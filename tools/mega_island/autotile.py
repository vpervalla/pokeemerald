# Learns FRLG's rock-mountain and water autotiling from the Kanto maps: for every cell of a terrain,
# which metatile is used given which of its 8 neighbours are the same terrain.
import json, struct, os
from collections import Counter, defaultdict
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
ROCK=set([0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x70,0x71,0x72,0x73,0x75,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0xb2,0xb3,0xba,0xbb])
WATER=set([0x122,0x123,0x124,0x12a,0x12b,0x12c,0x130,0x131,0x132,0x133,0x134,0x135])
NB=[(-1,-1),(0,-1),(1,-1),(-1,0),(1,0),(-1,1),(0,1),(1,1)]
def mask(grid,x,y,isin,w,h):
    m=0
    for i,(dx,dy) in enumerate(NB):
        xx,yy=x+dx,y+dy
        inside=isin(grid[yy][xx]) if (0<=xx<w and 0<=yy<h) else True
        if inside: m|=1<<i
    return m
def learn(terrain, maps):
    L={l['id']:l for l in json.load(open(REPO+'data/layouts/layouts.json'))['layouts'] if 'id' in l}
    stats=defaultdict(Counter)
    for mid in maps:
        l=L[mid]; w,h=l['width'],l['height']
        d=[v&0x3ff for (v,) in struct.iter_unpack('<H',open(REPO+l['blockdata_filepath'],'rb').read())]
        grid=[d[y*w:(y+1)*w] for y in range(h)]
        for y in range(h):
            for x in range(w):
                if grid[y][x] in terrain:
                    stats[mask(grid,x,y,lambda m: m in terrain,w,h)][grid[y][x]]+=1
    return {k:c.most_common(1)[0][0] for k,c in stats.items()}, stats
MAPS=['LAYOUT_ROUTE3','LAYOUT_ROUTE4','LAYOUT_ROUTE9','LAYOUT_ROUTE10','LAYOUT_ROUTE22','LAYOUT_ROUTE23','LAYOUT_ROUTE25','LAYOUT_ONE_ISLAND_KINDLE_ROAD','LAYOUT_FOUR_ISLAND','LAYOUT_ROUTE24','LAYOUT_SIX_ISLAND_WATER_PATH']
def fill(design, terrain_char, table):
    # design: list of strings; returns {(x,y): metatile} for the terrain cells
    h=len(design); w=len(design[0]); out={}
    for y in range(h):
        for x in range(w):
            if design[y][x]!=terrain_char: continue
            m=mask(design,x,y,lambda c: c==terrain_char,w,h)
            if m in table: out[(x,y)]=table[m]; continue
            # nearest learned mask (fewest differing neighbours, sides weighted more)
            def dist(k):
                diff=k^m
                return sum((2 if i in (1,3,4,6) else 1) for i in range(8) if diff>>i&1)
            out[(x,y)]=table[min(table,key=dist)]
    return out
if __name__=='__main__':
    for name,T in (('rock',ROCK),('water',WATER)):
        tab,st=learn(T,MAPS); print(name,len(tab),'masks')
