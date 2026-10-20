# Plan: Day and night wild encounters

This is a plan for making wild POKéMON depend on the time of day, in Kanto, the Sevii Islands
and Hoenn: HOOTHOOT instead of PIDGEY on Kanto's routes at night, LEDYBA in the morning, VOLBEAT
and ILLUMISE swarming Route 117 after dark. It uses the day/night phases the game already has
(`src/day_night.c`); no new graphics, and nothing in the save.

## Where things are now

- **Phases**: `DayNight_GetPhase()` (`include/day_night.h`) returns one of four
  `DAY_NIGHT_PHASE_*` (`include/constants/day_night.h`): morning 6-10, day 10-17, evening 17-20,
  night 20-6. An RTC error gives `DAY`. The Test Area's time changer sets
  `VAR_DAY_NIGHT_OVERRIDE`, which `DayNight_GetPhase()` already honours. The tint and the dynamic
  weather use the same phases.
- **Tables**: `src/data/wild_encounters.json` → `jsonproc` with `wild_encounters.json.txt` →
  `src/data/wild_encounters.h` (`json_data_rules.mk`). One group, `gWildMonHeaders`, holds 257
  entries for both regions: Hoenn from Emerald, Kanto and the Sevii Islands ported from FireRed
  (`tools/kanto_port/port_kanto.py encounters`), then LeafGreen's exclusives added by
  `tools/kanto_port/add_leafgreen_exclusives.py` (`docs/catch_them_all.md`). Land tables have
  12 slots (20/20/10/10/10/10/5/5/4/4/1/1), water and Rock Smash 5, fishing 10.
- **Header**: `struct WildPokemonHeader` (`include/wild_encounter.h`) is a map plus four
  `WildPokemonInfo` pointers. A map can already have several entries: Altering Cave (both
  copies) has nine, one after the other, picked with `VAR_ALTERING_CAVE_WILD_SET` in
  `GetCurrentMapWildMonHeaderId()` (`src/wild_encounter.c:333`).
- **Users of the header**: `src/wild_encounter.c` (walking, surfing, Rock Smash, Sweet Scent,
  fishing, `GetLocalWildMon`/`GetLocalWaterMon` for TV and Match Call),
  `src/pokedex_area_screen.c` (scans every entry for a species) and `src/match_call.c:1754`
  (first entry of a map). Every wild-encounter path goes through `headerId` and reads one of the
  four pointers.

## 0. Decisions

| # | Decision | Proposal |
|---|----------|----------|
| D1 | Phases | The four existing phases. A timed table names the phases it covers (a mask), so "night" or "morning" or "morning + day" are all one table. Evening uses the base table unless a table names it |
| D2 | Base table | The current table stays as the base. A timed table **overrides one field** (land, water, Rock Smash, fishing); the fields it doesn't have fall back to the base. A map with no timed table is unchanged |
| D3 | Where | Outdoor routes and the two forests (Viridian Forest, Petalburg Woods) in both regions. Caves, towers, buildings, Safari Zones, the Battle Frontier and Mega Island stay fixed in this plan |
| D4 | Species pool | Kanto/Sevii may add Johto species, as HeartGold/SoulSilver did (HOOTHOOT, NOCTOWL, LEDYBA, SPINARAK, MURKROW…). Hoenn only uses species already wild in Hoenn, mostly re-weighting the slots that are there |
| D5 | No species lost | Every species in a map's base table stays catchable **on that map** at some phase. A timed table may not cover all four phases of a field, so the base is always reachable. Checked by a script (section 4) |
| D6 | Levels | A swapped slot keeps the level range of the slot it replaces, so difficulty doesn't change with the clock |
| D7 | No clock | RTC error → `DAY` (as now), so the base or day tables. The Test Area override applies, which is how it is tested |
| D8 | Which phase | `DayNight_GetPhase()`, so encounters change together with the tint. Encounters only happen in the field, where its phase is current |
| D9 | Pokédex area | Shows a species on every map where it appears at any phase. No time marker in this plan (section 7) |
| D10 | Unchanged systems | Roamers, mass outbreaks, Feebas, Altering Cave, the Tanoby Unown chambers and the Battle Pyramid / Pike keep their logic. Altering Cave and Tanoby get no timed tables |
| D11 | Source of truth | The timed tables are written as **swaps relative to the base** in a spec file, and a script writes them into `wild_encounters.json`. Re-porting Kanto, then running the LeafGreen script, then this one, gives the same result |

