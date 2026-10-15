#!/usr/bin/env python3
"""Generates the card game's Pokémon cards from the game's own data.

    python3 tools/card_game/gen_cards.py        (from the repository root)

Reads the species, evolutions, learnsets, moves, pokédex weights, Mega Evolutions and the
Kanto / Sevii / Mega Island maps, and writes:

    include/constants/card_game_cards.h     CARD_* ids of the Pokémon cards
    src/data/card_game/pokemon_cards.h      their PokemonCardInfo table

Then src/data/card_game/pokemon_card_overrides.json is applied on top, card by card, so
hand-tuned cards survive a rerun. See docs/card_game_plan.md, section 2.
"""

import json
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OVERRIDES = 'src/data/card_game/pokemon_card_overrides.json'
OUT_CONSTANTS = 'include/constants/card_game_cards.h'
OUT_TABLE = 'src/data/card_game/pokemon_cards.h'


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


# ---------------------------------------------------------------------------------------------
# The game's data

def parse_blocks(text, key_prefix):
    """Yields (key, body) for each `[KEY_x] = { ... }` block, matching braces."""
    for m in re.finditer(r'\[(%s\w+)\]\s*=\s*\{' % key_prefix, text):
        depth, i = 1, m.end()
        while depth:
            depth += {'{': 1, '}': -1}.get(text[i], 0)
            i += 1
        yield m.group(1), text[m.end():i - 1]


def load_species():
    species = {}
    for key, body in parse_blocks(read('src/data/pokemon/species_info.h'), 'SPECIES_'):
        stats = {}
        for stat in ('baseHP', 'baseAttack', 'baseDefense', 'baseSpeed', 'baseSpAttack', 'baseSpDefense'):
            m = re.search(r'\.%s\s*=\s*(\d+)' % stat, body)
            if m:
                stats[stat] = int(m.group(1))
        types = re.search(r'\.types\s*=\s*\{\s*(TYPE_\w+)\s*,\s*(TYPE_\w+)', body)
        if len(stats) == 6 and types:
            species[key] = {'stats': stats, 'types': types.groups(), 'bst': sum(stats.values())}
    return species


def load_evolutions():
    """Returns {species: pre-evolution}."""
    pre = {}
    for key, body in parse_blocks(read('src/data/pokemon/evolution.h'), 'SPECIES_'):
        for target in re.findall(r'\{\s*EVO_\w+\s*,\s*\w+\s*,\s*(SPECIES_\w+)\s*\}', body):
            pre.setdefault(target, key)
    return pre


def load_learnsets():
    arrays = {}
    for name, body in re.findall(r'static const u16 (\w+)\[\]\s*=\s*\{(.*?)\};', read('src/data/pokemon/level_up_learnsets.h'), re.S):
        arrays[name] = [(int(lvl), move) for lvl, move in re.findall(r'LEVEL_UP_MOVE\(\s*(\d+)\s*,\s*(MOVE_\w+)\)', body)]
    learnsets = {}
    for key, name in re.findall(r'\[(SPECIES_\w+)\]\s*=\s*(\w+)', read('src/data/pokemon/level_up_learnset_pointers.h')):
        learnsets[key] = arrays.get(name, [])
    return learnsets


def load_moves():
    moves = {}
    for key, body in parse_blocks(read('src/data/battle_moves.h'), 'MOVE_'):
        effect = re.search(r'\.effect\s*=\s*(EFFECT_\w+)', body)
        power = re.search(r'\.power\s*=\s*(\d+)', body)
        mtype = re.search(r'\.type\s*=\s*(TYPE_\w+)', body)
        if effect and power and mtype:
            moves[key] = {'effect': effect.group(1), 'power': int(power.group(1)), 'type': mtype.group(1)}
    return moves


def load_weights():
    weights = {}
    for key, body in parse_blocks(read('src/data/pokemon/pokedex_entries.h'), 'NATIONAL_DEX_'):
        m = re.search(r'\.weight\s*=\s*(\d+)', body)
        if m:
            weights['SPECIES_' + key[len('NATIONAL_DEX_'):]] = int(m.group(1))  # in 0.1 kg
    return weights


def load_megas():
    return re.findall(r'\{\s*(SPECIES_\w+)\s*,\s*ITEM_\w+\s*,\s*(SPECIES_\w+)\s*,', read('src/data/pokemon/mega_evolutions.h'))


