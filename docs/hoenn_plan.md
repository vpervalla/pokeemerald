# Plan: Opening Hoenn

This is a plan for making the Hoenn maps, which are still in the ROM, reachable during the
story: a ship from Vermilion to Slateport, Hoenn's own story played from Professor Birch's
lab to its Hall of Fame, with Team Rocket behind it (Operation Primal), and Hoenn's
legendaries and post-game. Nothing here is implemented yet.

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

## The story: Operation Primal

Why go to Hoenn: **GIOVANNI**. In Kanto he leaves his gym after his first battle, and nothing
says where he went. He went to Hoenn. The Hoenn chapter is Emerald's story as it is (Team
Aqua, Team Magma, Kyogre, Groudon, Rayquaza), with Team Rocket behind it: a few new scenes at
the beats Emerald already has, and one new climax.

**The plan.** Team Rocket funded the lab at the POKéMON MANSION where MEWTWO was made, and kept
copies of the notes that burned. They say Mega Evolution is a small echo of an older power:
**Primal Reversion**, the form Kyogre and Groudon take with the BLUE and RED ORBS. MEWTWO got
away from GIOVANNI; two Primal legendaries would not. Under a front company, GIOVANNI
secretly supplies both Aqua and Magma with SILPH CO.'s stolen technology, so that they find and
wake Kyogre and Groudon for him. Once the two have worn each other out, he means to take them
with SILPH's prototype MASTER BALLS and force their Primal Reversion with the ORBS, and to hold
both regions hostage with endless rain and drought.

**ALAIN** (already built, see `docs/mega_island.md`, not placed yet) is the face of the front
company: a young Mega Evolution researcher who believes he is studying Primal energy for a
foundation. He is the chapter's second rival (Hoenn's own rival is Brendan or May), with his
Charizard and its CHARIZARDITE X.

### Act 0: the hook, in Kanto (after the first Hall of Fame)

- The Viridian Gym is empty (as now).
- OAK has a letter from BIRCH: strange men in black with an R on their shirts are buying up
  DEVON parts in Slateport. OAK asks the player to go and see.
- Vermilion's pier: two ROCKET grunts board the cargo ship ("The BOSS wants the SILPH crates in
  Slateport before the new moon"). Then the sailor to Slateport appears (`FLAG_HOENN_OPEN`).
- Optional clue for players who did the Sevii quest first: in the ROCKET WAREHOUSE on Five
  Island, a dropped order: "SILPH prototypes → SLATEPORT. Operation PRIMAL. — G."

### Act 1: the arrival

- Slateport Harbor: ROCKET grunts carry crates marked SILPH CO. off the ship and are gone. BIRCH
  meets the player, then takes them to his lab, and Hoenn's story starts there (section 3).

### Act 2: the shadow, at Emerald's own beats

One short scene each; Emerald's scripts stay as they are around them.

