#!/usr/bin/env python3
"""The Primal and Mega colors of Act 3 (docs/hoenn_plan.md): Sootopolis's KYOGRE, GROUDON and RAYQUAZA
use Emerald's sprites with these palettes. Run from the root (it only rewrites the .pal files below):

    python3 tools/hoenn/legend_palettes.py

- Overworld objects (src/data/object_events/hoenn_legend_object_events.h): Emerald's KYOGRE pics use
  the NPC_4 palette, GROUDON's and RAYQUAZA's NPC_3. Each gets a copy with its own colors.
- Emerald's cutscene (src/rayquaza_scene.c), which only Sootopolis plays: KYOGRE and GROUDON in the
  fight (scene 1) and as RAYQUAZA chases them away (scene 5), and RAYQUAZA in scene 5.

PRIMAL KYOGRE is navy with glowing cyan markings, PRIMAL GROUDON a deeper red with dark plates and
glowing lines, MEGA RAYQUAZA black with gold rings (the colors of pokeemerald-expansion's forms).
"""
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OBJ = 'graphics/object_events/palettes'
SCENE = 'graphics/rayquaza_scene'

# (source, destination, {index: color})
PALETTES = [
    (f'{OBJ}/npc_4.pal', f'{OBJ}/hoenn/kyogre_primal.pal', {
        5: (72, 84, 210), 6: (40, 46, 150), 7: (10, 12, 56),           # body
        8: (170, 245, 255), 9: (100, 220, 255), 10: (40, 140, 230),    # markings
        11: (210, 210, 240), 12: (140, 140, 190), 13: (70, 70, 120),   # belly
    }),
    (f'{OBJ}/npc_3.pal', f'{OBJ}/hoenn/groudon_primal.pal', {
        5: (250, 215, 110), 6: (80, 80, 92), 7: (36, 36, 44),          # plates and their lines
        11: (240, 50, 70), 12: (180, 20, 36), 13: (100, 8, 14),        # body
    }),
    (f'{OBJ}/npc_3.pal', f'{OBJ}/hoenn/rayquaza_mega.pal', {
        8: (72, 80, 90), 9: (44, 48, 56), 10: (16, 18, 22),            # body
        11: (255, 220, 60), 12: (220, 170, 30), 13: (120, 80, 10),     # rings
    }),
    (f'{SCENE}/scene_1/kyogre.png', f'{SCENE}/scene_1/kyogre_primal.pal', 'kyogre'),
    (f'{SCENE}/scene_5/kyogre.png', f'{SCENE}/scene_5/kyogre_primal.pal', 'kyogre'),
    (f'{SCENE}/scene_1/groudon.png', f'{SCENE}/scene_1/groudon_primal.pal', 'groudon'),
    (f'{SCENE}/scene_5/groudon.png', f'{SCENE}/scene_5/groudon_primal.pal', 'groudon'),
    (f'{SCENE}/scene_5/rayquaza.png', f'{SCENE}/scene_5/rayquaza_mega.pal', {
        1: (20, 22, 26), 2: (34, 38, 44), 3: (50, 56, 64), 4: (72, 80, 90), 5: (110, 120, 132),  # body
    }),
]

# The cutscene's KYOGRE and GROUDON share their palette layout between scenes 1 and 5
SCENE_COLORS = {
    'kyogre': {
        5: (30, 40, 120), 9: (150, 170, 255),                          # shading
        6: (20, 90, 170), 7: (120, 240, 255), 8: (60, 170, 240),       # the red markings
        11: (70, 90, 220), 12: (55, 65, 190), 13: (40, 45, 150), 14: (22, 24, 80),  # body
    },
    'groudon': {
        1: (190, 30, 50), 5: (255, 110, 90), 6: (90, 5, 15), 7: (230, 30, 50), 8: (150, 15, 30),  # body
        2: (80, 80, 92), 3: (60, 60, 70), 4: (40, 40, 48),             # plates
        10: (250, 220, 120),                                           # their lines
    },
}


def read_colors(path):
    path = os.path.join(ROOT, path)
    if path.endswith('.png'):
        from PIL import Image
        pal = Image.open(path).getpalette()[:48]
        return [tuple(pal[i:i + 3]) for i in range(0, 48, 3)]
    lines = open(path).read().split('\n')
    assert lines[0].startswith('JASC-PAL'), path
    return [tuple(int(v) for v in line.split()) for line in lines[3:19]]


def main():
    for src, dst, changes in PALETTES:
        if isinstance(changes, str):
            changes = SCENE_COLORS[changes]
        colors = read_colors(src)
        for i, color in changes.items():
            colors[i] = color
        os.makedirs(os.path.dirname(os.path.join(ROOT, dst)), exist_ok=True)
        with open(os.path.join(ROOT, dst), 'w', newline='\r\n') as f:
            f.write('JASC-PAL\n0100\n16\n' + ''.join('%d %d %d\n' % c for c in colors))
        print(dst)


if __name__ == '__main__':
    main()
