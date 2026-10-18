# Plan: Opening Hoenn

This is a plan for making the Hoenn maps, which are still in the ROM, reachable during the
story: a ship from Vermilion to Slateport, Hoenn's own story played from Professor Birch's
lab to its Hall of Fame, with Team Aqua and Team Magma after Kyogre's and Groudon's Primal
Reversion, and Hoenn's legendaries and post-game. Nothing here is implemented yet.

## Status

Phase 1 is implemented: Kanto and Hoenn keep their progress apart.
`tools/hoenn/check_shared_state.py` checks the maps. Nothing has been tested in-game.

- **New state** (`include/constants/flags.h`, `vars.h`): `FLAG_HOENN_BADGE01_GET`-`08`,
  `FLAG_HOENN_GAME_CLEAR`, `FLAG_HOENN_OPEN`, `FLAG_ARRIVED_IN_HOENN` and
  `FLAG_HIDDEN_ITEM_HOENN_VICTORY_ROAD_1F_ULTRA_BALL` at `KANTO_EXTRA_FLAGS_START + 0x180`-`0x18B`
  (a block clear of the Kanto port's), and `VAR_HOENN_STARTER` at `KANTO_VARS_START + 0x70`.
- **Hoenn maps**: 38 files of Emerald's map groups read the new flags and var instead of
  `FLAG_BADGE0n_GET`, `FLAG_SYS_GAME_CLEAR`, `VAR_STARTER_MON` and the Victory Road item flag,
  and so do the scripts only Hoenn calls (`players_house.inc`, `prof_birch.inc`,
  `secret_base.inc`). Hoenn's Hall of Fame sets `FLAG_HOENN_GAME_CLEAR`
  (`hall_of_fame.inc`); it still calls `GameClear`, which phase 5 changes.
- **C, Hoenn's systems**: BIRCH's starter (`CB2_GiveStarter`) and `IsStarterInParty` use
  `VAR_HOENN_STARTER`; the Hoenn trainers' rematches (`battle_setup.c`), the gym leaders'
  rematches, Match Call and the PokéNav's lists, and Trainer Hill read the Hoenn badges and game
  clear.
- **HMs**: `HasBadgeForFieldMove` (`src/party_menu.c`) picks the badge from the map's region
  (`GetPlayerRegion`): FRLG's table on Kanto maps, Emerald's on Hoenn maps. The party menu,
  Surf, Waterfall and Dive (`field_control_avatar.c`), and Cut, Rock Smash and Strength
  (`field_move_scripts.inc`, through the special `CheckBadgeForFieldMove`) use it. The field
  move list moved to `include/constants/party_menu.h` so the scripts can name the moves.
- **Trainer card**: the player's own card shows Kanto's badges, with Kanto's pictures, outside
  Hoenn, and Hoenn's in Hoenn. Until now it showed the Kanto badge flags with Hoenn's pictures
  (BROCK's badge drawn as the STONE BADGE).

Left as they are, as Kanto's or both regions': the badges' stat boosts and obedience, the
badge counts on the Continue and save screens, TV shows, contests, link and the Hall of Fame
PC. Two things for phase 5: the credits pick their starter with `GetStarterPokemon(VAR_STARTER_MON)`,
a Hoenn starter for the Kanto index; and Hoenn's Hall of Fame clears the defeated flags of LUGIA,
HO-OH and DEOXYS, which Kanto's copies share.

Phase 2 is implemented: the MANSION raid and the ship. Hoenn is now reachable, but phase 3 (BIRCH's
welcome and Hoenn's opening) isn't done, so a player who sails to SLATEPORT finds Hoenn as a new
Emerald game left it. Nothing has been tested in-game.

- **The hook** (`data/scripts/hoenn_story.inc`, `VAR_HOENN_STORY_STATE`): after the first Hall of
  Fame, MOM passes on a call from BLAINE (`PalletTown_PlayersHouse_1F`; state 1 is set on the
  map's transition, so saves from after the Hall of Fame get it too, and the scene doesn't walk
  the player, who may come in from outside). Without the MEGA RING the story goes on all the
  same; BLAINE's gym still gives it.
- **BLAINE** (`CinnabarIsland_Gym`): while the raid is on (state 2) he asks the player to stop
  it, instead of his rematch.
- **The raid** (`PokemonMansion_1F`, `PokemonMansion_B1F`): two grunts on 1F and two on B1F,
  SHELLY and TABITHA at the lab's safe in B1F's north-east room. They're shown only in state 2
  (`FLAG_HIDE_MANSION_RAIDERS`, kept by the floors' transition scripts). Once both admins are
  beaten they flee with half of the PRIMAL NOTES each, BLAINE comes down, explains Primal
  Reversion and sends the player to VERMILION: state 3 and `FLAG_HOENN_OPEN`.
- **Trainers**: `TRAINER_MANSION_*`, `TRAINER_SHELLY_MANSION` and `TRAINER_TABITHA_MANSION`
  (`src/data/mansion_raid_trainers.h`, parties in `mansion_raid_trainer_parties.h`): grunts at
  55-57, the admins at 59-62. They take Emerald's spare ids 855-860, whose flags are free; the
  Kanto trainers' flag block has only 2 left. Emerald's spare ids are now 861-863.
- **The ship**: `SEAGALLOP_SLATEPORT` (`src/seagallop.c`, SEAGALLOP HI-SPEED 13, westbound from
  VERMILION), docking where the S.S. TIDAL does in `SLATEPORT_CITY_HARBOR` (8, 11). BLAINE's
  sailor friend stands on VERMILION's east pier, by the APEX ISLAND sailor, shown by
  `FLAG_HOENN_OPEN` on each transition (`FLAG_HIDE_VERMILION_HOENN_SAILOR`). A sailor in
  SLATEPORT's and in LILYCOVE's harbor sails back (`HoennStory_EventScript_SeagallopSailor`).
- **D7**: LILYCOVE's S.S. TIDAL no longer offers Hoenn's Navel Rock and Birth Island
  (`CreateLilycoveSSTidalMultichoice`, and the harbor's ticket scripts lose those branches).
- **Test Area**: a sailor east of the others sails to SLATEPORT, or, declined, puts the raid in
  the MANSION (state 2) without a Hall of Fame.
- **Checks**: `check_reachability.py` reads the Seagallop's harbors from `src/seagallop.c`. It
  finds 429 of the 518 non-Kanto maps: the rest are reached from C (secret bases, the Battle
  Pyramid and Pike, link rooms, the tides of Shoal Cave), unused, the truck of Emerald's intro,
  and Hoenn's Navel Rock and Birth Island.

## Where things are now

The game is a Kanto game (FRLG's maps and scripts ported onto Emerald). A new game starts in
`MAP_PALLET_TOWN_PLAYERS_HOUSE_2F`, and Kanto, the Sevii Islands and Mega Island are reached
from there. Emerald's Hoenn maps were never removed: they are built into the ROM, with their
scripts, trainers, wild encounters, berry trees and heal locations, and
`EventScript_ResetAllMapFlags` still sets their new-game flags. They are only cut off.

**Reachability.** A walk over every map's warps and connections, plus the warps in the maps'
scripts, the Seagallop's destinations (`src/seagallop.c`) and the Kanto heal locations, from
the player's room:

| Maps | Count | Reachable |
|------|------:|-----------|
| Kanto, Sevii, Mega Island, Test Area (`gMapGroup_Kanto*`, `gMapGroup_Custom`) | 438 | All of them, as the story allows |
| Hoenn's towns, routes, dungeons and interiors | 369 | None |
| Battle Frontier and Artisan Cave | 49 | None |
| Event islands (Hoenn's Navel Rock and Birth Island, Faraway and Southern Island, Marine/Terra Cave, Desert Underpass, Altering Cave) | 34 | None |
| Link rooms, contest halls, secret bases | 43 | Only `MAP_TRADE_CENTER` and `MAP_UNION_ROOM`, from every Pokémon Center's 2F |
| Safari Zone and Trainer Hill | 14 | None |
| Unused and prototype maps | 9 | None |

No warp, connection or script in Kanto leads to Hoenn, and no Hoenn map leads to Kanto.
Fly can already show Hoenn: the fly map switches region (`GetNextVisitedRegion` in
`src/region_map.c`), and Hoenn is offered once one of `FLAG_VISITED_LITTLEROOT_TOWN` to
`FLAG_VISITED_EVER_GRANDE_CITY` is set. None can be set today.

**Hoenn's content as it is.** Unchanged from Emerald: the trainers in `src/data/trainers.h`
(levels 3 to 58 in the story, up to 78 in the rematches), the 257 wild tables of
`gWildMonHeaders` (Route 101 at level 2-3), the story vars (`VAR_LITTLEROOT_INTRO_STATE`,
`VAR_BIRCH_LAB_STATE`, ...) all at 0 in a new game. For comparison, the Kanto champion is at
levels 57-63 the first time, 75-78 in the rematch, and MR. M.'s tournament at 85-90.

**State that Kanto and Hoenn share.** The Kanto port keeps its own flags and vars apart
(`KANTO_FLAGS_START`, `KANTO_VARS_START`), and the only Emerald vars it took are unused ones
(`VAR_TOURNAMENT_ROUND`, `VAR_TOURNAMENT_OPPONENTS`, `VAR_MEGA_STORY_STATE`). But these are
read by both regions' scripts:

| Flag or var | Set by Kanto | What it would do in Hoenn |
|-------------|--------------|---------------------------|
| `FLAG_BADGE01_GET`-`FLAG_BADGE08_GET` | The Kanto gym leaders | Every Hoenn gym, the Petalburg Gym door, the Trick House and the badge checks of 21 Hoenn maps already see 8 badges. Surf, Waterfall and Dive work at once (`src/field_control_avatar.c`, `src/party_menu.c`) |
| `FLAG_SYS_GAME_CLEAR` | The first Kanto Hall of Fame | Read by 19 Hoenn maps: the S.S. Tidal to the Battle Frontier, Steven's house, the Space Center, Meteor Falls, Norman's rematch, Trainer Hill... all open on arrival |
| `VAR_STARTER_MON` | Oak's lab (the Kanto starter) | Picks the Hoenn rival's team in 8 Hoenn maps, as Treecko/Torchic/Mudkip |
| `FLAG_HIDE_LUGIA`, `FLAG_HIDE_HO_OH`, `FLAG_HIDE_DEOXYS` | The Kanto Navel Rock and Birth Island | Hide or show Hoenn's copies of the same Pokémon |
| `FLAG_ENABLE_SHIP_NAVEL_ROCK`, `FLAG_ENABLE_SHIP_BIRTH_ISLAND`, `FLAG_SHOWN_*_TICKET` | Vermilion's Seagallop | Lilycove Harbor's ferry offers Hoenn's Navel Rock and Birth Island |
| `FLAG_HIDDEN_ITEM_VICTORY_ROAD_1F_ULTRA_BALL` | Kanto's Victory Road 1F | Takes the hidden item on Hoenn's Victory Road 1F |
| `FLAG_PENDING_DAYCARE_EGG` | Four Island's day care | Route 117's day care: the same save slot, so the same two Pokémon. That is consistent and stays |

The other docs assume Hoenn stays closed: `docs/card_game_plan.md` (D2b: no cards for
Hoenn-only species, "Hoenn is never reachable in this project") and
`docs/dynamic_weather_plan.md` (D1: Hoenn keeps its fixed weather).

## The story: the Primal notes

Team Aqua and Team Magma are the villains, as in Emerald, and they are the reason to go: they
come to Kanto first. The Hoenn chapter is Emerald's story, with the teams after something more
than awakening Kyogre and Groudon: their **Primal Reversion**. This is also how the game
introduces the Primal forms and MEGA RAYQUAZA.

**What they want.** The lab at the POKéMON MANSION studied Mega Evolution while it studied
MEWTWO. BLAINE says the notes burned (`docs/mega_island.md`); not all of them did. A fireproof
safe on B1F kept one file: the lab's research on where Mega Evolution comes from. It is the
power that a legend of Hoenn calls **Primal Reversion**: Kyogre and Groudon, touched by the
BLUE and RED ORBS, return to the form they had when they shaped the land and the sea. Mega
Evolution is its echo. Archie wants Kyogre's Primal form to flood the land; Maxie wants
Groudon's to dry the seas. Each team learns of the file and races the other for it.

### Act 0: the raid, in Kanto (after the first Hall of Fame)

- Gate: the first Hall of Fame and the MEGA RING from BLAINE (`FLAG_RECEIVED_MEGA_RING`). A
  player who skipped it gets it by talking to him again, as now.
- BLAINE (in his gym on Cinnabar, or by a call from OAK) has news: people in red and in blue
  uniforms were seen at the POKéMON MANSION.
- POKéMON MANSION: Aqua and Magma grunts searching the building, fighting each other as much as
  the player. On B1F, the open safe: an Aqua admin (SHELLY) and a Magma admin (TABITHA) have
  split the file and each run with half. Battles with grunts and with the two admins, at the
  start of the post-game levels (section 4).
- Vermilion's pier: the two teams' boats leave for Hoenn as the player arrives. A sailor
  offers the ship to Slateport (`FLAG_HOENN_OPEN`). BLAINE asks the player to get the file
  back: "If those fools force that power out of Kyogre and Groudon, nothing will stop it."

### Act 1: the arrival

- Slateport Harbor: BIRCH, warned by BLAINE and OAK, meets the player and takes them to his lab.
  Hoenn's story starts there (section 3). BIRCH has heard of the ORBS, kept at MT. PYRE.

### Act 2: the race, at Emerald's own beats

Emerald's scenes stay as they are; a few lines and one item are added where the teams talk
about their plans.

| Where (Emerald's beat) | Added |
|------------------------|-------|
| Rustboro, Devon Corporation (the stolen DEVON GOODS) | MR. STONE: the thief spoke of "the Kanto file" |
| Mt. Chimney (Maxie and the METEORITE) | Maxie: the file says the METEORITE's energy is what Mega Stones are made of; it will wake Groudon, and the RED ORB will make it Primal |
| Weather Institute (Shelly's raid) | Shelly: Aqua's half of the file says the BLUE ORB awakens Kyogre's "true form" |
| Mt. Pyre summit (both ORBS stolen) | Archie and Maxie name Primal Reversion. The old lady: the ORBS were never meant to make them stronger, only to calm them |
| Team Aqua Hideout or Magma Hideout | The player finds a half of the file (a key item, PRIMAL NOTES), and its last page: "When the two were in their primal fury, a third came from the sky… and changed, as our Mega Evolutions change, at the call of a person's wish" |

### Act 3: the climax

- **Seafloor Cavern and Sootopolis**: as in Emerald, Kyogre and Groudon awaken, but in their
  Primal forms: the ORBS glow, and PRIMAL KYOGRE and PRIMAL GROUDON clash over Sootopolis
  (Emerald's overworld objects with Primal palettes). The storm and the drought are worse;
  Archie and Maxie see they can't control them.
- **Sky Pillar**: RAYQUAZA wakes, as in Emerald. At the top, the player's MEGA RING glows:
  RAYQUAZA answers the wish, as in the last page of the notes.
- **Sootopolis**: RAYQUAZA comes down and **Mega Evolves**, and the two Primals revert and
  leave (Emerald's scene, with Mega Rayquaza's object and a flash for the change). This is the
  first time the player sees MEGA RAYQUAZA.
- **After**: Archie and Maxie give the ORBS back at MT. PYRE, as in Emerald. The player hands
  the PRIMAL NOTES to BIRCH, who sends them to BLAINE. Hoenn's story goes on to the eighth gym,
  Victory Road and the Hoenn League.

### Act 4: the legendaries

| Legendary | Where | When | Reward |
|-----------|-------|------|--------|
| RAYQUAZA | Sky Pillar (Emerald's) | After Sootopolis | MEGA RAYQUAZA, with DRAGON ASCENT and the player's MEGA RING (already in the game) |
| KYOGRE | Marine Cave (Emerald's roaming cave, found from the Weather Institute's reports) | After Hoenn's Hall of Fame | The old lady of MT. PYRE gives the BLUE ORB: held, **PRIMAL KYOGRE** in battle (new: phase 6 of `docs/mega_evolution_plan.md`) |
| GROUDON | Terra Cave (same) | After Hoenn's Hall of Fame | The RED ORB, from the same old lady: **PRIMAL GROUDON** (new, same phase) |
| LATIAS and LATIOS | Southern Island, today reachable only with the EON TICKET event. A sailor in Lilycove takes the player there | After Hoenn's Hall of Fame | Both can be caught; LATIASITE and LATIOSITE on the island |
| REGIROCK, REGICE, REGISTEEL | Emerald's Braille puzzles | Any time after Dive | As in Emerald |
| JIRACHI (optional) | New scene: the Millennium Comet, seen from the Mossdeep Space Center | After all of the above | A gift from the Space Center's director |

The old lady gives the ORBS once she has seen the player catch RAYQUAZA (or after Hoenn's Hall
of Fame, if they did it before): "The one RAYQUAZA trusts may keep them." DEOXYS stays on
Kanto's Birth Island, and Hoenn's copies of Navel Rock and Birth Island stay closed (D7). MEW
(Faraway Island, today event-only) is left for MEWTWO's story.

### Act 5: back in Kanto

- BLAINE, with the notes back: the last missing piece of the MANSION's research.
- BIRCH gives the Mega Stone of the player's Hoenn starter (SCEPTILITE, BLAZIKENITE or
  SWAMPERTITE) after Hoenn's Hall of Fame.
- In MR. M.'s tournament, STEVEN remembers the player from Hoenn.

GIOVANNI and ALAIN have no part in this chapter: GIOVANNI's rematch stays as it is, and ALAIN
is still free for another story.

## 0. Decisions

These need an answer before phase 1. The recommendation is in the second column.

| # | Question | Recommendation | Why |
|---|----------|----------------|-----|
| D1 | When Hoenn opens | After the first Kanto Hall of Fame (`FLAG_SYS_GAME_CLEAR`), with the MEGA RING, when Aqua and Magma raid the POKéMON MANSION (Act 0). It runs alongside the Sevii Islands quest | The player can Surf and Fly, so Hoenn must be gated by its own badges anyway (D4); opening it after the League gives a second region as the post-game, as in HGSS |
| D2 | How the player gets there | A new Seagallop destination, `SEAGALLOP_SLATEPORT`, from a sailor on Vermilion's pier, to `MAP_SLATEPORT_CITY_HARBOR`, and back from a sailor there and in Lilycove Harbor | `src/seagallop.c` already does the ride, the docking and the arrival; Slateport is Hoenn's port |
| D3 | Where Hoenn's story starts | At Birch's lab: BIRCH meets the player at Slateport Harbor and takes them to Littleroot (fade and warp). The truck, the bedroom, the clock and the Route 101 rescue are skipped; BIRCH gives a Hoenn starter, and the story goes on from the rival on Route 103 | Hoenn's scripts are a chain of state vars; starting from its first link keeps every later script valid. Starting in Slateport would skip the Devon Goods, Mr. Briney and Petalburg links |
| D4 | Badges in Hoenn | Eight new flags for the Hoenn badges. On a Hoenn map, field moves need the Hoenn badge (Cut with the Stone Badge, as in Emerald); on a Kanto map, the Kanto badge | Without it the player Surfs to Lilycove on arrival and skips the region. FRLG and Emerald already disagree on which badge unlocks which HM, so a per-region table is needed anyway |
| D5 | Levels | Raise Hoenn's trainers and wild Pokémon to a post-game band once, in the data, with a script: story trainers from 3-58 to about 55-80, rematches to 80-88, wild Pokémon in step. A rescaled Pokémon evolves to the stage its new level allows | The Kanto champion is at 63, the tournament at 85-90. A one-time rescale keeps the gyms' order meaningful; levels that follow the player's party would not |
| D6 | The player's home in Hoenn | None: the house of the player's gender in Littleroot becomes a guest room BIRCH lends them (no MOM, a bed that heals). The rival is the other house's child, chosen by `checkplayergender` as now | The player comes from Pallet Town; six characters can be picked, so Brendan's or May's own home can't be the player's |
| D7 | Hoenn's Navel Rock and Birth Island | Taken out of Lilycove Harbor's ferry menu. Kanto's are the ones with the event | Two copies of Lugia, Ho-Oh and Deoxys behind the same flags |
| D8 | Hoenn's Hall of Fame | Records the team and sets `FLAG_HOENN_GAME_CLEAR`, with Hoenn's credits but no `GameClear` reset of the Kanto story; Continue puts the player in the Littleroot guest room | `FLAG_SYS_GAME_CLEAR` and `FLAG_SYS_CAN_LINK_WITH_RS` already mean Kanto's progress |

## 1. Separate Hoenn's state from Kanto's

New flags, in the extra flags block (`KANTO_EXTRA_FLAGS_START`, 512 flags, 354 of them free
from `+ 0x64` up; check that `tools/kanto_port` doesn't allocate there first), or in the
Emerald flags marked `// Unused Flag` (53 left):

```c
#define FLAG_HOENN_BADGE01_GET ... FLAG_HOENN_BADGE08_GET   // Stone .. Rain
#define FLAG_HOENN_GAME_CLEAR                               // Hoenn's Hall of Fame
#define FLAG_HOENN_OPEN                                     // OAK's request: the Slateport sailor appears
#define FLAG_ARRIVED_IN_HOENN                               // BIRCH's welcome at Slateport Harbor, once
#define FLAG_HIDDEN_ITEM_HOENN_VICTORY_ROAD_1F_ULTRA_BALL
```

and one var, `VAR_HOENN_STARTER` (0 Treecko, 1 Torchic, 2 Mudkip), in `KANTO_VARS_START`'s
block or one of the 31 `// Unused Var` Emerald vars.

Then, in the Hoenn maps only (the groups that aren't `gMapGroup_Kanto*`), with a script and
checked by hand:

- `FLAG_BADGE0n_GET` to `FLAG_HOENN_BADGE0n_GET` (the gyms' `setflag`, the statues, the
  Petalburg Gym door, the Trick House, Route 110's and others' badge checks).
- `FLAG_SYS_GAME_CLEAR` to `FLAG_HOENN_GAME_CLEAR` (19 maps).
- `VAR_STARTER_MON` to `VAR_HOENN_STARTER` (8 maps), and the rival's party choice in
  `src/battle_setup.c` if it reads the var for Hoenn trainers.
- Victory Road's hidden item flag.

In C:

- A `HasBadgeForFieldMove(u8 fieldMove)` in `src/party_menu.c`, which picks the flag from the
  player's region (`GetPlayerRegion`, exported from `src/region_map.c`) and the per-region
  table; `src/field_control_avatar.c`'s Surf, Waterfall and Dive checks and the field effects
  that check badges call it.
- The trainer card: a second badge row (or page) for Hoenn, shown once one Hoenn badge is set.
- `GetNumBadges`-style counts used by scripts (Petalburg Gym, Trick House): count the Hoenn
  flags on Hoenn maps.

## 2. The ship

- `SEAGALLOP_SLATEPORT` in `include/constants/seagallop.h` and `src/seagallop.c`: harbor
  `MAP_SLATEPORT_CITY_HARBOR` and its dock coordinates, direction from Vermilion (south-west),
  ferry number.
- Vermilion's pier: a sailor shown by `FLAG_HOENN_OPEN` (hidden on a new game, as
  `FLAG_HIDE_VERMILION_APEX_SAILOR`), who sails to Slateport.
- Slateport Harbor and Lilycove Harbor: a sailor who sails back to Vermilion. Lilycove's ferry
  menu loses Navel Rock and Birth Island (D7).
- OAK's request after the first Hall of Fame, wherever the Kanto post-game sends the player
  (his lab, or a call): BIRCH has written to him, sets `FLAG_HOENN_OPEN`.
- Test Area: a sailor to Slateport, as for Mega Island.

## 3. The arrival and Hoenn's opening

- Slateport Harbor's map script: on the first arrival (`FLAG_ARRIVED_IN_HOENN` clear), BIRCH
  welcomes the player, then a fade and a warp to Birch's lab.
- The state that the truck, the bedroom, the clock and the Route 101 rescue would have left,
  set once by that script: `VAR_LITTLEROOT_INTRO_STATE`, `VAR_LITTLEROOT_HOUSES_STATE_*`,
  `VAR_ROUTE101_STATE`, `VAR_BIRCH_LAB_STATE`, `FLAG_HIDE_ROUTE_101_BIRCH_*`, the MOM and
  rival flags. To be read from `data/maps/InsideOfTruck`, `LittlerootTown*` and `Route101`
  before writing it.
- Birch's lab: BIRCH offers the three Hoenn starters (the lab's Johto Poké Balls stay
  post-game), sets `VAR_HOENN_STARTER`, and sends the player to meet the rival on Route 103,
  as in Emerald. The Pokédex scene gives nothing (the player has OAK's).
- Littleroot: the guest room (D6). `FLAG_VISITED_LITTLEROOT_TOWN` and the other visited
  flags are already set by the Hoenn maps' `OnTransition` scripts, so Fly works from the
  first Hoenn town on.
- Text: BIRCH and the rival should know the player is OAK's trainer from Kanto. The rest of
  Hoenn's text can stay as it is in a first pass.

## 4. Levels

A host script, `tools/hoenn/rescale_levels.py`, run once and committed with its output:

- `src/data/trainer_parties.h`, for the trainers before `KANTO_TRAINERS_START` (Emerald's),
  except the Battle Frontier's and the link ones: level `L` becomes `round(53 + L * 0.47)`
  (3 to 54, 58 to 80); rematch parties, with their own curve, to 80-88.
- `src/data/wild_encounters.json`, `gWildMonHeaders` entries of Hoenn maps: the same curve.
- A species below its level's stage evolves (level evolutions from `gEvolutionTable`; other
  methods at fixed levels in a table in the script). Parties with custom moves get their
  last four level-up moves at the new level, unless hand-set in a later pass.
- Gift and static Pokémon (the Hoenn starters, Wynaut's egg, the legendaries): by hand.

## 5. Hoenn's ending and post-game

- Ever Grande's Hall of Fame (`data/maps/EverGrandeCity_HallOfFame`): sets
  `FLAG_HOENN_GAME_CLEAR`, records the team, plays the credits and returns the player to the
  Littleroot guest room, without `GameClear`'s Kanto changes (D8).
- Everything gated by Hoenn's game clear (the S.S. Tidal to the Battle Frontier, Steven,
  Meteor Falls, the Space Center, Trainer Hill) opens then, through the flag of phase 1.
- Battle Frontier: its levels are fixed by its own rules (50 and open), so it needs nothing.

## 6. The other features

- **Card game** (`docs/card_game_plan.md`, D2b): Hoenn-only species become obtainable. Rerun
  the card generator with the Hoenn encounter tables, and decide whether Hoenn gets card
  players.
- **Dynamic weather** (`docs/dynamic_weather_plan.md`, D1): Hoenn keeps its fixed weather in
  this plan.
- **Mega stones**: the Hoenn starter's stone, the LATIASITE and the LATIOSITE (Acts 4 and 5).
  Other Hoenn stones (ABSOLITE, AGGRONITE, ...) can be placed later.

## 7. The story's content

What the story needs, on top of sections 1 to 5:

- **Act 0 in Kanto**: Aqua and Magma grunts and the two admins in the POKéMON MANSION
  (Emerald's trainer classes, pics and objects: `OBJ_EVENT_GFX_AQUA_MEMBER_M`/`_F`,
  `OBJ_EVENT_GFX_MAGMA_MEMBER_M`/`_F`); new trainers `TRAINER_SHELLY_MANSION` and
  `TRAINER_TABITHA_MANSION` and their grunts, at the post-game levels; the open safe on B1F;
  BLAINE's lines; the boats leaving Vermilion.
- **Lines** in Devon Corporation, Mt. Chimney, the Weather Institute and Mt. Pyre, and the
  PRIMAL NOTES (a key item, in a free item slot) in a hideout.
- **The Primal scene in Sootopolis**: Primal palettes for `OBJ_EVENT_GFX_KYOGRE_*` and
  `OBJ_EVENT_GFX_GROUDON_*`, used from the ORBS' glow on.
- **The Mega Rayquaza scene**: Mega Rayquaza's overworld object (a new sprite, or
  `OBJ_EVENT_GFX_RAYQUAZA` with a Mega palette), the MEGA RING's glow at Sky Pillar's top,
  and the change in Sootopolis.
- **Primal Reversion**: phase 6 of `docs/mega_evolution_plan.md`: `SPECIES_KYOGRE_PRIMAL` and
  `SPECIES_GROUDON_PRIMAL`, Primordial Sea and Desolate Land, the switch-in trigger with the
  ORBS (`ITEM_BLUE_ORB` and `ITEM_RED_ORB` already exist, as key items: they become hold items).
- **The ORBS**: the old lady of MT. PYRE gives them (Act 4).
- **Southern Island**: a sailor in Lilycove Harbor, shown after `FLAG_HOENN_GAME_CLEAR`,
  instead of the EON TICKET; the LATIASITE and LATIOSITE as items on the island.
- **Jirachi** (optional): the Space Center scene and a gift.
- New flags: one per scene, in the block of section 1.

## 8. Phases

1. Phase 1 (shared state). Nothing is reachable yet, so it can't break a playthrough.
2. Phase 2 (the ship and Act 0, the Mansion raid) with the Test Area sailor, then phase 3
   (the arrival and Act 1), together: Hoenn becomes reachable here.
3. Phase 4 (levels).
4. Phase 5 (ending and post-game).
5. Phase 6: Primal Reversion in battle (the Mega plan's phase 6), then Acts 2 and 3 (the
   lines, the PRIMAL NOTES, the Primal and Mega Rayquaza scenes).
6. Phase 7: Act 4 (the ORBS, Southern Island, then Jirachi) and Act 5.
7. The other features (section 6), each on its own.

## 9. Tests

- The reachability check, `tools/hoenn/check_reachability.py` (already written: it made the
  table at the top): from the player's room, the warps, connections, script warps, Seagallop
  destinations and heal locations. Phase 2 adds `SEAGALLOP_SLATEPORT`'s harbor. Today it finds
  2 of the 518 non-Kanto maps (the link rooms); after phase 2 it should find Hoenn's 369 story
  maps, the Battle Frontier and the event islands that Hoenn's own story opens, and still not
  Hoenn's Navel Rock and Birth Island.
- A check that no Hoenn map reads `FLAG_BADGE0n_GET`, `FLAG_SYS_GAME_CLEAR` or
  `VAR_STARTER_MON`, and no Kanto map reads the Hoenn flags.
- In-game: a save after the first Kanto Hall of Fame; the ship both ways; Birch's lab to the
  first gym; that Surf does nothing in Hoenn before Norman and works in Kanto; Fly between the
  regions; whiting out in Hoenn; the Hoenn Hall of Fame and Continue after it.
