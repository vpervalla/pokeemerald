# Writes the champions' trainer front pics and palettes into graphics/trainers/front_pics and
# graphics/trainers/palettes. Run after changing any of the drawings.
import os, importlib
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','..','..')+'/'
for name in ('cynthia','alder','diantha','leon'):
    m=importlib.import_module(name)
    p=m.draw()
    p.save_indexed(REPO+f'graphics/trainers/front_pics/champion_{name}.png')
    with open(REPO+f'graphics/trainers/palettes/champion_{name}.pal','w') as f:
        f.write('JASC-PAL\r\n0100\r\n16\r\n')
        pal=list(m.PAL)
        pal[0]=(112,200,160)          # transparent colour, as in the other trainer palettes
        for c in pal[:16]: f.write('%d %d %d\r\n'%c)
    print('wrote',name)
