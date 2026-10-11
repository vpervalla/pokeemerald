# The castle's interiors, in the style of the castle grounds: dark polished wood, white shoji paper,
# tatami, gold folding screens, paper lanterns, red and purple cloth. Drawn for the game's camera:
# north walls are a two-row face under a dark beam, side walls a dark strip with a wooden edge,
# floors fill the view. Imported by build_interiors.py.
#
# Each map is drawn on two canvases: the floor and walls (the metatiles' bottom layer) and the
# furniture (the top layer, transparent around it), so a piece of furniture and the floor under it
# can use different palettes. Three palettes, matched by RGB (shared colours repeat):
#   P1 (slot 7) wood, paper, lantern light, red, gold, lacquer
#   P2 (slot 8) tatami, its borders, blue cloth, stone
#   P3 (slot 9) painted screens and doors: pink blossom, greens, purple cloth, gold
import math
from art import Canvas

P1=[(0,0,0),(36,28,28),(64,40,30),(100,62,40),(140,90,56),(180,126,80),(214,166,112),(248,242,228),
    (222,210,188),(252,228,160),(232,180,96),(144,36,32),(200,56,44),(184,140,56),(232,196,96),(52,44,48)]
T,OUT,WD0,WD1,WD2,WD3,WD4,PP0,PP1,LIT0,LIT1,RD0,RD1,GLD0,GLD1,BLK=range(16)
P2=[(0,0,0),P1[OUT],(128,132,64),(168,170,92),(198,196,124),(222,218,154),(36,56,44),(64,92,68),
    (52,66,96),(84,108,148),(132,156,192),(104,100,96),(152,148,142),(200,196,188),P1[PP0],P1[RD1]]
TT0,TT1,TT2,TT3,BD0,BD1,BL0,BL1,BL2,ST0,ST1,ST2=range(18,30)
P3=[(0,0,0),P1[OUT],P1[PP0],P1[PP1],P1[BLK],P1[WD1],P1[WD2],(232,150,170),(250,200,210),(48,88,52),
    (80,128,64),(132,172,84),(88,52,104),(136,88,152),P1[GLD1],P1[LIT1]]
PK0,PK1,GR0,GR1,GR2,PU0,PU1=39,40,41,42,43,44,45
PAL=P1+P2+P3
PALETTES=((7,P1),(8,P2),(9,P3))

class Map:
    # A map's two canvases, its size in cells, and the cells whose top layer is above the player.
    def __init__(s,w,h):
        s.w,s.h=w,h; s.F=Canvas(w*16,h*16); s.O=Canvas(w*16,h*16); s.over=set()

