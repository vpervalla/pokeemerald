#!/usr/bin/env python3
"""Write the day and night wild encounter tables into src/data/wild_encounters.json (see
docs/day_night_encounters_plan.md).

Each table below is a change to a map's own table: the slots keep their levels and only change
species. The day table is used in the morning and the day, the night table in the evening and
the night; a field the timed table doesn't have comes from the map's own table.

Every table is checked: the map is outdoors (towns, cities, routes and the forests, which are
routes), a Hoenn table only uses species already in that map's own table, and every species of
the map's own table is still there by day or by night.

The timed tables are rebuilt from scratch each time, so run it again after any script that writes
the maps' own tables:

    python3 tools/kanto_port/port_kanto.py encounters
    python3 tools/kanto_port/add_leafgreen_exclusives.py --write
    python3 tools/hoenn/add_missing_wild.py --write
    python3 tools/day_night_encounters/apply.py --write
"""
import glob
import json
import os
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PATH = os.path.join(ROOT, 'src', 'data', 'wild_encounters.json')
MAPSECS = os.path.join(ROOT, 'src', 'data', 'region_map', 'region_map_sections.json')

DAY = 'WILD_TIME_DAY'
NIGHT = 'WILD_TIME_NIGHT'
LABEL_SUFFIX = {DAY: '_Day', NIGHT: '_Night'}
FIELDS = ('land_mons', 'water_mons', 'rock_smash_mons', 'fishing_mons')
OUTDOOR_MAP_TYPES = ('MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE', 'MAP_TYPE_OCEAN_ROUTE')

