# Gothic castle exterior pixel art, drawn with one 15-colour palette (index 0 = transparent).
from PIL import Image
# Two 16-colour palettes (tileset slots 7 and 8). The art uses indices 0-31: 0-15 are palette A,
# 16-31 palette B. Each 8x8 tile must fit one palette; colours both palettes share (outline, dark
# stone, gold, spire blues) are matched by RGB, so a tile may mix them with either palette's colours.
PAL_A=[(0,0,0),(8,12,20),(20,28,36),(36,48,56),(56,72,80),(80,100,108),(120,140,148),
       (32,32,80),(56,64,144),(104,120,200),(56,24,88),(96,48,144),(216,176,96),(24,56,96),(88,152,200),(64,44,40)]
T,OUT,S0,S1,S2,S3,S4,R0,R1,R2,C0,C1,G,W0,W1,WD=range(16)
PAL_B=[(0,0,0),PAL_A[OUT],PAL_A[S0],PAL_A[S1],PAL_A[S2],PAL_A[S3],
       (120,116,112),(160,156,148),(196,190,180),(224,218,206),(244,240,232),(64,108,52),PAL_A[S4],
       PAL_A[G],PAL_A[R1],PAL_A[R2]]
P0,P1,P2,P3,P4,M0=22,23,24,25,26,27   # pale stone ramp and moss, palette B
PAL=PAL_A+PAL_B
class Canvas:
    def __init__(s,w,h): s.w,s.h=w,h; s.p=[[0]*w for _ in range(h)]
    def set(s,x,y,c):
        if 0<=x<s.w and 0<=y<s.h: s.p[y][x]=c
    def get(s,x,y): return s.p[y][x] if 0<=x<s.w and 0<=y<s.h else 0
    def rect(s,x,y,w,h,c):
        for j in range(h):
            for i in range(w): s.set(x+i,y+j,c)
    def hline(s,x,y,w,c): s.rect(x,y,w,1,c)
    def vline(s,x,y,h,c): s.rect(x,y,1,h,c)
    def outline(s,c=OUT):
        # Dark outline around every non-transparent shape (Gen 3 style).
        add=[]
        for y in range(s.h):
            for x in range(s.w):
                if s.p[y][x]==0 and any(s.get(x+dx,y+dy) not in (0,OUT) for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))):
                    add.append((x,y))
        for x,y in add: s.p[y][x]=c
    def blit(s,o,x0,y0,flip=False):
        for y in range(o.h):
            for x in range(o.w):
                c=o.p[y][o.w-1-x if flip else x]
                if c: s.set(x0+x,y0+y,c)
    def img(s,bg=None):
        im=Image.new('RGBA',(s.w,s.h),(0,0,0,0))
        for y in range(s.h):
            for x in range(s.w):
                c=s.p[y][x]
                if c: im.putpixel((x,y),PAL[c]+(255,))
        return im

def stone_wall(c,x0,y0,w,h,light=True):
    # Bricks 16x8 with offset rows (period 16 px), mortar lines, top-left light.
    for y in range(h):
        for x in range(w):
            gy=(y0+y); gx=(x0+x)
            row=gy//8; off=8 if row%2 else 0
            bx=(gx+off)%16; by=gy%8
            if by==7 or bx==15: col=S0
            elif by==0 or bx==0: col=S3 if light else S2
            elif by==6 or bx==14: col=S1
            else: col=S2
            c.set(gx,gy,col)

