#!/usr/bin/env python3
"""Draws the Mega Evolution battle animation's sprites.

  graphics/battle_anims/sprites/mega_polyhedron.png  64x256: the golden rhombic triacontahedron,
                                                     whole and as its faces come apart
  graphics/battle_anims/sprites/mega_shards.png      16x64: four of its faces
  graphics/battle_anims/sprites/mega_light.png       16x48: a light of the spiral that rises
                                                     around it, at three sizes

and Primal Reversion's, which have the same layout and the same palette indices:

  graphics/battle_anims/sprites/primal_kyogre_rock.png    64x256: the blue rock around KYOGRE,
                                                          whole and as its cyan cracks spread
  graphics/battle_anims/sprites/primal_kyogre_shards.png  16x64: four pieces of it
  graphics/battle_anims/sprites/primal_groudon_rock.png   64x256: the red rock around GROUDON,
                                                          with orange cracks
  graphics/battle_anims/sprites/primal_groudon_shards.png 16x64: four pieces of it

Run from the repository's root: python3 tools/mega_evolutions/gen_anim_gfx.py
"""
import math
import random

from PIL import Image

OUT_DIR = "graphics/battle_anims/sprites"

# Index 0 is transparent. Every palette has the same layout.
# The Mega Evolution's polyhedron: gold
PALETTE = [
    (0, 168, 144),    # 0 transparent
    (88, 48, 0),      # 1 outline
    (136, 80, 0),     # 2 golds, dark to light
    (168, 104, 8),    # 3
    (200, 136, 16),   # 4
    (224, 168, 24),   # 5
    (240, 196, 48),   # 6
    (248, 220, 88),   # 7
    (248, 236, 136),  # 8
    (255, 250, 208),  # 9 highlight
    (120, 64, 0),     # 10 (the rocks' cracks: unused here)
    (232, 160, 0),    # 11
    (255, 244, 168),  # 12 a shard's lit edge
    (255, 252, 224),  # 13
    (248, 248, 248),  # 14 white
    (0, 0, 0),        # 15 unused
]

# KYOGRE's rock: deep blues, with cyan cracks
PALETTE_KYOGRE = [
    (0, 168, 144),    # 0 transparent
    (8, 16, 64),      # 1 outline
    (16, 32, 104),    # 2 blues, dark to light
    (24, 48, 136),    # 3
    (32, 72, 168),    # 4
    (40, 96, 192),    # 5
    (64, 128, 216),   # 6
    (96, 160, 232),   # 7
    (136, 192, 248),  # 8
    (200, 232, 248),  # 9 glowing rim
    (0, 80, 112),     # 10 crack edge
    (0, 176, 208),    # 11 crack
    (64, 240, 248),   # 12 crack
    (208, 255, 255),  # 13 crack core
    (248, 248, 248),  # 14 white
    (0, 0, 0),        # 15 unused
]
# GROUDON's rock: deep reds, with orange cracks
PALETTE_GROUDON = [
    (0, 168, 144),    # 0 transparent
    (56, 8, 8),       # 1 outline
    (96, 16, 16),     # 2 reds, dark to light
    (128, 24, 16),    # 3
    (160, 32, 24),    # 4
    (192, 48, 32),    # 5
    (216, 72, 40),    # 6
    (232, 104, 56),   # 7
    (248, 144, 80),   # 8
    (248, 208, 152),  # 9 glowing rim
    (120, 40, 0),     # 10 crack edge
    (232, 112, 0),    # 11 crack
    (248, 168, 16),   # 12 crack
    (248, 232, 120),  # 13 crack core
    (248, 248, 248),  # 14 white
    (0, 0, 0),        # 15 unused
]
OUTLINE, HIGHLIGHT = 1, 9
CRACK_EDGE, CRACK, CRACK_BRIGHT, CRACK_CORE = 10, 11, 12, 13

SIZE = 64
RADIUS = 31.5


def new_image(width, height, palette):
    img = Image.new("P", (width, height), 0)
    flat = []
    for colour in palette:
        flat.extend(colour)
    img.putpalette(flat)
    return img


PHI = (1 + math.sqrt(5)) / 2


