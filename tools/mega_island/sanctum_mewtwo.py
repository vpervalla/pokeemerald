# MEWTWO waiting in its sanctum (OBJ_EVENT_GFX_SANCTUM_MEWTWO): its idle frames facing down, up and
# left, cut from a Mystery Dungeon style sprite sheet (reference/mewtwo_sheet.png, ripped by
# redblueyellow, "no credit needed"), scaled by SCALE (averaged, then each pixel snapped back to the
# sheet's colours), each in a 32x32 frame, its feet on the bottom row; facing right
# is the left frame mirrored. Writes graphics/object_events/pics/kanto/pokemon/sanctum_mewtwo.png and
# its own palette, graphics/object_events/palettes/kanto/sanctum_mewtwo.pal (the sheet's colours,
# the background first, transparent).
import os
from PIL import Image
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
im=Image.open(REPO+'tools/mega_island/reference/mewtwo_sheet.png').convert('RGB')
BG=(0,128,128)
# The first frame of the idle column's rows: down, up, left (boxes inclusive).
BOXES=((74,22,100,49),(76,52,100,81),(79,85,104,114))
SCALE=0.75
ALPHA=110                      # an averaged pixel less covered than this is left transparent
cols=[BG]
for x0,y0,x1,y1 in BOXES:
    for y in range(y0,y1+1):
        for x in range(x0,x1+1):
            c=im.getpixel((x,y))
            if c not in cols: cols.append(c)
frames=[]
for x0,y0,x1,y1 in BOXES:
    src=im.crop((x0,y0,x1+1,y1+1)).convert('RGBA'); px=src.load()
    for y in range(src.height):
        for x in range(src.width):
            if px[x,y][:3]==BG: px[x,y]=(0,0,0,0)
    small=src.resize((round(src.width*SCALE),round(src.height*SCALE)),Image.BOX); sp=small.load()
    w,h=small.size
    assert w<=32 and h<=32
    f=[[0]*32 for _ in range(32)]
    ox,oy=(32-w)//2,32-h
    for y in range(h):
        for x in range(w):
            r,g,b,a=sp[x,y]
            if a<ALPHA: continue
            f[oy+y][ox+x]=min(range(1,len(cols)),key=lambda i:sum((p-q)**2 for p,q in zip(cols[i],(r,g,b))))
    frames.append(f)
assert len(cols)<=16
pal=cols+[(0,0,0)]*(16-len(cols))
out=Image.new('P',(32,32*len(frames))); out.putpalette([v for c in pal for v in c])
for i,f in enumerate(frames):
    for y in range(32):
        for x in range(32): out.putpixel((x,i*32+y),f[y][x])
out.save(REPO+'graphics/object_events/pics/kanto/pokemon/sanctum_mewtwo.png',transparency=0)
with open(REPO+'graphics/object_events/palettes/kanto/sanctum_mewtwo.pal','w',newline='\r\n') as fp:
    fp.write('JASC-PAL\n0100\n16\n'+''.join('%d %d %d\n'%c for c in pal))
print('sanctum_mewtwo: %d frames, %d colours'%(len(frames),len(cols)))
