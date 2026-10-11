# The indexed pixel canvas the island's art is drawn on (index 0 is transparent; what each index
# means is up to the palettes of the module drawing on it).
from PIL import Image
OUT=1   # the dark outline colour's index, in every palette set
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
    def img(s,pal):
        im=Image.new('RGBA',(s.w,s.h),(0,0,0,0))
        for y in range(s.h):
            for x in range(s.w):
                c=s.p[y][x]
                if c: im.putpixel((x,y),pal[c]+(255,))
        return im