# (map, time, field, changes). A change is (old, new) for every slot of old, or (slots, old, new)
# for some of its slots.
TABLES = [
    # The Test Area, to try it with its time changer
    ('MAP_TEST_AREA', NIGHT, 'land_mons', [('MEW', 'HOOTHOOT')]),

    # Kanto
    ('MAP_ROUTE1', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT')]),
    ('MAP_ROUTE2', DAY, 'land_mons', [([10], 'CATERPIE', 'LEDYBA')]),
    ('MAP_ROUTE2', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('WEEDLE', 'SPINARAK')]),
    ('MAP_VIRIDIAN_FOREST', NIGHT, 'land_mons', [([1, 3], 'WEEDLE', 'SPINARAK')]),
    ('MAP_ROUTE22', NIGHT, 'land_mons', [('SPEAROW', 'RATTATA')]),
    ('MAP_ROUTE24', DAY, 'land_mons', [([10], 'METAPOD', 'LEDYBA')]),
    ('MAP_ROUTE24', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('WEEDLE', 'SPINARAK')]),
    ('MAP_ROUTE25', DAY, 'land_mons', [([10], 'METAPOD', 'LEDYBA')]),
    ('MAP_ROUTE25', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('WEEDLE', 'SPINARAK')]),
    ('MAP_ROUTE5', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT')]),
    ('MAP_ROUTE6', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT')]),
    ('MAP_ROUTE7', NIGHT, 'land_mons', [('PIDGEY', 'MURKROW')]),
    ('MAP_ROUTE8', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ([2], 'VULPIX', 'GASTLY')]),
    ('MAP_ROUTE10', NIGHT, 'land_mons', [([4, 6], 'SPEAROW', 'GASTLY')]),
    ('MAP_ROUTE11', NIGHT, 'land_mons', [('SPEAROW', 'HOOTHOOT')]),
    ('MAP_ROUTE12', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT')]),
    ('MAP_ROUTE13', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('PIDGEOTTO', 'NOCTOWL')]),
    ('MAP_ROUTE14', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('PIDGEOTTO', 'NOCTOWL')]),
    ('MAP_ROUTE15', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('PIDGEOTTO', 'NOCTOWL')]),
    ('MAP_ROUTE16', NIGHT, 'land_mons', [('SPEAROW', 'MURKROW')]),

    # The Sevii Islands
    ('MAP_THREE_ISLAND_BERRY_FOREST', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('PIDGEOTTO', 'NOCTOWL')]),
    ('MAP_FIVE_ISLAND_MEADOW', NIGHT, 'land_mons', [('PIDGEY', 'HOOTHOOT'), ('SENTRET', 'HOOTHOOT'),
                                                    ('PIDGEOTTO', 'NOCTOWL')]),
    ('MAP_SIX_ISLAND_WATER_PATH', DAY, 'land_mons', [([4], 'SENTRET', 'LEDYBA')]),
    ('MAP_SIX_ISLAND_WATER_PATH', NIGHT, 'land_mons', [('SENTRET', 'SPINARAK')]),

    # Hoenn: only more slots for the species of the map that come out at night
    ('MAP_ROUTE101', NIGHT, 'land_mons', [([0, 2], 'WURMPLE', 'POOCHYENA')]),
    ('MAP_ROUTE102', NIGHT, 'land_mons', [([4], 'LOTAD', 'SEEDOT'), ([6], 'ZIGZAGOON', 'POOCHYENA')]),
    ('MAP_ROUTE103', NIGHT, 'land_mons', [([5, 6], 'ZIGZAGOON', 'POOCHYENA')]),
    ('MAP_ROUTE104', NIGHT, 'land_mons', [([6], 'TAILLOW', 'POOCHYENA'), ([8], 'WINGULL', 'POOCHYENA')]),
    ('MAP_PETALBURG_WOODS', NIGHT, 'land_mons', [([8], 'TAILLOW', 'SHROOMISH'), ([1], 'WURMPLE', 'POOCHYENA')]),
    ('MAP_ROUTE110', NIGHT, 'land_mons', [([8], 'WINGULL', 'ODDISH'), ([9], 'WINGULL', 'GULPIN')]),
    ('MAP_ROUTE116', NIGHT, 'land_mons', [([5], 'TAILLOW', 'WHISMUR'), ([6], 'TAILLOW', 'NINCADA')]),
    ('MAP_ROUTE117', NIGHT, 'land_mons', [([0], 'POOCHYENA', 'VOLBEAT'), ([1], 'ODDISH', 'ILLUMISE'),
                                          ([7, 9], 'ILLUMISE', 'VOLBEAT')]),
    ('MAP_ROUTE119', NIGHT, 'land_mons', [([9, 10], 'TROPIUS', 'ODDISH')]),
    ('MAP_ROUTE120', NIGHT, 'land_mons', [([4], 'MARILL', 'POOCHYENA'), ([7], 'MARILL', 'MIGHTYENA')]),
    ('MAP_ROUTE121', NIGHT, 'land_mons', [([8], 'WINGULL', 'SHUPPET'), ([9], 'WINGULL', 'ODDISH')]),
    ('MAP_ROUTE123', NIGHT, 'land_mons', [([8], 'WINGULL', 'SHUPPET'), ([9], 'WINGULL', 'ODDISH')]),
    ('MAP_MT_PYRE_EXTERIOR', NIGHT, 'land_mons', [([8, 9], 'WINGULL', 'SHUPPET')]),
]


def map_infos():
    """map id -> (map type, region) from data/maps/*/map.json and the map sections' order."""
    ids = [s['id'] for s in json.load(open(MAPSECS))['map_sections']]
    kanto = set(ids[ids.index('MAPSEC_PALLET_TOWN'):ids.index('MAPSEC_SPECIAL_AREA') + 1])
    kanto |= {'MAPSEC_TEST_AREA', 'MAPSEC_MEGA_ISLAND'}  # GetMapSecRegion
    infos = {}
    for path in glob.glob(os.path.join(ROOT, 'data', 'maps', '*', 'map.json')):
        m = json.load(open(path))
        infos[m['id']] = (m['map_type'], 'kanto' if m['region_map_section'] in kanto else 'hoenn')
    return infos


def species_of(entry):
    return {m['species'] for f in FIELDS if f in entry for m in entry[f]['mons']}


