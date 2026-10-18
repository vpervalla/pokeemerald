#!/usr/bin/env python3
"""Raise Hoenn's levels to the post-game (docs/hoenn_plan.md, phase 4). Run once, from the root:

    python3 tools/hoenn/rescale_levels.py [--dry-run]

Every level L becomes round(53 + 0.47 * L): Emerald's story (3-58) becomes 54-80, its rematches
end near 88. It changes, and marks as done:

- src/data/trainer_parties.h: Emerald's trainers, except the link placeholders. A POKéMON whose
  new level allows it evolves (level evolutions at their level, the others at NONLEVEL_EVOLUTION;
  a branching line takes its first branch). A party with custom moves keeps up to two moves the
  POKéMON doesn't learn by level (its TMs and tutor moves), and the rest are its latest level-up
  moves.
- src/data/wild_encounters.json: the encounter tables of Hoenn's maps (Emerald's map groups). The
  species stay, so every POKéMON can still be caught in its first stage.
- data/maps/*/scripts.inc of Hoenn's maps: the levels of setwildbattle (the legendaries,
  KECLEON, the ELECTRODE...). Gifts (givemon) keep their level.
"""
import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
MARKER = '// Levels raised for the Hoenn chapter by tools/hoenn/rescale_levels.py; do not run it again.\n'
EXCLUDED_TRAINERS = {'TRAINER_RED', 'TRAINER_LEAF', 'TRAINER_BRENDAN_PLACEHOLDER', 'TRAINER_MAY_PLACEHOLDER'}
NONLEVEL_EVOLUTION = {
    'EVO_FRIENDSHIP': 30, 'EVO_FRIENDSHIP_DAY': 30, 'EVO_FRIENDSHIP_NIGHT': 30,
    'EVO_BEAUTY': 35, 'EVO_ITEM': 40, 'EVO_TRADE': 45, 'EVO_TRADE_ITEM': 45,
}
KEPT_OTHER_MOVES = 2


def new_level(level):
    return min(100, int(53 + 0.47 * level + 0.5))


def read(path):
    with open(os.path.join(ROOT, path)) as f:
        return f.read()


def write(path, text):
    with open(os.path.join(ROOT, path), 'w') as f:
        f.write(text)


def load_evolutions():
    """species -> [(method, param, target)], in the table's order"""
    evos = {}
    text = read('src/data/pokemon/evolution.h')
    for species, body in re.findall(r'\[(SPECIES_\w+)\]\s*=\s*\{(.*?)\}\s*,?\s*(?=\[SPECIES_|\};)', text, re.S):
        evos[species] = re.findall(r'\{(EVO_\w+),\s*(\w+),\s*(SPECIES_\w+)\}', body)
    return evos


def load_learnsets():
    """species -> [(level, move)]"""
    arrays = {}
    text = read('src/data/pokemon/level_up_learnsets.h')
    for name, body in re.findall(r'static const u16 (\w+)\[\] = \{(.*?)\};', text, re.S):
        arrays[name] = [(int(l), m) for l, m in re.findall(r'LEVEL_UP_MOVE\(\s*(\d+),\s*(MOVE_\w+)\)', body)]
    pointers = read('src/data/pokemon/level_up_learnset_pointers.h')
    return {s: arrays[a] for s, a in re.findall(r'\[(SPECIES_\w+)\]\s*=\s*(\w+)', pointers) if a in arrays}


def evolve(species, level, evos):
    """The species a trainer's POKéMON of this level has evolved to, and the line it went through"""
    line = [species]
    while True:
        for method, param, target in evos.get(species, []):
            if method == 'EVO_LEVEL_SHEDINJA':
                continue
            at = int(param) if method.startswith('EVO_LEVEL') else NONLEVEL_EVOLUTION.get(method)
            if at is not None and level >= at:
                species = target
                line.append(species)
                break
        else:
            return species, line


def new_moves(old_moves, line, species, level, learnsets):
    by_level = set(m for s in line for _, m in learnsets.get(s, []))
    kept = [m for m in old_moves if m != 'MOVE_NONE' and m not in by_level][:KEPT_OTHER_MOVES]
    latest = []
    for _, move in sorted((e for e in learnsets.get(species, []) if e[0] <= level), reverse=True):
        if move not in kept and move not in latest:
            latest.append(move)
        if len(kept) + len(latest) == 4:
            break
    moves = kept + list(reversed(latest))
    return moves + ['MOVE_NONE'] * (4 - len(moves))