def pointed_window(c,x,y,w,h,glass=True):
    # Gothic pointed arch window, symmetric, stone frame.
    cx=x+w/2-0.5
    for j in range(h):
        for i in range(w):
            dx=abs(x+i-cx)
            arch=(w/2)*(1-max(0,(w/2-j))/(w/2))**0.5 if j<w/2 else w/2
            inside=dx<arch-1 if j>0 else False
            edge=dx<arch+0.5
            if not edge: continue
            if inside and glass:
                col=W0
                # leading lines: vertical mullion and a cross bar
                if abs(x+i-cx)<0.6 or (j==h*2//3): col=OUT
                elif (i+j)%4==0 and j>2: col=W1
                elif j<h//2 and (i*3+j)%5==0: col=W1
            elif inside: col=OUT
            else: col=S4 if (x+i)<cx else S3
            c.set(x+i,y+j,col)

def banner(c,x,y,w,h):
    # Hanging crimson drape with gold trim, gold rod, pointed tail, and an emblem.
    c.hline(x-1,y,w+2,G); c.hline(x-1,y+1,w+2,OUT)
    for j in range(2,h):
        for i in range(w):
            tail=j>=h-w//2 and abs(i-(w-1)/2)<(h-j)-0.5
            body=j<h-w//2
            if not (body or tail): continue
            col=C1
            if i==0 or i==w-1: col=G
            elif i==1: col=C1 if j%6 else C0
            elif i==w-2: col=C0
            c.set(x+i,y+j,col)
    # emblem: a gold ring with a dot (stylised Mega symbol)
    ex=x+w//2; ey=y+h//2-2
    for (dx,dy) in ((-2,0),(2,0),(0,-2),(0,2),(-1,-1),(1,-1),(-1,1),(1,1)): c.set(ex+dx-(1 if w%2==0 else 0)+ (1 if dx>0 and w%2==0 else 0),ey+dy,G)
    c.set(ex,ey,W1)

def crenellations(c,x0,y0,w,merlon=6,gap=4,h=7):
    # Parapet with merlons, period merlon+gap.
    per=merlon+gap
    for x in range(w):
        gx=x0+x
        m=(gx%per)<merlon
        top=y0 if m else y0+3
        for y in range(top,y0+h):
            col=S2
            if y==top: col=S4
            elif y==top+1: col=S3
            elif (gx%per)==merlon-1 and m: col=S1
            c.set(gx,y,col)
    c.hline(x0,y0+h,w,S0)

def slate_roof(c,x0,y0,w,h):
    # Roof seen from above: rows of slate shingles, darker towards the eaves.
    for y in range(h):
        for x in range(w):
            gx,gy=x0+x,y0+y
            row=gy//4; off=(row%2)*4
            bx=(gx+off)%8; by=gy%4
            shade=y/h
            base=R2 if shade<0.25 else (R1 if shade<0.75 else R0)
            col=base
            if by==3: col=R0 if base!=R0 else OUT
            elif bx==7: col=R0
            elif by==0 and base!=R2: col=R2 if base==R1 else R1
            c.set(gx,gy,col)

def cone_roof(c,cx,y0,r,h):
    # Conical tower roof (symmetric), shingled, with a gold finial.
    for j in range(h):
        half=max(1,int(r*(j+1)/h+0.5))
        for i in range(-half,half):
            x=cx+i
            d=abs(i+0.5)/half
            base=R2 if d<0.3 else (R1 if d<0.7 else R0)
            col=base
            if (j%4)==3: col=R0 if base!=R0 else OUT
            elif ((int(abs(i+0.5))+ (j//4)%2*2)%4)==0 and base!=R2: col=R0
            c.set(x,y0+j,col)
    # eave rim
    for i in range(-r-1,r+1): c.set(cx+i,y0+h,S1); c.set(cx+i,y0+h+1,OUT if abs(i+0.5)>r-1 else S0)
    # finial
    c.vline(cx-1,y0-6,6,G); c.vline(cx,y0-6,6,G); c.set(cx-1,y0-7,G); c.set(cx,y0-7,G)
    c.rect(cx-2,y0-3,4,1,G)

def round_tower(c,cx,y0,r,h):
    # Cylinder body: symmetric shading (light centre), brick courses every 8px, slit windows.
    for j in range(h):
        for i in range(-r,r):
            x=cx+i; d=abs(i+0.5)/r
            base=S3 if d<0.25 else (S2 if d<0.6 else (S1 if d<0.85 else S0))
            gy=y0+j; col=base
            if gy%8==7: col=S0 if base!=S0 else OUT
            elif ((int(abs(i+0.5))+ (gy//8)%2*8)%16)==7 and d<0.85: col=S0
            c.set(x,gy,col)
    # band
    for i in range(-r,r): c.set(cx+i,y0+20,G if abs(i+0.5)<r-1 else C0); c.set(cx+i,y0+21,C0)

def slit_window(c,x,y,h):
    c.rect(x,y,2,h,OUT); c.set(x,y+1,W1); c.set(x+1,y+2,W0)

def gate(c,x,y,w,h):
    # Big pointed archway with studded double doors.
    cx=x+w/2-0.5
    for j in range(h):
        for i in range(w):
            dx=abs(x+i-cx); half=w/2
            arch=half*(1-max(0,(half-j))/half)**0.5 if j<half else half
            if dx>=arch+0.5: continue
            if dx>=arch-3: col=S4 if dx>=arch-1.5 else S3  # voussoir frame
            elif dx>=arch-4: col=OUT
            else:
                col=WD
                if abs(x+i-cx)<0.6: col=OUT  # door seam
                elif (j%10)==4: col=OUT      # iron bands
                elif (j%10)==5: col=S1
                elif ((x+i)%6==2 and j%10==7): col=G  # studs
            c.set(x+i,y+j,col)
    # keystone
    c.rect(int(cx)-1,y,3,3,S4); c.set(int(cx),y+1,G)

def stairs(c,x,y,w,steps):
    for k in range(steps):
        for j in range(5):
            col=S4 if j==0 else (S3 if j<3 else S1)
            c.hline(x-k*2,y+k*5+j,w+k*4,col)
        c.hline(x-k*2,y+k*5+4,w+k*4,S0)

GARGOYLE=[ # 24 wide x 30 tall, symmetric half (12 cols) mirrored. '.'=transparent
"............",
"..........11",
".........1S1",
"........1SS1",
".......1SSS1",
"..1111.1SSSS",
".1MLL1.1SSSS",
"1MLLL11SSSSS",
"1MLMM1SSSSSS",
"1MMMSSSS1111",
"1MMSSSS1LLLL",
".1MSSS1LLMML",
".1MSS1LLMM11",
"..1S1LLMMM1E",
"..1S1LMMMM1E",
"...11MMMMM1R",
"....1MMMSSS1",
"....1MMSS1SS",
"...1MMSS1SSS",
"...1MSS1SSSS",
"..1MMS1SSSSS",
"..1MSS1SSS11",
"..1MS1SSS1MM",
"...1S1S11MMM",
"...11111MMMM",
"..1LLLLLLLLL",
"..1MMMMMMMMM",
"..1MMMMMMMMM",
"..1SSSSSSSSS",
"..1111111111",
]
def gargoyle(c,x,y):
    m={'.':0,'1':OUT,'S':S1,'M':S2,'L':S3,'E':C1,'R':S0}
    for j,row in enumerate(GARGOYLE):
        full=row+row[::-1]
        for i,ch in enumerate(full):
            col=m[ch]
            if col: c.set(x+i,y+j,col)

def fill_poly(c,pts,shade):
    # Scanline polygon fill; shade(x,y) returns a colour index.
    ys=[p[1] for p in pts]
    for y in range(int(min(ys)),int(max(ys))+1):
        xs=[]
        n=len(pts)
        for i in range(n):
            (x1,y1),(x2,y2)=pts[i],pts[(i+1)%n]
            if (y1<=y+0.5<y2) or (y2<=y+0.5<y1):
                xs.append(x1+(y+0.5-y1)*(x2-x1)/(y2-y1))
        xs.sort()
        for k in range(0,len(xs)-1,2):
            for x in range(int(round(xs[k])),int(round(xs[k+1]))):
                col=shade(x,y)
                if col: c.set(x,y,col)
def fill_ellipse(c,cx,cy,rx,ry,shade):
    for y in range(int(cy-ry),int(cy+ry)+1):
        for x in range(int(cx-rx),int(cx+rx)+1):
            if ((x+0.5-cx)/rx)**2+((y+0.5-cy)/ry)**2<=1: c.set(x,y,shade(x,y))

def gargoyle2(c,x0,y0):
    # 32x44: winged gargoyle crouching on a pedestal. Drawn on the left half, then mirrored.
    g=Canvas(32,44)
    lit=lambda x,y: S3 if x<10 else S2
    # wing: fan of three ribs from the shoulder (14,16) out to the left
    wing=[(14,15),(1,2),(3,9),(0,12),(4,17),(1,21),(8,23),(14,22)]
    fill_poly(g,wing,lambda x,y: S1 if (x+y)%5==0 else S2)
    for (ex,ey) in ((1,2),(0,12),(1,21)):
        n=12
        for k in range(n+1):
            t=k/n; g.set(int(14+(ex-14)*t),int(16+(ey-16)*t),S3)
    # body (haunch) and chest
    fill_ellipse(g,16,26,8,7,lambda x,y: S3 if x<12 and y<26 else (S2 if x<15 else S1))
    # head with horns
    fill_ellipse(g,16,13,5,5,lambda x,y: S4 if x<14 and y<12 else (S3 if x<15 else S2))
    fill_poly(g,[(12,10),(9,3),(14,8)],lambda x,y: S4 if x<11 else S3)
    g.set(13,13,W1); g.set(13,14,W0)                       # glowing eye
    g.hline(14,16,2,OUT)                                      # snarl
    g.set(14,17,S4)                                           # fang
    # claws gripping the ledge
    for k in range(3): g.set(10+k*2,32,S4); g.set(10+k*2,33,S1)
    # pedestal
    for y in range(33,44):
        for x in range(5,16):
            col=S2
            if y in (33,34): col=S4 if y==33 else S3
            elif y==43: col=S0
            elif x==5: col=S3
            elif y%4==0: col=S1
            g.set(x,y,col)
    # mirror left half to the right
    for y in range(44):
        for x in range(16): 
            if g.p[y][x]: g.p[y][31-x]=g.p[y][x]
    g.outline()
    c.blit(g,x0,y0)

def buttress(c,x,y0,h,w=10):
    # Vertical pier with a pointed pinnacle on top.
    for j in range(h):
        for i in range(w):
            col=S3 if i<3 else (S2 if i<w-3 else S1)
            if (y0+j)%16==15: col=S0
            c.set(x+i,y0+j,col)
    for j in range(14):   # pinnacle
        half=max(1,int((w/2)*(j+1)/14+0.5))
        for i in range(-half,half):
            c.set(x+w//2+i,y0-14+j,S4 if i<0 else S2)
    c.set(x+w//2-1,y0-16,G); c.set(x+w//2,y0-16,G); c.set(x+w//2-1,y0-15,G); c.set(x+w//2,y0-15,G)

def rose_window(c,cx,cy,r):
    for y in range(cy-r,cy+r+1):
        for x in range(cx-r,cx+r+1):
            d=((x+0.5-cx)**2+(y+0.5-cy)**2)**0.5
            if d>r: continue
            if d>r-2.5: col=S4 if x<cx else S3
            elif d>r-3.5: col=OUT
            else:
                import math
                a=math.atan2(y+0.5-cy,x+0.5-cx); petal=(math.cos(a*8)+1)/2
                col=W1 if (d<r*0.3 or petal>0.75) else W0
                if abs(d-r*0.55)<0.6: col=OUT
                if d<1.5: col=G
            c.set(x,y,col)

def drape_swag(c,x,y,w,h,side):
    # Heavy curtain hanging beside the gate, gathered at the bottom.
    for j in range(h):
        width=int(w-(w*0.5)*max(0,j-h*0.55)/(h*0.45))
        for i in range(width):
            xx=x+i if side<0 else x+w-1-i
            col=C1 if (i//2)%2==0 else C0
            if i==0: col=G
            c.set(xx,y+j,col)
    c.hline(x-1,y,w+2,G)

def pennant(c,x,y):
    c.vline(x,y,10,OUT)
    for j in range(5):
        c.hline(x+1,y+j,6-j,C1 if j<4 else C0)

FLAG_A=[ # 16x16 irregular flagstones: letters = slab ids, '#' = joint
"aaaaaa#bbbbbbbb#",
"aaaaaa#bbbbbbbb#",
"aaaaaa#bbbbbbbb#",
"aaaaaa#bbbbbbbb#",
"aaaaaa#bbbbbbbb#",
"#######bbbbbbbb#",
"cccccc##########",
"cccccccc#ddddddd",
"cccccccc#ddddddd",
"cccccccc#ddddddd",
"cccccccc#ddddddd",
"################",
"eeee#fffffff#ggg",
"eeee#fffffff#ggg",
"eeee#fffffff#ggg",
"################",
]
def flagstone(c,x0,y0,variant=0):
    rows=FLAG_A if variant==0 else [r[8:]+r[:8] for r in FLAG_A[8:]+FLAG_A[:8]]
    for y in range(16):
        for x in range(16):
            ch=rows[y][x]
            if ch=='#': col=S1
            else:
                up=rows[y-1][x] if y>0 else '#'; left=rows[y][x-1] if x>0 else '#'
                col=S3 if (up=='#' or left=='#') else S2
                if (x*5+y*3+ord(ch))%13==0: col=S1
            c.set(x0+x,y0+y,col)

def fence(c,x0,y0):
    # 16x24 wrought-iron fence segment: rails and spear-tipped bars (bottom 16 rows on the ground cell).
    for bx in (1,5,9,13):
        c.vline(x0+bx,y0+3,19,OUT); c.vline(x0+bx+1,y0+3,19,S1)
        c.set(x0+bx,y0+2,G); c.set(x0+bx+1,y0+2,G); c.set(x0+bx,y0+1,G)
    for ry in (6,18):
        c.hline(x0,y0+ry,16,OUT); c.hline(x0,y0+ry+1,16,S1)

def gate_pillar(c,x0,y0):
    # 16x40 stone pillar with a lantern on top.
    for y in range(14,40):
        for x in range(2,14):
            col=S2 if x<5 else (S1 if x>10 else S2)
            if x<4: col=S3
            if y in (14,15): col=S4
            if y%10==4 and y>15: col=S0
            c.set(x0+x,y0+y,col)
    # lantern
    c.rect(x0+5,y0+3,6,9,OUT); c.rect(x0+6,y0+4,4,7,G); c.rect(x0+7,y0+5,2,5,W1)
    c.hline(x0+4,y0+2,8,OUT); c.hline(x0+5,y0+1,6,S1); c.hline(x0+4,y0+12,8,OUT); c.hline(x0+5,y0+13,6,S1)

def lamp_post(c,x0,y0):
    # 16x32 iron lamp post.
    c.vline(x0+7,y0+10,20,OUT); c.vline(x0+8,y0+10,20,S1)
    c.rect(x0+5,y0+28,6,4,OUT); c.rect(x0+6,y0+28,4,3,S1)
    c.rect(x0+4,y0+2,8,8,OUT); c.rect(x0+5,y0+3,6,6,G); c.rect(x0+6,y0+4,4,4,W1); c.set(x0+7,y0+5,G)
    c.hline(x0+3,y0+1,10,OUT); c.hline(x0+6,y0,4,OUT)

def dead_tree(c,x0,y0):
    # 32x48 bare twisted tree.
    import math
    def branch(x,y,ang,length,width):
        for k in range(length):
            xx=x+math.cos(ang)*k; yy=y-math.sin(ang)*k
            for w in range(width):
                c.set(int(xx)+w,int(yy),WD if w==0 else OUT)
        return x+math.cos(ang)*length, y-math.sin(ang)*length
    # trunk
    for y in range(20,48):
        wdt=6 if y>40 else 4
        for w in range(wdt):
            c.set(x0+14+w-(1 if y>40 else 0),y0+y,WD if w<wdt-1 else OUT)
    for (ang,ln,sub) in ((2.2,14,1.6),(0.9,13,2.0),(1.5,16,0.9),(2.7,9,2.2),(0.4,9,0.7)):
        ex,ey=branch(x0+15,y0+22,ang,ln,2)
        branch(int(ex),int(ey),sub,6,1)

def tall_window(c,x,ytop,w,ybot):
    # A very tall lancet window whose top is out of sight. The glass pattern repeats every 16 px
    # (by absolute y) so the long middle part reuses the same tiles.
    cx=x+w/2-0.5
    for gy in range(max(0,ytop),ybot):
        for i in range(w):
            gx=x+i; dx=abs(gx-cx)
            if i<3 or i>=w-3: col=S4 if i<3 else S3      # stone frame
            elif i==3 or i==w-4: col=OUT
            else:
                col=W0
                if dx<0.6 or gy%16==15: col=OUT          # mullion and leading
                elif (i+gy)%4==0: col=W1
            c.set(gx,gy,col)
    for i in range(-1,w+1): c.set(x+i,ybot,S4); c.set(x+i,ybot+1,S3); c.set(x+i,ybot+2,S1)  # sill

def long_banner(c,x,ybot,w):
    # A banner hanging from far above: crimson with gold borders, a pointed tail and an emblem.
    for gy in range(0,ybot):
        for i in range(w):
            tail=gy>=ybot-w//2
            if tail and abs(i-(w-1)/2)>=(ybot-gy)-0.5: continue
            col=C1
            if i==0 or i==w-1: col=G
            elif i==1 and gy%8==3: col=C0
            elif i==w-2: col=C0
            c.set(x+i,gy,col)
    ey=ybot-w-6; ex=x+w//2
    for (dx,dy) in ((-2,0),(1,0),(0,-2),(-1,-2),(0,1),(-1,1),(-2,-1),(1,-1)): c.set(ex+dx,ey+dy,G)
    c.set(ex,ey-1,W1); c.set(ex-1,ey,W1)

def rocky_mound(c,x0,x1,ytop,ybot,cx):
    # A natural rocky mound: rounded boulders in a 64 px module (repeats, mirrored about cx),
    # a lumpy top edge and rocks spilling onto the grass at the foot.
    import math
    BOULDERS=[ # (x, y, rx, ry) in the module, y from ytop
        (8,10,12,9),(30,6,13,8),(52,12,12,10),(18,26,14,10),(44,28,15,10),(4,40,9,7),(30,42,12,7),(58,41,9,6)]
    def shade(px,py,bx,by,rx,ry):
        nx=(px+0.5-bx)/rx; ny=(py+0.5-by)/ry
        d=nx*nx+ny*ny
        if d>1: return None
        if d>0.78: return OUT if (ny>0.3 or nx>0.5) else S1
        l=-(nx*0.6+ny*0.8)
        return S4 if l>0.55 else (S3 if l>0.15 else (S2 if l>-0.35 else S1))
    for gx in range(x0,x1):
        m=(gx-cx)%64 if gx>=cx else (cx-1-gx)%64     # mirrored about the staircase axis
        for gy in range(ytop-6,ybot+8):
            py=gy-ytop; col=None
            for (bx,by,rx,ry) in BOULDERS:
                for mx in (m,m-64,m+64):
                    s=shade(mx,py,bx,by,rx,ry)
                    if s is not None: col=s
            if col is None and 4<=py<=ybot-ytop-2: col=S0       # crevices between boulders
            if col is not None: c.set(gx,gy,col)

def stone_rampart(c,x0,x1,ytop,ybot,cx):
    # A dressed stone retaining wall: coping along the top, courses of large ashlar blocks,
    # pilasters every 64 px (lined up with the castle's bays) and a dark plinth at the foot.
    # Everything repeats on the 8-pixel grid, relative to cx.
    for gx in range(x0,x1):
        rx=(gx-cx)%64
        for gy in range(ytop,ybot):
            y=gy-ytop
            if y<6:                                         # coping
                col=S4 if y==0 else (S3 if y<3 else (S1 if y==5 else S2))
                if rx%32==31 and 1<=y<=4: col=S1
            elif ybot-gy<=8:                                # plinth
                col=S1 if (ybot-gy)>1 else S0
                if ybot-gy==8: col=S2
                if rx%16==15: col=S0
            else:
                course=(y-6)//12; by=(y-6)%12
                bx=(rx+(course%2)*16)%32
                if by==11 or bx==31: col=OUT
                elif by==0 or bx==0: col=S3
                elif by>=9 or bx>=29: col=S1
                else: col=S2
                if bx in (12,13) and by==5 and course%2==0: col=S1   # chisel marks
            # pilasters, lined up with the buttresses above
            if 6<=y and ybot-gy>8 and (rx<5 or rx>=59):
                d=rx if rx<5 else 63-rx
                col=S3 if (d<2 and rx<5) else (S1 if rx>=59 else S2)
                if (y-6)%24==23: col=OUT
            c.set(gx,gy,col)

# ---- pieces for the blackstone design (pale stone uses palette B) ----

def pale_paving(c,x0,y0,w,h):
    # Pale weathered flagstones (period 16 px by absolute position), with a few moss specks.
    for gy in range(y0,y0+h):
        for gx in range(x0,x0+w):
            ch=FLAG_A[gy%16][gx%16]
            if ch=='#': col=P0
            else:
                up=FLAG_A[(gy-1)%16][gx%16]; left=FLAG_A[gy%16][(gx-1)%16]
                col=P3 if (up=='#' or left=='#') else P2
                lx,ly=gx%16,gy%16           # specks repeat every 16 px, like the slabs
                if (lx*5+ly*3+ord(ch))%13==0: col=P1
                if (lx,ly) in ((3,9),(12,2)): col=M0
            c.set(gx,gy,col)

def pale_gate(c,x,y,w,h):
    # Pointed archway with a pale stone frame and black iron-bound doors.
    cx=x+w/2-0.5; half=w/2
    for j in range(h):
        for i in range(w):
            dx=abs(x+i-cx)
            arch=half*(1-max(0,(half-j))/half)**0.5 if j<half else half
            if dx>=arch+0.5: continue
            if dx>=arch-4: col=P3 if dx>=arch-1.5 else P2
            elif dx>=arch-5: col=OUT
            else:
                col=S1
                if abs(x+i-cx)<0.6: col=OUT
                elif (j%12)==5: col=OUT
                elif (j%12)==6: col=S0
                elif ((x+i)%6==2 and j%12==9): col=G
            c.set(x+i,y+j,col)
    c.rect(int(cx)-2,y,4,4,P4); c.set(int(cx),y+2,G)

def arcade_wall(c,x0,x1,ytop,ybot,cx):
    # The bastion's front: dark ashlar with a blind arcade of pointed arches every 32 px,
    # a pale coping along the top and a plinth at the foot.
    for gx in range(x0,x1):
        rx=(gx-cx)%32
        for gy in range(ytop,ybot):
            y=gy-ytop
            if y<5: col=P4 if y==0 else (P3 if y<3 else (P1 if y==3 else OUT))
            elif ybot-gy<=6: col=S1 if ybot-gy>1 else S0
            else:
                course=(y-5)//8; by=(y-5)%8; bx=(rx+(course%2)*8)%16
                col=S2 if by and bx else S1
                if by==1 and bx: col=S3
                # blind arch: pillars at rx 0-3, pointed head between y 10 and 24
                ay=y-10
                if rx<4: col=S3 if rx<2 else S1
                elif ay>=0 and ybot-gy>6:
                    half=14; d=abs(rx-17.5)
                    top=half*(1-max(0,(half-ay))/half)**0.5 if ay<half else half
                    if d<top-1: col=S0
                    elif d<top+0.5: col=OUT if rx>=17 else S3
            c.set(gx,gy,col)

def obelisk(c,x,y,h):
    # A slim dark pillar ending in a blue spire, 16 px wide.
    for j in range(h):
        if j<20:
            half=max(1,int(5*(j+1)/20+0.5))
            for i in range(-half,half): c.set(x+8+i,y+j,R2 if i<0 else R1)
        else:
            for i in range(4,12): c.set(x+i,y+j,S3 if i<6 else (S2 if i<10 else S1))
    c.rect(x+3,y+20,10,3,S3); c.rect(x+2,y+h-6,12,6,S2); c.hline(x+2,y+h-6,12,S3)
    c.set(x+7,y-3,G); c.set(x+8,y-3,G); c.set(x+7,y-2,G); c.set(x+8,y-2,G)

def pale_balustrade(c,x0,x1,y):
    # A low pale parapet: rail, balusters and base, 12 px tall.
    for gx in range(x0,x1):
        c.set(gx,y,P4); c.set(gx,y+1,P3); c.set(gx,y+2,OUT)
        for j in range(3,9):
            c.set(gx,y+j,(P2 if (gx%6) in (1,2) else (P0 if (gx%6)==3 else 0)) if j<8 else P1)
        c.set(gx,y+9,P3); c.set(gx,y+10,P1); c.set(gx,y+11,OUT)

def turret(c,cx,ytop,ybody,ybot):
    # A bartizan: a small round turret corbelled out of the wall, under a tall blue spire.
    # Spire from ytop to ybody, body from ybody to ybot-12, corbel tapering to ybot. 24 px wide.
    r=11
    h=ybody-ytop
    for j in range(h):                       # spire
        half=max(1,int((r+1)*(j+1)/h+0.5))
        for i in range(-half,half):
            d=abs(i+0.5)/half
            col=R2 if d<0.35 else (R1 if d<0.75 else R0)
            if j%6==5: col=R0 if col!=R0 else OUT
            c.set(cx+i,ytop+j,col)
    c.set(cx-1,ytop-4,G); c.set(cx,ytop-4,G); c.vline(cx-1,ytop-3,3,G); c.vline(cx,ytop-3,3,G)
    for i in range(-r-1,r+1): c.set(cx+i,ybody,S3); c.set(cx+i,ybody+1,OUT)
    for j in range(ybody+2,ybot-12):         # body
        for i in range(-r,r):
            d=abs(i+0.5)/r
            col=S3 if d<0.3 else (S2 if d<0.65 else (S1 if d<0.9 else S0))
            if (j-ybody)%8==7: col=S0
            c.set(cx+i,j,col)
    c.rect(cx-1,ybody+8,2,10,OUT); c.set(cx-1,ybody+9,W1)   # arrow slit
    for j in range(12):                      # corbel
        half=max(1,int(r*(12-j)/12+0.5))
        for i in range(-half,half):
            c.set(cx+i,ybot-12+j,S2 if j%3 else S3)
