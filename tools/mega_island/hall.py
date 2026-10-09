# Draws the castle's interior: the entrance hall (1F) and the guest floor (2F).
# Imported by build_hall.py. Palettes: A for stone, glass, violet and gold; B for pale stone;
# C for wood, linen, rugs and candle light. Furniture fills whole 16 px cells so the floors
# around it (in other palettes) never share an 8x8 tile with it.
import math
from art import *

def marble(c,x0,y0,w,h):
    # Dark marble floor: 16 px squares, alternating shades, with bevelled edges.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            lx,ly=gx%16,gy%16
            dark=((gx//16)+(gy//16))%2
            col=S1 if dark else S2
            if lx==15 or ly==15: col=S0
            elif (lx==0 or ly==0) and not dark: col=S3
            elif lx+ly==20 and not dark: col=S3
            c.set(gx,gy,col)

def wall_face(c,x0,y0,w,h):
    # A wall seen from the front: blackstone courses above a wainscot.
    stone_wall(c,x0,y0,w,h)
    for gx in range(x0,x0+w):
        for gy in range(y0+h-10,y0+h):
            j=gy-(y0+h-10)
            c.set(gx,gy,S3 if j==0 else (S0 if j in (1,9) else S1))

def wall_top(c,x0,y0,w,h):
    # The top of a wall seen from above.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            col=S1
            if gx in (x0,x0+w-1) or gy in (y0,y0+h-1): col=OUT
            elif gy==y0+1: col=S0
            c.set(gx,gy,col)

def carpet(c,x0,y0,w,h):
    # A violet runner with gold borders and a diamond pattern (vertical).
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            i=gx-x0
            col=C1
            if i in (0,w-1): col=OUT
            elif i in (1,w-2): col=G
            elif i in (2,w-3): col=C0
            elif abs(i-(w-1)/2)+abs((gy%16)-7.5)<4: col=C0
            c.set(gx,gy,col)

def hcarpet(c,x0,y0,w,h):
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            j=gy-y0
            col=C1
            if j in (0,h-1): col=OUT
            elif j in (1,h-2): col=G
            elif j in (2,h-3): col=C0
            elif abs(j-(h-1)/2)+abs((gx%16)-7.5)<4: col=C0
            c.set(gx,gy,col)

def stairs_north(c,x0,y0,w,h):
    # Pale steps rising northwards (8 px each), shaded at the sides.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            j=(gy-y0)%8
            col=P4 if j==0 else (P3 if j<3 else (P2 if j<6 else P0))
            if gx-x0<2: col=S3 if j<6 else S2
            elif x0+w-1-gx<2: col=S1
            c.set(gx,gy,col)

def stairs_side(c,x0,y0,w,h,down_west):
    # Pale steps going down towards the west (or east), 8 px each.
    for gx in range(x0,x0+w):
        k=(gx-x0) if not down_west else (x0+w-1-gx)
        j=k%8
        for gy in range(y0,y0+h):
            col=P4 if j==0 else (P3 if j<3 else (P2 if j<6 else P0))
            if gy-y0<2: col=S3
            elif y0+h-1-gy<2: col=S1
            c.set(gx,gy,col)

def rail_v(c,x,y0,h):
    # A dark balustrade running north-south, 16 px wide cell (rail in the middle).
    for gy in range(y0,y0+h):
        for gx in range(x,x+16):
            i=gx-x
            if 4<=i<12:
                col=S3 if i<6 else (S2 if i<10 else S1)
                if gy%16 in (0,1): col=S3
                if i in (4,11): col=OUT
                c.set(gx,gy,col)

def pillar(c,x,y):
    # A dark column filling a 16x32 cell pair, with a gold band.
    for gy in range(y,y+32):
        for gx in range(x,x+16):
            i=gx-x
            col=S3 if i<4 else (S2 if i<10 else S1)
            if i in (0,15): col=OUT
            if gy-y<4: col=S3 if gy-y<2 else S0
            if gy-y in (12,13): col=G
            if y+32-gy<=3: col=S0
            c.set(gx,gy,col)

def candelabra(c,x,y):
    # A gold candelabra with three blue flames, 16x16, drawn on a wall.
    for i in range(3,13): c.set(x+i,y+9,G)
    c.vline(x+7,y+9,7,G); c.vline(x+8,y+9,7,G); c.hline(x+5,y+15,6,G)
    for cx in (4,8,12):
        c.vline(x+cx-1,y+5,4,S3)
        c.set(x+cx-1,y+3,W1); c.set(x+cx-1,y+4,W1); c.set(x+cx-1,y+2,R2)

def desk(c,x0,y0,w):
    # The reception desk: dark wood counter, 2 cells tall, with a gold trim, a bell and a lamp.
    for gy in range(y0,y0+32):
        for gx in range(x0,x0+w):
            j=gy-y0; i=gx-x0
            if j<10: col=WD3 if j>1 else (L0 if j==0 else WD2)
            elif j==10: col=OUT
            elif j==11 or j==29: col=G
            elif j>29: col=S0
            else:
                col=WD1 if ((i//16)%2) else WD2
                if i%16 in (0,15): col=WD0
            if i in (0,w-1): col=OUT
            c.set(gx,gy,col)
    bx=x0+w//2
    c.rect(bx-3,y0+4,6,3,G); c.set(bx,y0+3,G)              # bell
    c.rect(x0+12,y0+2,10,6,L1); c.hline(x0+12,y0+4,10,L0)    # open register
    c.rect(x0+w-20,y0+1,4,7,WD0); c.rect(x0+w-21,y0,6,2,FL)  # lamp

def wood_floor(c,x0,y0,w,h):
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            ly=gy%8; lx=gx%16
            off=8 if (gy//8)%2 else 0
            col=WD2 if (gy//8)%2 else WD1
            if ly==7: col=WD0
            elif (lx+off)%16==15: col=WD0
            elif ly==0: col=WD3 if col==WD2 else WD2
            c.set(gx,gy,col)

def rug(c,x0,y0,w,h):
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            i,j=gx-x0,gy-y0
            e=min(i,j,w-1-i,h-1-j)
            col=RG1
            if e==0: col=RG0
            elif e==1: col=G
            elif e==2: col=RG0
            elif (i+j)%8==0 or (i-j)%8==0: col=RG0
            c.set(gx,gy,col)

def bed(c,x,y):
    # 16x32 bed, head against the wall: headboard, pillow, violet blanket with gold trim.
    for gy in range(y,y+32):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            if j<6: col=WD0 if j<2 else WD1
            elif j<13: col=L0 if 2<=i<=13 else WD1
            elif j<29: col=C1 if 2<=i<=13 else (G if i in (1,14) else WD1)
            else: col=WD0
            if j>=13 and j<29 and 2<=i<=13 and (j-13)%5==0: col=C0
            if i in (0,15): col=OUT
            c.set(gx,gy,col)

def nightstand(c,x,y):
    for gy in range(y,y+16):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            col=WD3 if j<5 else (WD1 if j<14 else WD0)
            if j==9: col=WD0
            if i in (0,15): col=OUT
            c.set(gx,gy,col)
    c.vline(x+7,y+0,4,L0); c.set(x+7,y-2,FL); c.set(x+7,y-1,FL)   # candle (flame above)

def wardrobe(c,x,y):
    # 32x32 wardrobe against the wall.
    for gy in range(y,y+32):
        for gx in range(x,x+32):
            i,j=gx-x,gy-y
            col=WD1 if j>4 else WD2
            if j<2: col=WD0
            if i in (0,31) or j==31: col=OUT
            elif i in (15,16) and j>4: col=WD0
            elif (i in (3,28) or j in (7,27)) and j>4: col=WD2
            if j in (16,17) and i in (13,18): col=G
            c.set(gx,gy,col)

def table(c,x,y):
    for gy in range(y,y+16):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            col=L0 if j<9 else (L1 if j<11 else (WD0 if i in (2,13) else WD2))
            if j<11 and i in (0,15): col=L1
            c.set(gx,gy,col)
    c.rect(x+6,y+3,4,3,G)                                       # a little gold vase

# ---------------- maps ----------------
H1W,H1H=26,18        # entrance hall
H2W,H2H=30,20        # guest floor

def draw_hall(c):
    W,H=H1W*16,H1H*16
    marble(c,0,0,W,H)
    wall_face(c,16,0,W-32,64)                       # north wall, rows 0-3
    for x in (64,96,288,320):
        tall_window(c,x+2,8,24,52)
    for x in (128,264):
        long_banner(c,x,56,12)
    for x in (160,240): candelabra(c,x,32)
    pale_gate(c,192,8,32,56)                        # the door to the court (cols 12-13)
    for gx0 in (16,352):                            # openings to the upper floor above each flight
        for gy in range(8,64):
            for gx in range(gx0,gx0+48):
                c.set(gx,gy,OUT if gy<12 else S0)
    stairs_north(c,16,64,48,96)                     # west flight, cols 1-3 rows 4-9
    stairs_north(c,352,64,48,96)                    # east flight, cols 22-24
    rail_v(c,64,64,96); rail_v(c,336,64,96)         # balustrades, cols 4 and 21
    wall_top(c,0,0,16,H); wall_top(c,W-16,0,16,H)   # side walls
    carpet(c,192,64,32,48)                          # from the door to the desk
    carpet(c,192,160,32,H-160)                      # from the desk to the entrance
    desk(c,144,112,128)                             # cols 9-16, rows 7-8
    for (x,y) in ((96,80),(304,80),(96,208),(304,208)): pillar(c,x,y)
    for gx in range(192,224):                       # doormat at the entrance (row 17)
        for gy in range(H-16,H):
            c.set(gx,gy,G if (gy%4==0 or gx in (192,223)) else C0)

ROOM_COLS=((1,8),(9,15),(16,22),(23,29))           # four rooms per side: [x0, x1) in cells

def furnish(c,x0,x1,y0,wall_rows):
    # A guest room on wood: bed and nightstand against the wall, a wardrobe, a rug and a table.
    X0,X1=x0*16,x1*16
    wood_floor(c,X0,y0*16,X1-X0,5*16)
    bed(c,X0+16,y0*16); nightstand(c,X0+32,y0*16)
    wardrobe(c,X1-48,y0*16)
    rug(c,X0+16,y0*16+48,X1-X0-48,32)
    table(c,X1-32,y0*16+64)

def draw_guest_floor(c):
    W,H=H2W*16,H2H*16
    marble(c,0,0,W,H)
    # north rooms: back wall (rows 0-1) with a window each, floor rows 2-6
    for (x0,x1) in ROOM_COLS:
        wall_face(c,x0*16,0,(x1-x0)*16,32)
        tall_window(c,(x0+x1)*8-12,4,24,22)
        furnish(c,x0,x1,2,2)
    # wall between the north rooms and the corridor: top row 7, face row 8, doorways
    wall_top(c,0,112,W,16); wall_face(c,16,128,W-32,16)
    # south rooms: back wall face rows 12-13 (seen from inside), floor rows 14-18
    wall_top(c,0,192,W,8); wall_face(c,16,200,W-32,24)
    for (x0,x1) in ROOM_COLS:
        furnish(c,x0,x1,14,2)
    # partitions between rooms (wall tops), outer walls
    for xc in (8,15,22):
        wall_top(c,xc*16,0,16,112); wall_top(c,xc*16,192,16,H-192)
    wall_top(c,0,0,16,H); wall_top(c,W-16,0,16,H); wall_top(c,0,H-16,W,16)
    # the corridor: rows 9-11, violet runner, stairwells down at both ends
    hcarpet(c,48,152,W-96,16)
    stairs_side(c,16,144,32,48,True); stairs_side(c,W-48,144,32,48,False)
    # doorways: one per room into the corridor (north rooms at rows 7-8, south rooms at 12-13)
    for (x0,x1) in ROOM_COLS:
        dx=(x0+x1)//2
        wood_floor(c,dx*16,112,16,32); wood_floor(c,dx*16,192,16,32)
    for (x0,x1) in ROOM_COLS:                       # candelabras on the corridor walls
        candelabra(c,(x0+1)*16,128)
