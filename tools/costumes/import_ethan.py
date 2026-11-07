#!/usr/bin/env python3
"""Import Ethan's sprites from a sheet of HGSS-style sprites (see docs/costume_credits.md).

    python3 tools/costumes/import_ethan.py tools/costumes/sheets/ethan.jpg

Run from the pokeemerald root. SHEET is the 594x290 sheet: walking and running (row 1), the bike
(row 2), surfing (row 3, last three), the field move (row 4), the trainer front pic and the five
back pic frames. Its sprites are bigger than FRLG's (up to 19x25), so Ethan's overworld frames are
32x32. The sheet is a JPEG: its white background is taken out, and its colours are brought down to
one 15 colour overworld palette (PAL below), one for the front pic and one for the back pic.

It writes Ethan's overworld sheets (FRLG's layout), his palettes, his trainer front and back pics,
and draws the VS SEEKER frames of his item sheet (9-11) from his field move. His fishing frames
aren't on the sheet: the old ones are kept, in the new palette.
"""
import sys
from collections import deque
import numpy as np
from PIL import Image

PEOPLE = "graphics/object_events/pics/people/ethan"
TRANSPARENT = (115, 197, 164)
# The overworld palette, picked by hand from the sheet's colours (the JPEG tints its greys)
PAL = [(16, 16, 24), (48, 52, 56), (72, 80, 88), (100, 104, 116), (120, 64, 64), (192, 56, 64), (232, 104, 88),
       (176, 112, 32), (248, 176, 32), (244, 212, 192), (208, 160, 128), (172, 168, 188), (240, 240, 248), (24, 120, 232)]
ROW_TOP = {1: 6, 2: 40, 3: 71, 4: 106}   # the sheet's overworld rows; frames are 32 pixels apart from x = 6
BACK_CELLS = [231, 302, 374, 448, 518]   # left edges of the back pic frames (y 135-198)
FRONT_CELL = (388, 47)

# The VS SEEKER held up, drawn over the POKé BALL of the field move (screen 's': off, then lit)
DEVICE = [".111.",
          "1ccc1",
          "1sss1",
          "1sss1",
          "1ccc1",
          "1c6c1",
          ".111."]


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


def vs_frame(idx, top, lit, clear_rows, x0=24):
    out = idx.copy()
    for y in clear_rows:                 # the POKé BALL's reds, whites and greys
        for x in range(x0, x0 + 6):
            if out[y, x] in (5, 6, 7, 12, 13):
                out[y, x] = 0
    colour = {"1": 1, "c": 12, "6": 6, "s": 14 if lit else 2}
    for dy, row in enumerate(DEVICE):
        for dx, ch in enumerate(row):
            if ch != ".":
                out[top + dy, x0 + dx] = colour[ch]
    return out


def main():
    im = np.array(Image.open(sys.argv[1]).convert("RGB")).astype(int)
    frame = lambda r, k: index(*cut(im, 6 + 32 * k, ROW_TOP[r], 32, 32), PAL)
    cat = lambda frames: np.concatenate(frames, axis=1)

    normal = [frame(1, k) for k in range(9)]
    # the running frames are by direction (down, step, step, up...): FRLG's order is
    # down, up, side, then each direction's two steps
    run = [frame(1, 9 + i) for i in (0, 3, 6, 1, 2, 4, 5, 7, 8)]
    surf = [frame(3, k) for k in (9, 10, 11)]
    bike = [frame(2, k) for k in range(9)]
    item = [frame(4, k) for k in range(5)]
    vs = [vs_frame(item[3], 10, False, range(14, 20)),
          vs_frame(item[4], 6, False, range(9, 16)),
          vs_frame(item[4], 6, True, range(9, 16))]
    save(cat(normal), PAL, f"{PEOPLE}/normal.png")
    save(cat(surf + run + surf[:2]), PAL, f"{PEOPLE}/surf_run.png")   # surf 0-2, run 3-11
    save(cat(bike), PAL, f"{PEOPLE}/bike.png")
    save(cat(item + [item[0]] * 4 + vs), PAL, f"{PEOPLE}/item.png")   # field move 0-4, VS SEEKER 9-11

    # The fishing frames stay, in the new palette
    fish = Image.open(f"{PEOPLE}/fish.png")
    old = fish.getpalette()
    old = [tuple(old[i * 3:i * 3 + 3]) for i in range(16)]
    fa = np.array(fish)
    remap = [0] + [1 + int(np.argmin([sum((a - b) ** 2 for a, b in zip(old[i], c)) for c in PAL])) for i in range(1, 16)]
    save(np.vectorize(lambda v: remap[v])(fa), PAL, f"{PEOPLE}/fish.png")

    write_pal("graphics/object_events/palettes/player_ethan.pal", [TRANSPARENT] + PAL + [(0, 0, 0)])
    # The reflection: the same colours, paler and bluer (as the old one)
    refl = [(int(r * 0.6 + 80), int(g * 0.6 + 92), int(b * 0.6 + 102)) for r, g, b in [TRANSPARENT] + PAL + [(0, 0, 0)]]
    write_pal("graphics/object_events/palettes/player_ethan_reflection.pal", refl)

    backs = [cut(im, x, 135, 64, 64) for x in BACK_CELLS]
    bpal = quantize(backs)
    save(np.concatenate([index(a, m, bpal) for a, m in backs], axis=0), bpal, "graphics/trainers/back_pics/ethan.png")
    front = cut(im, FRONT_CELL[0], FRONT_CELL[1], 64, 64)
    fpal = quantize([front])
    save(index(*front, fpal), fpal, "graphics/trainers/front_pics/ethan.png")


if __name__ == "__main__":
    main()