def load_species_order():
    return [m for m in re.findall(r'#define (SPECIES_\w+) ', read('include/constants/species.h'))]


def kanto_species():
    """Species the player can meet in Kanto, the Sevii Islands and Mega Island."""
    groups = json.loads(read('data/maps/map_groups.json'))
    maps = [m for g in groups['group_order'] if 'Kanto' in g for m in groups[g]]
    map_ids = set()
    found = set()
    for m in maps:
        map_ids.add(json.loads(read('data/maps/%s/map.json' % m))['id'])
        path = 'data/maps/%s/scripts.inc' % m
        if os.path.exists(os.path.join(ROOT, path)):
            found.update(re.findall(r'SPECIES_\w+', read(path)))
            trades = re.findall(r'INGAME_TRADE_\w+', read(path))
            for key, body in parse_blocks(read('src/data/trade.h'), 'INGAME_TRADE_'):
                if key in trades:
                    found.update(re.findall(r'\.species\s*=\s*(SPECIES_\w+)', body))
    wild = json.loads(read('src/data/wild_encounters.json'))
    for group in wild['wild_encounter_groups']:
        for enc in group['encounters']:
            if enc.get('map') in map_ids:
                for field in ('land_mons', 'water_mons', 'rock_smash_mons', 'fishing_mons'):
                    for mon in enc.get(field, {}).get('mons', []):
                        found.add(mon['species'])
    found.discard('SPECIES_NONE')
    found.discard('SPECIES_EGG')
    return found


# ---------------------------------------------------------------------------------------------
# Card design

TYPE_TO_CARD = {
    'TYPE_NORMAL': 'C', 'TYPE_FLYING': 'C', 'TYPE_FIGHTING': 'F', 'TYPE_GROUND': 'F', 'TYPE_ROCK': 'F',
    'TYPE_POISON': 'D', 'TYPE_DARK': 'D', 'TYPE_BUG': 'G', 'TYPE_GRASS': 'G', 'TYPE_GHOST': 'P',
    'TYPE_PSYCHIC': 'P', 'TYPE_FAIRY': 'P', 'TYPE_STEEL': 'M', 'TYPE_FIRE': 'R', 'TYPE_WATER': 'W',
    'TYPE_ICE': 'W', 'TYPE_ELECTRIC': 'L', 'TYPE_DRAGON': 'N', 'TYPE_MYSTERY': 'C',
}
CARD_TYPE_NAMES = {
    'G': 'CARD_TYPE_GRASS', 'R': 'CARD_TYPE_FIRE', 'W': 'CARD_TYPE_WATER', 'L': 'CARD_TYPE_LIGHTNING',
    'P': 'CARD_TYPE_PSYCHIC', 'F': 'CARD_TYPE_FIGHTING', 'D': 'CARD_TYPE_DARKNESS', 'M': 'CARD_TYPE_METAL',
    'N': 'CARD_TYPE_DRAGON', 'C': 'CARD_TYPE_COLORLESS', '-': 'CARD_TYPE_NONE',
}
STATUS_NAMES = {
    'ASLEEP': 'CARD_STATUS_ASLEEP', 'PARALYZED': 'CARD_STATUS_PARALYZED', 'CONFUSED': 'CARD_STATUS_CONFUSED',
    'POISONED': 'CARD_STATUS_POISONED', 'BURNED': 'CARD_STATUS_BURNED',
}
EFFECT_NAMES = [
    'NONE', 'STATUS', 'FLIP_STATUS', 'HEAL_SELF', 'SELF_DAMAGE', 'FLIP_BONUS', 'FLIP_OR_FAIL',
    'DISCARD_ENERGY', 'DISCARD_ALL_ENERGY', 'FLIP_MULTI', 'SNIPE', 'SPREAD_BENCH', 'SELF_BENCH_DAMAGE',
    'FLIP_DISCARD_ENERGY', 'CANT_ATTACK_NEXT', 'BONUS_EXTRA_ENERGY', 'DRAW', 'SWITCH_DEFENDER',
    'BONUS_IF_DAMAGED',
]
ABILITY_NAMES = [
    'NONE', 'HEAL_ALL', 'ATTACH_OWN_TYPE', 'POISON_DEFENDER', 'FLIP_SLEEP', 'DRIVE_OFF', 'SNIPE',
    'REDUCE_DAMAGE', 'COUNTER', 'FREE_RETREAT',
]
# Babies are Basic, and so are the Pokémon they evolve into (as in Pocket)
BABIES = {
    'SPECIES_PICHU', 'SPECIES_CLEFFA', 'SPECIES_IGGLYBUFF', 'SPECIES_TOGEPI', 'SPECIES_TYROGUE',
    'SPECIES_SMOOCHUM', 'SPECIES_ELEKID', 'SPECIES_MAGBY', 'SPECIES_AZURILL', 'SPECIES_WYNAUT',
}
LEGENDARIES = {
    'SPECIES_ARTICUNO', 'SPECIES_ZAPDOS', 'SPECIES_MOLTRES', 'SPECIES_MEWTWO', 'SPECIES_MEW',
    'SPECIES_RAIKOU', 'SPECIES_ENTEI', 'SPECIES_SUICUNE', 'SPECIES_LUGIA', 'SPECIES_HO_OH',
    'SPECIES_CELEBI', 'SPECIES_REGIROCK', 'SPECIES_REGICE', 'SPECIES_REGISTEEL', 'SPECIES_LATIAS',
    'SPECIES_LATIOS', 'SPECIES_KYOGRE', 'SPECIES_GROUDON', 'SPECIES_RAYQUAZA', 'SPECIES_JIRACHI',
    'SPECIES_DEOXYS',
}

