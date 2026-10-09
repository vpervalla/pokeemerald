# The champions' sprites, from the images in source/ (at their native pixel size, backgrounds
# removed): each trainer pic is scaled to fit 64x64, and the overworld frames are cut from the
# walking sheets (4 rows of 4 frames: down, left, right, up; columns: standing, step, standing,
# other step). Every sprite gets its own 15-colour palette; on the maps the champions use the
# special NPC palette slot (one champion per map). Run after changing the sources.
import os
from PIL import Image
HERE=os.path.dirname(os.path.abspath(__file__))
REPO=os.path.join(HERE,'..','..','..')+'/'
BG=(112,200,160)
# name: (overworld cell w, h, scale, crop box in the scaled cell, colours to drop, frame w, h)
OW={
 'cynthia':(34,36,1,(1,3,33,35),[(57,41,57)],32,32),   # drop the round shadow of the DS games
 'diantha':(32,32,1,(0,0,32,32),[],32,32),
 'alder':  (16,24,1,(0,-8,16,24),[],16,32),
 'leon':   (64,64,2,(0,0,32,32),[],32,32),
}
def d(a,b): return sum((a[i]-b[i])**2 for i in range(3))
def reduce(imgs,n=15):
    # Merge the closest colours (weighted by use) until n are left; black stays.
    cnt={}
    for im in imgs:
        px=im.load()
        for y in range(im.height):
            for x in range(im.width):
                p=px[x,y]
                if p[3]>=128: cnt[p[:3]]=cnt.get(p[:3],0)+1
    if len(cnt)>40:      # noisy sources (a JPEG): pre-cluster with a median cut first
        flat=Image.new('RGB',(len(cnt),1))
        for i,c in enumerate(cnt): flat.putpixel((i,0),c)
        q=flat.quantize(32,method=Image.Quantize.MEDIANCUT).convert('RGB')
        merged={}
        for i,c in enumerate(cnt):
            k=q.getpixel((i,0)); merged[k]=merged.get(k,0)+cnt[c]
        cnt=merged
    pal=sorted(cnt,key=lambda c:-cnt[c]); w=dict(cnt)
    while len(pal)>n:
        _,i,j=min((d(pal[i],pal[j]),i,j) for i in range(len(pal)) for j in range(i+1,len(pal)))
        a,b=pal[i],pal[j]
        keep,drop=(a,b) if w[a]>=w[b] else (b,a)
        if drop==(0,0,0): keep,drop=drop,keep
        w[keep]+=w[drop]; pal.remove(drop)
    return pal
def index(im,pal):
    out=Image.new('P',im.size); flat=[v for c in [BG]+pal for v in c]
    out.putpalette(flat+[0]*(768-len(flat)))
    px=im.load()
    for y in range(im.height):
        for x in range(im.width):
            p=px[x,y]
            if p[3]>=128: out.putpixel((x,y),1+min(range(len(pal)),key=lambda k:d(pal[k],p[:3])))
    return out
def write_pal(path,pal):
    with open(path,'w') as f:
        f.write('JASC-PAL\r\n0100\r\n16\r\n')
        for c in ([BG]+pal+[(0,0,0)]*16)[:16]: f.write('%d %d %d\r\n'%c)
def front(name):
    im=Image.open(f'{HERE}/source/{name}_front.png').convert('RGBA')
    im=im.crop(im.getbbox())
    k=min(1,64/im.width,64/im.height)
    if k<1: im=im.resize((round(im.width*k),round(im.height*k)),Image.BOX)
    pic=Image.new('RGBA',(64,64),(0,0,0,0))
    pic.paste(im,((64-im.width)//2,64-im.height),im)      # centred, standing on the bottom
    pal=reduce([pic])
    index(pic,pal).save(REPO+f'graphics/trainers/front_pics/champion_{name}.png')
    write_pal(REPO+f'graphics/trainers/palettes/champion_{name}.pal',pal)
def overworld(name):
    cw,ch,sc,box,drop,fw,fh=OW[name]
    sheet=Image.open(f'{HERE}/source/{name}_overworld.png').convert('RGBA')
    px=sheet.load()
    for y in range(sheet.height):
        for x in range(sheet.width):
            if px[x,y][:3] in drop: px[x,y]=(0,0,0,0)
    def cell(r,c):
        im=sheet.crop((c*cw,r*ch,c*cw+cw,r*ch+ch))
        if sc>1: im=im.resize((cw//sc,ch//sc),Image.BOX)
        return im.crop(box)
    frames=[cell(0,0),cell(3,0),cell(1,0),cell(0,1),cell(0,3),cell(3,1),cell(3,3),cell(1,1),cell(1,3)]
    pal=reduce(frames)
    strip=Image.new('RGBA',(fw*len(frames),fh),(0,0,0,0))
    for i,f in enumerate(frames): strip.paste(f,(i*fw,0))
    index(strip,pal).save(REPO+f'graphics/object_events/pics/kanto/people/champion_{name}.png')
    write_pal(REPO+f'graphics/object_events/palettes/kanto/champion_{name}.pal',pal)
if __name__=='__main__':
    for n in ('cynthia','alder','diantha','leon'):
        front(n); overworld(n); print('wrote',n)