def triacontahedron_faces():
    """The rhombic triacontahedron's 30 faces, each a list of its 4 corners in order."""
    vertices = []
    for a in (-1, 1):
        for b in (-1, 1):
            for c in (-1, 1):
                vertices.append((a, b, c))
            # The other dodecahedron corners, and the icosahedron's
            for x, y, z in ((0, a / PHI, b * PHI), (a, 0, b * PHI)):
                vertices.extend(((x, y, z), (y, z, x), (z, x, y)))
    vertices = sorted(set(vertices))
    icosa = [v for v in vertices if abs(sum(c * c for c in v) - (1 + PHI * PHI)) < 1e-6]
    faces = []
    for i in range(len(icosa)):
        for j in range(i + 1, len(icosa)):
            if abs(math.dist(icosa[i], icosa[j]) - 2) > 1e-6:
                continue
            # One face lies over each edge of the icosahedron
            normal = tuple((a + b) / 2 for a, b in zip(icosa[i], icosa[j]))
            dots = [sum(n * c for n, c in zip(normal, v)) for v in vertices]
            corners = [v for v, d in zip(vertices, dots) if abs(d - max(dots)) < 1e-6]
            assert len(corners) == 4
            # In order around the face
            u = tuple(a - b for a, b in zip(icosa[i], normal))
            w = (normal[1] * u[2] - normal[2] * u[1], normal[2] * u[0] - normal[0] * u[2], normal[0] * u[1] - normal[1] * u[0])
            corners.sort(key=lambda v: math.atan2(sum((a - n) * c for a, n, c in zip(v, normal, w)),
                                                  sum((a - n) * c for a, n, c in zip(v, normal, u))))
            faces.append((normal, corners))
    assert len(faces) == 30
    return faces


def rotate(v, yaw, pitch):
    x, y, z = v
    x, z = x * math.cos(yaw) + z * math.sin(yaw), -x * math.sin(yaw) + z * math.cos(yaw)
    y, z = y * math.cos(pitch) - z * math.sin(pitch), y * math.sin(pitch) + z * math.cos(pitch)
    return (x, y, z)


def polyhedron_faces():
    """The faces turned towards the viewer, as (shade, [(x, y) corners]) in the 64x64 frame."""
    yaw, pitch = 0.33, 0.24
    faces = [(rotate(n, yaw, pitch), [rotate(c, yaw, pitch) for c in corners]) for n, corners in triacontahedron_faces()]
    extent = max(math.hypot(c[0], c[1]) for n, corners in faces for c in corners)
    scale = RADIUS / extent
    centre = SIZE / 2
    light = (-0.45, -0.55, 0.70)
    norm = math.sqrt(sum(c * c for c in light))
    light = tuple(c / norm for c in light)
    visible = []
    for n, corners in faces:
        length = math.sqrt(sum(c * c for c in n))
        n = tuple(c / length for c in n)
        if n[2] <= 1e-6:
            continue
        lum = max(0.0, sum(a * b for a, b in zip(n, light)))
        shade = max(3, min(8, int(round(2.5 + lum * 6.6))))
        visible.append((shade, [(centre + c[0] * scale, centre + c[1] * scale) for c in corners]))
    return visible


def in_polygon(points, x, y):
    hit = False
    for i in range(len(points)):
        (x0, y0), (x1, y1) = points[i], points[(i + 1) % len(points)]
        if (y0 > y) != (y1 > y) and x < x0 + (y - y0) * (x1 - x0) / (y1 - y0):
            hit = not hit
    return hit


def draw_polyhedron(faces, shrink, spread):
    """The polyhedron, each face shrunk towards its own middle by `shrink` and pushed
    `spread` times further from the polyhedron's: 1 and 1 draw it whole."""
    centre = SIZE / 2
    owner = [[-1] * SIZE for _ in range(SIZE)]
    for i, (shade, corners) in enumerate(faces):
        mx = sum(c[0] for c in corners) / 4
        my = sum(c[1] for c in corners) / 4
        nx, ny = centre + (mx - centre) * spread, centre + (my - centre) * spread
        moved = [(nx + (x - mx) * shrink, ny + (y - my) * shrink) for x, y in corners]
        for y in range(SIZE):
            for x in range(SIZE):
                if in_polygon(moved, x + 0.5, y + 0.5):
                    owner[y][x] = i

    def at(x, y):
        return owner[y][x] if 0 <= x < SIZE and 0 <= y < SIZE else -1

    px = [[0] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            i = owner[y][x]
            if i < 0:
                continue
            shade = faces[i][0]
            around = [at(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))]
            if -1 in around:
                shade = OUTLINE          # The polyhedron's outline, or a loose face's
            elif at(x + 1, y) != i or at(x, y + 1) != i:
                shade = max(2, shade - 3)  # The edges between faces
            px[y][x] = shade
    return px


