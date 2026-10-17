# Plan: Dynamic weather

This is a plan for giving Kanto, the Sevii Islands and Mega Island weather that changes
with the real-time clock: clear mornings, afternoon showers, evening storms, fog over
the sea, snow on Four Island. It builds on the weather engine that is already in the
game; no new weather effects are drawn.

## Status

Phase 1 is implemented: the engine, the story gate, the `VIRIDIAN` zone (Pallet Town,
Route 1, Viridian City, Route 2, Route 22) and a Test Area debug entry. The draw has host
tests (`tools/dynamic_weather/run_tests.sh`).

Phase 2 is implemented: the weather fades to the next phase's while the player stays on a
dynamic map, a weather set by a script or a coord event stays, and the change waits while a
script, a menu or a fade has the screen.

Phase 3 is implemented: all 14 zones of section 2 with the weights of section 3, on 65
maps. The host tests cover every zone. Nothing has been tested in-game yet, and snow has
not been looked at on Four Island (section 5).

Differences from the plan below:

- **Phases, not 6-hour periods** (D3): the game already has a day/night tint
  (`src/day_night.c`) with four phases: morning 6-10, day 10-17, evening 17-20, night
  20-6. The weather uses the same phases, so the weather and the tint change together, and
  the Test Area's time changer (`VAR_DAY_NIGHT_OVERRIDE`) also steps the weather. A pattern
  is in the order morning, day, evening, night, and the weather day starts at 06:00, so a
  night that runs past midnight keeps one weather. `DayNight_GetPhaseNow()` reads the
  clock at the moment, because the tint's own phase can be stale during a map load.
- **Files**: the tables and the draw are in `src/dynamic_weather_rules.c` (no game state,
  so the host tests can build it), the game side in `src/dynamic_weather.c`.
- **Hash**: MurmurHash3's 32-bit finalizer, twice, over `(trainer ID ^ day) + zone`.
- **No clock** (`FLAG_SYS_CLOCK_SET` clear, or an RTC error): the maps use their zone's
  fallback (clear), not a stuck pattern.
- **Test Area**: a new NPC (east of the card tester) reports the `VIRIDIAN` zone's day,
  pattern, the weather of each phase and the current phase, ignoring the story gate, then
  shows the current weather on the Test Area. It can step to the next day; that offset
  (EWRAM, not saved) also applies to the zone's maps until a reset. Pallet Town is just
  east of the Test Area.
- **Mega Island castle**: in the `MEGA_ISLAND` zone too. Its map has no scripts, so there
  is no scene that needs a fixed sky, and keeping it fixed would have stopped the rain at its
  gate (it is connected to `MegaIsland`).
- **Override rule**: setting the map's own `WEATHER_DYNAMIC_*` in a script is not an
  override; any other weather is, including another zone's dynamic weather.
- **Test Area debug entry**: a menu of every zone. It reports the chosen zone and shows that
  zone's weather on the Test Area (live for `VIRIDIAN`, the Test Area's own zone).
- **Zone borders**: weather can change where two zones meet, as planned: Route 2 / Pewter,
  Route 3 / Route 4, Route 9 / Route 10, Route 5 / Saffron, Route 6 / Saffron, Route 11 /
  Route 12, Route 8 / Saffron, Route 12 / Route 13, Route 18 / Fuchsia, Fuchsia / Route 19,
  Route 21 / Pallet Town and Route 22 / Route 23.

## Where things are now

- **Map weather** is a byte in each map header (`"weather"` in `map.json`). On map load,
  `SetSavedWeatherFromCurrMapHeader()` (`src/field_weather_effect.c:2522`) runs it
  through `TranslateWeatherNum()` and stores it in `gSaveBlock1Ptr->weather`;
  `DoCurrentWeather()` then fades it in. Both are called from `src/overworld.c:803-819`,
  just after `DoTimeBasedEvents()` and before the map's `ON_TRANSITION` script, so a
  script `setweather` still wins.
- **Changing weather already exists, in one place**: `WEATHER_ROUTE119_CYCLE` and
  `WEATHER_ROUTE123_CYCLE` pick from a 4-entry table indexed by
  `gSaveBlock1Ptr->weatherCycleStage`, which `UpdateWeatherPerDay()` advances once a day
  from `src/clock.c:56`. Those are Hoenn maps and stay as they are.
- **The clock**: `DoTimeBasedEvents()` (`src/clock.c:29`) runs on every map load and
  from a field task about every 2 minutes (`src/field_tasks.c:151`,
  `TIME_UPDATE_INTERVAL`). It already starts the RTC for a Kanto game, and is skipped
  in Pokémon Centers. `gLocalTime` gives days, hours and minutes.
