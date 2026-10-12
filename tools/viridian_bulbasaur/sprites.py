# Builds the overworld sprites of the lost BULBASAUR and the BEEDRILL in Viridian Forest.
# The sources in source/ are pokeemerald-expansion's follower sprites (six 32x32 frames:
# down, down step, up, up step, left, left step). Both sprites share one 15-colour palette
# (their colours merge into exactly 15), so they can be on screen together in the special
# NPC palette slot. Run from anywhere: python3 tools/viridian_bulbasaur/sprites.py
import os
from PIL import Image
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, '..', '..')
def load_pal(path):
    lines = open(path).read().split('\n')[3:]
    return [tuple(int(v) for v in l.split()) for l in lines if l.strip()]
# Close colours that become one entry of the shared palette
MERGE = {
    (192, 192, 192): (200, 200, 200),
    (80, 80, 80): (64, 64, 64),
    (192, 40, 64): (184, 48, 24),
}
TRANSPARENT = (115, 197, 164)
shared = [TRANSPARENT]
sprites = {}
for mon in ('bulbasaur', 'beedrill'):
    im = Image.open(os.path.join(HERE, 'source', mon + '.png'))
    pal = load_pal(os.path.join(HERE, 'source', mon + '.pal'))
    w, h = im.size
    px = list(im.getdata()) if not hasattr(im, 'get_flattened_data') else list(im.get_flattened_data())
    out = []
    for i in px:
        if i == 0:
            out.append(0)
            continue
        c = MERGE.get(pal[i], pal[i])
        if c not in shared:
            shared.append(c)
        out.append(shared.index(c))
    sprites[mon] = (w, h, out)
assert len(shared) <= 16, len(shared)
shared += [(0, 0, 0)] * (16 - len(shared))
flat = [v for c in shared for v in c]
for mon, (w, h, data) in sprites.items():
    im = Image.new('P', (w, h))
    im.putpalette(flat)
    im.putdata(data)
    im.save(os.path.join(REPO, 'graphics/object_events/pics/kanto/pokemon', mon + '.png'))
with open(os.path.join(REPO, 'graphics/object_events/palettes/kanto/viridian_bulbasaur.pal'), 'w', newline='\r\n') as f:
    f.write('JASC-PAL\n0100\n16\n' + ''.join('%d %d %d\n' % c for c in shared))
print(len([c for c in shared]), 'colours')
