#!/usr/bin/env python3
"""Draws the Mega Evolution battle animation's sprites.

  graphics/battle_anims/sprites/mega_sphere.png  64x256: the purple sphere, whole and at
                                                 three stages of its yellow cracks
  graphics/battle_anims/sprites/mega_shards.png  16x64: four pieces of its shell

Run from the repository's root: python3 tools/mega_evolutions/gen_anim_gfx.py
"""
import math
import random

from PIL import Image

OUT_DIR = "graphics/battle_anims/sprites"

# Index 0 is transparent.
PALETTE = [
    (0, 168, 144),    # 0 transparent
    (32, 8, 56),      # 1 outline
    (56, 16, 96),     # 2 purples, dark to light
    (80, 24, 128),    # 3
    (104, 40, 160),   # 4
    (128, 56, 184),   # 5
    (152, 80, 208),   # 6
    (176, 112, 224),  # 7
    (208, 160, 240),  # 8
    (240, 216, 248),  # 9 highlight
    (144, 88, 0),     # 10 crack edge
    (232, 160, 0),    # 11 crack
    (248, 216, 32),   # 12 crack
    (248, 248, 136),  # 13 crack core
    (248, 248, 248),  # 14 white
    (0, 0, 0),        # 15 unused
]
OUTLINE, HIGHLIGHT = 1, 9
CRACK_EDGE, CRACK, CRACK_BRIGHT, CRACK_CORE = 10, 11, 12, 13

SIZE = 64
RADIUS = 31.5


def new_image(width, height):
    img = Image.new("P", (width, height), 0)
    flat = []
    for colour in PALETTE:
        flat.extend(colour)
    img.putpalette(flat)
    return img


def draw_sphere():
    """The whole sphere: lit from the upper left, dithered between its shades."""
    px = [[0] * SIZE for _ in range(SIZE)]
    centre = (SIZE - 1) / 2
    light = (-0.45, -0.55, 0.70)
    norm = math.sqrt(sum(c * c for c in light))
    light = tuple(c / norm for c in light)
    bayer = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]
    for y in range(SIZE):
        for x in range(SIZE):
            dx, dy = (x - centre) / RADIUS, (y - centre) / RADIUS
            d2 = dx * dx + dy * dy
            if d2 > 1:
                continue
            if d2 > 0.90:
                px[y][x] = OUTLINE
                continue
            dz = math.sqrt(1 - d2)
            lum = max(0.0, dx * light[0] + dy * light[1] + dz * light[2])
            shade = 2 + lum * 5.6 + (bayer[y % 4][x % 4] / 16 - 0.5) * 0.9
            px[y][x] = max(2, min(8, int(round(shade))))
    # The glint
    for y in range(SIZE):
        for x in range(SIZE):
            if (x - 20) ** 2 / 30 + (y - 17) ** 2 / 14 <= 1:
                px[y][x] = HIGHLIGHT
    return px