## 1. Constants

`include/constants/day_night.h`, after `DAY_NIGHT_PHASE_COUNT`:

```c
// Masks of phases, for the wild encounter tables that only apply at some times of day.
#define DAY_NIGHT_MASK(phase)    (1 << (phase))
#define DAY_NIGHT_MASK_MORNING   DAY_NIGHT_MASK(DAY_NIGHT_PHASE_MORNING)
#define DAY_NIGHT_MASK_DAY       DAY_NIGHT_MASK(DAY_NIGHT_PHASE_DAY)
#define DAY_NIGHT_MASK_EVENING   DAY_NIGHT_MASK(DAY_NIGHT_PHASE_EVENING)
#define DAY_NIGHT_MASK_NIGHT     DAY_NIGHT_MASK(DAY_NIGHT_PHASE_NIGHT)
#define DAY_NIGHT_MASK_ALL       (DAY_NIGHT_MASK(DAY_NIGHT_PHASE_COUNT) - 1)
```

A mask of 0 means a base table (every phase).

## 2. Data

**JSON.** A timed table is an extra entry for the same map, right after its base entry, with a
`phases` key and only the fields it overrides:

```json
{
  "map": "MAP_ROUTE1",
  "base_label": "gRoute1_Night",
  "phases": "DAY_NIGHT_MASK_NIGHT",
  "land_mons": { "encounter_rate": 20, "mons": [ ... 12 slots ... ] }
}
```

The `encounter_rate` is copied from the base, so the clock changes who appears, not how often.

**Template** (`wild_encounters.json.txt`): each header gets
`.phases = {% if existsIn(encounter, "phases") %}{{ encounter.phases }}{% else %}0{% endif %},`.
The Battle Pyramid and Pike groups use the same loop and get 0. `wild_encounter.c` includes
`constants/day_night.h` before `data/wild_encounters.h`.

**Header** (`include/wild_encounter.h`): `u8 phases;` after `mapNum` (it fits in the padding, so
the struct keeps its size).

**Porymap** keeps unknown keys but would show the timed entries as extra tables of the map;
edit encounters with Porymap only on base tables, and run the script afterwards (section 4).

## 3. The code

All in `src/wild_encounter.c`.

1. `GetCurrentMapWildMonHeaderId()` keeps returning the **base** entry (the first entry of the
   map with `phases == 0`, plus the Altering Cave offset). Altering Cave's nine entries all have
   `phases == 0`, so nothing changes there.
2. New `static const struct WildPokemonInfo *GetWildMonsInfo(u16 headerId, u8 field)`:
   - `field` is a new `WILD_FIELD_LAND/WATER/ROCK_SMASH/FISHING`.
   - Looks at the entries right after `headerId` that have the same map and `phases != 0`; the
     first whose mask has `DayNight_GetPhase()` and a non-NULL pointer for `field` wins.
   - Otherwise returns the base entry's pointer (which can be NULL, as now).
3. Every `gWildMonHeaders[headerId].xxxMonsInfo` read in `StandardWildEncounter`,
   `RockSmashWildEncounter`, `SweetScentWildEncounter`, `DoesCurrentMapHaveFishingMons`,
   `FishingWildEncounter`, `GetLocalWildMon` and `GetLocalWaterMon` goes through
   `GetWildMonsInfo`. That's about 15 call sites, all mechanical.
