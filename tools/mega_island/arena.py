# Draws the castle's inner courtyard, used as the tournament stadium. Imported by build_arena.py.
# Map 32x30 cells. Rows 0-9: the keep's roofs, wall-walk and short inner face, with the host's
# balcony; rows 10-11 and cols 0-4 / 27-31: tiered stands; rows 12-24: a paved walkway around the field (rows 13-23,
# cols 7-24); rows 25-29: the south wall top with the entrance passage (cols 15-16).
from art import *

MW,MH=32,30
CX=256                       # centre axis (between cols 15 and 16)
WALL_B=160                   # bottom of the north wall (row 10)
FX0,FX1,FY0,FY1=112,400,208,384   # the field

def bench_tiers_south(c,x0,x1,y0,tiers):
    # Stands facing south, seen from above: each 16 px tier is mostly the bench top (pale), with a
    # thin dark riser at its front.
    for t in range(tiers):
        y=y0+t*16
        for gx in range(x0,x1):
            for j in range(16):
                if j<11: col=P4 if j==0 else (P3 if j<4 else P2)
                elif j==11: col=P0
                else: col=S1 if j<15 else S0
                if j<11 and gx%32==31: col=P1                 # seat dividers
                c.set(gx,y+j,col)

def bench_tiers_side(c,x0,y0,y1,tiers,face):
    # Side stands, seen from above: tiers step down towards the field. face=+1: the field is to the
    # east (west stand), -1: to the west. Each tier is 16 px: a wide bench top and a thin riser.
    for t in range(tiers):
        for k in range(16):
            gx=x0+t*16+k if face>0 else x0-t*16-k-1
            for gy in range(y0,y1):
                if k<12: col=P3 if k<2 else (P2 if k<11 else P1)
                elif k==12: col=P0
                else: col=S1 if k<15 else S0
                if gy%32==31 and k<12: col=P1                 # seat dividers
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
    # The host's box, seen from above: a pale platform projecting from the keep, a throne, a
    # violet canopy with a gold fringe, a thin rail along the front and a short shadowed face.
    x0,x1=CX-48,CX+48
    for gy in range(136,176):                          # platform floor
        for gx in range(x0,x1):
            ch=FLAG_A[gy%16][gx%16]
            c.set(gx,gy,P0 if ch=='#' else P3)
    tx=CX-12
    for gy in range(152,174):                          # throne: back (dark) and seat (indigo)
        for gx in range(tx,tx+24):
            i,j=gx-tx,gy-152
            if j<8: col=S1 if 2<=i<=21 else 0
            elif j<22: col=R1 if 5<=i<=18 else (S2 if 2<=i<=21 else 0)
            else: col=S0 if 2<=i<=21 else 0
            if j==8 and 2<=i<=21: col=G
            if col: c.set(gx,gy,col)
    c.rect(tx+10,153,4,3,G)
    for gy in range(128,152):                          # canopy over the back of the box
        for gx in range(x0,x1):
            col=C1 if ((gx-x0)//8)%2 else C0
            if gy>=148: col=G if (gx%4)<2 else C0      # fringe
            c.set(gx,gy,col)
    for gx in range(x0,x1):                            # rail (top seen from above) and posts
        c.set(gx,174,P4); c.set(gx,175,P3)
        for gy in range(176,192):
            col=S0 if gy>176 else OUT
            if (gx-x0)%16 in (0,1) and gy<186: col=S2
            c.set(gx,gy,col)

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
    # the inner face of the keep, for the high camera: roofs and the wall-walk above a short face
    YW=WALL_B-24   # from the walkway (row 12) the camera sees down to about y 120
    roof_top(c,0,W,0,YW-26)
    wall_top(c,0,W,YW)
    short_wall(c,0,W,YW,WALL_B)
    for k in range(1,5):
        off=64*k
        for x in (CX-off-5, CX+off-5):
            if 0<=x<W-10:
                for j in range(YW-4,WALL_B):
                    for i in range(10):
                        col=S3 if i<3 else (S2 if i<7 else S1)
                        if j<YW: col=S4 if i<7 else S3
                        if j%16==15 and j>=YW: col=S0
                        c.set(x+i,j,col)
        for x in (CX-off+18, CX+off-46):
            if 0<=x<W-28 and k>2: pointed_window(c,x+8,YW+3,12,16)   # k=2 bays carry the banners
    for k in (1,3):
        for tx in (CX-64*k, CX+64*k): bartizan(c,tx,YW)
    for x in (CX-96-6, CX+96-6):
        banner(c,x,YW+2,12,WALL_B-YW-8)
    balcony(c)
    bench_tiers_south(c,80,CX-48,WALL_B,2)
    bench_tiers_south(c,CX+48,W-80,WALL_B,2)
    bench_tiers_side(c,0,WALL_B,400,5,+1)
    bench_tiers_side(c,W,WALL_B,400,5,-1)
    # paved walkway around the field
    pale_paving(c,80,192,W-160,208)
    field(c)
    for (bx,by) in ((80,176),(W-96,176),(80,368),(W-96,368)):
        brazier(c,bx,by)
    south_wall(c)
