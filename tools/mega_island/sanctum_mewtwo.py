# MEWTWO waiting in its sanctum (OBJ_EVENT_GFX_SANCTUM_MEWTWO): its idle frames facing down, up and
# left, cut from a Mystery Dungeon style sprite sheet (reference/mewtwo_sheet.png, ripped by
# redblueyellow, "no credit needed"), each in a 32x32 frame, its feet on the bottom row; facing right
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
cols=[BG]; frames=[]
for x0,y0,x1,y1 in BOXES:
    w,h=x1-x0+1,y1-y0+1
    assert w<=32 and h<=32
    f=[[0]*32 for _ in range(32)]
    ox,oy=(32-w)//2,32-h
    for y in range(h):
        for x in range(w):
            c=im.getpixel((x0+x,y0+y))
            if c==BG: continue
            if c not in cols: cols.append(c)
            f[oy+y][ox+x]=cols.index(c)
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