def rescale_trainers(evos, learnsets, report):
    text = read('src/data/trainer_parties.h')
    if text.startswith(MARKER):
        sys.exit('src/data/trainer_parties.h is already rescaled')
    trainers = read('src/data/trainers.h')
    excluded = set()
    for trainer, party in re.findall(r'\[(TRAINER_\w+)\]\s*=\s*\{.*?\.party = \w+\((sParty_\w+)\)', trainers, re.S):
        if trainer in EXCLUDED_TRAINERS:
            excluded.add(party)

    mon_re = re.compile(r'(\{\s*\.iv = \d+,\s*\.lvl = )(\d+)(,\s*\.species = )(SPECIES_\w+)'
                        r'(,(?:\s*\.heldItem = \w+,?)?)(\s*\.moves = \{)?([^}]*?)(\}\s*)?(\s*\})', re.S)

    def rescale_party(match):
        name, body = match.group(1), match.group(2)
        if name in excluded:
            return match.group(0)

        def rescale_mon(m):
            level = new_level(int(m.group(2)))
            species, line = evolve(m.group(4), level, evos)
            report['trainer_mons'] += 1
            if species != m.group(4):
                report['evolved'] += 1
            out = m.group(1) + str(level) + m.group(3) + species + m.group(5)
            if m.group(6):
                old = [x.strip() for x in m.group(7).split(',')]
                out += m.group(6) + ', '.join(new_moves(old, line, species, level, learnsets)) + m.group(8)
            else:
                out += m.group(7)
            return out + m.group(9)

        return 'sParty_' + name[len('sParty_'):] + '[] = {' + mon_re.sub(rescale_mon, body) + '};'

    text = re.sub(r'(sParty_\w+)\[\] = \{(.*?)\};', rescale_party, text, flags=re.S)
    return MARKER + text


def hoenn_maps():
    groups = json.loads(read('data/maps/map_groups.json'))
    maps = {}
    for group in groups['group_order']:
        if group.startswith('gMapGroup_Kanto') or group == 'gMapGroup_Custom':
            continue
        for name in groups[group]:
            maps[json.loads(read(f'data/maps/{name}/map.json'))['id']] = name
    return maps


def rescale_wild(maps, report):
    data = json.loads(read('src/data/wild_encounters.json'))
    for group in data['wild_encounter_groups']:
        if group['label'] != 'gWildMonHeaders':
            continue  # the Battle Pyramid's and Pike's are set by their facilities
        for encounter in group['encounters']:
            if encounter.get('map') not in maps:
                continue
            report['wild_tables'] += 1
            for key, table in encounter.items():
                if isinstance(table, dict) and 'mons' in table:
                    for mon in table['mons']:
                        mon['min_level'] = new_level(mon['min_level'])
                        mon['max_level'] = new_level(mon['max_level'])
    return json.dumps(data, indent=2, ensure_ascii=False) + '\n'


def rescale_static(maps, report):
    changed = {}
    for name in sorted(set(maps.values())):
        path = f'data/maps/{name}/scripts.inc'
        if not os.path.exists(os.path.join(ROOT, path)):
            continue
        text = read(path)

        def repl(m):
            report['static'] += 1
            return m.group(1) + str(new_level(int(m.group(2))))

        new = re.sub(r'(\bsetwildbattle\s+SPECIES_\w+,\s*)(\d+)', repl, text)
        if new != text:
            changed[path] = new
    return changed


def main():
    dry = '--dry-run' in sys.argv
    evos, learnsets = load_evolutions(), load_learnsets()
    report = {'trainer_mons': 0, 'evolved': 0, 'wild_tables': 0, 'static': 0}
    parties = rescale_trainers(evos, learnsets, report)
    maps = hoenn_maps()
    wild = rescale_wild(maps, report)
    static = rescale_static(maps, report)
    print(f"{report['trainer_mons']} trainer POKéMON ({report['evolved']} evolved), "
          f"{report['wild_tables']} wild tables, {report['static']} static encounters")
    if dry:
        return
    write('src/data/trainer_parties.h', parties)
    write('src/data/wild_encounters.json', wild)
    for path, text in static.items():
        write(path, text)


if __name__ == '__main__':
    main()
