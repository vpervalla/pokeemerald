# Draws the giant castle in blackstone: a facade spanning the whole map width (its top is above
# anything the camera can show), a raised bastion with a pale forecourt, round corner towers with
# blue spires, a grand pale staircase, and a moat with a bridge. Imported by build_castle.py.
from art import *

# Vertical layout, in pixels (rows are 16 px)
FB=160            # bottom of the facade (row 10); the forecourt is rows 10-15
PAR=244           # front parapet of the forecourt (bottom of row 15)
BW0,BW1=256,320   # the bastion's front wall (rows 16-19)
M0,M1=320,352     # the moat (rows 20-21)
BX0,BX1=192,768   # the bastion spans cols 12-47
SW=64             # staircase and bridge width (cols 28-31)

def draw(c, MW, CX):
    W=MW*16
    # ---- the castle ----
    stone_wall(c,0,0,W,FB)
    for x in range(W):          # plinth
        for y in range(FB-10,FB): c.set(x,y,S1 if y%8 else S0)
        c.set(x,FB-11,S3)
    for k in range(1,8):        # bays every 64 px: buttresses and towering windows
        off=64*k
        for x in (CX-off-5, CX+off-5):
            if 0<=x<W-10: buttress(c,x,0,FB)
        for x in (CX-off+18, CX+off-46):
            if 0<=x<W-28 and k>1: tall_window(c,x,0,28,FB-24)   # the gate takes the first bays
    for (tx,r) in ((CX-160,44),(CX+160,44),(CX-352,32),(CX+352,32)):
        round_tower(c,tx,0,r,FB)
        for i in range(-r-4,r+4):
            for y in range(FB-16,FB):
                d=abs(i+0.5)/(r+4)
                c.set(tx+i,y,S2 if d<0.5 else (S1 if d<0.85 else S0))
            c.set(tx+i,FB-17,S3)
        long_banner(c,tx-6,FB-36,12)       # violet banners down the fronts of the great towers
    # turrets with blue spires on the buttresses the great towers don't hide; their spires
    # run up out of sight, so the castle still has no visible top
    for k in (1,4,7):
        for tx in (CX-64*k, CX+64*k):
            if 12<=tx<=W-12: turret(c,tx,40,96,148)
    pale_gate(c,CX-32,FB-96,64,96)

    # ---- the forecourt on the bastion ----
    pale_paving(c,BX0+64,FB,BX1-BX0-128,PAR-FB)           # the corner towers stand on either side
    pale_paving(c,BX0+56,208,8,PAR-208); pale_paving(c,BX1-64,208,8,PAR-208)    # beside the towers, below their spires
    pale_balustrade(c,BX0+56,CX-SW//2-8,PAR); pale_balustrade(c,CX+SW//2+8,BX1-56,PAR)
    for (lx0,lx1) in ((272,384),(576,688)):      # two lawns with a stone curb (rows 11-14)
        for y in range(176,240):
            for x in range(lx0,lx1):
                edge=min(x-lx0,lx1-1-x,y-176,239-y)
                c.p[y][x]=0 if edge>=2 else (P3 if edge==0 else P1)
    gargoyle2(c,CX-104,FB-28); gargoyle2(c,CX+72,FB-28)
    for x in (CX-SW//2-24, CX+SW//2+8):
        obelisk(c,x,PAR-56,56)

    # ---- the bastion wall, with round corner towers and blue spires ----
    arcade_wall(c,BX0,BX1,BW0,BW1,CX)
    for tx in (BX0+24, BX1-24):
        round_tower(c,tx,200,26,BW1-200)
        for i in range(-26,26):                # the base curves where it meets the moat (seen from above)
            dy=int(6*(1-((i+0.5)/26)**2)**0.5+0.5)
            for k in range(6-dy): c.set(tx+i,BW1-1-k,0)
            c.set(tx+i,BW1-1-(6-dy),OUT)
        cone_roof3q(c,tx,132,30,60)
        slit_window(c,tx-1,236,12)

    # ---- grand staircase from the forecourt to the moat, with spired balustrades ----
    for k in range(10):          # 8 px steps, so the flight repeats the same tiles
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