def build(base, time, field, changes, errors):
    where = f'{base["map"]} {time[10:].lower()}'
    if field not in base:
        errors.append(f'{where}: the map has no {field}')
        return None
    mons = [dict(m) for m in base[field]['mons']]
    for change in changes:
        slots, old, new = change if len(change) == 3 else (None, *change)
        old, new = 'SPECIES_' + old, 'SPECIES_' + new
        if slots is None:
            slots = [i for i, m in enumerate(mons) if m['species'] == old]
        if not slots:
            errors.append(f'{where}: no {old[8:]} in {field}')
        for i in slots:
            if base[field]['mons'][i]['species'] != old:
                errors.append(f'{where}: {field}[{i}] is {base[field]["mons"][i]["species"][8:]}, not {old[8:]}')
            mons[i]['species'] = new
    return {
        'map': base['map'],
        'base_label': base['base_label'] + LABEL_SUFFIX[time],
        'time': time,
        field: {'encounter_rate': base[field]['encounter_rate'], 'mons': mons},
    }


def main():
    data = json.load(open(PATH))
    group = next(g for g in data['wild_encounter_groups'] if g['label'] == 'gWildMonHeaders')
    old_timed = [e for e in group['encounters'] if 'time' in e]
    bases = [e for e in group['encounters'] if 'time' not in e]
    by_map = {}
    for e in bases:
        by_map.setdefault(e['map'], e)
    infos = map_infos()
    errors = []

    timed = {}  # map -> {time: entry}
    for map_id, time, field, changes in TABLES:
        base = by_map.get(map_id)
        if base is None:
            errors.append(f'{map_id}: no wild encounter table')
            continue
        map_type, region = infos[map_id]
        if map_type not in OUTDOOR_MAP_TYPES:
            errors.append(f'{map_id}: {map_type} is not outdoors')
        entry = build(base, time, field, changes, errors)
        if entry is None:
            continue
        entries = timed.setdefault(map_id, {})
        if time in entries:
            if field in entries[time]:
                errors.append(f'{map_id}: two {time} tables for {field}')
            entries[time][field] = entry[field]
        else:
            entries[time] = entry
        if region == 'hoenn' and species_of(entry) - species_of(base):
            new = sorted(s[8:] for s in species_of(entry) - species_of(base))
            errors.append(f'{map_id}: {", ".join(new)} not on this map yet, and only Kanto gets new species')

    for map_id, entries in timed.items():
        base = by_map[map_id]
        kept = set()
        for time in (DAY, NIGHT):
            entry = entries.get(time, {})
            for f in FIELDS:
                if f in entry:
                    kept |= {m['species'] for m in entry[f]['mons']}
                elif f in base:
                    kept |= {m['species'] for m in base[f]['mons']}
        lost = species_of(base) - kept
        if lost:
            errors.append(f'{map_id}: {", ".join(sorted(s[8:] for s in lost))} neither by day nor by night')

    if errors:
        sys.exit('\n'.join(errors))

    # Each map's timed tables go right after its own table (the first one, for Altering Cave)
    encounters = []
    for e in bases:
        encounters.append(e)
        if by_map[e['map']] is e:
            encounters += [timed[e['map']][t] for t in (DAY, NIGHT) if t in timed.get(e['map'], {})]
    new_timed = [e for e in encounters if 'time' in e]

    for e in new_timed:
        for f in FIELDS:
            if f not in e:
                continue
            changed = [f'{i}:{a["species"][8:]}->{b["species"][8:]}'
                       for i, (a, b) in enumerate(zip(by_map[e['map']][f]['mons'], e[f]['mons']))
                       if a['species'] != b['species']]
            print(f'{e["base_label"]} {f}: {" ".join(changed)}')
    if encounters == group['encounters']:
        print('Nothing to change')
        return
    print(f'{len(old_timed)} timed tables before, {len(new_timed)} now')
    if '--write' in sys.argv:
        group['encounters'] = encounters
        with open(PATH, 'w') as f:
            f.write(json.dumps(data, indent=2) + '\n')


if __name__ == '__main__':
    main()