def draw_light(radius):
    """A glowing light: white at its heart, gold at its rim, with four short rays."""
    px = [[0] * 16 for _ in range(16)]
    centre = 7.5
    for y in range(16):
        for x in range(16):
            d = math.hypot(x - centre, y - centre)
            on_ray = (abs(x - centre) < 1 or abs(y - centre) < 1) and d <= radius + 2.5
            if d <= radius * 0.45:
                px[y][x] = 14
            elif d <= radius * 0.75:
                px[y][x] = HIGHLIGHT
            elif d <= radius:
                px[y][x] = 7
            elif on_ray:
                px[y][x] = 8 if d <= radius + 1.5 else 6
            elif d <= radius + 0.8:
                px[y][x] = 5
    return px


def draw_rock(rng):
    """A boulder: a rough outline cut into flat facets, lit from the upper left, with a
    glowing rim."""
    centre = (SIZE - 1) / 2
    count = 12
    outline = []
    for i in range(count):
        angle = 2 * math.pi * (i + rng.uniform(-0.25, 0.25)) / count
        radius = rng.uniform(28.0, 31.5)
        outline.append((centre + math.cos(angle) * radius, centre + math.sin(angle) * radius))

    def contains(x, y):
        hit = False
        for i in range(count):
            (x0, y0), (x1, y1) = outline[i], outline[(i + 1) % count]
            if (y0 > y) != (y1 > y) and x < x0 + (y - y0) * (x1 - x0) / (y1 - y0):
                hit = not hit
        return hit

    # The facets: each pixel belongs to the nearest of a few points, and each of those
    # faces its own way
    light = (-0.45, -0.55, 0.70)
    norm = math.sqrt(sum(c * c for c in light))
    light = tuple(c / norm for c in light)
    seeds = [(centre + rng.uniform(-3, 3), centre + rng.uniform(-3, 3))]
    for i in range(11):
        angle = 2 * math.pi * i / 11 + rng.uniform(-0.2, 0.2)
        radius = rng.uniform(13, 24)
        seeds.append((centre + math.cos(angle) * radius, centre + math.sin(angle) * radius))
    shades = []
    for (sx, sy) in seeds:
        nx = (sx - centre) / RADIUS + rng.uniform(-0.2, 0.2)
        ny = (sy - centre) / RADIUS + rng.uniform(-0.2, 0.2)
        nz = 0.75
        n = math.sqrt(nx * nx + ny * ny + nz * nz)
        lum = max(0.0, (nx * light[0] + ny * light[1] + nz * light[2]) / n)
        shades.append(max(2, min(8, int(round(1.5 + lum * 7.0)))))

    facet = [[-1] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            if contains(x + 0.5, y + 0.5):
                facet[y][x] = min(range(len(seeds)), key=lambda i: (seeds[i][0] - x) ** 2 + (seeds[i][1] - y) ** 2)

    def at(x, y):
        return facet[y][x] if 0 <= x < SIZE and 0 <= y < SIZE else -1

    px = [[0] * SIZE for _ in range(SIZE)]
    for y in range(SIZE):
        for x in range(SIZE):
            f = facet[y][x]
            if f < 0:
                continue
            shade = shades[f]
            # The creases between facets: dark below and to the right of a facet, light on its other sides
            if at(x + 1, y) not in (f, -1) or at(x, y + 1) not in (f, -1):
                shade = max(2, shade - 2)
            elif at(x - 1, y) not in (f, -1) or at(x, y - 1) not in (f, -1):
                shade = min(8, shade + 1)
            px[y][x] = shade
    edge = {(x, y) for y in range(SIZE) for x in range(SIZE)
            if facet[y][x] >= 0 and any(at(x + dx, y + dy) < 0 for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))}
    for (x, y) in edge:
        px[y][x] = OUTLINE
    for (x, y) in edge:
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            p = (x + dx, y + dy)
            if at(*p) >= 0 and p not in edge:
                px[p[1]][p[0]] = HIGHLIGHT
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


