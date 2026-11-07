#!/usr/bin/env python3
"""Import Ethan's sprites from a sheet of HGSS-style sprites (see docs/costume_credits.md).

    python3 tools/costumes/import_ethan.py tools/costumes/sheets/ethan.jpg

Run from the pokeemerald root. SHEET is the 594x290 sheet: walking and running (row 1), the bike
(row 2), surfing (row 3, last three), the field move (row 4), the trainer front pic and the five
back pic frames. Its sprites are bigger than FRLG's (up to 19x25): they are scaled down to the other
costumes' height (shrink below). Ethan's overworld frames stay 32x32. The sheet is a JPEG: its white background is taken out, and its colours are brought down to
one 15 colour overworld palette (PAL below), one for the front pic and one for the back pic.

It writes Ethan's overworld sheets (FRLG's layout), his palettes, his trainer front and back pics,
and makes the VS SEEKER frames of his item sheet (9-11) from his field move, with FRLG's VS SEEKER. His fishing frames
aren't on the sheet: the old ones are kept, in the new palette.
"""
import sys
from collections import Counter, deque
import numpy as np
from PIL import Image

PEOPLE = "graphics/object_events/pics/people/ethan"
TRANSPARENT = (115, 197, 164)
# The overworld palette, picked by hand from the sheet's colours (the JPEG tints its greys)
PAL = [(16, 16, 24), (48, 52, 56), (72, 80, 88), (100, 104, 116), (120, 64, 64), (192, 56, 64), (232, 104, 88),
       (176, 112, 32), (248, 176, 32), (244, 212, 192), (208, 160, 128), (172, 168, 188), (240, 240, 248), (24, 120, 232),
       (115, 164, 197)]   # the last: the VS SEEKER's lit screen (FRLG's)
ROW_TOP = {1: 6, 2: 40, 3: 71, 4: 106}   # the sheet's overworld rows; frames are 32 pixels apart from x = 6
BACK_CELLS = [231, 302, 374, 448, 518]   # left edges of the back pic frames (y 135-198)
FRONT_CELL = (388, 47)

# FRLG's VS SEEKER (tools/costumes/vs_seeker_frames.py). Its frames are shown with their own
# palette (player_ethan_vs_seeker.pal, OBJ_EVENT_PAL_TAG_PLAYER_ETHAN_VS_SEEKER): Ethan's, with the
# VS SEEKER's dark blue in place of his blue (slot 14) and its light blue in slot 15. Its pale top is
# white, its lavender sides light grey.
DEVICE_COLOURS = {"6": 14, "9": 13, "a": 12, "7": 15, "5": 14}
VS_SEEKER_NAVY = (57, 57, 123)


def cut(im, x0, y0, w, h):
    """The cell's pixels and the mask of the sprite (the background is white)."""
    a = im[y0:y0 + h, x0:x0 + w]
    light = a.min(axis=2) >= 200
    bg = np.zeros(light.shape, bool)
    q = deque()
    for y in range(h):
        for x in range(w):
            if (y in (0, h - 1) or x in (0, w - 1)) and light[y, x]:
                bg[y, x] = True
                q.append((y, x))
    while q:
        y, x = q.popleft()
        for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
            if 0 <= ny < h and 0 <= nx < w and light[ny, nx] and not bg[ny, nx]:
                bg[ny, nx] = True
                q.append((ny, nx))
    # the JPEG's pale grey halo around the outline (but not the skin)
    halo = [(y, x) for y in range(h) for x in range(w)
            if not bg[y, x] and a[y, x].min() >= 165 and a[y, x].max() - a[y, x].min() < 40
            and any(0 <= y + dy < h and 0 <= x + dx < w and bg[y + dy, x + dx] for dy, dx in ((1, 0), (-1, 0), (0, 1), (0, -1)))]
    for p in halo:
        bg[p] = True
    return a, ~bg


def index(a, mask, pal):
    pa = np.array(pal)
    idx = ((a[:, :, None, :] - pa[None, None, :, :]) ** 2).sum(axis=3).argmin(axis=2) + 1
    idx[~mask] = 0
    return idx


def quantize(cells, n=15):
    pix = np.concatenate([a[m] for a, m in cells]).astype(np.uint8)
    q = Image.fromarray(pix.reshape(1, -1, 3)).quantize(colors=n, method=Image.Quantize.MEDIANCUT, kmeans=10)
    p = q.getpalette()[:3 * n]
    return sorted({tuple(p[i * 3:i * 3 + 3]) for i in range(n)}, key=sum)


def save(idx, pal, path):
    img = Image.fromarray(idx.astype(np.uint8), "P")
    flat = []
    for c in [TRANSPARENT] + list(pal) + [(0, 0, 0)] * (255 - len(pal)):
        flat += c
    img.putpalette(flat)
    img.save(path)
    print(f"{path}: {idx.shape[1]}x{idx.shape[0]}")


def write_pal(path, colours):
    with open(path, "w", newline="\r\n") as f:
        f.write(f"JASC-PAL\n0100\n{len(colours)}\n" + "".join(f"{r} {g} {b}\n" for r, g, b in colours))


def vs_frame(idx, lit):
    """A field move frame (scaled) with FRLG's VS SEEKER in place of the POKé BALL held up.

    The POKé BALL is right of the raised hand: the VS SEEKER takes its place, its bottom on the
    POKé BALL's, so the hand (left of it) holds it as Red does. Only the POKé BALL's rows are
    cleared, and never the hand's skin."""
    from vs_seeker_frames import DEVICE_OFF, DEVICE_ON
    out = idx.copy()
    cols, rows = np.arange(32)[None, :], np.arange(32)[:, None]
    ys, xs = np.where(np.isin(out, (5, 6, 7, 13)) & (cols >= 23) & (rows <= 22))
    y0, y1, x0 = ys.min(), ys.max(), xs.min()
    for y in range(y0, y1 + 1):
        for x in range(x0, 32):
            if out[y, x] not in (10, 11):
                out[y, x] = 0
    top = y1 - 5
    for dy, row in enumerate(DEVICE_ON if lit else DEVICE_OFF):
        for dx, ch in enumerate(row):
            if ch != ".":
                out[top + dy, x0 + dx] = DEVICE_COLOURS[ch]
    return out