| Where (Emerald's beat) | New scene |
|------------------------|-----------|
| Rustboro, Devon Corporation (the stolen DEVON GOODS) | MR. STONE: the thief's team got its tools from someone else; the parts came from Kanto |
| Slateport, the Oceanic Museum (Aqua's raid) | A ROCKET grunt among the Aqua grunts: a battle; he lets slip "the ORBS are for the BOSS" |
| Mt. Chimney (Magma and the METEORITE) | ALAIN's first battle. He measures the volcano's energy for "the foundation" |
| Weather Institute (Aqua's raid) | ALAIN's second battle; a man in a black suit is seen leaving by helicopter |
| Mt. Pyre summit (both ORBS stolen) | ROCKET grunts watching from the mist, gone after the scene |

### Act 3: the climax, after Rayquaza

Emerald's crisis plays as it is: Kyogre in the Seafloor Cavern, Groudon from the Magma
Hideout, the storm and the drought over Sootopolis, the climb up Sky Pillar, RAYQUAZA calming
them. Then the new part:

- **Cave of Origin**: GIOVANNI is there with ALAIN, the two ORBS taken from Archie and Maxie,
  and a MASTER BALL for each legendary. ALAIN sees what the research was for and refuses.
- **Battle with GIOVANNI** (about level 78, between the Kanto champion's rematch and MR. M.'s
  tournament; his MEGA KANGASKHAN is the ace). Beaten, he leaves Hoenn: "Defeated twice by the
  same child… I shall return to VIRIDIAN." His Viridian Gym rematch, which today is open right
  after the first Hall of Fame, moves to after this battle (`FLAG_DEFEATED_GIOVANNI_HOENN`).
- **ALAIN** gives the player the BLUE ORB and the RED ORB: only someone the legendaries trust
  should keep them. Optionally, he enters MR. M.'s tournament later.
- Hoenn's story then goes on to the eighth gym, Victory Road and the Hoenn League, as in Emerald.

### Act 4: the legendaries

| Legendary | Where | When | Reward |
|-----------|-------|------|--------|
| RAYQUAZA | Sky Pillar (Emerald's) | After Sootopolis | MEGA RAYQUAZA with DRAGON ASCENT (already in the game) |
| KYOGRE | Marine Cave (Emerald's roaming cave, found from the Weather Institute's reports) | After Hoenn's Hall of Fame | Holding the BLUE ORB: **PRIMAL KYOGRE** (new: phase 6 of `docs/mega_evolution_plan.md`) |
| GROUDON | Terra Cave (same) | After Hoenn's Hall of Fame | Holding the RED ORB: **PRIMAL GROUDON** (new, same phase) |
| LATIAS and LATIOS | Southern Island, today reachable only with the EON TICKET event. A sailor in Lilycove takes the player there | After Hoenn's Hall of Fame | GIOVANNI's last grunts are trying to take LATIAS: a battle, then both can be caught. LATIASITE and LATIOSITE on the island |
| REGIROCK, REGICE, REGISTEEL | Emerald's Braille puzzles | Any time after Dive | As in Emerald |
| JIRACHI (optional) | New scene: the Millennium Comet, seen from the Mossdeep Space Center | After all of the above | A gift from the Space Center's director |

DEOXYS stays on Kanto's Birth Island, and Hoenn's copies of Navel Rock and Birth Island stay
closed (D7). MEW (Faraway Island, today event-only) is left for MEWTWO's story, not this one.

### Act 5: back in Kanto

- GIOVANNI is back in the Viridian Gym for his rematch (and the KANGASKHANITE).
- In MR. M.'s tournament, STEVEN remembers the player from Hoenn.
- BIRCH gives the Mega Stone of the player's Hoenn starter (SCEPTILITE, BLAZIKENITE or
  SWAMPERTITE) after Hoenn's Hall of Fame.

## 0. Decisions

These need an answer before phase 1. The recommendation is in the second column.

| # | Question | Recommendation | Why |
|---|----------|----------------|-----|
| D1 | When Hoenn opens | After the first Kanto Hall of Fame (`FLAG_SYS_GAME_CLEAR`), with BIRCH's letter and the ROCKETS boarding at Vermilion (Act 0). It runs alongside the Sevii Islands quest | The player can Surf and Fly, so Hoenn must be gated by its own badges anyway (D4); opening it after the League gives a second region as the post-game, as in HGSS |
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

What Operation Primal needs, on top of sections 1 to 5:

- **Trainers**: `TRAINER_GIOVANNI_HOENN` (a new party, about level 78, Mega Kangaskhan);
  ALAIN's two battles (Mt. Chimney, Weather Institute) at the rescaled levels, his full team
  in the second; ROCKET grunts in Hoenn reuse the Kanto grunts' class, pics
  (`TRAINER_PIC_KANTO_ROCKET_GRUNT_M`/`_F`) and objects (`OBJ_EVENT_GFX_KANTO_ROCKET_M`/`_F`);
  GIOVANNI's object is `OBJ_EVENT_GFX_KANTO_GIOVANNI`. ALAIN's are ready.
- **Scenes**: Act 0 in Pallet Town (OAK) and Vermilion City; the optional order in the Rocket
  Warehouse; Act 1 in Slateport Harbor; the five scenes of Act 2; the Cave of Origin; Southern
  Island's grunts; Act 5's lines. New flags: one per scene, in the block of section 1.
- **Viridian Gym**: `ViridianCity_Gym_OnTransition` shows GIOVANNI after
  `FLAG_DEFEATED_GIOVANNI_HOENN` instead of after `FLAG_SYS_GAME_CLEAR`.
- **Primal Reversion**: phase 6 of `docs/mega_evolution_plan.md`: `SPECIES_KYOGRE_PRIMAL` and
  `SPECIES_GROUDON_PRIMAL`, Primordial Sea and Desolate Land, the switch-in trigger with the
  ORBS (`ITEM_BLUE_ORB` and `ITEM_RED_ORB` already exist, as key items: they become hold items).
- **Southern Island**: a sailor in Lilycove Harbor, shown after `FLAG_HOENN_GAME_CLEAR`,
  instead of the EON TICKET; the LATIASITE and LATIOSITE as items on the island.
- **Jirachi** (optional): the Space Center scene and a gift.

## 8. Phases

1. Phase 1 (shared state). Nothing is reachable yet, so it can't break a playthrough.
2. Phase 2 (the ship and Act 0) with the Test Area sailor, then phase 3 (the arrival and
   Act 1), together: Hoenn becomes reachable here.
3. Phase 4 (levels).
4. Phase 5 (ending and post-game).
5. Phase 6: Acts 2 and 3 (ALAIN, the ROCKET scenes, GIOVANNI's battle, the Viridian Gym).
6. Phase 7: Act 4 (Primal Reversion, Southern Island, then Jirachi) and Act 5.
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
