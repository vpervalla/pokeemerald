# Renders a tileset pair's metatiles, or a map layout, to PNG (for previews), and loads
# tilesets for build_castle.py.
# Usage: python3 render.py <primary> <secondary|-> out.png  (e.g. kanto_general kanto_mega_castle)
import struct, sys, os
from PIL import Image, ImageDraw
import os
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..')+'/'
R=REPO+'data/tilesets/'
def load_pal(path):
    L=open(path).read().split('\n')[3:]
    return [tuple(int(x) for x in l.split()) for l in L if l.strip()][:16]
def load_tiles(d):
    im=Image.open(d+'tiles.png'); assert im.mode=='P', im.mode
    w,h=im.size; px=im.load(); out=[]
    for ty in range(h//8):
        for tx in range(w//8):
            out.append([[px[tx*8+x,ty*8+y]&15 for x in range(8)] for y in range(8)])
    return out
class TS:
    def __init__(s, prim, sec, layout=(640,640,7)):
        s.ntp,s.nmp,s.npp=layout
        P=R+'primary/'+prim+'/'; S=R+'secondary/'+sec+'/' if sec else None
        s.tiles=load_tiles(P)[:s.ntp]
        s.tiles += [[[0]*8 for _ in range(8)]]*(s.ntp-len(s.tiles))
        s.pals=[load_pal(P+'palettes/%02d.pal'%i) for i in range(16)]
        s.meta=list(struct.iter_unpack('<8H',open(P+'metatiles.bin','rb').read()))
        s.meta += [(0,)*8]*(s.nmp-len(s.meta))
        if S:
            s.tiles+=load_tiles(S)
            sp=[load_pal(S+'palettes/%02d.pal'%i) for i in range(16)]
            for i in range(s.npp,13): s.pals[i]=sp[i]
            s.meta+=list(struct.iter_unpack('<8H',open(S+'metatiles.bin','rb').read()))
    def tile(s,img,ent,x0,y0,transparent):
        t=ent&0x3ff; hf=ent>>10&1; vf=ent>>11&1; p=ent>>12
        if t>=len(s.tiles): return
        T=s.tiles[t]; pal=s.pals[p]
        for y in range(8):
            for x in range(8):
                c=T[7-y if vf else y][7-x if hf else x]
                if c==0 and transparent: continue
                img.putpixel((x0+x,y0+y),pal[c])
    def metatile(s,img,m,x0,y0):
        if m>=len(s.meta): return
        e=s.meta[m]
        for layer in range(2):
            for i in range(4):
                s.tile(img,e[layer*4+i],x0+(i&1)*8,y0+(i>>1)*8,layer==1 or False)
def sheet(ts,start,end,cols,out,scale=2):
    n=end-start; rows=(n+cols-1)//cols
    img=Image.new('RGB',(cols*16,rows*16),(255,0,255))
    for i in range(n): ts.metatile(img,start+i,(i%cols)*16,(i//cols)*16)
    img=img.resize((img.width*scale,img.height*scale),Image.NEAREST)
    d=ImageDraw.Draw(img)
    for i in range(0,n,cols): d.text((0,(i//cols)*16*scale),hex(start+i),fill=(255,255,0))
    img.save(out)
if __name__=='__main__':
    prim,sec,out=sys.argv[1],sys.argv[2],sys.argv[3]
    lay=(640,640,7) if prim.startswith('kanto') else (512,512,6)
    ts=TS(prim,sec if sec!='-' else None,lay)
    start=lay[1] if sec!='-' else 0
    sheet(ts,start,len(ts.meta),16,out)

def render_layout(name, out, scale=1, marks=None):
    import json
    L=[l for l in json.load(open(REPO+'data/layouts/layouts.json'))['layouts'] if l.get('name')==name+'_Layout' or l.get('id')==name][0]
    sym=lambda s: __import__('re').sub(r'(?<=[a-z])(?=[0-9])','_',__import__('re').sub(r'(?<!^)(?=[A-Z])','_',s.replace('gTileset_',''))).lower()
    prim=sym(L['primary_tileset']); sec=sym(L['secondary_tileset'])
    ts=TS(prim,sec)
    w,h=L['width'],L['height']
    data=list(struct.iter_unpack('<H',open(REPO+L['blockdata_filepath'],'rb').read()))
    img=Image.new('RGB',(w*16,h*16))
    for i,(b,) in enumerate(data):
        ts.metatile(img,b&0x3ff,(i%w)*16,(i//w)*16)
    if scale!=1: img=img.resize((img.width*scale,img.height*scale),Image.NEAREST)
    img.save(out); return L
