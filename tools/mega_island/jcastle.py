# The castle grounds' art, after the user's reference map: a Japanese castle (tenshu) of white
# plaster and dark wood under blue-grey tiled roofs, a low compound wall with a navy tile cap, a red
# torii as the gate, red-and-white nobori banners along the approach, komainu, stone lanterns and a
# cobbled plaza. Imported by build_castle.py.
#
# Two palettes: J (tileset slot 7) for the castle and the compound wall, K (slot 8) for the rest. The art uses
# indices 0-15 for J and 16-31 for K; colours both palettes share (outline, white plaster, wood) are
# matched by RGB, so a tile may use them with either palette. Each 8x8 tile must fit one palette.
# J's colours 13 and 14 are the lit window panels: day_night.c lights them in the evening.
#
# Symmetric pieces are drawn on their left half and mirrored about the gate axis, which is on the
# 8-pixel grid, so each right-hand tile is a flipped left-hand one and costs nothing.
from art import Canvas

PAL_J=[(0,0,0),(56,56,64),(64,80,104),(80,100,128),(96,122,148),(132,156,180),(240,232,224),(204,194,186),
       (88,56,40),(136,88,56),(176,124,80),(208,164,112),(232,196,88),(248,232,184),(224,196,136),(40,40,48)]
T,OUT,RF0,RF1,RF2,RF3,WL0,WL1,WD0,WD1,WD2,WD3,GLD,LIT0,LIT1,DK=range(16)
PAL_K=[(0,0,0),PAL_J[OUT],(112,108,104),(152,148,144),(184,180,176),(214,210,204),PAL_J[WL0],PAL_J[WL1],
       PAL_J[RF0],PAL_J[RF1],PAL_J[RF2],(136,32,32),(196,52,40),(236,104,80),(72,64,60),PAL_J[WD1]]
ST0,ST1,ST2,ST3,RD0,RD1,RD2,TK,KWD=18,19,20,21,27,28,29,30,31
PAL=PAL_J+PAL_K

def mirror(c,cx,x0,y0,y1):
    # Copies the left half [x0,cx) onto the right half, flipped about cx.
    for y in range(y0,y1):
        for x in range(x0,cx):
            v=c.get(x,y)
            if v: c.set(2*cx-1-x,y,v)

# ---- the castle ----
def roof(c,cx,ytop,ybot,hw0,hw1,flare=3):
    # A tiled roof seen from the front, as a trapezoid: hw0 wide at the top, hw1 at the eave, whose
    # ends curl up over the last rows. Tiles are 8 px courses with a seam every 8 px.
    for y in range(ytop,ybot):
        t=(y-ytop)/max(1,ybot-1-ytop)
        hw=round(hw0+(hw1-hw0)*t)+max(0,flare-(ybot-1-y))
        for x in range(cx-hw,cx):
            d=x-(cx-hw)
            yy=y%8; xx=x%8
            if d==0: col=OUT
            elif d==1: col=RF3
            elif y==ybot-1: col=OUT
            elif y==ybot-2: col=RF0
            elif y==ybot-3: col=RF3
            elif yy==7: col=RF0
            elif yy==6: col=RF1
            elif xx==7: col=RF1
            elif yy==0: col=RF3
            else: col=RF2
            c.set(x,y,col)

def ridge(c,cx,y,hw):
    # The ridge along a roof's top, with a gold shachihoko at its end.
    for x in range(cx-hw,cx):
        c.set(x,y,OUT); c.set(x,y+1,RF3); c.set(x,y+2,RF1); c.set(x,y+3,RF0)
    x=cx-hw
    for j,(a,b) in enumerate(((1,2),(0,3),(0,3),(1,3),(2,3),(2,4))):
        for i in range(a,b): c.set(x+i,y-6+j,GLD)

def wall(c,cx,ytop,ybot,hw):
    # White plaster between a beam at the top and the bottom, with a corner post.
    for y in range(ytop,ybot):
        for x in range(cx-hw,cx):
            d=x-(cx-hw)
            if d==0: col=OUT
            elif d<=3: col=WD1 if d<3 else WD0
            elif y-ytop<2: col=WD1 if y==ytop else WD0
            elif y-ytop==2: col=WL1
            elif ybot-y<=3: col=(WD2,WD1,WD0)[3-(ybot-y)]
            else: col=WL0
            c.set(x,y,col)