# The sheet's sprites are a fifth taller than the other player costumes' (23 pixels against 19):
# they are brought down to the same height. Each pixel takes the colour that covers most of its
# area (the outline wins with a third of it), so the palette is kept and nothing is blurred.
SCALE = 19 / 23
def shrink(idx, out_w=32, outline_share=0.34):
    """Majority-vote downscale of a paletted 32x32 frame to SCALE, feet kept on the same row."""
    ys,xs=np.where(idx>0); y0,y1,x0,x1=ys.min(),ys.max()+1,xs.min(),xs.max()+1
    h,w=y1-y0,x1-x0
    nh,nw=max(1,round(h*SCALE)),max(1,round(w*SCALE))
    small=np.zeros((nh,nw),int)
    for ty in range(nh):
        sy0=y0+ty*h/nh; sy1=y0+(ty+1)*h/nh
        for tx in range(nw):
            sx0=x0+tx*w/nw; sx1=x0+(tx+1)*w/nw
            c=Counter()
            for sy in range(int(sy0),int(np.ceil(sy1))):
                wy=min(sy+1,sy1)-max(sy,sy0)
                for sx in range(int(sx0),int(np.ceil(sx1))):
                    wx=min(sx+1,sx1)-max(sx,sx0)
                    c[idx[sy,sx]]+=wy*wx
            tot=sum(c.values())
            if c[0]>tot*0.5: v=0
            elif c[1]>=tot*outline_share: v=1
            else:
                c.pop(0,None); v=c.most_common(1)[0][0]
            small[ty,tx]=v
    out=np.zeros((32,out_w),int)
    feet=y1-1
    oy=feet-nh+1; ox=(out_w-nw)//2 + round(((x0+x1)/2-16)*SCALE) if out_w==32 else (out_w-nw)//2
    ox=max(0,min(out_w-nw,ox))
    out[oy:oy+nh,ox:ox+nw]=small
    return out


def main():
    im = np.array(Image.open(sys.argv[1]).convert("RGB")).astype(int)
    sprite_pal = PAL[:-1]   # the sheet's pixels never take the VS SEEKER's light blue
    frame = lambda r, k: index(*cut(im, 6 + 32 * k, ROW_TOP[r], 32, 32), sprite_pal)
    small = lambda frames: [shrink(f) for f in frames]
    cat = lambda frames: np.concatenate(frames, axis=1)

    normal = [frame(1, k) for k in range(9)]
    # the running frames are by direction (down, step, step, up...): FRLG's order is
    # down, up, side, then each direction's two steps
    run = [frame(1, 9 + i) for i in (0, 3, 6, 1, 2, 4, 5, 7, 8)]
    surf = [frame(3, k) for k in (9, 10, 11)]
    bike = [frame(2, k) for k in range(9)]
    item = [frame(4, k) for k in range(5)]
    normal, run, surf, bike, item = map(small, (normal, run, surf, bike, item))
    vs = [vs_frame(item[3], False), vs_frame(item[4], False), vs_frame(item[4], True)]
    save(cat(normal), PAL, f"{PEOPLE}/normal.png")
    save(cat(surf + run + surf[:2]), PAL, f"{PEOPLE}/surf_run.png")   # surf 0-2, run 3-11
    save(cat(bike), PAL, f"{PEOPLE}/bike.png")
    save(cat(item + [item[0]] * 4 + vs), PAL, f"{PEOPLE}/item.png")   # field move 0-4, VS SEEKER 9-11

    # The fishing frames stay, in the new palette
    fish = Image.open(f"{PEOPLE}/fish.png")
    old = fish.getpalette()
    old = [tuple(old[i * 3:i * 3 + 3]) for i in range(16)]
    fa = np.array(fish)
    remap = [0] + [1 + int(np.argmin([sum((a - b) ** 2 for a, b in zip(old[i], c)) for c in sprite_pal])) for i in range(1, 16)]
    save(np.vectorize(lambda v: remap[v])(fa), PAL, f"{PEOPLE}/fish.png")

    write_pal("graphics/object_events/palettes/player_ethan.pal", [TRANSPARENT] + PAL)
    write_pal("graphics/object_events/palettes/player_ethan_vs_seeker.pal", [TRANSPARENT] + PAL[:13] + [VS_SEEKER_NAVY] + PAL[14:])
    # The reflection: the same colours, paler and bluer (as the old one)
    refl = [(int(r * 0.6 + 80), int(g * 0.6 + 92), int(b * 0.6 + 102)) for r, g, b in [TRANSPARENT] + PAL]
    write_pal("graphics/object_events/palettes/player_ethan_reflection.pal", refl)

    backs = [cut(im, x, 135, 64, 64) for x in BACK_CELLS]
    bpal = quantize(backs)
    save(np.concatenate([index(a, m, bpal) for a, m in backs], axis=0), bpal, "graphics/trainers/back_pics/ethan.png")
    front = cut(im, FRONT_CELL[0], FRONT_CELL[1], 64, 64)
    fpal = quantize([front])
    save(index(*front, fpal), fpal, "graphics/trainers/front_pics/ethan.png")


if __name__ == "__main__":
    main()
