# Small pixel-art helpers for the champions' trainer pics (64x64, 16 colours, index 0 transparent).
from PIL import Image
class Pic:
    def __init__(s,pal,w=64,h=64): s.w,s.h,s.pal=w,h,pal; s.p=[[0]*w for _ in range(h)]
    def set(s,x,y,c):
        x,y=int(x),int(y)
        if 0<=x<s.w and 0<=y<s.h: s.p[y][x]=c
    def get(s,x,y): return s.p[y][x] if 0<=x<s.w and 0<=y<s.h else 0
    def poly(s,pts,c):
        ys=[p[1] for p in pts]
        for y in range(int(min(ys)),int(max(ys))+1):
            xs=[]
            for i in range(len(pts)):
                (x1,y1),(x2,y2)=pts[i],pts[(i+1)%len(pts)]
                if (y1<=y+0.5<y2) or (y2<=y+0.5<y1): xs.append(x1+(y+0.5-y1)*(x2-x1)/(y2-y1))
            xs.sort()
            for k in range(0,len(xs)-1,2):
                for x in range(int(round(xs[k])),int(round(xs[k+1]))):
                    s.set(x,y,c(x,y) if callable(c) else c)
    def ell(s,cx,cy,rx,ry,c):
        for y in range(int(cy-ry-1),int(cy+ry+2)):
            for x in range(int(cx-rx-1),int(cx+rx+2)):
                if ((x+0.5-cx)/rx)**2+((y+0.5-cy)/ry)**2<=1: s.set(x,y,c(x,y) if callable(c) else c)
    def rect(s,x,y,w,h,c):
        for j in range(h):
            for i in range(w): s.set(x+i,y+j,c)
    def line(s,x0,y0,x1,y1,c):
        n=int(max(abs(x1-x0),abs(y1-y0)))+1
        for k in range(n+1):
            t=k/max(1,n); s.set(round(x0+(x1-x0)*t),round(y0+(y1-y0)*t),c)
    def outline(s,c=1):
        add=[(x,y) for y in range(s.h) for x in range(s.w) if s.p[y][x]==0 and
             any(s.get(x+dx,y+dy) not in (0,c) for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)))]
        for x,y in add: s.p[y][x]=c
    def img(s,bg=(112,200,160)):
        im=Image.new('RGB',(s.w,s.h),bg)
        for y in range(s.h):
            for x in range(s.w):
                if s.p[y][x]: im.putpixel((x,y),s.pal[s.p[y][x]])
        return im
    def save_indexed(s,path):
        im=Image.new('P',(s.w,s.h)); flat=[]
        for c in s.pal: flat+=list(c)
        flat+=[0]*(768-len(flat)); im.putpalette(flat)
        for y in range(s.h):
            for x in range(s.w): im.putpixel((x,y),s.p[y][x])
        im.save(path)
def shade3(lit,mid,dark,cx,spread):
    # left-lit shading across a body part centred on cx
    return lambda x,y: lit if x<cx-spread else (mid if x<cx+spread else dark)
