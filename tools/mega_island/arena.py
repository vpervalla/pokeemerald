# Draws the castle's inner courtyard, used as the tournament stadium. Imported by build_arena.py.
# Map 32x30 cells. Rows 0-8: the inner face of the keep with the host's balcony; rows 9-11 and
# cols 0-4 / 27-31: tiered stands; rows 12-24: a paved walkway around the field (rows 13-23,
# cols 7-24); rows 25-29: the south wall top with the entrance passage (cols 15-16).
from art import *

MW,MH=32,30
CX=256                       # centre axis (between cols 15 and 16)
WALL_B=144                   # bottom of the north wall (row 9)
FX0,FX1,FY0,FY1=112,400,208,384   # the field

def bench_tiers_south(c,x0,x1,y0,tiers):
    # Stands facing south (seen from the front): each 16 px tier is a pale bench over a dark riser.
    for t in range(tiers):
        y=y0+t*16
        for gx in range(x0,x1):
            for j in range(16):
                if j<5: col=P4 if j==0 else (P3 if j<3 else P2)
                elif j==5: col=OUT
                else: col=S1 if j<14 else S0
                if j>5 and gx%16==15: col=S0
                c.set(gx,y+j,col)

def bench_tiers_side(c,x0,y0,y1,tiers,face):
    # Side stands, seen from above: tiers step down towards the field. face=+1: the field is to the
    # east (west stand), -1: to the west. Each tier is 16 px: a pale bench and a dark riser.
    for t in range(tiers):
        for i in range(16):
            gx=x0+t*16+i if face>0 else x0-t*16-i-1
            k=i if face>0 else i      # distance from the tier's back edge
            for gy in range(y0,y1):
                if k<10: col=P3 if k<2 else (P2 if k<8 else P1)
                elif k==10: col=OUT
                else: col=S1 if k<15 else S0
                if gy%16==15 and k<10: col=P0
                c.set(gx,gy,col)

def brazier(c,x,y):
    # 16x32: a dark pedestal with a bowl of blue fire.
    for j in range(14,32):
        for i in range(3,13):
            col=S2 if i<6 else (S1 if i<11 else S0)
            if j in (14,15): col=S3
            if j==31: col=OUT
            c.set(x+i,y+j,col)
    for i in range(1,15):
        c.set(x+i,y+10,G); c.set(x+i,y+11,S1); c.set(x+i,y+12,S0)
    flame=[(7,1),(8,1),(6,2),(9,2),(5,3),(10,3),(5,4),(6,4),(9,4),(10,4),(4,5),(11,5),(4,6),(11,6),(3,7),(12,7),(3,8),(12,8),(3,9),(12,9)]
    for j in range(1,10):
        for i in range(3,13):
            d=abs(i-7.5)
            if d<(j+1)*0.6: c.set(x+i,y+j,P4 if d<j*0.25 else (R2 if d<j*0.45 else R1))

def field(c):
    # Pale sand with white lines, trainer boxes at both ends and a Mega emblem in the centre circle.
    for gy in range(FY0,FY1):
        for gx in range(FX0,FX1):
            lx,ly=gx%16,gy%16
            col=P2
            if (lx*3+ly*5)%11==0: col=P1
            if (lx,ly) in ((4,12),(13,3)): col=P3
            c.set(gx,gy,col)
    def line_rect(x0,y0,x1,y1):
        for gx in range(x0,x1):
            for w in (0,1): c.set(gx,y0+w,P4); c.set(gx,y1-1-w,P4)
        for gy in range(y0,y1):
            for w in (0,1): c.set(x0+w,gy,P4); c.set(x1-1-w,gy,P4)
    line_rect(FX0+8,FY0+8,FX1-8,FY1-8)
    cy=(FY0+FY1)//2
    for gx in range(FX0+8,FX1-8): c.set(gx,cy-1,P4); c.set(gx,cy,P4)
    line_rect(CX-32,FY0+8,CX+32,FY0+40); line_rect(CX-32,FY1-40,CX+32,FY1-8)   # trainer boxes
    import math
    for gy in range(cy-34,cy+34):
        for gx in range(CX-34,CX+34):
            d=math.hypot(gx+0.5-CX,gy+0.5-cy)
            if 30<=d<32: c.set(gx,gy,P4)
            elif d<22:
                # Mega Evolution emblem: a gold ring around a blue three-armed swirl
                a=math.atan2(gy+0.5-cy,gx+0.5-CX)
                sw=(math.sin(3*a+d*0.35)+1)/2
                if d>=18: col=G
                elif d>=16.5: col=OUT
                elif sw>0.62: col=R2 if d<9 else R1
                elif d<4: col=G
                else: col=S1
                c.set(gx,gy,col)