4. `TryCreateTanobyUnown` and Feebas use slot numbers / map positions; they keep working because
   their maps have no timed tables (D10).
5. `src/match_call.c:1754` takes the first entry of a map, which is the base: fine, a trainer
   talking about "the POKéMON around here" uses the base table.
6. `src/pokedex_area_screen.c` already scans every entry, so night-only species show up (D9). The
   loop over `gWildMonHeaders` is unchanged.

A timed table and a base table can't both be "the first entry of the map" because the base comes
first in the JSON; the script keeps that order and the check enforces it.

## 4. Tools

`tools/day_night_encounters/`:

- **`tables.py`**: the spec. One record per timed table, written against the base table so it
  survives a re-port:

  ```python
  Timed("MAP_ROUTE1", "land_mons", NIGHT, swap={"PIDGEY": "HOOTHOOT"}),
  Timed("MAP_ROUTE2", "land_mons", MORNING, swap={"CATERPIE": "LEDYBA"}),
  Timed("MAP_ROUTE117", "land_mons", NIGHT, slots={0: "VOLBEAT", 1: "ILLUMISE", ...}),
  ```

  `swap` replaces every slot of a species (keeping its levels, D6); `slots` sets single slots by
  index, for re-weighting. Both can be used together.
- **`apply.py --write`**: reads the base entries from `wild_encounters.json`, removes any
  existing timed entries, builds each timed table and inserts it right after its base. Without
  `--write` it prints the diff. Run order after a re-port:
  `port_kanto.py encounters` → `add_leafgreen_exclusives.py --write` → `apply.py --write`.
- **`check.py`** (also run by `apply.py`): every timed entry follows a base entry of the same map;
  slot counts match the base; the masks of one field's timed tables don't overlap and don't cover
  all four phases (D5); every base species of the map stays in some phase (D5); every species
  wild in the game before still is; labels are unique.
- **`run_tests.sh`**: a host test, like `tools/dynamic_weather/run_tests.sh`, that builds the
  lookup of section 3 against a small fake header table (base only, night override, morning and
  night, a field missing from the timed table, Altering Cave offset).

## 5. Tables

Two shapes are used: a **swap** (a species is replaced at some phases) and a **re-weight** (the
same species, with more slots for the nocturnal ones at night). Slot levels stay as in the base.
The species below are proposals to tune when playing.

### Kanto and the Sevii Islands

| Map | Morning | Night |
|---|---|---|
| Route 1 | | PIDGEY → HOOTHOOT |
| Route 2 | CATERPIE → LEDYBA | PIDGEY → HOOTHOOT, WEEDLE → SPINARAK |
| Viridian Forest | | WEEDLE/KAKUNA slots → SPINARAK, PIKACHU kept |
| Route 22 | | SPEAROW → RATTATA slots (re-weight) |
| Routes 24, 25 | CATERPIE → LEDYBA | PIDGEY → HOOTHOOT, ABRA/ODDISH/BELLSPROUT up (re-weight) |
| Route 5, 6 | | PIDGEY → HOOTHOOT, MEOWTH up |
| Route 7 | | PIDGEY → MURKROW, MEOWTH up |
| Route 8 | | PIDGEY → HOOTHOOT, GROWLITHE/VULPIX slot → GASTLY |
| Route 10 | | SPEAROW → GASTLY |
| Route 11 | | SPEAROW → HOOTHOOT, DROWZEE up |
| Routes 12, 13 | | ODDISH/GLOOM and VENONAT up, PIDGEY → HOOTHOOT (12), PIDGEOTTO → NOCTOWL (13) |
| Routes 14, 15 | | VENONAT up, PIDGEOTTO → NOCTOWL |
| Routes 16, 17, 18 | | SPEAROW → MURKROW (16), RATTATA/RATICATE up |
| Three Isle Berry Forest | | DROWZEE, VENONAT up |
| Five Isle Meadow | SENTRET up | SENTRET → HOOTHOOT |
| Six Isle Water Path | SPEAROW → LEDYBA | SPEAROW → SPINARAK |

