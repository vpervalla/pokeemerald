#!/usr/bin/env python3
"""Put the five Hoenn species Emerald has no wild slot for into Hoenn's wild tables (see
docs/catch_them_all.md, "Johto and Hoenn").

Each takes slots from the species it shares its area with in Ruby and Sapphire:

- LUNATONE: every other SOLROCK slot of each METEOR FALLS table, land and water. Which of the
  two gets the first slot alternates from one map to the next, and is the other way round on water.
- ZANGOOSE: ROUTE 114's first SEVIPER slot.
- ROSELIA: two of ROUTE 117's ODDISH slots.
- MEDITITE: two of MT. PYRE's VULPIX slots, outside.
- SURSKIT: the second surfing slot (30%) of the ROUTE 102, 111, 114, 117 and 120 ponds, and ROUTE
  102's last but one grass slot (1%), as in Ruby.

The slots keep their levels. Run it again after editing those tables by hand; it changes nothing
twice.

    python3 tools/hoenn/add_missing_wild.py [--write]
"""
import json
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PATH = os.path.join(ROOT, 'src', 'data', 'wild_encounters.json')

# (map, field, slot indices, species the slots had, species they get)
FIXED = [
    ('MAP_ROUTE114', 'land_mons', [8], 'SEVIPER', 'ZANGOOSE'),
    ('MAP_ROUTE117', 'land_mons', [3, 5], 'ODDISH', 'ROSELIA'),
    ('MAP_MT_PYRE_EXTERIOR', 'land_mons', [5, 7], 'VULPIX', 'MEDITITE'),
    ('MAP_ROUTE102', 'land_mons', [10], 'ZIGZAGOON', 'SURSKIT'),
] + [(m, 'water_mons', [1], 'MARILL', 'SURSKIT')
     for m in ('MAP_ROUTE102', 'MAP_ROUTE111', 'MAP_ROUTE114', 'MAP_ROUTE117', 'MAP_ROUTE120')]


def main():
    data = json.load(open(PATH))
    tables = [e for g in data['wild_encounter_groups'] if g['label'] == 'gWildMonHeaders'
              for e in g['encounters']]
    by_map = {e['map']: e for e in tables}
    changes = []

    for map_id, field, slots, old, new in FIXED:
        mons = by_map[map_id][field]['mons']
        for i in slots:
            if mons[i]['species'] == 'SPECIES_' + old:
                mons[i]['species'] = 'SPECIES_' + new
                changes.append(f'{map_id} {field}[{i}]: {old} -> {new}')
            elif mons[i]['species'] != 'SPECIES_' + new:
                sys.exit(f'{map_id} {field}[{i}] is {mons[i]["species"]}, not {old}')

    lunatone_first = False
    for e in tables:
        if not e['map'].startswith('MAP_METEOR_FALLS'):
            continue
        lunatone_first = not lunatone_first
        for field in ('land_mons', 'water_mons'):
            if field not in e:
                continue
            mons = e[field]['mons']
            pair = [i for i, m in enumerate(mons) if m['species'] in ('SPECIES_SOLROCK', 'SPECIES_LUNATONE')]
            if not pair:
                continue
            first = lunatone_first if field == 'land_mons' else not lunatone_first
            for n, i in enumerate(pair):
                want = 'SPECIES_LUNATONE' if (n % 2 == 0) == first else 'SPECIES_SOLROCK'
                if mons[i]['species'] != want:
                    changes.append(f'{e["map"]} {field}[{i}]: {mons[i]["species"][8:]} -> {want[8:]}')
                    mons[i]['species'] = want

    print('\n'.join(changes) or 'Nothing to change')
    if '--write' in sys.argv and changes:
        with open(PATH, 'w') as f:
            f.write(json.dumps(data, indent=2) + '\n')


if __name__ == '__main__':
    main()
