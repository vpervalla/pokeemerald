# Shared tileset building for the island maps: cuts indexed art into 8x8 tiles (merging flipped
# duplicates and giving each tile the palette that holds its colours), builds metatiles, and
# writes a secondary tileset and a layout.
import os, struct
from PIL import Image
from art import PAL, PAL_A, PAL_B, PAL_C

EMPTY_TOP=0   # primary tile 0 is blank

class Builder:
    def __init__(self):
        self.tiles=[]; self.tindex={}
        self.metatiles=[]; self.mindex={}; self.attrs=[]
        # (slot, colour -> index) for each palette, tried in order
        self.banks=[(slot,{rgb:i for i,rgb in enumerate(pal) if i}) for slot,pal in ((7,PAL_A),(8,PAL_B),(9,PAL_C))]
        self.add_tile([[0]*8]*8)   # secondary tile 0 = transparent

    def to_bank(self,px):
        # Picks the palette (slot 7, 8 or 9) that has every colour of the tile; returns (pal, local pixels).
        rgbs={PAL[p] for r in px for p in r if p}
        for pal,idx in self.banks:
            if rgbs<=set(idx):
                return pal,[[idx[PAL[p]] if p else 0 for p in r] for r in px]
        raise SystemExit('tile mixes colours of different palettes: %s' % sorted(rgbs))

    def add_tile(self,px):   # px: 8 rows of 8 art colour indices; returns (index, hflip, vflip, palette)
        pal,px=self.to_bank(px)
        rows=[tuple(r) for r in px]
        cands=[(tuple(rows),0,0),(tuple(r[::-1] for r in rows),1,0),(tuple(rows[::-1]),0,1),(tuple(r[::-1] for r in rows[::-1]),1,1)]
        for k,h,v in cands:
            if k in self.tindex: return self.tindex[k],h,v,pal
        self.tindex[cands[0][0]]=len(self.tiles); self.tiles.append(cands[0][0]); return len(self.tiles)-1,0,0,pal

    @staticmethod
    def tile_entry(i,h,v,pal): return (640+i)|(h<<10)|(v<<11)|(pal<<12)

    def cell_tiles(self,canvas,x0,y0):
        # The four 8x8 tiles of a 16x16 cell: (index, hflip, vflip, pal), or None where empty.
        out=[]
        for ty in (0,8):
            for tx in (0,8):
                px=[[canvas.get(x0+tx+x,y0+ty+y) for x in range(8)] for y in range(8)]
                if all(p==0 for r in px for p in r): out.append(None)
                else: out.append(self.add_tile(px))
        return out

    def entries(self,canvas,x0,y0):
        return [self.tile_entry(*e) if e else EMPTY_TOP for e in self.cell_tiles(canvas,x0,y0)]

    def metatile(self,bottom,top,attr):
        key=(tuple(bottom),tuple(top),attr)
        if key in self.mindex: return self.mindex[key]
        self.mindex[key]=640+len(self.metatiles); self.metatiles.append(key); self.attrs.append(attr)
        return self.mindex[key]

    def check(self):
        print('secondary tiles',len(self.tiles),'metatiles',len(self.metatiles))
        assert len(self.tiles)<=384 and len(self.metatiles)<=384

    def write_tileset(self,tsdir):
        os.makedirs(tsdir+'palettes',exist_ok=True)
        n=len(self.tiles); rows_=(n+15)//16
        im=Image.new('P',(128,rows_*8),0)
        im.putpalette([v for col in PAL_A for v in col])
        for i,t in enumerate(self.tiles):
            for y in range(8):
                for x in range(8): im.putpixel(((i%16)*8+x,(i//16)*8+y),t[y][x])
        im.save(tsdir+'tiles.png')
        for p in range(16):
            cols={7:PAL_A,8:PAL_B,9:PAL_C}.get(p,[(0,0,0)]*16)
            with open(tsdir+'palettes/%02d.pal'%p,'w',newline='\r\n') as f:
                f.write('JASC-PAL\n0100\n16\n'+''.join('%d %d %d\n'%c for c in cols))
        with open(tsdir+'metatiles.bin','wb') as f:
            for b,t,a in self.metatiles: f.write(struct.pack('<8H',*b,*t))
        with open(tsdir+'metatile_attributes.bin','wb') as f:
            for a in self.attrs: f.write(struct.pack('<H',a))

def write_layout(ld,grid,coll,border,elevation=3):
    os.makedirs(ld,exist_ok=True)
    with open(ld+'map.bin','wb') as f:
        for y in range(len(grid)):
            for x in range(len(grid[0])): f.write(struct.pack('<H',grid[y][x]|(coll[y][x]<<10)|(elevation<<12)))
    with open(ld+'border.bin','wb') as f:
        f.write(struct.pack('<4H',*border))