def dark_window(c,x,y,w=12,h=6):
    c.rect(x,y,w,h,DK); c.hline(x,y+h,w,WL1)

def barred_window(c,x,y,w=14,h=10):
    # A window with a wooden frame and bars.
    c.rect(x,y,w,h,WD0)
    c.hline(x+1,y+1,w-2,WD2); c.hline(x+1,y+2,w-2,WD1)
    for i in range(1,w-1):
        for j in range(3,h-1): c.set(x+i,y+j,WD1 if i%3==0 else DK)

def gable(c,cx,yapex,ybase,hw,truss=True):
    # A triangular gable over a roof: blue barge boards, a white face with a wooden truss.
    for y in range(yapex,ybase):
        t=(y-yapex)/max(1,ybase-1-yapex)
        hw_=max(1,round(1+(hw-1)*t))
        for x in range(cx-hw_-3,cx):
            d=x-(cx-hw_-3)
            if d==0: col=OUT
            elif d==1: col=RF3
            elif d==2: col=RF1
            elif d==3: col=OUT
            elif y>=ybase-3: col=(WD1,WD0,OUT)[y-(ybase-3)]
            elif truss and (cx-x<=1 or abs((cx-x)-(y-yapex)*0.9)<1.2): col=WD1
            else: col=WL0 if y<ybase-4 else WL1
            c.set(x,y,col)

def gallery(c,cx,ytop,ybot,hw):
    # The third floor: a row of lit shoji panels between posts above a wooden railing.
    mid=ytop+10
    for y in range(ytop,ybot):
        for x in range(cx-hw,cx):
            d=x-(cx-hw)
            if d==0: col=OUT
            elif y==ytop: col=WD0
            elif y<mid:
                if x%16 in (0,15) or d<3: col=WD1
                elif y==mid-1: col=WD0
                else: col=LIT0 if y<mid-3 else LIT1
            elif y==mid: col=WD2
            elif y==mid+1 or y==ybot-1: col=WD0
            elif y==ybot-2: col=WD1
            else: col=WD0 if x%4==0 else WD1
            c.set(x,y,col)

def door(c,cx,ytop,ybot,hw=16):
    # The entrance: a lit transom over dark sliding doors, in a wooden frame.
    for y in range(ytop,ybot):
        for x in range(cx-hw,cx):
            d=x-(cx-hw)
            if d<2: col=WD0 if d==0 else WD1
            elif y<ytop+2: col=WD1
            elif y<ytop+8: col=WD1 if x%8==0 else (LIT0 if y<ytop+6 else LIT1)
            elif y==ytop+8: col=WD0
            else: col=DK if (cx-x)!=1 else OUT
            c.set(x,y,col)

def castle(c,cx,y0):
    # The tenshu, 15 rows tall, 12 wide, its bottom at y0+240. Drawn bottom-up so each roof covers
    # the wall below its eave; the left half, then mirrored.
    Y=lambda v: y0+v
    wall(c,cx,Y(212),Y(240),90)                       # 1F
    for ox in (40,72): barred_window(c,cx-ox-7,Y(220))
    door(c,cx,Y(220),Y(240))
    roof(c,cx,Y(204),Y(214),92,96,flare=2)            # the skirt over 1F
    wall(c,cx,Y(178),Y(206),90)                       # 2F
    for ox in (0,32,64): barred_window(c,cx-ox-7,Y(186))
    gallery(c,cx,Y(118),Y(146),82)                    # 3F
    roof(c,cx,Y(140),Y(180),78,96,flare=4)
    gable(c,cx,Y(140),Y(174),34)                      # the big front gable
    wall(c,cx,Y(86),Y(106),70)                        # 4F
    for ox in (16,48): dark_window(c,cx-ox-6,Y(94))
    roof(c,cx,Y(102),Y(122),70,88,flare=3)
    for ox in (44,):                                  # two small dormers on the 3F roof
        gable(c,cx-ox,Y(106),Y(119),8,truss=False)
        mirror_part(c,cx-ox,Y(106),Y(119),12)
    wall(c,cx,Y(44),Y(62),54)                         # 5F
    for ox in (0,32): dark_window(c,cx-ox-6,Y(50))
    roof(c,cx,Y(58),Y(90),50,80,flare=4)
    roof(c,cx,Y(10),Y(48),34,62,flare=4)              # the top roof
    ridge(c,cx,Y(6),36)
    gable(c,cx,Y(8),Y(42),24)
    mirror(c,cx,cx-110,y0-8,y0+240)