def make_cracks(rng):
    """Jagged lines from a point near the middle out to the sphere's edge, with branches."""
    centre = (SIZE - 1) / 2
    origin = (centre + 3, centre - 2)
    cracks = []
    count = 7
    for i in range(count):
        angle = 2 * math.pi * i / count + rng.uniform(-0.25, 0.25)
        points = [origin]
        x, y = origin
        while (x - centre) ** 2 + (y - centre) ** 2 < (RADIUS - 2) ** 2:
            angle += rng.uniform(-0.75, 0.75)
            # Keep heading outwards
            out = math.atan2(y - origin[1], x - origin[0]) if (x, y) != origin else angle
            diff = (angle - out + math.pi) % (2 * math.pi) - math.pi
            angle = out + max(-0.9, min(0.9, diff))
            step = rng.uniform(4, 7)
            x += math.cos(angle) * step
            y += math.sin(angle) * step
            points.append((x, y))
        cracks.append((points, 0.0))
        # A branch from the middle of the crack, which starts once the crack has got there
        if len(points) > 4:
            start = points[len(points) // 2]
            bangle = math.atan2(points[-1][1] - start[1], points[-1][0] - start[0]) + rng.choice((-1, 1)) * rng.uniform(0.6, 1.0)
            branch = [start]
            x, y = start
            for _ in range(rng.randint(2, 3)):
                bangle += rng.uniform(-0.5, 0.5)
                step = rng.uniform(4, 6)
                x += math.cos(bangle) * step
                y += math.sin(bangle) * step
                branch.append((x, y))
            cracks.append((branch, 0.5))
    return cracks


def plot_line(mask, p0, p1):
    steps = int(max(abs(p1[0] - p0[0]), abs(p1[1] - p0[1])) * 2) + 1
    for i in range(steps + 1):
        t = i / steps
        mask.add((int(round(p0[0] + (p1[0] - p0[0]) * t)), int(round(p0[1] + (p1[1] - p0[1]) * t))))


def draw_cracks(sphere, cracks, fraction, thick):
    """The sphere with the first `fraction` of each crack's length drawn on it."""
    px = [row[:] for row in sphere]
    core = set()
    for points, start in cracks:
        total = sum(math.dist(points[i], points[i + 1]) for i in range(len(points) - 1))
        left = total * (fraction - start) / (1 - start)
        for i in range(len(points) - 1):
            seg = math.dist(points[i], points[i + 1])
            if left <= 0:
                break
            t = min(1.0, left / seg)
            end = (points[i][0] + (points[i + 1][0] - points[i][0]) * t,
                   points[i][1] + (points[i + 1][1] - points[i][1]) * t)
            plot_line(core, points[i], end)
            left -= seg

    def inside(x, y):
        return 0 <= x < SIZE and 0 <= y < SIZE and sphere[y][x] != 0

    core = {(x, y) for (x, y) in core if inside(x, y)}
    near = set()
    for (x, y) in core:
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            if inside(x + dx, y + dy) and (x + dx, y + dy) not in core:
                near.add((x + dx, y + dy))
    if thick:
        edge = set()
        for (x, y) in near:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                p = (x + dx, y + dy)
                if inside(*p) and p not in core and p not in near:
                    edge.add(p)
        for (x, y) in edge:
            px[y][x] = CRACK_EDGE
        for (x, y) in near:
            px[y][x] = CRACK_BRIGHT
        for (x, y) in core:
            px[y][x] = CRACK_CORE
    else:
        for (x, y) in near:
            if px[y][x] != HIGHLIGHT:
                px[y][x] = CRACK_EDGE
        for (x, y) in core:
            px[y][x] = CRACK_BRIGHT
    return px


def draw_shard(points, lit):
    """A piece of the shell: a polygon, shaded, with the glow of a crack along one side."""
    px = [[0] * 16 for _ in range(16)]

    def contains(x, y):
        hit = False
        for i in range(len(points)):
            (x0, y0), (x1, y1) = points[i], points[(i + 1) % len(points)]
            if (y0 > y) != (y1 > y) and x < x0 + (y - y0) * (x1 - x0) / (y1 - y0):
                hit = not hit
        return hit

    for y in range(16):
        for x in range(16):
            if contains(x + 0.5, y + 0.5):
                px[y][x] = 4 + (1 if x + y < 14 else 0) + (1 if x + y < 9 else 0)
    for y in range(16):
        for x in range(16):
            if px[y][x] == 0:
                continue
            border = any(not (0 <= x + dx < 16 and 0 <= y + dy < 16) or px[y + dy][x + dx] == 0
                         for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if border:
                px[y][x] = -1
    for y in range(16):
        for x in range(16):
            if px[y][x] == -1:
                px[y][x] = CRACK_BRIGHT if lit(x, y) else OUTLINE
    return px


def save(name, frames, width):
    img = new_image(width, sum(len(f) for f in frames))
    y0 = 0
    for frame in frames:
        for y, row in enumerate(frame):
            for x, colour in enumerate(row):
                img.putpixel((x, y0 + y), colour)
        y0 += len(frame)
    img.save(f"{OUT_DIR}/{name}.png", bits=4)


def main():
    rng = random.Random(0x4D454741)
    sphere = draw_sphere()
    cracks = make_cracks(rng)
    save("mega_sphere", [
        sphere,
        draw_cracks(sphere, cracks, 0.35, False),
        draw_cracks(sphere, cracks, 0.70, False),
        draw_cracks(sphere, cracks, 1.00, True),
    ], SIZE)
    save("mega_shards", [
        draw_shard([(2, 3), (12, 1), (14, 9), (6, 14)], lambda x, y: y > x),
        draw_shard([(4, 1), (13, 5), (10, 14), (1, 10)], lambda x, y: x + y > 16),
        draw_shard([(7, 1), (14, 12), (2, 13)], lambda x, y: x < 8 and y < 12),
        draw_shard([(3, 5), (9, 3), (12, 8), (8, 12), (4, 11)], lambda x, y: y < 7),
    ], 16)


if __name__ == "__main__":
    main()
