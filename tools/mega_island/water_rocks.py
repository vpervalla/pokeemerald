# Rocks in the water of the island's route and port, in unbroken lines near the maps' edges, so a
# surfer can't reach the edges: on the route, across both ends of the river; at the port, down both
# sides of the sea and across it below the pier (the harbor building closes the line in the middle).
# FRLG's small grey rock in the sea (kanto_general, 2x2 with its darker water, as in Five Island's
# harbour), solid at elevation 0. A rock goes only where its four cells are water (or already a rock),
# so this can be run again; run it after build_route.py, which writes the maps without them.
import os, struct
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
ROCK=(0x1cb,0x1cc,0x1d3,0x1d4)
BOULDER=(0x1db,0x1dc,0x1e3,0x1e4)  # (the big brown boulder, no longer used: replaced by the rock)
MB_OCEAN_WATER=21
attr=[a for (a,) in struct.iter_unpack('<H',open(REPO+'data/tilesets/primary/kanto_general/metatile_attributes.bin','rb').read())]
attr+=[a for (a,) in struct.iter_unpack('<H',open(REPO+'data/tilesets/secondary/kanto_sevii_islands_45/metatile_attributes.bin','rb').read())]
PIECES=set(ROCK)|set(BOULDER)
def place(layout,w,corners):
    p=REPO+'data/layouts/%s/map.bin'%layout
    d=[v for (v,) in struct.iter_unpack('<H',open(p,'rb').read())]
    n=0
    for k,(x,y) in enumerate(corners):
        cells=[(x,y),(x+1,y),(x,y+1),(x+1,y+1)]
        ms=[d[cy*w+cx]&0x3ff for cx,cy in cells]
        if all(m in ROCK for m in ms): continue
        assert all(attr[m]&0xff==MB_OCEAN_WATER or m in PIECES for m in ms), (layout,x,y,[hex(m) for m in ms])
        for (cx,cy),m in zip(cells,ROCK): d[cy*w+cx]=m|1<<10
        n+=1
    open(p,'wb').write(b''.join(struct.pack('<H',v) for v in d))
    print(layout,': %d rocks placed'%n)
# The route's river, rows 3-6: two rocks at each end.
place('MegaIsland',30,[(1,3),(1,5),(27,3),(27,5)])
# The port's sea, from row 11: down the left edge (x 0-1) and the right (x 27-28), and across rows
# 15-16 from them to the harbor building (x 12-18).
place('MegaIsland_Port',30,[(0,11),(0,13),(0,15)]+[(x,15) for x in range(2,12,2)]
                          +[(27,11),(27,13),(27,15)]+[(x,15) for x in range(19,27,2)])
