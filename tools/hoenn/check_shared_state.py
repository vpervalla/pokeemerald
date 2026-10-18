#!/usr/bin/env python3
"""Checks that Kanto and Hoenn keep their progress apart (see docs/hoenn_plan.md, section 1).

The Hoenn maps (Emerald's map groups) must not read Kanto's badges, game clear or starter,
and the Kanto maps (gMapGroup_Kanto*, gMapGroup_Custom) must not read Hoenn's. Also checks
the shared event scripts that only Hoenn uses.

    python3 tools/hoenn/check_shared_state.py

Exits with 1 and lists the offending lines if any.
"""
import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
MAPS = os.path.join(ROOT, 'data', 'maps')

KANTO_STATE = re.compile(r'\b(FLAG_BADGE0[1-8]_GET|FLAG_SYS_GAME_CLEAR|VAR_STARTER_MON|FLAG_HIDDEN_ITEM_VICTORY_ROAD_1F_ULTRA_BALL)\b')
HOENN_STATE = re.compile(r'\b(FLAG_HOENN_BADGE0[1-8]_GET|FLAG_HOENN_GAME_CLEAR|VAR_HOENN_STARTER|FLAG_HIDDEN_ITEM_HOENN_VICTORY_ROAD_1F_ULTRA_BALL)\b')
# Shared scripts that only Hoenn maps call
HOENN_SCRIPTS = ['players_house.inc', 'prof_birch.inc', 'secret_base.inc']


def is_kanto_group(group):
    return group.startswith('gMapGroup_Kanto') or group == 'gMapGroup_Custom'


def scan(path, pattern, problems):
    with open(path) as f:
        for num, line in enumerate(f, 1):
            if pattern.search(line):
                problems.append(f'{os.path.relpath(path, ROOT)}:{num}: {line.strip()}')


def main():
    groups = json.load(open(os.path.join(MAPS, 'map_groups.json')))
    problems = []
    for group in groups['group_order']:
        wrong = HOENN_STATE if is_kanto_group(group) else KANTO_STATE
        for name in groups[group]:
            for filename in sorted(os.listdir(os.path.join(MAPS, name))):
                scan(os.path.join(MAPS, name, filename), wrong, problems)
    for filename in HOENN_SCRIPTS:
        scan(os.path.join(ROOT, 'data', 'scripts', filename), KANTO_STATE, problems)

    if problems:
        print('\n'.join(problems))
        print(f'{len(problems)} lines read the other region\'s progress')
        sys.exit(1)
    print('Kanto and Hoenn keep their progress apart')


if __name__ == '__main__':
    main()
