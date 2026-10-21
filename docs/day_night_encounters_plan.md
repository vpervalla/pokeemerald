# Plan: Day and night wild encounters

This is a plan for making wild POKéMON depend on the time of day, in Kanto, the Sevii Islands
and Hoenn: HOOTHOOT instead of PIDGEY on Kanto's routes at night, LEDYBA by day, VOLBEAT and
ILLUMISE swarming Route 117 after dark. It uses the day/night phases the game already has
(`src/day_night.c`); no new graphics, and nothing in the save.

## Status

Phases 1 to 4 are implemented: the engine, the script that writes the tables, and 38 timed
tables (the Test Area, 16 Kanto maps, 3 Sevii maps, 13 Hoenn maps). The ROM builds (agbcc and
modern).

Checked in mGBA (libmgba) with a test build that starts a new game in the Test Area's grass with
a MEWTWO: the first wild POKéMON, three runs per case, walking left and right until a battle.

| Clock | VAR_DAY_NIGHT_OVERRIDE | Wild POKéMON |
|---|---|---|
| 07:30 (morning) | none | MEW, MEW, MEW |
| 12:30 (day) | none | MEW, MEW, MEW |
| 18:30 (evening) | none | HOOTHOOT, HOOTHOOT, HOOTHOOT |
| 22:30 (night) | none | HOOTHOOT, HOOTHOOT, HOOTHOOT |
| 03:30 (night) | none | HOOTHOOT, HOOTHOOT, HOOTHOOT |
| 22:30 | MORNING | MEW, MEW, MEW |
| 22:30 | DAY | MEW, MEW, MEW |
| 12:30 | EVENING | HOOTHOOT, HOOTHOOT, HOOTHOOT |
| 12:30 | NIGHT | HOOTHOOT, HOOTHOOT, HOOTHOOT |

The other maps' tables, Sweet Scent, fishing, Rock Smash and the Pokédex area haven't been
checked in-game yet.

Differences from the first version of this plan:

- **Two times of day, not four phases** (D1): morning and day share the **day** tables, evening
  and night the **night** tables. A map has its own table, plus at most one day and one night
  table, so no masks: `WILD_TIME_ANY`, `WILD_TIME_DAY`, `WILD_TIME_NIGHT`.
- **Outdoor maps and forests only** (D3): checked by the script on the map's type (towns, cities,
  routes; the forests are routes).
- **New species only in Kanto** (D4): a Hoenn table can only give more slots to species already in
  that map's own table. DUSKULL is not added to the Mt. Pyre exterior.
- **One script** (section 4): `tools/day_night_encounters/apply.py` holds the tables and checks
  them; no separate `tables.py`, `check.py` or host test.

## Where things are now

- **Phases**: `DayNight_GetPhase()` (`include/day_night.h`) returns one of four
  `DAY_NIGHT_PHASE_*` (`include/constants/day_night.h`): morning 6-10, day 10-17, evening 17-20,
  night 20-6. An RTC error gives `DAY`. The Test Area's time changer sets
  `VAR_DAY_NIGHT_OVERRIDE`, which `DayNight_GetPhase()` already honours. The tint and the dynamic
  weather use the same phases.
- **Tables**: `src/data/wild_encounters.json` → `jsonproc` with `wild_encounters.json.txt` →
  `src/data/wild_encounters.h` (`json_data_rules.mk`). One group, `gWildMonHeaders`, holds the
  tables of both regions: Hoenn from Emerald (plus `tools/hoenn/add_missing_wild.py`), Kanto and
  the Sevii Islands ported from FireRed (`tools/kanto_port/port_kanto.py encounters`), then
  LeafGreen's exclusives (`tools/kanto_port/add_leafgreen_exclusives.py`). Land tables have
  12 slots (20/20/10/10/10/10/5/5/4/4/1/1).
- **Header**: `struct WildPokemonHeader` (`include/wild_encounter.h`) is a map plus four
  `WildPokemonInfo` pointers. Altering Cave already has nine entries for one map, picked with
  `VAR_ALTERING_CAVE_WILD_SET` in `GetCurrentMapWildMonHeaderId()`.
- **Users of the header**: `src/wild_encounter.c`, `src/pokedex_area_screen.c` (scans every
  entry for a species) and `src/match_call.c` (first entry of a map).

## 0. Decisions