# Battle effect -> (card effect, argument, damage change in %)
# The damage change pays for drawbacks and charges for extras.
MOVE_EFFECTS = {
    'EFFECT_POISON_HIT': ('FLIP_STATUS', 'POISONED', -10),
    'EFFECT_POISON_FANG': ('STATUS', 'POISONED', -25),
    'EFFECT_POISON_TAIL': ('FLIP_STATUS', 'POISONED', -10),
    'EFFECT_BURN_HIT': ('FLIP_STATUS', 'BURNED', -10),
    'EFFECT_BLAZE_KICK': ('FLIP_STATUS', 'BURNED', -10),
    'EFFECT_THAW_HIT': ('FLIP_STATUS', 'BURNED', -10),
    'EFFECT_FREEZE_HIT': ('FLIP_STATUS', 'PARALYZED', -15),
    'EFFECT_PARALYZE_HIT': ('FLIP_STATUS', 'PARALYZED', -15),
    'EFFECT_THUNDER': ('FLIP_STATUS', 'PARALYZED', -15),
    'EFFECT_TRI_ATTACK': ('FLIP_STATUS', 'PARALYZED', -15),
    'EFFECT_CONFUSE_HIT': ('FLIP_STATUS', 'CONFUSED', -10),
    'EFFECT_ABSORB': ('HEAL_SELF', 'HALF', -10),
    'EFFECT_DRAINING_KISS': ('HEAL_SELF', 'HALF', -10),
    'EFFECT_RECOIL': ('SELF_DAMAGE', 'QUARTER', 20),
    'EFFECT_DOUBLE_EDGE': ('SELF_DAMAGE', 'THIRD', 25),
    'EFFECT_RECOIL_IF_MISS': ('FLIP_OR_FAIL', 0, 40),
    'EFFECT_FOCUS_PUNCH': ('FLIP_OR_FAIL', 0, 40),
    'EFFECT_EXPLOSION': ('SELF_DAMAGE', 50, 40),
    'EFFECT_MULTI_HIT': ('FLIP_MULTI', 3, 0),
    'EFFECT_DOUBLE_HIT': ('FLIP_MULTI', 2, 0),
    'EFFECT_TWINEEDLE': ('FLIP_MULTI', 2, 0),
    'EFFECT_TRIPLE_KICK': ('FLIP_MULTI', 3, 0),
    'EFFECT_FURY_CUTTER': ('FLIP_MULTI', 3, 0),
    'EFFECT_ROLLOUT': ('FLIP_MULTI', 3, 0),
    'EFFECT_QUICK_ATTACK': ('FLIP_BONUS', 10, -10),
    'EFFECT_HIGH_CRITICAL': ('FLIP_BONUS', 20, -15),
    'EFFECT_RECHARGE': ('CANT_ATTACK_NEXT', 0, 40),
    'EFFECT_RAMPAGE': ('CANT_ATTACK_NEXT', 0, 30),
    'EFFECT_EARTHQUAKE': ('SELF_BENCH_DAMAGE', 10, 15),
    'EFFECT_MAGNITUDE': ('SELF_BENCH_DAMAGE', 10, 15),
    'EFFECT_PAY_DAY': ('DRAW', 1, -15),
    'EFFECT_THIEF': ('DRAW', 1, -15),
    'EFFECT_KNOCK_OFF': ('FLIP_DISCARD_ENERGY', 0, -20),
    'EFFECT_OVERHEAT': ('DISCARD_ENERGY', 1, 30),
    'EFFECT_SUPERPOWER': ('DISCARD_ENERGY', 1, 30),
    'EFFECT_REVENGE': ('BONUS_IF_DAMAGED', 'BONUS', -20),
    'EFFECT_SMELLINGSALT': ('BONUS_IF_DAMAGED', 'BONUS', -20),
    'EFFECT_FACADE': ('BONUS_IF_DAMAGED', 'BONUS', -20),
    'EFFECT_ROAR': ('SWITCH_DEFENDER', 0, -10),
    'EFFECT_GUST': ('NONE', 0, 0),
}
# Moves that make no sense as a card attack
EXCLUDED_EFFECTS = {
    'EFFECT_OHKO', 'EFFECT_DREAM_EATER', 'EFFECT_SNORE', 'EFFECT_SUPER_FANG', 'EFFECT_DRAGON_RAGE',
    'EFFECT_SONICBOOM', 'EFFECT_LEVEL_DAMAGE', 'EFFECT_PSYWAVE', 'EFFECT_COUNTER', 'EFFECT_MIRROR_COAT',
    'EFFECT_BIDE', 'EFFECT_SPIT_UP', 'EFFECT_FUTURE_SIGHT', 'EFFECT_PRESENT', 'EFFECT_FLAIL',
    'EFFECT_RETURN', 'EFFECT_FRUSTRATION', 'EFFECT_HIDDEN_POWER', 'EFFECT_ENDEAVOR', 'EFFECT_FALSE_SWIPE',
    'EFFECT_FAKE_OUT', 'EFFECT_BEAT_UP', 'EFFECT_LOW_KICK', 'EFFECT_ERUPTION', 'EFFECT_UPROAR',
    'EFFECT_TRAP', 'EFFECT_SPLASH',
}
# Energy count by move power, and the damage an attack of that cost does (before the card's
# strength and the effect's change)
def cost_for_power(power):
    if power <= 35:
        return 1
    if power <= 60:
        return 2
    if power <= 95:
        return 3
    return 4