def mirror_part(c,cx,y0,y1,hw):
    # Mirrors a small symmetric piece (a dormer) about its own axis.
    for y in range(y0,y1):
        for x in range(cx-hw,cx):
            v=c.get(x,y)
            if v: c.set(2*cx-1-x,y,v)

# ---- the compound wall ----
def wall_h(c,x0,x1,y0):
    # The low wall seen from the front, 32 px: a tiled cap, white panels between dark posts,
    # a dark base. Panels repeat every 16 px.
    for x in range(x0,x1):
        for j in range(32):
            y=y0+j
            if j==0: col=OUT
            elif j==1: col=RF3
            elif j<8: col=RF0 if x%4==3 else RF1
            elif j==8: col=RF0
            elif j==9: col=OUT
            elif j<26:
                if x%16 in (0,1): col=WD0
                elif j==10: col=WL1
                else: col=WL0
            elif j<31: col=WD0 if j>26 else WD1
            else: col=OUT
            c.set(x,y,col)

def wall_v(c,x0,y0,y1):
    # A side wall seen from above: its tiled cap, 10 px wide.
    for y in range(y0,y1):
        for i in range(10):
            col=(OUT,RF3,RF2,RF1,RF1,RF1,RF1,RF0,RF0,OUT)[i]
            if 0<i<9 and y%4==3: col=RF0
            c.set(x0+i,y,col)