- **Kanto weather today**: 121 maps use `WEATHER_SUNNY` (the plain daylight look, with
  no battle effect), 29 use fog, 21 use `WEATHER_SHADE` (forests: Viridian Forest, Berry
  Forest). Nothing in Kanto rains.
- **Overworld weather in battle**: `ABILITYEFFECT_SWITCH_IN_WEATHER`
  (`src/battle_util.c:2501`) turns rain, thunderstorm and downpour into permanent rain,
  sandstorm into sandstorm and drought into sun, with the message from
  `gWeatherStartsStringIds` (`src/battle_message.c:1183`). Snow does nothing, and
  battle hail has no permanent bit (`include/constants/battle.h:242`).
- **Other readers of the weather**: the battle background turns to sand in a sandstorm
  (`src/battle_setup.c:742`); `GAME_STAT_GOT_RAINED_ON` counts rain; Route 111's desert
  music (`src/overworld.c:1102`).
- **Palettes**: the weather colour maps cover all 13 map palettes
  (`sBasePaletteColorMapTypes`, `src/field_weather.c:114`), so rain and shade darken the
  FRLG tilesets the same way as Hoenn's.
- **Test Area** has a weather changer (`data/maps/TestArea/scripts.inc:23`) for every
  weather up to `WEATHER_UNDERWATER_BUBBLES`, snow included.

## 0. Decisions

