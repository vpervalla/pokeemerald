# Draws the castle facade (22x18 metatiles) into `c`. Imported by build_castle.py.
from art import *
MW,MH=22,18
W,H=MW*16,MH*16
c=Canvas(W,H)
CX=176                     # facade centre (gate axis)
KX0,KX1=64,288
TOP=3
slate_roof(c,KX0,TOP*16,KX1-KX0,3*16)
stone_wall(c,KX0,(TOP+3)*16+8,KX1-KX0,7*16+8)
crenellations(c,KX0,(TOP+3)*16-4,KX1-KX0,merlon=8,gap=8,h=12)
for x in range(KX0,KX1):
    for y in range((TOP+10)*16+8,(TOP+11)*16): c.set(x,y,S1 if y%8 else S0)
round_tower(c,CX,(TOP-1)*16,24,3*16+4)
cone_roof(c,CX,8,26,(TOP-1)*16-8+4)
pennant(c,CX,0)
def sym(f,x,*a):           # draw at x and mirrored about the gate axis (w = width of the piece)
    w=a[-1]; f(x,*a[:-1]); f(2*CX-x-w,*a[:-1])
for bx in (91,131):
    sym(lambda x: buttress(c,x,(TOP+3)*16+2,8*16-2),bx,10)
for cx in (32,320):
    round_tower(c,cx,(TOP+1)*16,30,10*16)
    cone_roof(c,cx,10,34,(TOP+1)*16-10)
    pennant(c,cx,2)
    for yy in ((TOP+3)*16,(TOP+6)*16): slit_window(c,cx-1,yy,10)
sym(lambda x: pointed_window(c,x,(TOP+4)*16+4,14,34),69,14)
sym(lambda x: pointed_window(c,x,(TOP+7)*16+4,14,30),69,14)
sym(lambda x: banner(c,x,(TOP+4)*16-4,12,60),109,12)
sym(lambda x: pointed_window(c,x,(TOP+8)*16-2,10,22),111,10)
rose_window(c,CX,(TOP+5)*16+4,15)
gate(c,CX-24,(TOP+7)*16,48,64)
drape_swag(c,CX-34,(TOP+7)*16+2,10,52,-1)
drape_swag(c,CX+24,(TOP+7)*16+2,10,52,1)
stairs(c,CX-28,(TOP+11)*16,56,3)
gargoyle2(c,CX-72,(TOP+9)*16+4); gargoyle2(c,CX+40,(TOP+9)*16+4)
c.outline()
