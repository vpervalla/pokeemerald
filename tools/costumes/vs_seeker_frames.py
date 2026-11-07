#!/usr/bin/env python3
"""Brendan's and May's VS SEEKER frames, with FRLG's VS SEEKER (the one Red, Leaf and Kris hold).

    python3 tools/costumes/vs_seeker_frames.py

Run from the pokeemerald root. Their field move sheets (frames 0-4: the POKé BALL held up) get
frames 5-10 for ANIM_VS_SEEKER: frames 0-2 again, then frames 3 and 4 with the VS SEEKER in place
of the POKé BALL (its screen dark, then lit). It is FRLG's VS SEEKER, pixel for pixel (DEVICE_*,
from frames 7 and 8 of Red's item sheet), and the same size as the POKé BALL.

Their palettes have no room for its blues, so the VS SEEKER frames have their own palette
(OBJ_EVENT_PAL_TAG_*_VS_SEEKER): the player's, with a colour these frames barely use merged into
a near one (MERGE) and the VS SEEKER's blues in the slots that frees. import_ethan.py uses DEVICE_*
too. It can be run again: frames 0-4 are kept, 5-10 are made again.
"""
from PIL import Image

PEOPLE = "graphics/object_events/pics/people"
PALETTES = "graphics/object_events/palettes"

# FRLG's VS SEEKER (frames 8 and 7 of red/item.png), as Red holds it up: '6' its dark blue
# outline, '9' its pale top, 'a' its lavender sides, '7' and '5' its lit screen (light blue, blue)
DEVICE_OFF = [".6666.",
              "699996",
              "696696",
              "6a66a6",
              "6a66a6",
              ".6aa6."]
DEVICE_ON = [".6666.",
             "699996",
             "697796",
             "6a77a6",
             "6a55a6",
             ".6aa6."]
DEVICE_RGB = {"6": (57, 57, 123), "9": (238, 238, 255), "a": (180, 180, 213), "7": (115, 164, 197), "5": (65, 65, 213)}

# Per player: the colours to merge (index -> index it becomes in the VS SEEKER frames), the slots
# that frees and what goes in them, and the device's colours as palette indices
PLAYERS = {
    "brendan": dict(pal="brendan", merge={6: 5}, slots={6: (65, 65, 213), 7: (115, 164, 197)},
                    device={"6": 5, "9": 14, "a": 9, "7": 7, "5": 6}),
    "may": dict(pal="may", merge={12: 13}, slots={12: (115, 164, 197)},
                device={"6": 6, "9": 14, "a": 9, "7": 12, "5": 5}),
}


def read_pal(path):
    lines = open(path).read().split("\n")
    return [tuple(int(v) for v in l.split()) for l in lines[3:] if l.strip()]


def write_pal(path, colours):
    with open(path, "w", newline="\r\n") as f:
        f.write(f"JASC-PAL\n0100\n{len(colours)}\n" + "".join(f"{r} {g} {b}\n" for r, g, b in colours))


def paste_device(frame, x0, y0, colours, lit):
    for dy, row in enumerate(DEVICE_ON if lit else DEVICE_OFF):
        for dx, ch in enumerate(row):
            if ch != ".":
                frame.putpixel((x0 + dx, y0 + dy), colours[ch])


def find_ball(frame):
    """Top left of the POKé BALL held up (its top is three of colour 4, the outline)."""
    for y in range(32):
        for x in range(12):
            if frame.getpixel((x, y)) == frame.getpixel((x + 1, y)) == frame.getpixel((x + 2, y)) == 4:
                return x - 2, y
    raise ValueError("no POKé BALL")


def main():
    for who, cfg in PLAYERS.items():
        path = f"{PEOPLE}/{who}/field_move.png"
        sheet = Image.open(path)
        frames = [sheet.crop((f * 32, 0, f * 32 + 32, 32)) for f in range(5)]

        def merged(f):
            out = f.copy()
            for y in range(32):
                for x in range(32):
                    v = out.getpixel((x, y))
                    if v in cfg["merge"]:
                        out.putpixel((x, y), cfg["merge"][v])
            return out

        vs = [merged(frames[i]) for i in (0, 1, 2)]
        for src, lit in ((3, False), (4, False), (4, True)):
            f = merged(frames[src])
            x0, y0 = find_ball(frames[src])
            for y in range(y0, y0 + 6):
                for x in range(x0, x0 + 6):
                    f.putpixel((x, y), 0)
            paste_device(f, x0, y0, cfg["device"], lit)
            vs.append(f)

        out = Image.new("P", (32 * 11, 32))
        out.putpalette(sheet.getpalette())
        for i, f in enumerate(frames + vs):
            out.paste(f, (i * 32, 0))
        out.save(path)
        pal = read_pal(f"{PALETTES}/{cfg['pal']}.pal")
        for slot, rgb in cfg["slots"].items():
            pal[slot] = rgb
        write_pal(f"{PALETTES}/{cfg['pal']}_vs_seeker.pal", pal)
        print(f"{path}: VS SEEKER frames 5-10; {PALETTES}/{cfg['pal']}_vs_seeker.pal")


if __name__ == "__main__":
    main()