def torii(c,cx,ytop,ybase):
    # The gate: a black kasagi with upturned ends, a red shimaki, the red nuki and two red pillars
    # with black collars at the foot. Left half, then mirrored.
    for x in range(cx-70,cx):
        d=x-(cx-70)
        lift=max(0,4-d//3)
        for j in range(-lift,8):
            y=ytop+j
            col=OUT if j in (-lift,7) or d==0 else (ST0 if j==-lift+1 else TK)
            c.set(x,y,col)
    for x in range(cx-62,cx):
        for j in range(8,13): c.set(x,ytop+j,OUT if j==12 or x==cx-62 else (RD2 if j==8 else RD1))
    for x in range(cx-6,cx):                      # the gakuzuka, the strut under the shimaki
        for j in range(13,22): c.set(x,ytop+j,RD0 if x==cx-6 else RD1)
    for x in range(cx-60,cx):
        for j in range(22,28): c.set(x,ytop+j,OUT if j in (22,27) or x==cx-60 else (RD2 if j==23 else RD1))
    px=cx-46
    for y in range(ytop+13,ybase):
        for i in range(12):
            col=(OUT,RD2,RD1,RD1,RD1,RD1,RD1,RD1,RD1,RD0,RD0,OUT)[i]
            if 22<=y-ytop<28 and 0<i<11: continue    # the nuki passes in front
            if ybase-y<=10: col=OUT if i in (0,11) or y==ybase-1 else (ST0 if i==1 else TK)
            c.set(px+i,y,col)
    mirror(c,cx,cx-72,ytop-6,ybase)

def nobori(c,x,ybot,flip=False):
    # A red-and-white banner on a pole, 16 x 48: the pole on the outer side, a crossbar at the top,
    # the cloth red above a diagonal and white below, on a small stone base.
    s=Canvas(16,48)
    s.rect(2,2,2,44,KWD)                                # the pole
    s.rect(2,2,12,2,KWD)                                # the crossbar
    for j in range(5,40):
        for i in range(4,14):
            red=(j-5)<22+(i-4)*1.2
            col=(RD1 if i>5 else RD2) if red else (WL0 if i>5 else WL1)
            if i==13: col=RD0 if red else WL1
            if red and j==5: col=RD2
            s.set(i,j,col)
    for j in (8,14,20):                                  # ties holding the cloth to the pole
        s.set(4,j,WL0)
    s.rect(0,44,7,4,ST1); s.hline(0,44,7,ST3)
    c.blit(s,x,ybot-48,flip)

KOMAINU=[
    "......aaaa......",
    ".....abbcca.....",
    "....abcddcba....",
    "...abcdeedcba...",
    "...abdOddOdba...",
    "...abcdddddba...",
    "....abOOOOba....",
    "....abcddcba....",
    "...abbcddccba...",
    "..abcddddddcba..",
    "..abcdbccbdcba..",
    "..abcdbccbdcba..",
    "..abcdbccbdcba..",
    "..abcddccddcba..",
    "..abbcddddccba..",
    ".aabcbbddbbcbaa.",
    ".abcddddddddcba.",
    ".abbbbbbbbbbbba.",
    ".acccccccccccca.",
    ".abbbbbbbbbbbba.",
    ".aaaaaaaaaaaaaa.",
]
def komainu(c,x,ybot):
    # A guardian lion-dog in grey stone on a pedestal, 16 x 21.
    m={'a':OUT,'b':ST0,'c':ST1,'d':ST2,'e':ST3,'O':OUT}
    for j,row in enumerate(KOMAINU):
        for i,ch in enumerate(row):
            if ch in m: c.set(x+i,ybot-len(KOMAINU)+j,m[ch])

TORO=[
    ".......a........",
    "......aea.......",
    "....aaaeaaaa....",
    "..aaeeeedddcaa..",
    ".abbbbbbbbbbbba.",
    "..aaaaaaaaaaaa..",
    "....acdddcca....",
    "....aWWaWWca....",
    "....aWWaWWca....",
    "....acdddcca....",
    "...aaaaaaaaaa...",
    "...abccccccba...",
    "...aaaaaaaaaa...",
    "......adca......",
    "......adca......",
    "......adca......",
    "......adca......",
    ".....aadcaa.....",
    "....abbbbbba....",
    "...abccccccba...",
    "...aaaaaaaaaa...",
]
def toro(c,x,ybot):
    # A stone lantern, 16 x 21.
    m={'a':OUT,'b':ST0,'c':ST1,'d':ST2,'e':ST3,'W':WL0}
    for j,row in enumerate(TORO):
        for i,ch in enumerate(row):
            if ch in m: c.set(x+i,ybot-len(TORO)+j,m[ch])

# ---- the cobbled plaza ----
# Rounded cobbles in offset courses, 8 px each: a bright top-left, a shaded bottom-right, darker gaps.
STONE=[
    ".hhhhh..",
    "hhbbbbs.",
    "hbbbbbs.",
    "hbbbbbs.",
    "hbbbbbs.",
    "hbbbbbs.",
    ".sssss..",
    "........",
]
def cobble_px(x,y):
    yy=y%16; xx=(x+(4 if yy>=8 else 0))%8
    return {'.':ST2,'h':WL0,'b':ST3,'s':WL1}[STONE[yy%8][xx]]

def plaza(c,cells,r=5):
    # Fills a set of map cells with cobbles; corners where the plaza meets grass are rounded.
    for (cx,cy) in cells:
        for j in range(16):
            for i in range(16):
                x,y=cx*16+i,cy*16+j
                # round off a convex corner: both neighbours on that side are grass
                cut=False
                for (sx,sy,ci,cj) in ((-1,-1,0,0),(1,-1,15,0),(-1,1,0,15),(1,1,15,15)):
                    if (cx+sx,cy) not in cells and (cx,cy+sy) not in cells:
                        dx=r-1-abs(i-ci); dy=r-1-abs(j-cj)
                        if dx>0 and dy>0 and dx*dx+dy*dy>(r-1)**2: cut=True
                if cut: continue
                edge=((i==0 and (cx-1,cy) not in cells) or (i==15 and (cx+1,cy) not in cells) or
                      (j==0 and (cx,cy-1) not in cells) or (j==15 and (cx,cy+1) not in cells))
                c.set(x,y,ST2 if edge else cobble_px(x,y))
