# Draws the castle's interior from the game's high camera: walls show a pale top edge and a short
# face (2 cells), floors fill the view, stairs are diagonal flights in the corners.
# Imported by build_hall.py. Palettes: A for stone, glass, violet and gold; B for pale stone;
# C for wood, linen, rugs and candle light. Pieces in different palettes fill whole 16 px cells
# (or 8 px tiles) so they never share an 8x8 tile.
from art import *

# ---------------- floors ----------------
def marble(c,x0,y0,w,h):
    # Dark marble squares seen from above.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            lx,ly=gx%16,gy%16
            dark=((gx//16)+(gy//16))%2
            col=S1 if dark else S2
            if lx==15 or ly==15: col=S0
            elif (lx==0 or ly==0) and not dark: col=S3
            c.set(gx,gy,col)

def planks(c,x0,y0,w,h):
    # Wooden floorboards seen from above (running east-west).
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            row=gy//8; ly=gy%8
            col=WD2 if row%2 else WD1
            if ly==7: col=WD0
            elif (gx+(row%4)*12)%48==47: col=WD0
            elif ly==0: col=WD3 if col==WD2 else WD2
            c.set(gx,gy,col)

def runner(c,x0,y0,w,h,horizontal):
    # A violet carpet runner with gold borders.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            i=(gy-y0) if horizontal else (gx-x0)
            n=h if horizontal else w
            along=gx if horizontal else gy
            col=C1
            if i in (0,n-1): col=OUT
            elif i in (1,n-2): col=G
            elif i in (2,n-3): col=C0
            elif abs(i-(n-1)/2)+abs((along%16)-7.5)<4: col=C0
            c.set(gx,gy,col)

# ---------------- walls ----------------
def wall_north(c,x0,x1,y0=0):
    # The wall along the top of a room: a pale top edge (8 px), a dark stone face (20 px) and a
    # baseboard (4 px). 2 cells tall.
    for gx in range(x0,x1):
        for j in range(32):
            gy=y0+j
            if j<2: col=S3
            elif j<6: col=S2
            elif j==6: col=S0
            elif j==7: col=OUT
            elif j>=28: col=S0 if j<31 else OUT
            else:
                course=(j-8)//5; by=(j-8)%5; bx=(gx+(course%2)*8)%16
                col=S1 if by==4 or bx==15 else S2
            c.set(gx,gy,col)

def wall_side(c,x,y0,y1,w=16):
    # The top of a side wall: a pale edge along the room and dark beyond.
    for gy in range(y0,y1):
        for gx in range(x,x+w):
            c.set(gx,gy,S1)
    edge=x+w-1 if x<64 else x
    for gy in range(y0,y1):
        c.set(edge,gy,S3); c.set(edge+(-1 if x<64 else 1),gy,S2)

def wall_bottom(c,x0,x1,y):
    for gx in range(x0,x1):
        for j in range(16):
            c.set(gx,y+j,S3 if j==0 else (S2 if j==1 else S1))

def window(c,x,y):
    # A small arched window, 16x20, on a wall face.
    for j in range(20):
        for i in range(16):
            half=6 if j>5 else int(6*(1-((6-j)/6)**2)**0.5+0.5)
            d=abs(i-7.5)
            if d<half-1: col=W0 if (i+j)%4 else W1
            elif d<half+0.5: col=S3
            else: continue
            if d<0.6 and j>3: col=OUT
            c.set(x+i,y+j,col)

def door(c,x,y):
    # A wooden door with a gold plaque, 16x24, on a wall face (bottom at y+24).
    for j in range(24):
        for i in range(16):
            if i in (0,15) or j==0: col=OUT
            elif i in (1,14) or j==1: col=WD3
            else:
                col=WD1 if i<8 else WD0
                if j in (6,18): col=WD0
                if i==11 and j in (12,13): col=G
            c.set(x+i,y+j,col)
    c.rect(x+5,y+3,6,2,G)

def candelabra(c,x,y):
    # A small gold candelabra with three blue flames, on a wall face (12 px tall).
    c.hline(x+4,y+8,8,G); c.vline(x+7,y+8,4,G); c.vline(x+8,y+8,4,G)
    for cx in (5,8,11):
        c.vline(x+cx-1,y+5,3,S3); c.set(x+cx-1,y+4,W1); c.set(x+cx-1,y+3,R2)

# ---------------- stairs ----------------
def stairs_diag(c,x0,y0,up_left):
    # A diagonal flight in a 48x48 box, rising towards the upper-left (or upper-right) corner,
    # with a railing along its outer side. Steps are 8 px tall, shifted 4 px per step.
    for k in range(6):                     # k=0: top step
        y=y0+k*8
        sx=(x0+4+k*4) if up_left else (x0+48-4-k*4-28)
        for j in range(8):
            for i in range(28):
                col=P4 if j==0 else (P3 if j<3 else (P2 if j<6 else P0))
                if (i==0 and up_left) or (i==27 and not up_left): col=S2
                c.set(sx+i,y+j,col)
        # railing post and rail on the outer side
        rx=sx+28 if up_left else sx-3
        for j in range(8): c.set(rx,y+j,S3); c.set(rx+1,y+j,S2); c.set(rx+2,y+j,OUT)
        c.rect(rx,y,3,2,P4)

# ---------------- furniture ----------------
def desk(c,x0,y0,w):
    # The reception desk seen from above: a wide pale-wood top and a short front with gold trim.
    for gy in range(y0,y0+32):
        for gx in range(x0,x0+w):
            j=gy-y0; i=gx-x0
            if j<20: col=WD3 if j>1 else WD2
            elif j==20: col=OUT
            elif j==21 or j==29: col=G
            elif j>29: col=S0
            else: col=WD1 if ((i//16)%2) else WD2
            if i in (0,w-1): col=OUT
            c.set(gx,gy,col)
    bx=x0+w//2
    c.rect(bx-3,y0+8,6,4,G); c.set(bx,y0+7,G)               # bell
    c.rect(x0+12,y0+5,14,10,L1); c.rect(x0+13,y0+6,5,8,L0); c.rect(x0+20,y0+6,5,8,L0)   # register
    c.rect(x0+w-22,y0+6,6,6,WD0); c.rect(x0+w-21,y0+4,4,3,FL)   # lamp

def pillar(c,x,y):
    # A dark column (16x32): round top seen from above, short shaft.
    for gy in range(y,y+32):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            if j<12:
                d=((i-7.5)/8)**2+((j-6)/6)**2
                if d>1: continue
                col=S3 if d<0.35 else (S2 if d<0.8 else OUT)
            else:
                col=S3 if i<4 else (S2 if i<10 else S1)
                if i in (0,15): col=OUT
                if j in (16,17): col=G
                if j>=29: col=S0
            c.set(gx,gy,col)

def bed(c,x,y):
    # 16x32 bed seen from above: headboard, pillow, violet blanket with gold edge.
    for gy in range(y,y+32):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            if j<4: col=WD0 if j<1 else WD1
            elif j<11: col=L0 if 2<=i<=13 else WD1
            elif j<30: col=C1 if 2<=i<=13 else (G if i in (1,14) else WD1)
            else: col=WD0
            if 11<=j<30 and 2<=i<=13 and j==12: col=L0              # folded sheet
            if i in (0,15): col=OUT
            c.set(gx,gy,col)

def nightstand(c,x,y):
    for gy in range(y,y+16):
        for gx in range(x,x+16):
            i,j=gx-x,gy-y
            col=WD3 if j<11 else (WD1 if j<15 else WD0)
            if i in (1,14) and j<15: col=WD0
            if i in (0,15): col=0
            if col: c.set(gx,gy,col)
    c.rect(x+6,y+3,4,5,L0); c.set(x+7,y+1,FL); c.set(x+8,y+1,FL); c.set(x+7,y+2,FL)

def wardrobe(c,x,y):
    # 32x32 wardrobe against the wall: top seen from above (8 px), front face below.
    for gy in range(y,y+32):
        for gx in range(x,x+32):
            i,j=gx-x,gy-y
            col=WD2 if j<8 else WD1
            if j==8: col=WD0
            if i in (0,31) or j==31: col=OUT
            elif i in (15,16) and j>8: col=WD0
            if j in (19,20) and i in (13,18): col=G
            c.set(gx,gy,col)

def table_set(c,x,y):
    # A round table with a cloth, seen from above, and two chairs (32x16).
    for gy in range(y,y+16):
        for gx in range(x,x+32):
            i,j=gx-x,gy-y
            d=((i-15.5)/11)**2+((j-7)/6.5)**2
            if d<=1: col=L0 if d<0.6 else (L1 if d<0.9 else OUT)
            elif (i<5 or i>26) and 4<=j<=12: col=WD1 if 1<=j%8<7 else WD0
            else: continue
            c.set(gx,gy,col)
    c.rect(x+14,y+4,4,4,G)

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

def doormat(c,x,y,w):
    for gx in range(x,x+w):
        for gy in range(y,y+16):
            c.set(gx,gy,G if (gy%4==0 or gx in (x,x+w-1)) else C0)

# ---------------- maps ----------------
H1W,H1H=26,16        # entrance hall
H2W,H2H=26,9         # guest corridor
RW,RH=11,9           # a guest room
DOORS=(4,8,12,13,17,21)   # door columns on the guest corridor (12-13 is the double door... see below)

def draw_hall(c):
    W,H=H1W*16,H1H*16
    marble(c,0,0,W,H)
    wall_north(c,16,W-16)
    for x in (88,120,280,312): window(c,x,6)
    for x in (152,248): candelabra(c,x,12)
    pale_gate(c,192,0,32,32)                         # the door to the court (cols 12-13, row 1)
    for (x0,ul) in ((16,True),(W-64,False)):         # stairs up in both top corners
        for gy in range(0,16):                       # dark landing through the wall
            for gx in range(x0,x0+48): c.set(gx,gy,S0 if gy>3 else OUT)
        stairs_diag(c,x0,16,ul)
    wall_side(c,0,0,H); wall_side(c,W-16,0,H)
    runner(c,192,32,32,48,False)                     # from the door to the desk
    runner(c,192,112,32,H-128,False)                 # from the desk to the entrance
    desk(c,144,80,128)                               # cols 9-16, rows 5-6
    for (x,y) in ((96,64),(304,64),(96,160),(304,160)): pillar(c,x,y)
    doormat(c,192,H-16,32)

def draw_corridor(c):
    W,H=H2W*16,H2H*16
    marble(c,0,0,W,H)
    wall_north(c,16,W-16)
    planks(c,16,32,W-32,H-48)
    runner(c,48,56,W-96,16,True)
    for x in (4,7,10,15,18,21): door(c,x*16,8)
    for x in (88,136,184,216,264,312): candelabra(c,x,12)   # between the doors
    for (x0,ul) in ((16,False),(W-64,True)):         # stairs down in both bottom corners
        marble(c,x0,H-64,48,48)
        stairs_diag(c,x0,H-64,ul)
    wall_side(c,0,0,H); wall_side(c,W-16,0,H)
    wall_bottom(c,0,W,H-16)

def draw_room(c):
    W,H=RW*16,RH*16
    planks(c,0,0,W,H)
    wall_north(c,16,W-16)
    window(c,72,6); window(c,104,6)
    wall_side(c,0,0,H); wall_side(c,W-16,0,H)
    wall_bottom(c,0,W,H-16)
    bed(c,32,32); nightstand(c,48,32)
    wardrobe(c,128,16)
    rug(c,48,72,64,32)
    table_set(c,112,96)
    doormat(c,80,H-32,16)