BASE_DAMAGE = {1: 20, 2: 50, 3: 80, 4: 120}
# The final attack of an evolved Pokémon hits harder, as it cost more cards to get there
STAGE_DAMAGE = (1.0, 1.1, 1.25)
# Most damage an attack can do for its Energy (ex: MAX_DAMAGE_EX), as in Pocket
MAX_DAMAGE = {1: 40, 2: 70, 3: 120, 4: 150}
MAX_DAMAGE_EX = {1: 50, 2: 90, 3: 150, 4: 180}


def round10(value, minimum=10):
    return max(minimum, int((value + 5) // 10) * 10)


def card_type(species_types):
    t1, t2 = (TYPE_TO_CARD[t] for t in species_types)
    if species_types[0] in ('TYPE_NORMAL', 'TYPE_FLYING') and t2 != 'C':
        return t2
    return t1


def weakness(ctype, species_types):
    if ctype == 'F':
        return 'G' if set(species_types) & {'TYPE_ROCK', 'TYPE_GROUND'} else 'P'
    if ctype == 'C':
        return 'L' if 'TYPE_FLYING' in species_types else 'F'
    return {'G': 'R', 'R': 'W', 'W': 'L', 'L': 'F', 'P': 'D', 'D': 'F', 'M': 'R', 'N': '-'}[ctype]


def retreat_cost(weight):
    kg = weight / 10
    if kg < 50:
        return 1
    if kg < 100:
        return 2
    if kg < 200:
        return 3
    return 4


def cost_list(ctype, count, heavy, move_type):
    """Energy for an attack: the card's type first, the rest Colorless."""
    if ctype == 'C' or move_type == 'TYPE_NORMAL':
        return 'C' * count
    if ctype == 'N':
        return ('C', 'WL', 'WLC', 'WLCC')[count - 1]
    typed = 2 if heavy and count >= 3 else 1
    return ctype * typed + 'C' * (count - typed)


class Card:
    pass


def design_attack(move_name, move, ctype, stage_role, card):
    """stage_role: 'small' (a Basic that evolves), 'mid' (a Stage 1 that evolves), 'big'."""
    power = move['power']
    count = cost_for_power(power)
    if stage_role == 'small':
        count = min(count, 2)
    elif stage_role == 'mid':
        count = min(max(count, 2), 3)
    else:
        count = max(count, 2)
    effect, arg, change = MOVE_EFFECTS.get(move['effect'], ('NONE', 0, 0))
    damage = BASE_DAMAGE[count] * (0.8 + 0.4 * card.strength) * (100 + change) / 100
    if stage_role == 'big':
        damage *= STAGE_DAMAGE[card.stage]
    if effect == 'FLIP_MULTI':
        damage = damage * 2 / arg
    attack = {
        'name': move_name,
        'cost': cost_list(ctype, count, stage_role == 'big' and card.strength > 0.6, move['type']),
        'damage': round10(damage),
        'effect': effect,
        'arg': arg,
    }
    if arg == 'HALF':
        attack['arg'] = round10(attack['damage'] / 2)
    elif arg == 'QUARTER':
        attack['arg'] = round10(attack['damage'] / 4)
    elif arg == 'THIRD':
        attack['arg'] = round10(attack['damage'] / 3)
    elif arg == 'BONUS':
        attack['arg'] = round10(attack['damage'] / 2)
    return attack


def move_card_type(move):
    return TYPE_TO_CARD[move['type']]


def pick_attacks(card, learnset, moves, ctype):
    candidates = []
    seen = set()
    for level, move_name in learnset:
        move = moves.get(move_name)
        if move is None or move_name in seen or move['power'] <= 1 or move['effect'] in EXCLUDED_EFFECTS:
            continue
        seen.add(move_name)
        candidates.append((level, move_name, move))
    if not candidates:
        candidates = [(1, 'MOVE_TACKLE', moves['MOVE_TACKLE'])]

    def stab(m):
        return move_card_type(m) == ctype or (ctype == 'N' and m['type'] == 'TYPE_DRAGON')

    if card.role == 'small':
        pool = [c for c in candidates if c[0] <= 25 and c[2]['power'] <= 70] or candidates[:1]
        best = max(pool, key=lambda c: (stab(c[2]), c[2]['power']))
        return [design_attack(best[1], best[2], ctype, 'small', card)]
    if card.role == 'mid':
        pool = [c for c in candidates if c[0] <= 45 and c[2]['power'] <= 90] or candidates
        best = max(pool, key=lambda c: (stab(c[2]), c[2]['power']))
        return [design_attack(best[1], best[2], ctype, 'mid', card)]
    big = max(candidates, key=lambda c: (stab(c[2]), c[2]['power']))
    attacks = [design_attack(big[1], big[2], ctype, 'big', card)]
    small = [c for c in candidates if c[1] != big[1] and c[2]['power'] <= 60]
    if card.bst >= 450 and small:
        first = max(small, key=lambda c: (c[2]['effect'] != 'EFFECT_HIT', stab(c[2]), c[2]['power']))
        attacks.insert(0, design_attack(first[1], first[2], ctype, 'small', card))
    return attacks


def hp_for(stats, bst, stage, is_ex):
    hp = 10 + stats['baseHP'] * 0.4 + bst * 0.12 + (0, 10, 40)[stage] + (30 if is_ex else 0)
    return min(250, max(30, round10(hp)))


def build_cards():
    species = load_species()
    pre = load_evolutions()
    learnsets = load_learnsets()
    moves = load_moves()
    weights = load_weights()
    megas = load_megas()
    order = load_species_order()

    # The pool: every species met in Kanto, with its whole evolution family
    pool = kanto_species()
    changed = True
    while changed:
        changed = False
        for s in list(pool):
            if s in pre and pre[s] not in pool:
                pool.add(pre[s])
                changed = True
        for child, parent in pre.items():
            if parent in pool and child not in pool and child in species:
                pool.add(child)
                changed = True
    pool = {s for s in pool if s in species and not s.startswith('SPECIES_UNOWN_') and s != 'SPECIES_NONE'}

    def evolves_from(s):
        p = pre.get(s)
        if p is None or p in BABIES or p not in pool:
            return None
        return p

    def stage_of(s):
        p = evolves_from(s)
        return 0 if p is None else 1 + stage_of(p)

    has_evolution = {evolves_from(s) for s in pool}

    cards = []
    by_species = {}
    for s in [s for s in order if s in pool]:
        card = Card()
        card.species = s
        card.id = 'CARD_' + s[len('SPECIES_'):]
        card.types = species[s]['types']
        card.bst = species[s]['bst']
        card.stage = stage_of(s)
        parent = evolves_from(s)
        card.evolves_from = 'CARD_' + parent[len('SPECIES_'):] if parent else None
        card.role = 'big' if s not in has_evolution else ('small' if card.stage == 0 else 'mid')
        card.is_ex = False
        card.is_mega = False
        card.strength = min(1.2, max(0.0, (card.bst - 300) / 300))
        card.learnset = learnsets.get(s, [])
        card.weight = weights.get(s, 300)
        cards.append(card)
        by_species[s] = card

    # Mega ex: they take the place of their base species, evolving from what it evolves from
    for base, mega in megas:
        if base not in species or mega not in species:
            continue
        parent = evolves_from(base) if base in pool else pre.get(base)
        if parent is not None and parent not in by_species:
            continue  # its line can't be played
        card = Card()
        card.species = mega
        card.id = 'CARD_' + mega[len('SPECIES_'):] + '_EX'
        card.types = species[mega]['types']
        card.bst = species[mega]['bst']
        card.stage = by_species[base].stage if base in by_species else (0 if parent is None else 1 + by_species[parent].stage)
        card.evolves_from = by_species[parent].id if parent else None
        card.role = 'big'
        card.is_ex = True
        card.is_mega = True
        card.strength = min(1.4, max(0.0, (card.bst - 300) / 300))
        card.learnset = learnsets.get(base, [])
        card.weight = weights.get(base, 300)
        cards.append(card)

    for card in cards:
        ctype = card_type(card.types)
        stats = species[card.species]['stats']
        card.type = ctype
        card.weakness = weakness(ctype, card.types)
        card.hp = hp_for(stats, card.bst, card.stage, card.is_ex)
        if card.species in LEGENDARIES and not card.is_ex:
            card.hp = min(card.hp, 130)
        card.retreat = retreat_cost(card.weight)
        card.attacks = pick_attacks(card, card.learnset, moves, ctype)
        for attack in card.attacks:
            if card.is_ex:
                attack['damage'] = round10(attack['damage'] + 20)
            limit = (MAX_DAMAGE_EX if card.is_ex else MAX_DAMAGE)[len(attack['cost'])]
            if attack['effect'] == 'FLIP_MULTI':
                limit = round10(limit / attack['arg'])
            attack['damage'] = min(attack['damage'], limit)
        card.ability = ('NONE', 0)
    return cards


def apply_overrides(cards, moves):
    path = os.path.join(ROOT, OVERRIDES)
    if not os.path.exists(path):
        return
    with open(path, encoding='utf-8') as f:
        overrides = json.load(f)
    by_id = {c.id: c for c in cards}
    for key, fields in overrides.items():
        if key.startswith('_'):
            continue
        card_id = 'CARD_' + key
        if card_id not in by_id:
            sys.exit('%s: unknown card %s' % (OVERRIDES, key))
        card = by_id[card_id]
        for field, value in fields.items():
            if field == 'attacks':
                card.attacks = [dict({'effect': 'NONE', 'arg': 0}, **a) for a in value]
                for attack in card.attacks:
                    if attack['name'].startswith('MOVE_') and attack['name'] not in moves:
                        sys.exit('%s: %s has unknown move %s' % (OVERRIDES, key, attack['name']))
            elif field == 'ability':
                card.ability = (value[0], value[1] if len(value) > 1 else 0)
            elif field in ('hp', 'retreat', 'weakness', 'type'):
                setattr(card, field, value)
            elif field.startswith('_'):
                pass
            else:
                sys.exit('%s: %s has unknown field %s' % (OVERRIDES, key, field))


# ---------------------------------------------------------------------------------------------
# Output

HEADER = '// Generated by tools/card_game/gen_cards.py from the game\'s data and\n' \
         '// src/data/card_game/pokemon_card_overrides.json. Do not edit by hand: rerun the script.\n'


def c_string_name(name):
    return 'sCardText_' + re.sub(r'[^A-Za-z0-9]', '', name.title())


def write_constants(cards):
    lines = [HEADER, '#ifndef GUARD_CONSTANTS_CARD_GAME_CARDS_H', '#define GUARD_CONSTANTS_CARD_GAME_CARDS_H', '']
    for i, card in enumerate(cards):
        lines.append('#define %-36s %d' % (card.id, i + 1))
    first_ex = next((i + 1 for i, card in enumerate(cards) if card.is_ex), len(cards) + 1)
    lines.append('#define %-36s %d  // the ex cards come last' % ('CARD_FIRST_EX', first_ex))
    lines.append('#define %-36s %d' % ('CARD_POKEMON_END', len(cards) + 1))
    lines += ['', '#endif // GUARD_CONSTANTS_CARD_GAME_CARDS_H', '']
    with open(os.path.join(ROOT, OUT_CONSTANTS), 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))


def effect_arg(attack):
    arg = attack['arg']
    if attack['effect'] in ('STATUS', 'FLIP_STATUS'):
        return STATUS_NAMES[arg]
    return str(arg)


def write_table(cards):
    custom = {}
    for card in cards:
        for attack in card.attacks:
            if not attack['name'].startswith('MOVE_'):
                custom[c_string_name(attack['name'])] = attack['name']
    lines = [HEADER]
    for var, name in sorted(custom.items()):
        lines.append('static const u8 %s[] = _("%s");' % (var, name.upper()))
    if custom:
        lines.append('')
    lines.append('static const struct PokemonCardInfo sPokemonCards[CARD_POKEMON_END] =')
    lines.append('{')
    for card in cards:
        for attack in card.attacks:
            if attack['effect'] not in EFFECT_NAMES:
                sys.exit('%s: unknown effect %s' % (card.id, attack['effect']))
            if len(attack['cost']) > 5 or any(c not in CARD_TYPE_NAMES or c == '-' for c in attack['cost']):
                sys.exit('%s: bad cost %s' % (card.id, attack['cost']))
        if card.ability[0] not in ABILITY_NAMES:
            sys.exit('%s: unknown ability %s' % (card.id, card.ability[0]))
        lines.append('    [%s] =' % card.id)
        lines.append('    {')
        lines.append('        .species = %s,' % card.species)
        lines.append('        .evolvesFrom = %s,' % (card.evolves_from or 'CARD_NONE'))
        lines.append('        .hp = %d,' % card.hp)
        lines.append('        .type = %s,' % CARD_TYPE_NAMES[card.type])
        lines.append('        .weakness = %s,' % CARD_TYPE_NAMES[card.weakness])
        lines.append('        .stage = CARD_STAGE_%s,' % ('BASIC', '1', '2')[card.stage])
        lines.append('        .retreatCost = %d,' % card.retreat)
        if card.is_ex:
            lines.append('        .isEx = TRUE,')
        if card.is_mega:
            lines.append('        .isMega = TRUE,')
        lines.append('        .numAttacks = %d,' % len(card.attacks))
        lines.append('        .attacks =')
        lines.append('        {')
        for attack in card.attacks:
            name = 'gMoveNames[%s]' % attack['name'] if attack['name'].startswith('MOVE_') else c_string_name(attack['name'])
            cost = ', '.join(CARD_TYPE_NAMES[c] for c in attack['cost'])
            lines.append('            {%s, {%s}, %d, CARD_EFFECT_%s, %s},' % (name, cost, attack['damage'], attack['effect'], effect_arg(attack)))
        lines.append('        },')
        if card.ability[0] != 'NONE':
            lines.append('        .ability = CARD_ABILITY_%s,' % card.ability[0])
            lines.append('        .abilityArg = %d,' % card.ability[1])
        lines.append('    },')
    lines.append('};')
    lines.append('')
    with open(os.path.join(ROOT, OUT_TABLE), 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))


def main():
    cards = build_cards()
    apply_overrides(cards, load_moves())
    write_constants(cards)
    write_table(cards)
    print('%d Pokémon cards (%d ex)' % (len(cards), sum(c.is_ex for c in cards)))


if __name__ == '__main__':
    main()