NOCTOWL, wild nowhere now, becomes catchable at night on Routes 13-15. HOOTHOOT, LEDYBA and
SPINARAK, now only in the Hoenn Safari Zone and Pattern Bush, appear early in Kanto.

### Hoenn

| Map | Morning / day | Night |
|---|---|---|
| Route 101 | ZIGZAGOON, WURMPLE up | POOCHYENA up (WURMPLE kept at 1 slot) |
| Route 102 | LOTAD up | SEEDOT up, POOCHYENA up |
| Route 103 | ZIGZAGOON up | POOCHYENA up |
| Route 104 | TAILLOW, WINGULL up | POOCHYENA up, TAILLOW down |
| Petalburg Woods | WURMPLE, TAILLOW up | SHROOMISH, POOCHYENA up |
| Route 110 | WINGULL up | ODDISH, GULPIN up |
| Route 116 | TAILLOW up | WHISMUR, NINCADA up |
| Route 117 | MARILL, ODDISH up | VOLBEAT and ILLUMISE take 8 slots |
| Route 119 | TROPIUS up | ODDISH up, TROPIUS down (kept) |
| Route 120 | | ODDISH, POOCHYENA/MIGHTYENA up (ABSOL kept) |
| Route 121 | | ODDISH, SHUPPET up |
| Route 123 | | SHUPPET, ODDISH up |
| Mt. Pyre Exterior | | SHUPPET up, WINGULL → DUSKULL |

DUSKULL on the exterior is the only Hoenn species moved to a new map; it's already on Mt. Pyre.
The exact slot numbers are written in `tables.py`; `check.py` makes sure no species of a base
table disappears.

## 6. Phases

1. **Engine.** Constants, the header field, the template, `GetWildMonsInfo` and its call sites,
   the host test. One hand-written night table on the Test Area (its only wild POKéMON is MEW;
   at night, HOOTHOOT) to try it with the Test Area's time changer. The game builds and matches nothing else's behaviour: with
   no timed tables, every lookup returns the base.
2. **Tools.** `tables.py`, `apply.py`, `check.py`; the Test Area table moves into the spec.
3. **Kanto and the Sevii Islands.** The tables of section 5, then a pass in-game at each phase
   with the time changer.
4. **Hoenn.** The tables of section 5. Check Route 117 and Mt. Pyre with the time changer.
5. **Hints (optional).** A line or two from NPCs where it matters: a Bug Catcher on Route 2 about
   LEDYBA in the morning, a hiker on Route 117 about the lights at night, a Pokémon Center NPC
   in Celadon about NOCTOWL.

## 7. Later, not in this plan

- Timed tables for water, fishing and Rock Smash (the engine supports them; no table uses them
  yet), and for caves and towers (GASTLY at night in Pokémon Tower…).
- A day/night marker on the Pokédex area screen.
- Weather-dependent encounters on top of the dynamic weather (`docs/dynamic_weather_plan.md`,
  section 8).
- Evolutions by time of day (ESPEON/UMBREON already need it in Gen 2+).

## 8. Tests

- `tools/day_night_encounters/run_tests.sh` (lookup) and `check.py` (data), run after every
  change to the tables.
- `make` and `make compare`-style sanity: the build must succeed with and without timed tables;
  `wild_encounters.h` regenerates when the JSON or template changes (already a dependency).
- In-game, with the Test Area's time changer: each phase on the Test Area, Route 1, Route 2,
  Route 117 and Mt. Pyre Exterior; Sweet Scent, a Repel, a Super Rod and Rock Smash on a map with
  a timed land table (they must still use the right field); Altering Cave and the Tanoby chambers
  unchanged; the Pokédex area of HOOTHOOT shows Route 1.

## Status

Not started.