def make_spread_cracks(seed):
    """make_cracks, from the first of 64 seeds whose cracks end in the most directions."""
    centre = (SIZE - 1) / 2
    best, best_score = None, -1
    for i in range(64):
        cracks = make_cracks(random.Random(seed + i))
        sectors = {int((math.atan2(points[-1][1] - centre, points[-1][0] - centre) + math.pi) / (2 * math.pi) * 12) % 12
                   for points, start in cracks if start == 0.0}
        if len(sectors) > best_score:
            best, best_score = cracks, len(sectors)
    return best


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


def save(name, frames, width, palette=PALETTE):
    img = new_image(width, sum(len(f) for f in frames), palette)
    y0 = 0
    for frame in frames:
        for y, row in enumerate(frame):
            for x, colour in enumerate(row):
                img.putpixel((x, y0 + y), colour)
        y0 += len(frame)
    img.save(f"{OUT_DIR}/{name}.png", bits=4)


def main():
    faces = polyhedron_faces()
    whole = draw_polyhedron(faces, 1, 1)
    save("mega_polyhedron", [
        whole,
        draw_polyhedron(faces, 0.88, 1.00),
        draw_polyhedron(faces, 0.74, 1.04),
        draw_polyhedron(faces, 0.56, 1.10),
    ], SIZE)
    # Loose faces, seen from four sides
    save("mega_shards", [
        draw_shard([(8, 1), (14, 8), (8, 15), (2, 8)], lambda x, y: y < 8 and x < 8),
        draw_shard([(1, 8), (8, 4), (15, 8), (8, 12)], lambda x, y: y < 8),
        draw_shard([(3, 2), (13, 5), (13, 14), (3, 11)], lambda x, y: x < 5),
        draw_shard([(13, 2), (13, 11), (3, 14), (3, 5)], lambda x, y: y < x - 6),
    ], 16)

    save("mega_light", [draw_light(4.2), draw_light(3.0), draw_light(1.8)], 16)

    rock = draw_rock(random.Random(0x4B594F47))
    cracks = make_spread_cracks(0x4B594F47)
    save("primal_kyogre_rock", [
        rock,
        draw_cracks(rock, cracks, 0.35, False),
        draw_cracks(rock, cracks, 0.70, False),
        draw_cracks(rock, cracks, 1.00, True),
    ], SIZE, PALETTE_KYOGRE)
    save("primal_kyogre_shards", [
        draw_shard([(1, 4), (9, 1), (14, 6), (12, 13), (4, 14)], lambda x, y: y > x + 2),
        draw_shard([(5, 1), (13, 4), (14, 11), (7, 14), (1, 9)], lambda x, y: x + y > 17),
        draw_shard([(6, 1), (14, 9), (9, 14), (2, 12)], lambda x, y: x < 7),
        draw_shard([(2, 6), (8, 2), (13, 7), (10, 13), (4, 12)], lambda x, y: y < 6),
    ], 16, PALETTE_KYOGRE)

    rock = draw_rock(random.Random(0x47524F55))
    cracks = make_spread_cracks(0x47524F55)
    save("primal_groudon_rock", [
        rock,
        draw_cracks(rock, cracks, 0.35, False),
        draw_cracks(rock, cracks, 0.70, False),
        draw_cracks(rock, cracks, 1.00, True),
    ], SIZE, PALETTE_GROUDON)
    save("primal_groudon_shards", [
        draw_shard([(2, 2), (11, 1), (14, 8), (9, 14), (3, 11)], lambda x, y: y > x + 1),
        draw_shard([(6, 1), (14, 5), (12, 13), (4, 14), (1, 7)], lambda x, y: x + y > 18),
        draw_shard([(8, 1), (14, 10), (7, 14), (2, 9)], lambda x, y: x < 6),
        draw_shard([(1, 5), (7, 2), (14, 6), (11, 13), (5, 13)], lambda x, y: y < 6),
    ], 16, PALETTE_GROUDON)


if __name__ == "__main__":
    main()