# ---- floors ----
def wood_floor(c,x0,y0,w,h):
    # Polished planks, 4 px boards with staggered joints, repeating every 16 px.
    for y in range(y0,y0+h):
        yy=y%16; k=yy%4
        for x in range(x0,x0+w):
            xx=(x+(0,8,4,12)[yy//4])%16
            if k==3 or xx==15: col=WD2
            elif k==0: col=WD4
            else: col=WD3
            c.set(x,y,col)

def tatami(c,x0,y0,w,h):
    # Mats 32x16 in staggered rows: dark cloth borders along the long sides, woven rush between.
    for y in range(y0,y0+h):
        row=(y-y0)//16; my=(y-y0)%16; off=16 if row%2 else 0
        for x in range(x0,x0+w):
            mx=(x-x0+off)%32
            if my==15: col=BD0
            elif my==0: col=BD1
            elif mx==0: col=TT0
            elif mx==31 or my==14: col=TT1
            elif my==1: col=TT3
            else: col=TT3 if mx%4<2 else TT2
            c.set(x,y,col)

def runner(c,x0,y0,w,h):
    # A red runner with gold-stitched edges.
    for y in range(y0,y0+h):
        for x in range(x0,x0+w):
            d=min(x-x0,x0+w-1-x)
            col=RD0 if d<2 else (GLD1 if d==3 and y%4<2 else RD1)
            c.set(x,y,col)

# ---- walls ----
def beam(c,x,y0):
    # The dark beam at the top of a wall face (rows 0-3 of it).
    for j,col in enumerate((OUT,WD2,WD1,BLK)): c.set(x,y0+j,col)

def baseboard(c,x,y0):
    # The wooden skirting at the bottom of a wall face (rows 26-31).
    for j,col in enumerate((BLK,WD2,WD2,WD2,WD1,OUT)):
        c.set(x,y0+26+j,WD1 if (0<j<4 and x%16==0) else col)

def shoji_wall(c,x0,x1,y0):
    # A wall face 32 px tall: shoji panels between posts every 32 px.
    for x in range(x0,x1):
        p=(x-x0)%32
        beam(c,x,y0); baseboard(c,x,y0)
        for j in range(4,26):
            if p<3: col=(WD1,WD3,WD2)[p]
            elif p in (3,31) or j in (4,25): col=WD1
            elif (p-3)%7==0 or (j-4)%6==0: col=WD3
            else: col=PP0
            c.set(x,y0+j,col)

def plaster_wall(c,x0,x1,y0):
    # A wall face 32 px tall: white plaster above a tie beam, wooden panels below, posts every 32 px.
    for x in range(x0,x1):
        p=(x-x0)%32
        beam(c,x,y0); baseboard(c,x,y0)
        for j in range(4,26):
            if p<3: col=(WD1,WD3,WD2)[p]
            elif j<15: col=PP1 if j==4 else PP0
            elif j<18: col=(WD3,WD2,WD1)[j-15]
            else: col=WD1 if (p-3)%14==0 else (WD3 if j==18 else WD2)
            c.set(x,y0+j,col)

def side_wall(c,x0,y0,y1,right=False):
    # A side wall seen from above: dark, with a wooden edge on the room's side.
    edge=(WD0,WD2,WD3,WD1)
    for y in range(y0,y1):
        for i in range(16):
            col=BLK if i<12 else edge[i-12]
            c.set(x0+(15-i if right else i),y,col)

def bottom_wall(c,x0,x1,y0):
    for x in range(x0,x1):
        for j in range(16): c.set(x,y0+j,(WD1,WD3,WD2,WD0)[j] if j<4 else BLK)

def room(m,wall=shoji_wall,floor=wood_floor):
    # The shell of a room: north wall (rows 0-1), side walls, south wall, floor.
    W,H=m.w,m.h
    floor(m.F,16,32,(W-2)*16,(H-3)*16)
    wall(m.F,16,(W-1)*16,0)
    side_wall(m.F,0,0,H*16); side_wall(m.F,(W-1)*16,0,H*16,right=True)
    bottom_wall(m.F,0,W*16,(H-1)*16)

# ---- things on the walls (bottom layer) ----
def scroll(c,cx,y0):
    # A hanging scroll with brush calligraphy, on a wall face whose top is y0.
    x=cx-5
    c.rect(x-1,y0+4,12,2,WD1)
    c.rect(x,y0+6,10,18,PP1)
    c.rect(x+2,y0+7,6,15,PP0)
    for (i,j) in ((4,8),(5,8),(3,9),(4,10),(5,10),(6,10),(4,11),(4,12),(3,14),(4,14),(5,14),(5,15),
                  (5,16),(4,17),(3,18),(4,19),(5,19),(6,19),(4,20)):
        c.set(x+i,y0+j,BLK)
    c.rect(x-1,y0+24,12,2,WD1); c.set(x-1,y0+25,BLK); c.set(x+10,y0+25,BLK)

def byobu(c,x0,x1,y0):
    # A gold folding screen across a wall face: panels 16 px wide, folded (alternate panels in
    # shadow), with a pine painted across them.
    for x in range(x0,x1):
        pn=(x-x0)//16; px=(x-x0)%16
        for j in range(2,32):
            if j in (2,31) or px==0: col=BLK
            elif j==3 or j==30: col=WD1
            else: col=GLD1 if pn%2==0 else LIT1
            c.set(x,y0+j,col)
    w=x1-x0
    for i in range(w):                            # the trunk, low and leaning
        t=i/w
        yb=y0+26-int(10*math.sin(t*math.pi))
        if 0.1<t<0.9:
            c.set(x0+i,yb,WD1); c.set(x0+i,yb+1,WD2 if i%5 else WD1)
    for (fx,fy,r) in ((0.22,14,7),(0.45,9,8),(0.68,12,7),(0.85,18,5),(0.32,20,5)):
        cx=x0+int(fx*w); cy=y0+fy
        for dy in range(-r//2,r//2+1):
            for dx in range(-r,r+1):
                if dx*dx/(r*r)+dy*dy*4/(r*r)<=1:
                    col=GR2 if dy<-1 else (GR1 if dy<1 else GR0)
                    c.set(cx+dx,cy+dy,col)

def maku(c,x0,x1,y0):
    # A purple ceremonial curtain across a wall face, with white crests and a gold rope.
    for x in range(x0,x1):
        p=(x-x0)%32
        for j in range(2,32):
            if j in (2,3): col=GLD1 if (x+j)%3 else LIT1
            elif j==4 or j==30: col=BLK
            elif j==31: col=OUT
            else:
                col=PU1 if p%8<5 else PU0
                if j>26 and (p%8 in (5,6,7)): col=BLK if j>27 else PU0
            c.set(x,y0+j,col)
    for x in range(x0+15,x1-6,32):                 # crests: a white ring round a four-petal flower
        cy=y0+15
        for dy in range(-6,7):
            for dx in range(-6,7):
                d=dx*dx+dy*dy
                if d>36: continue
                if d>=20: col=PP0
                elif (abs(dx)+abs(dy)<=3 and min(abs(dx),abs(dy))<=1) or d<=1: col=PP0
                else: col=PU0
                c.set(x+dx,cy+dy,col)

def fusuma(c,x0,y0,motif=True):
    # A sliding door, 16 px wide, in a wall face whose top is y0: a lacquer frame, cream paper
    # (with a blossoming branch), a round pull, and the lintel above.
    for j in range(4,32):
        for i in range(16):
            if i in (0,15) or j in (4,31): col=BLK
            elif i in (1,14) or j in (5,30): col=WD1
            elif j>=24: col=GLD1 if j>24 else LIT1          # a gold band at the foot
            else: col=PP1 if (i+j)%7 else PP0
            c.set(x0+i,y0+j,col)
    if motif:
        for (i,j) in ((3,17),(4,16),(5,15),(6,15),(7,14),(8,13),(9,13),(10,12),(7,15),(8,16)):
            c.set(x0+i,y0+j,WD1)
        for (i,j) in ((4,13),(8,10),(11,10),(6,18),(10,15)):
            for (a,b,col) in ((0,0,PK0),(1,0,PK1),(0,1,PK1),(1,1,PK0)): c.set(x0+i+a,y0+j+b,col)
    c.rect(x0+11,y0+18,2,3,BLK); c.set(x0+11,y0+19,GLD1)
    for x in range(x0,x0+16): c.set(x,y0,OUT); c.set(x,y0+1,WD2); c.set(x,y0+2,WD1); c.set(x,y0+3,BLK)

def stairs_north(c,x0,y0,w,rows):
    # Straight stairs going up north into a dark opening (the top row): treads every 8 px with
    # stringers on both sides.
    for y in range(y0,y0+rows*16):
        for x in range(x0,x0+w):
            d=min(x-x0,x0+w-1-x)
            k=(y-y0)%8
            if y<y0+16: col=BLK if y<y0+10 or d<2 else (WD1 if k==2 else WD0)
            elif d<2: col=(WD0,WD1)[d]
            else: col=(WD4,WD3,WD3,WD3,WD3,WD3,WD2,WD1)[k]
            c.set(x,y,col)

def stairs_side(c,x0,y0,w,h,up_east):
    # Stairs at the end of a corridor, the treads running north-south. Up stairs lighten toward
    # the far end, down stairs darken into the opening.
    for x in range(x0,x0+w):
        far=(x-x0) if up_east else (x0+w-1-x)      # distance from the corridor
        for y in range(y0,y0+h):
            d=min(y-y0,y0+h-1-y)
            k=(x-x0)%8 if up_east else (x0+w-1-x)%8
            if d<2: col=(WD0,WD1)[d]
            elif far>=w-6 and not up_east: col=BLK
            else: col=(WD4,WD3,WD3,WD3,WD3,WD3,WD2,WD1)[k]
            if up_east and far>=w-4: col=WD4 if k<2 else WD3
            c.set(x,y,col)

def stairs_south(c,x0,y0,w,rows):
    # Stairs going down south out of the room, darkening into the opening.
    for y in range(y0,y0+rows*16):
        for x in range(x0,x0+w):
            d=min(x-x0,x0+w-1-x)
            k=(y-y0)%8; n=(y-y0)//8
            if d<2: col=(WD0,WD1)[d]
            elif n>=rows*2-2: col=BLK
            else: col=((WD4,WD3,WD3,WD3,WD3,WD2,WD1,WD0) if n<2 else (WD3,WD2,WD2,WD2,WD1,WD1,WD0,BLK))[k]
            c.set(x,y,col)

# ---- furniture (top layer) ----
def andon(m,cx,cy):
    # A paper floor lantern standing in cell (cx,cy); its top is in the cell above.
    c=m.O; x=cx*16; yb=(cy+1)*16
    c.rect(x+2,yb-25,12,2,WD1)
    for y in range(yb-23,yb-5):
        for i in range(3,13):
            col=LIT0 if i<10 else LIT1
            if y in (yb-23,yb-15,yb-6) or i in (3,12): col=BLK
            c.set(x+i,y,col)
    for i in (3,12):
        c.vline(x+i,yb-5,4,BLK)
    m.over.add((cx,cy-1))

def bonsai(m,cx,cy):
    # A potted pine.
    c=m.O; x=cx*16; yb=(cy+1)*16
    c.rect(x+3,yb-7,10,5,WD2); c.hline(x+2,yb-8,12,BLK); c.hline(x+4,yb-2,8,WD1)
    c.vline(x+7,yb-14,6,WD1); c.vline(x+8,yb-12,4,WD2); c.hline(x+5,yb-13,3,WD1)
    for (px,py,r) in ((4,-16,4),(11,-18,4),(7,-22,4),(8,-15,3)):
        for dy in range(-2,3):
            for dx in range(-r,r+1):
                if dx*dx/(r*r)+dy*dy/4<=1:
                    c.set(x+px+dx,yb+py+dy,GR2 if dy<0 else (GR1 if dy==0 else GR0))
    m.over.add((cx,cy-1))

def desk(m,x0,x1,cy):
    # The registration desk: a wooden counter across cells x0..x1-1 of row cy.
    c=m.O; y=cy*16
    for x in range(x0*16,x1*16):
        p=x%16
        for j in range(16):
            if j<5: col=WD4 if j<2 else WD3
            elif j==5: col=WD1
            elif j==15: col=OUT
            else: col=WD1 if p==0 else (WD3 if p==1 else WD2)
            c.set(x,y+j,col)
    bx=(x0+1)*16+3                                 # the register, open
    c.rect(bx,y,11,4,PP0); c.vline(bx+5,y,4,PP1); c.hline(bx+1,y+1,3,BLK); c.hline(bx+7,y+2,3,BLK)
    lx=(x1-2)*16+6                                 # a small lamp
    c.rect(lx,y-4,4,6,LIT0); c.hline(lx-1,y-5,6,BLK); c.hline(lx-1,y+2,6,BLK)

def zabuton(m,cx,cy,col0=PU0,col1=PU1):
    # A floor cushion.
    c=m.O; x=cx*16+2; y=cy*16+3
    for j in range(11):
        for i in range(12):
            col=col0 if (i>9 or j>8) else col1
            c.set(x+i,y+j,col)
    c.set(x+6,y+5,GLD1)
    for (i,j) in ((0,0),(11,0),(0,10),(11,10)): c.set(x+i,y+j,GLD1)

def low_table(m,cx,cy,tea=True):
    # A low oval table with a teapot and a cup.
    c=m.O; x=cx*16; y=cy*16
    for j in range(16):
        for i in range(16):
            dx=(i-7.5)/7.5; dy=(j-6.5)/5.5
            d=dx*dx+dy*dy
            if d<=1: c.set(x+i,y+j,WD0 if d>0.75 else (WD3 if (d<0.5 and dx<-0.2 and dy<-0.2) else WD2))
    c.rect(x+3,y+12,2,3,WD0); c.rect(x+11,y+12,2,3,WD0)
    if tea:
        c.rect(x+4,y+4,5,4,BLK); c.set(x+9,y+5,BLK); c.set(x+3,y+5,BLK); c.set(x+6,y+3,GLD1)
        c.rect(x+11,y+6,2,2,PP0); c.set(x+11,y+7,PP1)

def futon(m,cx,cy):
    # A futon two cells wide and two tall: a white pillow and a blue quilt.
    c=m.O; x=cx*16+2; y=cy*16+1
    for j in range(30):
        for i in range(28):
            if j<8: col=PP0 if 6<i<21 and 1<j<7 else ST2
            elif j<11: col=PP0
            else: col=BL1 if (i+j)%6 else BL2
            if i in (0,27) or j==29: col=BL0 if j>=8 else ST1
            c.set(x+i,y+j,col)

def nobori(m,cx,cy):
    # A red-and-white banner on a pole, its foot in cell (cx,cy), three cells tall.
    c=m.O; x=cx*16; yb=(cy+1)*16
    c.rect(x+3,yb-46,2,44,WD1); c.rect(x+3,yb-46,10,2,WD1)
    for y in range(yb-43,yb-10):
        for i in range(5,13):
            red=(y-(yb-43))<16+(i-5)*1.5
            col=(RD1 if i<12 else RD0) if red else (PP0 if i<12 else PP1)
            c.set(x+i,y,col)
    c.rect(x+1,yb-3,6,3,WD2)
    m.over.add((cx,cy-1)); m.over.add((cx,cy-2))

def vase(m,cx,cy):
    # A flower vase with a blossom branch, on the alcove's platform.
    c=m.O; x=cx*16; yb=cy*16+12
    for j in range(8):
        w=3 if j<2 else 4 if j<6 else 3
        for i in range(-w,w):
            c.set(x+8+i,yb-8+j,BLK if i in (-w,w-1) else WD2)
    for (i,j) in ((0,-10),(1,-11),(2,-13),(3,-14),(-1,-12),(-2,-14),(1,-16)):
        c.set(x+8+i,yb+j,WD1)
    for (i,j) in ((3,-15),(-2,-15),(1,-17),(4,-13),(-3,-13)):
        c.set(x+8+i,yb+j,PK1); c.set(x+9+i,yb+j,PK0)

# ---- the battlefield ----
def battlefield(c,x0,y0,x1,y1,boxes,center):
    # Tatami inside a wooden frame, with white lines: the boundary, the centre line, a box for
    # each trainer, and a red emblem with a white triple swirl (mitsudomoe) in the middle.
    tatami(c,x0,y0,x1-x0,y1-y0)
    for x in range(x0-3,x1+3):
        for k,col in enumerate((WD1,WD0,OUT)):
            c.set(x,y0-1-k,col); c.set(x,y1+k,col)
    for y in range(y0-3,y1+3):
        for k,col in enumerate((WD1,WD0,OUT)):
            c.set(x0-1-k,y,col); c.set(x1+k,y,col)
    def line_rect(a,b,c_,d):
        for x in range(a,c_):
            for t in (0,1): c.set(x,b+t,PP0); c.set(x,d-1-t,PP0)
        for y in range(b,d):
            for t in (0,1): c.set(a+t,y,PP0); c.set(c_-1-t,y,PP0)
    line_rect(x0+6,y0+6,x1-6,y1-6)
    cx,cy=center
    for x in range(x0+6,x1-6):
        for t in (0,1): c.set(x,cy+t,PP0)
    for (bx,by) in boxes:
        line_rect(bx-18,by-12,bx+18,by+12)
    R=22
    for y in range(cy-R,cy+R+1):
        for x in range(cx-R,cx+R+1):
            dx=x-cx+0.5; dy=y-cy+0.5; r=math.hypot(dx,dy)
            if r>R: continue
            if r>R-2: col=OUT
            elif r>R-5: col=PP0
            elif r>R-6: col=RD1
            else:
                a=(math.degrees(math.atan2(dy,dx))+360)%120
                col=PP0 if (2<r<R-7 and (a+r*7)%120<34) or r<2.5 else RD1
            c.set(x,y,col)
