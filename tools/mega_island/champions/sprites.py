# The champions' sprites, from the images in source/ (at their native pixel size, backgrounds
# removed): each trainer pic is scaled to fit 64x64, and the overworld frames are cut from the
# walking sheets (4 rows of 4 frames: down, left, right, up; columns: standing, step, standing,
# other step). Every sprite gets its own 15-colour palette; on the maps the champions use the
# special NPC palette slot (one champion per map). The DS games' walking sprites are much bigger than
# Gen 3's, so their figures are scaled down to the height of a Kanto NPC (about 21 px with the outline
# drawn round them) in their 32x32 frames, feet on the same row. Run after changing the sources.
import os
from PIL import Image
HERE=os.path.dirname(os.path.abspath(__file__))
REPO=os.path.join(HERE,'..','..','..')+'/'
BG=(112,200,160)
# name: (overworld cell w, h, scale, crop box in the scaled cell, colours to drop, frame w, h,
#        figure height to shrink to, or None)
OW={
 'cynthia':(34,36,1,(1,3,33,35),[(57,41,57)],32,32,19),   # drop the round shadow of the DS games
 'diantha':(32,32,1,(0,0,32,32),[],32,32,19),
 'alder':  (16,24,1,(0,-8,16,24),[],16,32,None),           # already Gen 3 sized
 'leon':   (64,64,2,(0,0,32,32),[],32,32,19),
 'alain':  (34,36,1,(1,3,33,35),[],32,32,19),
}
OUTLINE=(40,36,44)
def shrink(frames,h):
    # Scales every frame by the same factor so the tallest figure is h px tall, keeps each centred
    # with the feet on the bottom row, and draws a dark outline round the figure (Gen 3 style).
    boxes=[f.getbbox() for f in frames]
    k=h/max(b[3]-b[1] for b in boxes)
    bottom=max(b[3] for b in boxes)
    out=[]
    for f in frames:
        w,hh=f.size
        sm=f.resize((round(w*k),round(hh*k)),Image.BOX)
        sm.putalpha(sm.getchannel('A').point(lambda v:255 if v>=110 else 0))
        o=Image.new('RGBA',f.size,(0,0,0,0))
        o.paste(sm,(round(w/2*(1-k)),hh-2-round(bottom*k)),sm)
        px=o.load(); add=[]
        for y in range(hh):
            for x in range(w):
                if px[x,y][3]==0 and any(0<=x+dx<w and 0<=y+dy<hh and px[x+dx,y+dy][3]
                                         for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))):
                    add.append((x,y))
        for x,y in add: px[x,y]=OUTLINE+(255,)
        out.append(o)
    dy=31-out[0].getbbox()[3]          # the standing frame's feet on row 30, like Kanto NPCs
    moved=[]
    for o in out:
        m=Image.new('RGBA',o.size,(0,0,0,0)); m.paste(o,(0,dy)); moved.append(m)
    return moved
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
    cw,ch,sc,box,drop,fw,fh,fig=OW[name]
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
    if fig: frames=shrink(frames,fig)
    pal=reduce(frames)
    strip=Image.new('RGBA',(fw*len(frames),fh),(0,0,0,0))
    for i,f in enumerate(frames): strip.paste(f,(i*fw,0))
    index(strip,pal).save(REPO+f'graphics/object_events/pics/kanto/people/champion_{name}.png')
    write_pal(REPO+f'graphics/object_events/palettes/kanto/champion_{name}.pal',pal)
if __name__=='__main__':
    for n in ('cynthia','alder','diantha','leon','alain'):
        front(n); overworld(n); print('wrote',n)
