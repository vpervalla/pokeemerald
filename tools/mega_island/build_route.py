# Builds the island route (MegaIsland): harbor and pier in the south, beach, meadow, cliffs,
# and a forest corridor leading north to the castle grounds. Tilesets: kanto_general + sevii 45.
import struct, random, os
random.seed(11)
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
W,H=30,58
WATER,SAND,TALL=0x12b,0x115,0xd
GRASS=[0x8,0x8,0x9,0x10]
g=[[None]*W for _ in range(H)]; col=[[0]*W for _ in range(H)]
def put(x,y,m,c=0): g[y][x]=m; col[y][x]=c
def grass(x,y): put(x,y,random.choice(GRASS))
for y in range(H):
    for x in range(W): put(x,y,WATER)
def tree(x,y,below_tree=False):
    put(x,y,0x1e,1); put(x+1,y,0x1f,1)
    put(x,y+1,0x16 if below_tree else 0x24,1); put(x+1,y+1,0x17 if below_tree else 0x25,1)
def forest(x0,y0,w,h,tips=True):
    for ty in range(y0,y0+h,2):
        for tx in range(x0,x0+w,2):
            tree(tx,ty,below_tree=(ty+2<y0+h))
    if tips and y0>0:
        for tx in range(x0,x0+w,2):
            put(tx,y0-1,0xe); put(tx+1,y0-1,0xf)
def lone_tree(x,y):  # tip at y, tree at y+1..y+2 (on grass)
    put(x,y,0xe); put(x+1,y,0xf); tree(x,y+1)
def cliff_band(y,x0,x1,stairs):
    for x in range(x0,x1+1): put(x,y,0x79,1)
    put(stairs,y,0x91); put(stairs+1,y,0x89)
# --- land: rows 0-45 ---
for y in range(0,46):
    for x in range(W): grass(x,y)
# north forest corridor (path cols 12-17)
forest(0,0,12,16,tips=False); forest(18,0,12,16,tips=False)
for y in range(0,16):
    for x in (12,17):
        if random.random()<0.5: put(x,y,TALL)
forest(0,16,4,6); forest(26,16,4,6)
cliff_band(19,4,25,14)
# west / east coasts from row 22 down to the beach
for y in range(22,46):
    put(0,y,WATER); put(1,y,WATER); put(2,y,0x12c); put(3,y,0x73,1)
    put(26,y,0x75,1); put(27,y,0x12a); put(28,y,WATER); put(29,y,WATER)
# meadow: tall grass patches and trees
def patch(x0,y0,w,h):
    for y in range(y0,y0+h):
        for x in range(x0,x0+w): put(x,y,TALL)
patch(5,21,6,4); patch(19,22,6,5); patch(8,27,5,3); patch(15,33,7,4); patch(5,35,5,3)
for (tx,ty) in ((11,21),(5,27),(22,28),(17,26),(10,32),(23,35)):
    lone_tree(tx,ty)
cliff_band(31,4,25,8)
cliff_band(39,4,25,18)
# beach rows 40-44
for y in range(40,46):
    for x in range(4,26): put(x,y,SAND)
for y in range(40,46): put(3,y,SAND); put(26,y,SAND)
for y in range(40,46): put(2,y,0x12c); put(27,y,0x12a)
# south coast row 46 (from Five Island) with the pier start
for x in range(3,27): put(x,46,0x123,1)
put(2,46,0x130,1); put(27,46,0x131,1)
# copy the pier and harbor building from Five Island rows 13-19, cols 9-15 -> rows 46-52, cols 12-18
fd=list(struct.iter_unpack('<H',open(REPO+'data/layouts/FiveIsland/map.bin','rb').read()))
for dy in range(0,7):
    for dx in range(0,7):
        v=fd[(13+dy)*24+9+dx][0]
        if dy==0 and (v&0x3ff)==0x123: continue
        put(12+dx,46+dy,v&0x3ff,(v>>10)&3)
# deep water at the very bottom
for y in range(54,H):
    for x in range(W):
        if g[y][x]==WATER: put(x,y,0x1d9)
LD=REPO+'data/layouts/MegaIsland/'
os.makedirs(LD,exist_ok=True)
with open(LD+'map.bin','wb') as f:
    for y in range(H):
        for x in range(W): f.write(struct.pack('<H',g[y][x]|(col[y][x]<<10)|(3<<12)))
with open(LD+'border.bin','wb') as f:
    f.write(struct.pack('<4H',*([WATER|0x3000]*4)))
print('route written',W,H)
