# The champions' overworld sprites (3 frames, 16x32: facing down, up, left), made from Lance's
# FRLG sprite with new hair, clothes and one of the four Kanto NPC palettes.
import os
from PIL import Image
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..')+'/'
BASE=REPO+'graphics/object_events/pics/kanto/people/lance.png'
PALDIR=REPO+'graphics/object_events/palettes/kanto/'
def load_pal(n):
    L=open(PALDIR+n+'.pal').read().split()[3:]
    return [tuple(int(v) for v in L[i*3:i*3+3]) for i in range(16)]
def grid():
    im=Image.open(BASE); return [[im.getpixel((x,y)) for x in range(48)] for y in range(32)]
def recolor(g,m,x0=0,x1=48,y0=0,y1=32):
    for y in range(y0,y1):
        for x in range(x0,x1):
            if g[y][x] in m: g[y][x]=m[g[y][x]]
def put(g,f,rows,x0=0,y0=0):
    # rows: strings, '.' = keep, ' ' = clear, hex digit = colour
    for j,r in enumerate(rows):
        for i,ch in enumerate(r):
            if ch=='.': continue
            g[y0+j][f*16+x0+i]=0 if ch==' ' else int(ch,16)
F,D,U,L=15,0,1,2   # outline colour; frames: down, up, left

def leon():
    g=grid()
    # a black snapback cap over the purple hair (frames down, up, left), long hair down the back
    put(g,D,["....FFFFFF.....",
             "...FDDDDDDF....",
             "..FDDD5DDDDF...",
             "..FFFFFFFFFFFF.",],1,10)
    put(g,U,["....FFFFFF.....",
             "...FDDDDDDF....",
             "..FDDDDDDDDF...",
             "..FFFFFFFFFF...",],1,10)
    put(g,L,["....FFFFF......",
             "...FDDDDDF.....",
             "..FDDDDDD5F....",
             "FFFFFFFFFFF....",],2,10)
    for (f,x) in ((D,2),(D,13),(U,2),(U,13),(L,12)):
        for y in range(18,25):
            if g[y][f*16+x] in (0,F): g[y][f*16+x]=9
    for y in range(20,27):                  # hair down the back
        for x in range(19,29): 
            if g[y][x] in (12,13): g[y][x]=9 if (x+y)%3 else 10
    return g,'npc_blue'

def alder():
    g=grid()
    recolor(g,{12:5,13:6})                   # beige poncho, olive shade
    for f in (D,U,L):                        # wild spiky hair, sticking out further
        for (x,y) in ((1,14),(0,16),(1,18),(14,14),(15,16),(14,18),(4,10),(7,9),(10,10),(12,11)):
            if f==L and x>12: continue
            g[y][f*16+x]=9
            if y+1<32 and g[y+1][f*16+x]==0: g[y+1][f*16+x]=10
    return g,'npc_white'

def cynthia():
    g=grid()
    recolor(g,{8:5,9:6,10:7},y1=21)          # blonde hair
    recolor(g,{12:13,13:15,8:13,9:15,10:15},y0=21)   # black coat
    for f,xs in ((D,(1,2,13,14)),(U,(2,3,4,11,12,13)),(L,(10,11,12))):
        for x in xs:
            for y in range(19,27):           # long hair down to the waist
                if g[y][f*16+x] in (0,15,13,1,2,3,4,12):
                    g[y][f*16+x]=5 if y<23 else 6
    for x in (5,10): g[20][x]=15             # dark eyes (Lance's are red)
    for x in range(6,10): g[19][x]=7
    g[20][32+5]=15
    for f in (D,L):                           # grey fur collar
        for x in range(4,12):
            if g[21][f*16+x] not in (0,): g[21][f*16+x]=11
    return g,'npc_white'

def diantha():
    g=grid()
    recolor(g,{8:13,9:10,10:4},y1=21)        # dark hair
    recolor(g,{12:11,13:14,8:14,9:11,10:12},y0=21)   # white gown, light grey shade
    for f in (D,U,L):
        for x in range(3,13):                 # pink sash
            if g[25][f*16+x] in (11,14): g[25][f*16+x]=6
        for x in range(4,12):
            if g[29][f*16+x] in (10,13,15): g[29][f*16+x]=14
    for f,xs in ((D,(1,14)),(U,(1,14))):     # curled ends of the bob
        for x in xs:
            g[20][f*16+x]=10; g[19][f*16+x]=13
    return g,'npc_pink'

def save(name,g,pal):
    im=Image.new('P',(48,32)); flat=[]
    for c in load_pal(pal): flat+=list(c)
    im.putpalette(flat+[0]*(768-len(flat)))
    for y in range(32):
        for x in range(48): im.putpixel((x,y),g[y][x])
    im.save(REPO+f'graphics/object_events/pics/kanto/people/champion_{name}.png')
    return im
if __name__=='__main__':
    sheet=Image.new('RGB',(48*4,32),(112,200,160))
    for i,(n,fn) in enumerate((('cynthia',cynthia),('alder',alder),('diantha',diantha),('leon',leon))):
        g,pal=fn(); im=save(n,g,pal).convert('RGBA')
        rgb=Image.new('RGB',(48,32),(112,200,160)); px=save(n,g,pal)
        for y in range(32):
            for x in range(48):
                if g[y][x]: rgb.putpixel((x,y),load_pal(pal)[g[y][x]])
        sheet.paste(rgb,(i*48,0))
    sheet.resize((48*4*4,128),0).save('/tmp/claude-0/ow_champs.png')
