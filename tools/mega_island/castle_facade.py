# Draws the giant castle: a facade spanning the whole map width (its top is above anything the
# camera can show), standing on a stone rampart with a grand staircase. Imported by build_castle.py.
from art import *

def draw(c, MW, CX):
    W=MW*16
    FB=160                      # bottom of the facade (row 10)
    stone_wall(c,0,0,W,FB)
    for x in range(W):          # plinth
        for y in range(FB-10,FB): c.set(x,y,S1 if y%8 else S0)
        c.set(x,FB-11,S3)
    # bays every 64 px, mirrored about the gate axis: buttresses and towering windows
    for k in range(1,8):
        off=64*k
        for x in (CX-off-5, CX+off-5):
            if 0<=x<W-10: buttress(c,x,0,FB)
        for x in (CX-off+18, CX+off-46):
            if 0<=x<W-28: tall_window(c,x,0,28,FB-24)
    for x in (CX-56,CX+44):
        long_banner(c,x,FB-40,12)
    # great towers by the keep, smaller ones on the wings
    for (tx,r) in ((CX-160,44),(CX+160,44),(CX-352,32),(CX+352,32)):
        round_tower(c,tx,0,r,FB)
        for i in range(-r-4,r+4):       # battered base
            for y in range(FB-16,FB):
                d=abs(i+0.5)/(r+4)
                c.set(tx+i,y,S2 if d<0.5 else (S1 if d<0.85 else S0))
            c.set(tx+i,FB-17,S3)
        slit_window(c,tx-1,96,12)
    gate(c,CX-32,FB-96,64,96)
    drape_swag(c,CX-48,FB-92,14,80,-1)
    drape_swag(c,CX+34,FB-92,14,80,1)
    # the rampart: terrace on top (rows 10-11), stone retaining wall (rows 12-15)
    F0,F1=FB+32,FB+96
    stone_rampart(c,0,W,F0-2,F1,CX)
    SW=64
    for k in range(10):                # grand staircase
        y=F0-4+k*7
        for j in range(7):
            col=S4 if j==0 else (S3 if j<3 else (S2 if j<6 else S0))
            c.hline(CX-SW//2,y+j,SW,col)
    for bx in (CX-SW//2-8, CX+SW//2):  # balustrades with gold finials
        c.rect(bx,F0-10,8,F1-F0+12,S2); c.vline(bx,F0-10,F1-F0+12,S3); c.vline(bx+7,F0-10,F1-F0+12,S1)
        c.rect(bx-1,F0-12,10,3,S4)
        c.rect(bx+2,F0-17,4,5,G)
    gargoyle2(c,CX-104,FB-28); gargoyle2(c,CX+72,FB-28)
    # the moat (rows 16-17): shadow where the rampart meets the water, a stone edge on the far bank
    M0,M1=F1,F1+32
    for x in range(W):
        c.set(x,M0,OUT); c.set(x,M0+1,S0)
        if x%3==0: c.set(x,M0+2,S0)
        for y in range(M1,M1+6):
            col=S4 if y==M1 else (S3 if y<M1+3 else (S2 if y<M1+5 else OUT))
            if (x-CX)%32==31 and M1<y<M1+5: col=S1
            c.set(x,y,col)
    # a stone bridge carries the staircase over the moat, between the balustrades
    for y in range(M0,M1+6):
        for x in range(CX-SW//2,CX+SW//2):
            col=S2 if (y-M0)%8 else S1
            if (y-M0)%8==1: col=S3
            c.set(x,y,col)
    for bx in (CX-SW//2-8, CX+SW//2):
        c.rect(bx,M0-2,8,M1-M0+8,S2); c.vline(bx,M0-2,M1-M0+8,S3); c.vline(bx+7,M0-2,M1-M0+8,S1)
        c.rect(bx-1,M1+4,10,3,S4); c.rect(bx+2,M1-1,4,5,G)
    # gargoyles at the courtyard end of the bridge
    gargoyle2(c,CX-SW//2-44,M1+2); gargoyle2(c,CX+SW//2+12,M1+2)
