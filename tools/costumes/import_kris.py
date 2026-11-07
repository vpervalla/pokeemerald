#!/usr/bin/env python3
"""Import Kris's sprites from FourLeafSunny's sheet (credit: docs/costume_credits.md).

    python3 tools/costumes/import_kris.py tools/costumes/sheets/kris_fourleafsunny.png

Run from the pokeemerald root. SHEET.png is the 520x520 sheet with the Standard, Running, Biking,
Fishing, Sitting, Map, Speech, Item, Back and Front sections and its palettes. It writes Kris's
overworld sheets (FRLG's layout, as Red's and Leaf's), her palettes, her trainer front and back
pics, her intro picture and her map icon. The sheet's colours are used as they are.
"""
import sys
from PIL import Image

PEOPLE = "graphics/object_events/pics/people/kris"
BG = (162, 136, 187)        # the sheet's frame background: transparent
# The sheet's palette strips (each 15 colours, 2 pixels wide): overworld, its reflection, front, back
STRIPS = {"ow": (253, 502), "ow_reflection": (253, 513), "front": (297, 502), "back": (297, 513), "speech": (340, 502)}

# Sections: (x, y) of the top left frame, frame size, columns (down, up, side) x rows (steps)
STANDARD = ((5, 29), 18, 33)    # x0, y0, column step, row step for 16x32 frames
RUNNING = ((81, 29), 18, 33)
BIKING = ((140, 29), 34, 33)    # 32x32 frames
FISHING = ((244, 29), 34, 33)
SITTING = [(348, 29), (366, 29), (384, 29)]
ITEM = [(5, 189), (22, 189), (39, 189), (56, 189), (73, 189), (93, 189), (110, 189), (127, 189), (144, 189)]
BACK = [(5, 342), (70, 342), (135, 342), (200, 342), (265, 342)]
FRONT = (333, 342)
SPEECH = (448, 29)
MAP_ICON = (418, 37)


def strip(sheet, name):
    x, y = STRIPS[name]
    cols, prev = [], None
    for i in range(40):
        c = sheet.getpixel((x + i, y))[:3]
        if c == (100, 98, 106):     # the sheet's grey around the strip
            if cols:
                break
            continue
        if c != prev:
            cols.append(c)
            prev = c
    return cols[:15]


def to_indexed(img, palette, transparent=BG, offset=0, pal_len=16):
    """img (RGB) -> P image using [transparent] + palette; colours not in it go to the nearest."""
    full = [transparent] + palette
    out = Image.new("P", img.size)
    flat = []
    for c in full + [(0, 0, 0)] * (256 - len(full)):
        flat += c
    if offset:
        flat = flat[:0]
        pal = [(0, 0, 0)] * 256
        for i, c in enumerate(full):
            pal[offset + i] = c
        pal[0] = transparent
        for c in pal:
            flat += c
    out.putpalette(flat)
    misses = 0
    for y in range(img.height):
        for x in range(img.width):
            c = img.getpixel((x, y))[:3]
            if c == transparent:
                i = 0
            elif c in full:
                i = full.index(c)
            else:
                misses += 1
                i = min(range(1, len(full)), key=lambda k: sum((a - b) ** 2 for a, b in zip(full[k], c)))
            out.putpixel((x, y), (offset + i) if (offset and i) else i)
    return out, misses


def frames(sheet, section, w, h, order):
    (x0, y0), dx, dy = section
    return [sheet.crop((x0 + c * dx, y0 + r * dy, x0 + c * dx + w, y0 + r * dy + h)) for c, r in order]


def row(imgs, w, h):
    out = Image.new("RGB", (w * len(imgs), h), BG)
    for i, im in enumerate(imgs):
        out.paste(im, (i * w, 0))
    return out


def save(img, palette, path, **kw):
    p, misses = to_indexed(img, palette, **kw)
    p.save(path)
    print(f"{path}: {img.size[0]}x{img.size[1]}" + (f", {misses} pixels not in the palette" if misses else ""))


def write_pal(path, colours):
    with open(path, "w", newline="\r\n") as f:
        f.write(f"JASC-PAL\n0100\n{len(colours)}\n" + "".join(f"{r} {g} {b}\n" for r, g, b in colours))


def main():
    sheet = Image.open(sys.argv[1]).convert("RGB")
    pal = {k: strip(sheet, k) for k in STRIPS}
    ow = pal["ow"]
    # FRLG's 9 walking frames: down, up, side standing, then each direction's two steps.
    # The sheet's columns are down, up, side; its rows stand, step, stand, step.
    walk = [(0, 0), (1, 0), (2, 0), (0, 1), (0, 3), (1, 1), (1, 3), (2, 1), (2, 3)]
    save(row(frames(sheet, STANDARD, 16, 32, walk), 16, 32), ow, f"{PEOPLE}/normal.png")
    sit = [sheet.crop((x, y, x + 16, y + 32)) for x, y in SITTING]
    run = frames(sheet, RUNNING, 16, 32, walk)
    save(row(sit + run + sit[:2], 16, 32), ow, f"{PEOPLE}/surf_run.png")   # surf 0-2, run 3-11
    save(row(frames(sheet, BIKING, 32, 32, walk), 32, 32), ow, f"{PEOPLE}/bike.png")
    # Fishing: side frames 0-3, up 4-7, down 8-11 (the sheet's rows are the cast's steps)
    fish = [(2, r) for r in range(4)] + [(1, r) for r in range(4)] + [(0, r) for r in range(4)]
    save(row(frames(sheet, FISHING, 32, 32, fish), 32, 32), ow, f"{PEOPLE}/fish.png")
    # Item: the field move (0-4), then FRLG's VS SEEKER frames (5-8)
    save(row([sheet.crop((x, y, x + 16, y + 32)) for x, y in ITEM], 16, 32), ow, f"{PEOPLE}/item.png")
    write_pal("graphics/object_events/palettes/player_kris.pal", [(115, 197, 164)] + ow)
    write_pal("graphics/object_events/palettes/player_kris_reflection.pal", [(115, 197, 164)] + pal["ow_reflection"])

    back = Image.new("RGB", (64, 64 * len(BACK)), BG)
    for i, (x, y) in enumerate(BACK):
        back.paste(sheet.crop((x, y, x + 64, y + 64)), (0, 64 * i))
    save(back, pal["back"], "graphics/trainers/back_pics/kris.png")
    save(sheet.crop((FRONT[0], FRONT[1], FRONT[0] + 64, FRONT[1] + 64)), pal["front"], "graphics/trainers/front_pics/kris.png")

    # The intro picture: 8bpp, at BG palettes 4 and 5 (oak_speech.c loads and fades 32 colours
    # there). It has more colours than one palette: the speech strip's first, then the others.
    sp = sheet.crop((SPEECH[0], SPEECH[1], SPEECH[0] + 64, SPEECH[1] + 96))
    colours = list(pal["speech"])
    for y in range(sp.height):
        for x in range(sp.width):
            c = sp.getpixel((x, y))
            if c != BG and c not in colours:
                colours.append(c)
    if len(colours) > 31:
        sys.exit(f"the intro picture has {len(colours)} colours, more than 31")
    save(sp, colours, "graphics/oak_speech/kris/pic.png", offset=64)
    write_pal("graphics/oak_speech/kris/pal.pal", ([(115, 197, 164)] + colours + [(0, 0, 0)] * 32)[:32])

    save(sheet.crop((MAP_ICON[0], MAP_ICON[1], MAP_ICON[0] + 16, MAP_ICON[1] + 16)), ow, "graphics/pokenav/region_map/kris_icon.png")


if __name__ == "__main__":
    main()
