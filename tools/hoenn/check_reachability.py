#!/usr/bin/env python3
"""Which maps the player can reach (see docs/hoenn_plan.md).

Walks every map's warps and connections, and the warps in its scripts.inc, from the
player's room, the Kanto heal locations and the Seagallop's harbors. Prints how many of
the non-Kanto maps (Emerald's map groups) are reached, and how.

    python3 tools/hoenn/check_reachability.py [-v]
"""
import collections
import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
MAPS = os.path.join(ROOT, 'data', 'maps')

START = 'MAP_PALLET_TOWN_PLAYERS_HOUSE_2F'
# The harbors of src/seagallop.c's destinations, which the ride warps to from C
SEAGALLOP_HARBORS = [
    'MAP_ONE_ISLAND_HARBOR', 'MAP_TWO_ISLAND_HARBOR', 'MAP_THREE_ISLAND_HARBOR',
    'MAP_FOUR_ISLAND_HARBOR', 'MAP_FIVE_ISLAND_HARBOR', 'MAP_SIX_ISLAND_HARBOR',
    'MAP_SEVEN_ISLAND_HARBOR', 'MAP_KANTO_NAVEL_ROCK_HARBOR', 'MAP_KANTO_BIRTH_ISLAND_HARBOR',
    'MAP_MEGA_ISLAND_HARBOR',
]
SCRIPT_WARP = re.compile(r'^\s*(warp\w*|setwarp|setdynamicwarp|setescapewarp|setdivewarp|setholewarp)\b')
MAP_ID = re.compile(r'\b(MAP_[A-Z0-9_]+)\b')


def is_kanto_group(group):
    return group.startswith('gMapGroup_Kanto') or group == 'gMapGroup_Custom'


def load_maps():
    groups = json.load(open(os.path.join(MAPS, 'map_groups.json')))
    group_of, edges = {}, collections.defaultdict(set)
    for group in groups['group_order']:
        for name in groups[group]:
            header = json.load(open(os.path.join(MAPS, name, 'map.json')))
            map_id = header['id']
            group_of[map_id] = group
            for warp in header.get('warp_events') or []:
                edges[map_id].add((warp['dest_map'], 'warp'))
            for conn in header.get('connections') or []:
                edges[map_id].add((conn['map'], 'connection ' + conn['direction']))
            scripts = os.path.join(MAPS, name, 'scripts.inc')
            if os.path.exists(scripts):
                for line in open(scripts):
                    if SCRIPT_WARP.search(line):
                        for dest in MAP_ID.findall(line):
                            edges[map_id].add((dest, 'script ' + line.split()[0]))
    return group_of, edges


def main():
    verbose = '-v' in sys.argv
    group_of, edges = load_maps()
    heal = json.load(open(os.path.join(ROOT, 'src', 'data', 'heal_locations.json')))['heal_locations']
    roots = [START] + [h['map'] for h in heal if is_kanto_group(group_of.get(h['map'], ''))] + SEAGALLOP_HARBORS

    came_from = {root: None for root in roots}
    queue = collections.deque(roots)
    while queue:
        map_id = queue.popleft()
        for dest, how in sorted(edges[map_id]):
            if dest in group_of and dest not in came_from:
                came_from[dest] = (map_id, how)
                queue.append(dest)

    others = [m for m in group_of if not is_kanto_group(group_of[m])]
    reached = [m for m in others if m in came_from]
    print(f'{len(reached)} of {len(others)} non-Kanto maps reachable')
    for map_id in reached:
        path, step = [map_id], came_from[map_id]
        while step:
            path.append(f'{step[0]} ({step[1]})')
            step = came_from[step[0]]
        print('  ' + ' <- '.join(path if verbose else path[:2]))


if __name__ == '__main__':
    main()
