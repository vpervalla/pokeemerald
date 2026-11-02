#!/usr/bin/env python3
"""Makes the title screen's background (BG0): a picture of MEWTWO in RAYQUAZA's arch.

    python3 tools/title_screen/make_background.py ART.png [--preview OUT.png]

ART is the source painting (MEWTWO floating over purple mountains, 1080x1298); it isn't in the
repository. The script:
  - erases its floating stones and cuts MEWTWO out, filling the sky behind them (OpenCV inpainting),
  - crops it to 240x160 and pastes MEWTWO back smaller, left of centre,
  - draws Emerald's own title screen RAYQUAZA (graphics/title_screen/rayquaza.png/.bin) behind it,
    1.35x, dark violet with glowing rings, its arch around MEWTWO,
  - redraws MEWTWO's pink aura, then MEWTWO,
  - converts the result to 4bpp tiles with 3 palettes of 15 colours (BG palette rows 5, 14 and 15,
    the ones the POKéMON logo leaves free; see src/title_screen.c), and writes
        graphics/title_screen/ancient_bg.png   the tiles (indices only; their colours are palette 0's)
        graphics/title_screen/ancient_bg.bin   the 32x32 tilemap
        graphics/title_screen/ancient_bg.pal   the 3 palettes, 48 colours

Needs numpy, Pillow and opencv-python-headless.
"""
import argparse, os, struct
import numpy as np, cv2
from PIL import Image, ImageFilter

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
GFX = os.path.join(ROOT, 'graphics', 'title_screen')
PAL_ROWS = (5, 14, 15)

# The source painting, worked on at half size (540x649)
HW, HH = 540, 649
TOP, CH = 39.5, 360          # the 3:2 crop that becomes the 240x160 screen
STONES = [(372,132,18,30),(216,142,6,8),(176,183,18,24),(59,191,45,64),(421,212,56,81),(162,382,15,20),
          (370,358,17,22),(288,431,29,41),(371,295,5,5),(382,349,6,5),(168,318,5,6)]   # x, y, w, h
MEWTWO_BOX = (205,195,335,425)   # x0, y0, x1, y1, its orb included
ORB = (298,212,14,14)

# The composition
MEWTWO_SCALE, MEWTWO_X, MEWTWO_TOP = 0.8, 100, 82
RAY_SCALE, RAY_ALPHA, RING_ALPHA = 1.35, 0.7, 0.85
RAY_BODY, RAY_RING, RAY_EYES = (28,10,50), (130,215,245), (255,240,200)
AURA, AURA_COLOR = 1.0, (225,140,235)
FADE_FROM, FADE_TO = 145, 175    # RAYQUAZA fades into the mountains' mist towards the bottom
HOLLOW = (92, 113)               # the centre of the hollow of RAYQUAZA's arch in rayquaza.bin's layout