| # | Decision | Choice |
|---|----------|--------|
| D1 | Times of day | Two: **day** (morning and day, 6-17) and **night** (evening and night, 17-6) |
| D2 | Own table | The map's current table stays. A day or night table **overrides some fields** (land, water, Rock Smash, fishing); the others come from the map's own table. A map with no timed table is unchanged |
| D3 | Where | Outdoor maps and forests: towns, cities, routes (Viridian Forest, Petalburg Woods and the Berry Forest are routes), and the Test Area. Caves, towers, buildings, Safari Zones and the Battle Frontier stay fixed |
| D4 | Species | Kanto and the Sevii Islands get Johto species, as in HeartGold/SoulSilver (HOOTHOOT, NOCTOWL, LEDYBA, SPINARAK, MURKROW) and GASTLY. Hoenn gets no new species on any map: only more slots for its nocturnal ones at night |
| D5 | No species lost | Every species of a map's own table is still on that map by day or by night |
| D6 | Levels and rate | A slot keeps its levels; the table keeps the map's encounter rate |
| D7 | No clock | RTC error → `DAY`, so the day tables. The Test Area's time changer applies |
| D8 | Which phase | `DayNight_GetPhase()`, so encounters change with the tint |
| D9 | Pokédex area | Shows a species on every map where it appears, by day or by night |
| D10 | Unchanged systems | Roamers, mass outbreaks, Feebas, Altering Cave, the Tanoby chambers and the Battle Pyramid / Pike |
| D11 | Source of truth | The timed tables are written as changes to the map's own table in `apply.py`, which rebuilds them each time |

## 1. Constants and data