| # | Decision | Proposal |
|---|----------|----------|
| D1 | Where | Outdoor maps in Kanto, the Sevii Islands and Mega Island. Hoenn, caves, interiors, forests, the Test Area and the prototype Sevii maps keep their fixed weather |
| D2 | Unit of weather | A **zone**: a group of neighbouring maps that always share the weather, so it doesn't change at every map border (section 2) |
| D3 | How often it changes | Four times a day, with the day/night phases: morning 6-10, day 10-17, evening 17-20, night 20-6 |
| D4 | How it's chosen | Each day, each zone draws one **day pattern** (four weathers, one per phase) from its weighted list. The draw is a hash of the save's trainer ID, the day and the zone, so it is the same every time it's computed and needs no save space, and two saves get different weather |
| D5 | Story gate | Off until `FLAG_SYS_POKEDEX_GET` (Oak's lab). Before that every dynamic map uses its zone's fallback (clear), so the opening stays as it is |
| D6 | Battles | Keep the existing links (rain, sandstorm, drought). Add **snow → permanent hail**. Fog, shade and clouds stay visual only. `WEATHER_SUNNY` stays without a battle effect; harsh sun only comes from `WEATHER_DROUGHT`, which is rare |
| D7 | Change while on a map | Yes, with the normal weather fade, when the phase changes and the player has control. Not during scripts, battles or menus |
| D8 | Forecast | A forecaster in each Pokémon Center 1F of Kanto (Viridian, Cerulean, Celadon) and One Island tells today's and tomorrow's weather for a few zones. Phase 4, optional |
| D9 | Wild Pokémon | Not changed in this plan. Weather-only encounters can come later (section 8) |
| D10 | Seasons | None. The RTC counts days, not calendar months |

## 1. Constants

`include/constants/weather.h`, after `WEATHER_ROUTE123_CYCLE`:

```c
// Dynamic weather: the map's zone decides, see src/dynamic_weather.c
#define WEATHER_DYNAMIC_START            32
#define WEATHER_DYNAMIC_VIRIDIAN         (WEATHER_DYNAMIC_START + DYN_ZONE_VIRIDIAN)
#define WEATHER_DYNAMIC_PEWTER           (WEATHER_DYNAMIC_START + DYN_ZONE_PEWTER)
// ... one per zone in section 2
#define WEATHER_DYNAMIC_END              (WEATHER_DYNAMIC_START + DYN_ZONE_COUNT)
```

with the zone ids (`DYN_ZONE_*`, 0 to `DYN_ZONE_COUNT - 1`) above them. The map header
weather is a `u8`, so values 32-63 fit; `mapjson` writes the symbol as it is.

Also `#define B_WEATHER_HAIL_PERMANENT (1 << 9)` in `include/constants/battle.h`, and
`B_WEATHER_HAIL` becomes `(B_WEATHER_HAIL_TEMPORARY | B_WEATHER_HAIL_PERMANENT)`.
`gBattleWeather` is a `u16`, so bit 9 is free (bit 8 is Delta Stream's strong winds).

## 2. Zones

Each zone has a **fallback** (used before the story gate) and a list of day patterns
with weights (section 3). The maps listed get `"weather": "WEATHER_DYNAMIC_<ZONE>"`.

| Zone | Maps | Character |
|------|------|-----------|
| `VIRIDIAN` | Pallet Town, Route 1, Viridian City, Route 2, Route 22 (and the Test Area, for testing) | Mild: mostly clear, some showers |
| `PEWTER` | Pewter City, Route 3 | Dry and rocky: clear, dust in the afternoon (sandstorm) |
| `CERULEAN` | Route 4, Cerulean City, Routes 24, 25, 5, 9 | Wet: showers and storms |
| `VERMILION` | Vermilion City, Routes 6, 11 | Harbour: morning fog, showers |
| `LAVENDER` | Lavender Town, Routes 8, 10, 12 | Gloomy: overcast, fog, rain at night |
| `CELADON` | Celadon City, Saffron City (+ `SaffronCity_Connection`), Routes 7, 16, 17, 18 | City: clear or cloudy, the odd storm |
| `FUCHSIA` | Fuchsia City, Routes 13, 14, 15, Safari Zone (`SafariZone_Center`, `_East`, `_West`, `KantoSafariZone_North`) | Coastal: fog, warm afternoons, showers |
| `SEA` | Routes 19, 20, 21 (north and south), Cinnabar Island | Open sea: heat, sudden downpours, sea fog |
| `INDIGO` | Route 23, Indigo Plateau | Mountain: cloud, fog, storms |
| `SEVII_NORTH` | One, Two and Three Island; Kindle Road, Treasure Beach, Cape Brink, Bond Bridge, Three Isle Port | Volcanic and warm: heat waves near Mt. Ember |
| `FOUR_ISLAND` | Four Island | Cold: snow and flurries |
| `SEVII_FOUR_FIVE` | Five Island; Five Isle Meadow, Memorial Pillar, Resort Gorgeous, Water Labyrinth | Cool: fog, cloud |
| `SEVII_SOUTH` | Six and Seven Island; Green Path, Outcast Island, Ruin Valley, Water Path, Sevault Canyon (+ entrance), Tanoby Ruins, Trainer Tower | Wild: storms, canyon dust |
| `MEGA_ISLAND` | `MegaIsland`, `MegaIsland_Port`, `MegaIsland_Castle` | Mostly clear |

Kept fixed: Viridian Forest, Berry Forest and Pattern Bush (shade or none), Mt. Ember
exterior, Navel Rock, Birth Island and `Prototype_SeviiIsle_6`-`9`.

Four Island is its own zone so that only it gets snow. That works because it has no
map connection to Five Island (you arrive by boat), so there is no border where snow
would fade in and out. (`FourIsland/map.json` has no connections.)

## 3. Day patterns

A pattern is four weathers (morning, day, evening, night), like the existing
Route 119/123 tables. A shared list in `src/dynamic_weather_rules.c`:

| Pattern | Morning | Day | Evening | Night |
|---------|---------|-----|---------|-------|
| `CLEAR` | SUNNY | SUNNY | SUNNY | SUNNY |
| `CLOUDY` | SUNNY_CLOUDS | SUNNY_CLOUDS | SUNNY | SUNNY |
| `OVERCAST` | SHADE | SHADE | SHADE | SHADE |
| `MORNING_FOG` | FOG | SUNNY | SUNNY | FOG |
| `SHOWERS` | SUNNY | RAIN | SUNNY_CLOUDS | SUNNY |
| `RAINY_DAY` | RAIN | RAIN | SHADE | SHADE |
| `NIGHT_RAIN` | SUNNY | SUNNY | SHADE | RAIN |
| `STORM` | RAIN | THUNDERSTORM | RAIN | SHADE |
| `DOWNPOUR` | THUNDERSTORM | DOWNPOUR | RAIN | RAIN |
| `HEAT_WAVE` | SUNNY | DROUGHT | SUNNY | SUNNY |
| `DUST` | SANDSTORM | SANDSTORM | SUNNY | SUNNY |
| `SNOWFALL` | SNOW | SHADE | SNOW | SNOW |
| `FLURRIES` | SUNNY | SUNNY | SHADE | SNOW |

(`FOG` is `WEATHER_FOG_HORIZONTAL`. `WEATHER_FOG_DIAGONAL` is unused in vanilla; check it
in the Test Area before using it.)

Weights per zone, in percent (blank = 0):

| Zone | CLEAR | CLOUDY | OVERCAST | M. FOG | SHOWERS | RAINY | N. RAIN | STORM | DOWNPOUR | HEAT | DUST | SNOW | FLURRIES |
|------|------:|-------:|---------:|-------:|--------:|------:|--------:|------:|---------:|-----:|-----:|-----:|---------:|
| VIRIDIAN | 45 | 20 | | 5 | 15 | 5 | 5 | 5 | | | | | |
| PEWTER | 45 | 15 | 5 | | 10 | | | 5 | | 5 | 15 | | |
| CERULEAN | 25 | 15 | 5 | 5 | 20 | 15 | 5 | 10 | | | | | |
| VERMILION | 35 | 15 | | 20 | 15 | 5 | 5 | 5 | | | | | |
| LAVENDER | 20 | 10 | 25 | 15 | 5 | 10 | 10 | 5 | | | | | |
| CELADON | 40 | 25 | 5 | | 15 | 5 | 5 | 5 | | | | | |
| FUCHSIA | 35 | 10 | | 20 | 15 | 5 | 5 | 5 | | 5 | | | |
| SEA | 30 | 10 | | 15 | 10 | | 5 | 10 | 10 | 10 | | | |
| INDIGO | 25 | 20 | 15 | 15 | 5 | 5 | | 15 | | | | | |
| SEVII_NORTH | 40 | 15 | | 5 | 10 | | 5 | 5 | | 20 | | | |
| SEVII_FOUR_FIVE | 30 | 20 | 10 | 20 | 10 | 5 | 5 | | | | | | |
| FOUR_ISLAND | 20 | 10 | 15 | 5 | | | | | | | | 30 | 20 |
| SEVII_SOUTH | 30 | 15 | 5 | 5 | 10 | 5 | 5 | 15 | | | 10 | | |
| MEGA_ISLAND | 55 | 20 | | 5 | 10 | | 5 | 5 | | | | | |

Each row sums to 100. These are a first guess; tune after play-testing.

## 4. The code

New `src/dynamic_weather.c` and `include/dynamic_weather.h`:

```c
bool8 IsDynamicWeather(u32 weather);          // WEATHER_DYNAMIC_START <= w < WEATHER_DYNAMIC_END
u8 GetDynamicWeather(u8 zone);                // now (phase 1)
u8 GetDynamicWeatherAt(u8 zone, u16 day, u8 phase); // for the forecast (phase 5)
void DynamicWeather_OnTimeUpdate(void);       // from DoTimeBasedEvents() (phase 2)
```

- **Phase**: `DayNight_GetPhaseNow()` (`src/day_night.c`).
- **Pattern draw**: hash `(trainer ID, day, zone)` with a small local mixer. It must **not** use
  `Random()`, which would shift the battle and encounter RNG. Take the result mod 100
  and walk the zone's weights.
- **Gate**: if `!FlagGet(FLAG_SYS_POKEDEX_GET)` or the clock isn't set, return the
  zone's fallback.
- **No RTC** (an emulator without one, or a dead battery): the zone's fallback.

Hook into the existing code:

1. `TranslateWeatherNum()` (`src/field_weather_effect.c:2595`): before `default`,
   `if (IsDynamicWeather(weather)) return GetDynamicWeather(weather - WEATHER_DYNAMIC_START);`.
   This covers map loads, `resetweather` and `setweather WEATHER_DYNAMIC_*`.
2. **Change while on a map** (D7, done): `DynamicWeather_UpdateField()` runs each overworld
   frame from `OverworldBasic()`, next to the day/night tint, and checks once a second (not
   from `DoTimeBasedEvents()`, which also runs during map loads and only every ~2 minutes in
   the field). On a dynamic map whose weather wasn't overridden, if the zone's weather now
   differs from `GetSavedWeather()`, it calls `SetSavedWeatherFromCurrMapHeader()` then
   `DoCurrentWeather()`, which fades. Comparing the weather itself, not a `(day, phase)`
   key, means a new phase with the same weather, or a continued save, changes nothing.
   While `ArePlayerFieldControlsLocked()` or a palette fade is active it tries again a
   second later.
3. **Script override** (done): an EWRAM flag, set by `SetSavedWeather()` when the weather
   isn't a dynamic one (so `setweather`, `SetWeather()` and the coord events) and cleared by
   `SetSavedWeatherFromCurrMapHeader()` (map load, `resetweather`). Map scripts that
   `setweather` on transition run after the map load, so they keep their weather.
4. **Battles**: in `ABILITYEFFECT_SWITCH_IN_WEATHER` add
   `case WEATHER_SNOW: gBattleWeather = B_WEATHER_HAIL; animArg1 = B_ANIM_HAIL_CONTINUES;`,
   and `[WEATHER_SNOW] = STRINGID_HAILCONTINUES` (or a new "It's hailing!" string) in
   `gWeatherStartsStringIds`. In `ENDTURN_HAIL` (`src/battle_util.c:1420`) skip the
   countdown when `B_WEATHER_HAIL_PERMANENT` is set, as `ENDTURN_SUN` does. Check every
   `gBattleWeather = B_WEATHER_HAIL_TEMPORARY` and `&= ~B_WEATHER_HAIL_TEMPORARY`
   (`src/battle_script_commands.c:9245`, Air Lock / Cloud Nine, the Mega and primal
   weather code) clears both bits where it should.
5. **Battle background**: `BattleSetup_GetEnvironmentId()` already uses sand in a
   sandstorm, so `DUST` days on Route 3 give a sand background. Keep it.

Nothing new goes in the save: the weather is recomputed from the clock and the
trainer ID. `gSaveBlock1Ptr->weather` still holds the translated weather as now.
`weatherCycleStage` and `UpdateWeatherPerDay()` stay for Routes 119 and 123.

## 5. Maps

- ✅ Change `"weather"` in the `map.json` of every map in section 2, one commit per
  region (Kanto, Sevii, Mega Island) so they can be reviewed.
- ✅ Search the Kanto scripts for `setweather`, `resetweather`, `doweather` and weather
  coord events. None of the zone maps has a weather coord event or a weather command. The
  only Kanto one is `Kanto_EventScript_TryDarkenRuins` (`setweather WEATHER_SHADE`), used
  by the Tanoby chambers, which are indoor and keep their own weather.
- Snow is unused in vanilla Emerald. It uses the same weather sprite palette slot
  (`PALTAG_WEATHER`) as rain, so there is no new palette conflict, but it still needs a
  look in the game: pick FOUR ISLAND in the Test Area's dynamic weather menu (or SNOW in
  the weather changer) and check the snowflakes over the FRLG tiles, walking, Surf and
  the reflections on Four Island's water.

## 6. Forecast (optional, D8)

- A FORECASTER NPC in the Viridian, Cerulean and Celadon Pokémon Centers and on
  One Island, with a script special `GetWeatherForecast` that fills
  `gStringVar1`-`3` for three nearby zones: "Today on ROUTE 24: rain this afternoon.
  Tomorrow: clear."
- Text per pattern, not per phase, to keep the strings short: `SHOWERS` → "afternoon
  showers", `STORM` → "storms later", `MORNING_FOG` → "fog in the morning".
- The Test Area debug entry already has a menu of every zone (phase 3).

## 7. Phases

1. ✅ **Engine**: constants, `src/dynamic_weather.c`, the `TranslateWeatherNum` hook, the
   story gate, and one zone (`VIRIDIAN`) on its maps. Debug entry in the Test Area.
   `make` and `make modern` build.
2. ✅ **Live changes**: the phase update while on a map, the script override, locked-
   controls retry. The Test Area is now in the `VIRIDIAN` zone, so the time changer
   there shows the change live, and the dynamic weather NPC gives it back its zone weather
   with `resetweather`.
3. ✅ **All zones**: the remaining zones and patterns, the map changes of section 5.
   The in-game snow check is still to do.
4. **Battles**: permanent hail from snow, the start message, the end-turn change.
5. **Forecast** (optional).
6. **Tuning**: play the story with the clock at different hours; check that rain
   doesn't make an early fight (Brock, Misty's route trainers) much harder or easier,
   and that the weights in section 3 feel right.

## 8. Later, not in this plan

- Weather-only wild encounters (for example more Water types in rain, Ice types in
  Four Island snow). The encounter tables are per map; the cheapest form is a second
  table per zone that swaps one or two slots when the weather matches.
- Weather that follows the story (a storm over Saffron while Team Rocket holds Silph).

## 9. Tests

- Host test for the pattern draw, like the card game's host tests: for each zone,
  10,000 days give pattern frequencies within 1% of the weights, and the same
  `(trainer ID, day, zone)` always gives the same pattern.
- In game, with the Test Area debug entry: each phase of each zone; crossing every
  zone border (the fade should look like the existing map-to-map weather change);
  entering and leaving buildings; a battle in rain, sandstorm, drought and snow;
  saving and loading in each weather; a scripted `setweather` scene surviving a phase
  change.