def soft(mask, blur):
    im = Image.fromarray((np.clip(mask, 0, 1) * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(blur))
    return np.asarray(im).astype(float) / 255

def rayquaza_layer():
    """Emerald's title RAYQUAZA as palette indices: 11 its body, 15 its rings, 2 its eyes."""
    ts = np.asarray(Image.open(os.path.join(GFX, 'rayquaza.png'))); tw = ts.shape[1] // 8
    m = open(os.path.join(GFX, 'rayquaza.bin'), 'rb').read()
    idx = np.zeros((256, 256), np.uint8)
    for i in range(1024):
        e = struct.unpack_from('<H', m, 2 * i)[0]; t = e & 0x3ff
        tile = ts[(t // tw) * 8:(t // tw) * 8 + 8, (t % tw) * 8:(t % tw) * 8 + 8]
        if e >> 10 & 1: tile = tile[:, ::-1]
        if e >> 11 & 1: tile = tile[::-1]
        idx[(i // 32) * 8:(i // 32) * 8 + 8, (i % 32) * 8:(i % 32) * 8 + 8] = tile
    return idx[:170, :256]

def ray_layers(idx, s, dx, dy, smooth=1.1):
    out = {}
    for name, vals, sm in (('body', (11, 15, 2), smooth), ('ring', (15,), smooth * 0.7), ('eyes', (2,), 0.4)):
        m = np.isin(idx, vals).astype(np.uint8) * 255
        big = Image.fromarray(m).resize((int(256 * s), int(170 * s)), Image.BICUBIC).filter(ImageFilter.GaussianBlur(sm))
        canvas = Image.new('L', (240, 160), 0); canvas.paste(big, (dx, dy))
        a = np.asarray(canvas).astype(float) / 255
        out[name] = np.clip((a - 0.5) * 2.5 + 0.5, 0, 1)
    return out

def compose(art_path):
    art = cv2.resize(np.asarray(Image.open(art_path).convert('RGB')), (HW, HH), interpolation=cv2.INTER_AREA)
    lum = cv2.cvtColor(art, cv2.COLOR_RGB2GRAY).astype(float)
    # The stones (with their glowing rims) and MEWTWO are erased
    stones = np.zeros(lum.shape, np.uint8)
    for x, y, w, h in STONES:
        r = int(0.3 * max(w, h)) + 3
        cv2.ellipse(stones, (x + w // 2, y + h // 2), (w // 2 + r, h // 2 + r), 0, 0, 360, 1, -1)
    x0, y0, x1, y1 = MEWTWO_BOX
    mm = np.zeros(lum.shape, np.uint8)
    mm[y0:y1, x0:x1] = (lum[y0:y1, x0:x1] < 105)
    x, y, w, h = ORB
    mm[y - 2:y + h + 2, x - 2:x + w + 2] |= (lum[y - 2:y + h + 2, x - 2:x + w + 2] < 110).astype(np.uint8)
    n, lab, st, _ = cv2.connectedComponentsWithStats(mm, 8)
    mm = np.isin(lab, [i for i in range(1, n) if st[i, 4] >= 20]).astype(np.uint8)
    hole = stones | cv2.dilate(mm, np.ones((3, 3), np.uint8), iterations=9)
    bg = cv2.inpaint(art, hole * 255, 9, cv2.INPAINT_TELEA)
    frame = cv2.resize(bg[int(TOP):int(TOP) + CH], (240, 160), interpolation=cv2.INTER_AREA).astype(float)

    # MEWTWO, smaller
    a = cv2.GaussianBlur(mm.astype(float), (0, 0), 0.8)
    ys, xs = np.where(mm[y0:y1, x0:x1])
    bx0, by0, bx1, by1 = xs.min() + x0, ys.min() + y0, xs.max() + x0 + 1, ys.max() + y0 + 1
    rgba = np.dstack([art[by0:by1, bx0:bx1], (a[by0:by1, bx0:bx1] * 255).clip(0, 255).astype(np.uint8)])
    f = 240 / HW
    mw = np.asarray(Image.fromarray(rgba).resize((int((bx1 - bx0) * f * MEWTWO_SCALE), int((by1 - by0) * f * MEWTWO_SCALE)),
                                                 Image.LANCZOS)).astype(float)
    px, py = int(MEWTWO_X - mw.shape[1] / 2), MEWTWO_TOP
    mrgb = np.zeros((160, 240, 3)); ma = np.zeros((160, 240))
    h_, w_ = mw.shape[:2]
    ys0, xs0, ys1, xs1 = max(0, py), max(0, px), min(160, py + h_), min(240, px + w_)
    mrgb[ys0:ys1, xs0:xs1] = mw[ys0 - py:ys1 - py, xs0 - px:xs1 - px, :3]
    ma[ys0:ys1, xs0:xs1] = mw[ys0 - py:ys1 - py, xs0 - px:xs1 - px, 3] / 255

    # RAYQUAZA, its arch's hollow on MEWTWO
    ys_, xs_ = np.where(ma > 0.5)
    cx, cy = (xs_.min() + xs_.max()) / 2, (ys_.min() + ys_.max()) / 2
    L = ray_layers(rayquaza_layer(), RAY_SCALE, int(round(cx - HOLLOW[0] * RAY_SCALE)), int(round(cy - HOLLOW[1] * RAY_SCALE)))
    rows = np.arange(160)[:, None] * np.ones((1, 240))
    vis = np.clip((FADE_TO - rows) / (FADE_TO - FADE_FROM), 0, 1)
    out = frame
    ba = L['body'] * RAY_ALPHA * vis
    out = out * (1 - ba[..., None]) + np.array(RAY_BODY) * ba[..., None]
    halo = soft(L['ring'], 1.6)
    out = out + np.array(RAY_RING) * (halo * 0.35 * vis)[..., None] * 0.6
    ra = L['ring'] * RING_ALPHA * vis
    out = out * (1 - ra[..., None]) + np.array(RAY_RING) * ra[..., None]
    e = L['eyes'] * vis
    out = out * (1 - e[..., None]) + np.array(RAY_EYES) * e[..., None]
    # MEWTWO's aura, then MEWTWO
    au = np.asarray(Image.fromarray((ma * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(5))
                    .filter(ImageFilter.GaussianBlur(4))).astype(float) / 255
    au = np.clip(au * 1.6, 0, 1) * AURA
    out = out * (1 - au[..., None]) + np.array(AURA_COLOR) * au[..., None]
    out = out * (1 - ma[..., None]) + mrgb * ma[..., None]
    return out.clip(0, 255)

def kmeans(px, k, iters=12, seed=0):
    rng = np.random.default_rng(seed)
    uniq = np.unique(px, axis=0)
    if len(uniq) <= k: return uniq.astype(float)
    c = uniq[rng.choice(len(uniq), k, replace=False)].astype(float)
    for _ in range(iters):
        l = ((px[:, None, :] - c[None]) ** 2).sum(2).argmin(1)
        for j in range(k):
            s = px[l == j]
            if len(s): c[j] = s.mean(0)
    return np.round(c)

def to_tiles(rgb, npal=3, ncol=15, rounds=6):
    """4bpp tiles: each 8x8 tile picks the best of npal palettes of ncol colours (GBA 15-bit colours)."""
    a = np.round(rgb / 255 * 31).clip(0, 31)
    tiles = a.reshape(20, 8, 30, 8, 3).transpose(0, 2, 1, 3, 4).reshape(-1, 64, 3)
    means = tiles.mean(1)
    centres = kmeans(means, npal, seed=1)
    assign = ((means[:, None] - centres[None]) ** 2).sum(2).argmin(1)
    for r in range(rounds):
        pals = [kmeans(tiles[assign == p].reshape(-1, 3), ncol, seed=r) for p in range(npal)]
        pals = [np.vstack([p, np.repeat(p[-1:], ncol - len(p), 0)]) if len(p) < ncol else p for p in pals]
        err = np.stack([((tiles[:, :, None, :] - p[None, None]) ** 2).sum(3).min(2).sum(1) for p in pals], 1)
        assign = err.argmin(1)
    idx = np.zeros((len(tiles), 64), np.uint8)
    for i, t in enumerate(tiles):
        idx[i] = ((t[:, None] - pals[assign[i]][None]) ** 2).sum(2).argmin(1) + 1   # colour 0 is transparent
    return idx.reshape(-1, 8, 8), assign, pals

def write(idx, assign, pals):
    # unique tiles, flips included
    uniq, keys, entries = [], {}, []
    for t, p in zip(idx, assign):
        for hf, vf in ((0, 0), (1, 0), (0, 1), (1, 1)):
            v = t[:, ::-1] if hf else t
            v = v[::-1] if vf else v
            k = v.tobytes()
            if k in keys:
                entries.append(keys[k] | hf << 10 | vf << 11 | PAL_ROWS[p] << 12); break
        else:
            keys[t.tobytes()] = len(uniq); uniq.append(t)
            entries.append((len(uniq) - 1) | PAL_ROWS[p] << 12)
    assert len(uniq) <= 640, len(uniq)   # charblock 2 up to the tilemaps at screen base 26
    # tiles as a 16-colour PNG, 16 tiles a row
    rows = (len(uniq) + 15) // 16
    sheet = np.zeros((rows * 8, 128), np.uint8)
    for i, t in enumerate(uniq):
        sheet[(i // 16) * 8:(i // 16) * 8 + 8, (i % 16) * 8:(i % 16) * 8 + 8] = t
    im = Image.fromarray(sheet, 'P')
    p0 = [[0, 0, 0]] + [[int(c * 255 / 31) for c in col] for col in pals[0]]
    im.putpalette([v for c in p0 for v in c] + [0] * (768 - 48))
    im.save(os.path.join(GFX, 'ancient_bg.png'), bits=4)
    # 32x32 tilemap: the screen's 30x20, the rest tile 0
    tm = np.zeros((32, 32), np.uint16)
    tm[:20, :30] = np.array(entries, np.uint16).reshape(20, 30)
    open(os.path.join(GFX, 'ancient_bg.bin'), 'wb').write(tm.astype('<u2').tobytes())
    with open(os.path.join(GFX, 'ancient_bg.pal'), 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n48\n')
        for p in pals:
            f.write('0 0 0\n')
            for c in p[:15]: f.write('%d %d %d\n' % tuple(int(v * 255 / 31) for v in c))
    return len(uniq)

if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('art'); ap.add_argument('--preview')
    args = ap.parse_args()
    rgb = compose(args.art)
    idx, assign, pals = to_tiles(rgb)
    n = write(idx, assign, pals)
    print('%d unique tiles' % n)
    if args.preview:
        prev = np.zeros((160, 240, 3), np.uint8)
        for i, (t, p) in enumerate(zip(idx, assign)):
            y, x = (i // 30) * 8, (i % 30) * 8
            prev[y:y + 8, x:x + 8] = (np.vstack([[0, 0, 0], pals[p]])[t] * 255 / 31).astype(np.uint8)
        Image.fromarray(prev).save(args.preview)
