# Draws the giant castle in blackstone: a facade spanning the whole map width (its top is above
# anything the camera can show), a raised bastion with a pale forecourt, round corner towers with
# blue spires, a grand pale staircase, and a moat with a bridge. Imported by build_castle.py.
from art import *

# Vertical layout, in pixels (rows are 16 px). Drawn for the game's high camera: every face is
# short and we see the tops -- the castle's roofs and wall-walk, the towers' cones and rims.
FB=192            # bottom of the facade (row 12); the forecourt is rows 12-15
YW=160            # top of the facade's face; the wall-walk and the roofs are above it. From the
                  # forecourt the camera sees down to about y 120, so the tops all show
YG=136            # top of the gatehouse's face
PAR=244           # front parapet of the forecourt (bottom of row 15)
BW0,BW1=256,288   # the bastion's front wall (rows 16-17)
M0,M1=288,320     # the moat (rows 18-19)
BX0,BX1=192,768   # the bastion spans cols 12-47
SW=64             # staircase and bridge width (cols 28-31)

def draw(c, MW, CX):
    W=MW*16
    # ---- the castle: roofs with dormers, the wall-walk, a short face ----
    roof_top(c,0,W,0,YW-26)
    wall_top(c,0,W,YW)
    short_wall(c,0,W,YW,FB)
    for k in range(1,8):        # bays every 64 px: buttresses and pointed windows
        off=64*k
        for x in (CX-off-5, CX+off-5):
            if 0<=x<W-10:
                for j in range(YW-4,FB):
                    for i in range(10):
                        col=S3 if i<3 else (S2 if i<7 else S1)
                        if j<YW: col=S4 if i<7 else S3          # its sloped cap, seen from above
                        if j%16==15 and j>=YW: col=S0
                        c.set(x+i,j,col)
        for x in (CX-off+18, CX+off-46):
            if 0<=x<W-28 and k>1: pointed_window(c,x+8,YW+5,12,20)
    # the gatehouse: a taller block around the gate, with its own wall-walk
    wall_top(c,CX-56,CX+56,YG)
    short_wall(c,CX-56,CX+56,YG,FB)
    for x in (CX-57,CX+56):
        c.vline(x,YG-26,FB-YG+26,OUT)
    pale_gate(c,CX-28,FB-48,56,48)
    for x in (CX-50,CX+38):     # violet banners either side of the gate
        banner(c,x,YG+4,12,FB-YG-18)
    # the great towers: short bodies, wide squat cones
    for tx in (CX-160,CX+160):  # the wings run on into the forest
        tower3q(c,tx,FB,30,150,30)
        slit_window(c,tx-1,170,10)
    # bartizans with blue spires on the buttresses the towers don't hide
    for k in (4,):
        for tx in (CX-64*k, CX+64*k):
            if 12<=tx<=W-12: bartizan(c,tx,YW)

    # ---- the forecourt on the bastion ----
    pale_paving(c,BX0,FB,BX1-BX0,PAR-FB)
    pale_balustrade(c,BX0+56,CX-SW//2-8,PAR); pale_balustrade(c,CX+SW//2+8,BX1-56,PAR)
    for (lx0,lx1) in ((272,384),(576,688)):      # two lawns with a stone curb (rows 13-14)
        for y in range(208,240):
            for x in range(lx0,lx1):
                edge=min(x-lx0,lx1-1-x,y-208,239-y)
                c.p[y][x]=0 if edge>=2 else (P3 if edge==0 else P1)
    gargoyle2(c,CX-104,FB-28); gargoyle2(c,CX+72,FB-28)
    for x in (CX-SW//2-24, CX+SW//2+8):
        obelisk(c,x,PAR-56,56)

    # ---- the bastion wall, with round corner towers and blue spires ----
    arcade_wall(c,BX0,BX1,BW0,BW1,CX,a0=4,half=11,cope=0)
    for tx in (BX0+24, BX1-24):
        tower3q(c,tx,BW1+4,26,236,28,dark=S0)
        slit_window(c,tx-1,260,10)

    # ---- grand staircase from the forecourt to the moat, with spired balustrades ----
    for k in range(6):           # 8 px steps, so the flight repeats the same tiles
        y=PAR-4+k*8
        for j in range(8):
            col=P4 if j==0 else (P3 if j<3 else (P2 if j<6 else (P1 if j<7 else P0)))
            c.hline(CX-SW//2,y+j,SW,col)
    for bx in (CX-SW//2-8, CX+SW//2):
        c.rect(bx,PAR-8,8,M1-PAR+14,S2); c.vline(bx,PAR-8,M1-PAR+14,S3); c.vline(bx+7,PAR-8,M1-PAR+14,S1)
        for py in (PAR-8, M1+2):
            for j in range(10):     # small blue spire on each end post
                half=max(1,int(4*(j+1)/10+0.5))
                for i in range(-half,half): c.set(bx+4+i,py-10+j,R2 if i<0 else R1)

    # ---- the moat: shadow under the wall, a pale edge on the far bank, a pale bridge ----
    for x in range(W):
        c.set(x,M0,OUT); c.set(x,M0+1,S0)
        if x%3==0: c.set(x,M0+2,S0)
        for y in range(M1,M1+6):
            col=P4 if y==M1 else (P3 if y<M1+3 else (P1 if y<M1+5 else OUT))
            if (x-CX)%32==31 and M1<y<M1+5: col=P0
            c.set(x,y,col)
    for y in range(M0,M1+6):
        for x in range(CX-SW//2,CX+SW//2):
            col=P2 if (y-M0)%8 else P0
            if (y-M0)%8==1: col=P3
            c.set(x,y,col)
    gargoyle2(c,CX-SW//2-44,M1+2); gargoyle2(c,CX+SW//2+12,M1+2)