def balcony(c):
    # The host's box in the middle of the north wall: violet drapes, a throne, a pale balustrade.
    x0,x1=CX-48,CX+48
    for gy in range(88,140):           # drapes
        for gx in range(x0,x1):
            k=(gx-x0)%12
            col=C1 if k<6 else C0
            if gy<92: col=G
            c.set(gx,gy,col)
    for gx in range(x0,x1):            # gathered curtain edge
        if (gx-x0)%12 in (5,6,11,0): c.set(gx,140,C0); c.set(gx,141,C0)
    tx=CX-12                           # throne
    for gy in range(104,148):
        for gx in range(tx,tx+24):
            dx=min(gx-tx,tx+23-gx)
            if gy<112 and dx<6+(gy-104): continue
            col=S1 if dx>3 else S2
            if dx==0: col=OUT
            if gy in (126,127): col=G
            c.set(gx,gy,col)
    c.rect(tx+8,104,8,4,G)
    for gy in range(148,168):          # balcony floor (pale stone)
        for gx in range(x0-8,x1+8):
            c.set(gx,gy,P3 if gy==148 else (P2 if gy<164 else P0))
    for gx in range(x0-8,x1+8):        # front balustrade
        for j in range(12):
            gy=168+j
            col=P4 if j==0 else (P3 if j==1 else (P2 if (gx%6) in (1,2) else (P0 if (gx%6)==3 else 0)))
            if j>=9: col=P1 if j<11 else OUT
            if col: c.set(gx,gy,col)
    for gy in range(180,192):          # corbels under the balcony, in its shadow
        for gx in range(x0-8,x1+8):
            c.set(gx,gy,S0)
            if (gx-x0)%16<10 and (gy-180)<(10-abs((gx-x0)%16-5)*2):
                c.set(gx,gy,S2 if (gx-x0)%16<5 else S1)

def south_wall(c):
    # The wall-walk along the south side, seen from above: dark flagstones behind a crenellated
    # parapet, with the entrance passage in the middle.
    for gy in range(400,MH*16,16):
        for gx in range(0,MW*16,16):
            flagstone(c,gx,gy,(gx//16+gy//16)%2)
    for gx in range(0,MW*16):
        for gy in range(396,404):
            y=gy-396
            col=S3 if y<2 else (S2 if y<6 else OUT)
            if gx%16>=10 and y<4: col=0      # gaps between the merlons
            if col: c.set(gx,gy,col)
    for gy in range(396,MH*16):        # the passage
        for gx in range(CX-24,CX+24):
            if abs(gx+0.5-CX)>=20: col=S3 if gx<CX else S1
            else: col=P2 if (gy%16) else P0
            c.set(gx,gy,col)

def draw(c):
    W=MW*16
    stone_wall(c,0,0,W,WALL_B)
    for x in range(W):
        for y in range(WALL_B-10,WALL_B): c.set(x,y,S1 if y%8 else S0)
        c.set(x,WALL_B-11,S3)
    for k in range(1,5):
        off=64*k
        for x in (CX-off-5, CX+off-5):
            if 0<=x<W-10: buttress(c,x,0,WALL_B)
        for x in (CX-off+18, CX+off-46):
            if 0<=x<W-28 and k>1: tall_window(c,x,0,28,WALL_B-24)
    for k in (1,3):
        for tx in (CX-64*k, CX+64*k): turret(c,tx,24,80,132)
    for x in (CX-96-6, CX+96-6):
        long_banner(c,x,WALL_B-30,12)
    balcony(c)
    bench_tiers_south(c,80,CX-56,WALL_B,3)
    bench_tiers_south(c,CX+56,W-80,WALL_B,3)
    bench_tiers_side(c,0,WALL_B,400,5,+1)
    bench_tiers_side(c,W,WALL_B,400,5,-1)
    # paved walkway around the field
    pale_paving(c,80,192,W-160,208)
    field(c)
    for (bx,by) in ((80,176),(W-96,176),(80,368),(W-96,368)):
        brazier(c,bx,by)
    south_wall(c)