`include/constants/day_night.h`: `WILD_TIME_ANY` (0, a map's own table), `WILD_TIME_DAY` (1),
`WILD_TIME_NIGHT` (2).

A timed table is an extra entry for the same map, right after the map's own table, with a `time`
key and only the fields it overrides:

```json
{
  "map": "MAP_ROUTE1",
  "base_label": "gRoute1_Night",
  "time": "WILD_TIME_NIGHT",
  "land_mons": { "encounter_rate": 20, "mons": [ ... 12 slots ... ] }
}
```

The template gives every header `.timeOfDay` (`WILD_TIME_ANY` when there is no `time` key, so
the Battle Pyramid and Pike get it too), and `struct WildPokemonHeader` has `u8 timeOfDay` after
`mapNum` (in the padding: the struct keeps its size).

**Porymap** shows the timed entries as extra tables of the map. Edit only the maps' own tables
there, then run `apply.py --write`.

## 2. The code

All in `src/wild_encounter.c`:

- `GetCurrentMapWildMonHeaderId()` returns the map's own table (`WILD_TIME_ANY`), plus the
  Altering Cave offset as before.
- `GetWildMonsInfo(headerId, area)`: looks at the entries right after `headerId` for the same
  map; one for the current time of day (`GetWildMonsTimeOfDay()`: morning and day → `DAY`,
  evening and night → `NIGHT`) that has the area wins, else the map's own table's pointer (which
  can be NULL, as before).
- Walking and surfing (`StandardWildEncounter`), Rock Smash, Sweet Scent, fishing
  (`DoesCurrentMapHaveFishingMons`, `FishingWildEncounter`) and `GetLocalWildMon` /
  `GetLocalWaterMon` (TV, Match Call) all go through `GetWildMonsInfo`.
- `src/match_call.c` takes the first entry of a map, the map's own table. `src/pokedex_area_screen.c`
  scans every entry, so night-only species show up on the area map (D9). Neither changed.

## 3. Tools

`tools/day_night_encounters/apply.py` holds the tables (`TABLES`) as changes to the map's own
table: `(old, new)` for every slot of a species, `([slots], old, new)` for some of them (it stops
if a slot isn't `old`). It removes the existing timed tables, builds them again, checks them and,
with `--write`, writes them right after each map's own table. The checks: the map has a table and
is outdoors (D3), a Hoenn table adds no species to its map (D4), no species of the map's own table
is lost (D5), one table per map, time and field. Its region is the map section's, as in
`GetMapSecRegion`.

After rewriting the maps' own tables, run the scripts in this order:

    python3 tools/kanto_port/port_kanto.py encounters
    python3 tools/kanto_port/add_leafgreen_exclusives.py --write
    python3 tools/hoenn/add_missing_wild.py --write
    python3 tools/day_night_encounters/apply.py --write

`add_missing_wild.py` now skips the timed tables when it looks a map up.

## 4. Tables

All on the land. Slot numbers are in `apply.py`.

### Kanto, the Sevii Islands and the Test Area

| Map | Day | Night |
|---|---|---|
| Test Area | | MEW → HOOTHOOT |
| Route 1 | | PIDGEY → HOOTHOOT |
| Route 2 | one CATERPIE slot (1%) → LEDYBA | PIDGEY → HOOTHOOT, WEEDLE → SPINARAK |
| Viridian Forest | | two WEEDLE slots (30%) → SPINARAK |
| Route 22 | | SPEAROW → RATTATA |
| Routes 24, 25 | one METAPOD slot (1%) → LEDYBA | PIDGEY → HOOTHOOT, WEEDLE → SPINARAK |
| Routes 5, 6 | | PIDGEY → HOOTHOOT |
| Route 7 | | PIDGEY → MURKROW |
| Route 8 | | PIDGEY → HOOTHOOT, its VULPIX slot → GASTLY |
| Route 10 | | two SPEAROW slots → GASTLY |
| Route 11 | | SPEAROW → HOOTHOOT |
| Route 12 | | PIDGEY → HOOTHOOT |
| Routes 13, 14, 15 | | PIDGEY → HOOTHOOT, PIDGEOTTO → NOCTOWL |
| Route 16 | | SPEAROW → MURKROW |
| Three Isle Berry Forest | | PIDGEY → HOOTHOOT, PIDGEOTTO → NOCTOWL |
| Five Isle Meadow | | PIDGEY and SENTRET → HOOTHOOT, PIDGEOTTO → NOCTOWL |
| Six Isle Water Path | one SENTRET slot → LEDYBA | SENTRET → SPINARAK |

NOCTOWL, wild nowhere before, can be caught at night on Routes 13-15, in the Berry Forest and the
Five Isle Meadow.

### Hoenn

Night only; the day uses the map's own table.

| Map | Night |
|---|---|
| Route 101 | two WURMPLE slots → POOCHYENA |
| Route 102 | a LOTAD slot → SEEDOT, a ZIGZAGOON slot → POOCHYENA |
| Route 103 | two ZIGZAGOON slots → POOCHYENA |
| Route 104 | a TAILLOW and a WINGULL slot → POOCHYENA |
| Petalburg Woods | a TAILLOW slot → SHROOMISH, a WURMPLE slot → POOCHYENA |
| Route 110 | two WINGULL slots → ODDISH, GULPIN |
| Route 116 | two TAILLOW slots → WHISMUR, NINCADA |
| Route 117 | VOLBEAT 30%, ILLUMISE 29% (from POOCHYENA, ODDISH and ILLUMISE slots) |
| Route 119 | two TROPIUS slots → ODDISH |
| Route 120 | two MARILL slots → POOCHYENA, MIGHTYENA |
| Routes 121, 123 | two WINGULL slots → SHUPPET, ODDISH |
| Mt. Pyre Exterior | two WINGULL slots → SHUPPET |

## 5. Phases

1. **Engine** (done): constants, the header field, the template, `GetWildMonsInfo` and its
   callers.
2. **Script** (done): `apply.py`, with the Test Area table.
3. **Kanto and the Sevii Islands** (done): the tables of section 4.
4. **Hoenn** (done): the tables of section 4.
5. **In-game pass**: the checks of section 7, then tune the tables.
6. **Hints (optional)**: a Bug Catcher on Route 2 about LEDYBA, a hiker on Route 117 about the
   lights at night, a Pokémon Center NPC in Celadon about NOCTOWL.

## 6. Later, not in this plan

- Timed tables for water, fishing and Rock Smash (the engine supports them; no table uses them
  yet).
- A day/night marker on the Pokédex area screen.
- Weather-dependent encounters on top of the dynamic weather (`docs/dynamic_weather_plan.md`).
- Evolutions by time of day (ESPEON/UMBREON).

## 7. Tests

- `python3 tools/day_night_encounters/apply.py` checks the tables and prints each one's changes;
  "Nothing to change" when the JSON is up to date.
- `make` and `make modern`.
- In-game, with the Test Area's time changer: HOOTHOOT at night and MEW by day on the Test Area;
  the evening counts as night, the morning as day; Route 1, Route 2, Viridian Forest, Route 117;
  Sweet Scent, a Repel, a rod and Rock Smash on a map with a night land table (they must still use
  the right field); Altering Cave and the Tanoby chambers unchanged; the Pokédex area of HOOTHOOT
  shows Route 1.
