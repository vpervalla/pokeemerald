# Plan: Opening Hoenn

This is a plan for making the Hoenn maps, which are still in the ROM, reachable during the
story: a ship from Vermilion to Slateport, Hoenn's own story played from Professor Birch's
lab to its Hall of Fame, with Team Aqua and Team Magma after Kyogre's and Groudon's Primal
Reversion, and Hoenn's legendaries and post-game. Phases 1 to 8 are implemented (see Status).

**Changed after phase 6** (D5, D9-D11, section 3b, phase 7): on the first arrival, Team Aqua and Team
Magma steal the player's POKéMON and BAG and cut the link with Kanto (the ship home and Kanto's PC
boxes). The player starts Hoenn with BIRCH's starter, at Emerald's own levels: phase 4's rescale is
undone. Only the MEGA RING comes back during the story (Magma Hideout); the team, the rest of the bag
and the link come back after Hoenn's Elite Four, so nothing from Kanto helps in Hoenn's League (ARCHIE
and MAXIE bring them to Littleroot). Until then the PC is split, 7 boxes for Kanto and 7 for Hoenn,
merged into 14 after the League.

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
PC. The credits' starter and the Hall of Fame's flags are handled in phase 5.

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

Phase 3 is implemented: BIRCH's welcome and Hoenn's opening. Checked in an emulator (a test build
that starts a new game in SLATEPORT's harbor, `VAR_HOENN_STORY_STATE` 3): the welcome, the warp, the
starter bag, the nickname, BIRCH's last lines, and walking out into LITTLEROOT. Hoenn's maps draw
right since the fix below.

- **Welcome** (`SlateportCity_Harbor`): on the first arrival (`VAR_HOENN_STORY_STATE` 3), BIRCH
  (`FLAG_HIDE_SLATEPORT_HARBOR_BIRCH`, shown only for the scene) walks up, says OAK and BLAINE called
  him, and takes the player to his lab: state 4, `FLAG_ARRIVED_IN_HOENN`.
- **Emerald's opening skipped** (`HoennStory_EventScript_ReadyHoennOpening`): the truck, the
  clock, the rival's bedroom and the Route 101 rescue are marked done; the player's-gender house in
  LITTLEROOT has no MOM (both MOMs, the one outside and the movers are hidden), and the player
  respawns in its bedroom.
- **Starter** (`LittlerootTown_ProfessorBirchsLab`): BIRCH's bag (`ChooseHoennStarter`, which
  only stores the choice in `VAR_HOENN_STARTER`, without Route 101's battle), then the script gives
  it at level 5, to the PC if the party is full, with a nickname; then BIRCH sends the player to
  ROUTE 103: `VAR_BIRCH_LAB_STATE` 3, state 5. From here Emerald's own vars lead.
- **Pokédex**: after the rival, BIRCH gives no POKéDEX (the player has OAK's) and the lab sets
  `VAR_LITTLEROOT_TOWN_STATE` to 4, past MOM's RUNNING SHOES. The rivals' Route 103 greeting
  knows the player is the KANTO CHAMPION.
- **Test Area**: the ride to SLATEPORT first sets state 3, so BIRCH's welcome plays.
- Not done: a bed that heals in the guest room (D6); NORMAN is still the player's father in
  Emerald's text.

Phase 4 was implemented, and **undone** in phase 7 (D5 changed: Hoenn keeps Emerald's levels, since the
player plays it with a new team). What it did: Hoenn's levels, by
`tools/hoenn/rescale_levels.py`, run once (it marks `src/data/trainer_parties.h` and refuses to run
again). Every level L becomes round(53 + 0.47 L):

- **Trainers** (`src/data/trainer_parties.h`, all of Emerald's but the four link placeholders):
  1821 POKéMON, now 54-80 in the story (WALLACE 79-80) and up to 90 in the rematches. 980 of them
  evolved to the stage their level allows: level evolutions at their level, friendship at 30,
  beauty at 35, stones at 40, trades at 45, a branching line by its first branch (SHEDINJA never).
  Custom movesets keep up to two moves the POKéMON doesn't learn by level (ROXANNE's ROCK TOMB, the
  TMs), and the rest are its latest level-up moves. The Mansion's raiders are in their own file and
  keep their levels.
- **Wild POKéMON** (`src/data/wild_encounters.json`, the 124 tables of Hoenn's maps): the same curve
  (ROUTE 101 at 54, VICTORY ROAD at 72). The species stay, so every POKéMON can still be caught in
  its first stage. The Battle Pyramid's and Pike's tables are their facilities'.
- **Static encounters** (`setwildbattle` in Hoenn's maps): KYOGRE, GROUDON and RAYQUAZA at 86, the
  REGIS at 72, KECLEON, the ELECTRODE, the VOLTORB and SUDOWOODO. Gifts (`givemon`: the Hoenn
  starter, CASTFORM, BELDUM, the fossils, the egg) keep their levels, to be raised.
- Not done: the roamer. `src/roamer.c` now makes the Kanto beasts roam at 50 (the Kanto port), so
  Emerald's roaming LATIAS or LATIOS after Hoenn's Hall of Fame needs phase 5 or 7.

Phase 5 is implemented: Hoenn's ending. Checked in an emulator with a test build that starts a new
game in EVER GRANDE's Hall of Fame (Kanto cleared, a TORCHIC in the party): the record, the save,
the credits, Continue, and the guest room's two floors.

- **Hall of Fame**: Emerald's, plus `FLAG_HOENN_GAME_CLEAR` (phase 1). `GameClear` already leaves
  Kanto alone: `FLAG_SYS_GAME_CLEAR` is set by then, and Continue puts the player in the
  LITTLEROOT guest room (only Kanto's Hall of Fame sends them to PALLET TOWN). Hoenn's post-game
  (the S.S. TIDAL, the Battle Frontier, Steven, Meteor Falls, Trainer Hill) opens through
  `FLAG_HOENN_GAME_CLEAR`.
- **No S.S. TICKET scene**: Emerald's DAD bringing the ticket and MOM's LATIAS/LATIOS report need
  the MOM the guest room doesn't have; `FLAG_RECEIVED_SS_TICKET` is set instead. The S.S. TIDAL only
  checks Hoenn's Hall of Fame. Without the report, no LATIAS or LATIOS roams: Southern Island
  (phase 7) has both.
- **Credits**: they show the starter of the region whose Hall of Fame it is (`DeterminePokemonToShow`
  in `src/credits.c`); after Kanto's they showed a Hoenn starter.
- **Guest room**: the arrival now also hides what Emerald's truck and the rival's bedroom would have
  (both trucks, the rival's family and POKé BALL in the player's house, the rival at home) and shows
  LITTLEROOT's fat man.
- The defeated flags Hoenn's Hall of Fame clears (LUGIA, HO-OH, DEOXYS, MEW, the LATI) are only
  read by Hoenn's own copies of those maps, so Kanto's legendaries aren't affected.

Phase 6 is implemented: Primal Reversion in battle, and Acts 2 and 3.

- **Primal Reversion**: see "Phase 6 status" in `docs/mega_evolution_plan.md`. PRIMAL KYOGRE and
  PRIMAL GROUDON, Primordial Sea and Desolate Land, the ORBS as held items. Checked in an emulator in
  wild battles on both sides.
- **Act 2's lines**: MR. STONE (DEVON's 3F, giving the POKéNAV) heard the thief speak of "the KANTO
  file"; MAXIE (Mt. Chimney, after his battle) explains the METEORITE, MEGA STONES and the RED ORB;
  SHELLY (Weather Institute) recognizes the player from the MANSION and names KYOGRE's "true form";
  ARCHIE (Mt. Pyre summit) names PRIMAL REVERSION and means to control KYOGRE with the RED ORB, and the
  old lady says the ORBS were made to calm the two. In Seafloor Cavern, ARCHIE sees the file never
  said its primal form could be controlled. Emerald's MAXIE and ARCHIE already argue there, so the
  scene stays as it is.
- **PRIMAL NOTES** (`ITEM_PRIMAL_NOTES`, a key item after the CARD CASE, with the LETTER's icon): in
  the AQUA HIDEOUT, where MATT's submarine leaves, the player finds TEAM AQUA's half of the file and
  reads its last page (the third POKéMON from the sky, changing "as our MEGA EVOLUTIONS change").
  BIRCH takes it whenever the player talks to him with it, and sends it to BLAINE
  (`FLAG_GAVE_PRIMAL_NOTES_TO_BIRCH`, `KANTO_EXTRA_FLAGS_START + 0x190`, for Act 5).
- **Sky Pillar**: when RAYQUAZA wakes, the player's MEGA RING glows and RAYQUAZA stares at it.
- **Sootopolis**: the KYOGRE, GROUDON and RAYQUAZA objects are new graphics ids
  (`OBJ_EVENT_GFX_KYOGRE_PRIMAL`, `_GROUDON_PRIMAL`, `_RAYQUAZA_MEGA`,
  `src/data/object_events/hoenn_legend_object_events.h`): Emerald's pics with Primal and Mega palettes,
  in the special NPC's two palette slots (the slots they shared before, NPC_3 and NPC_4, are also
  STEVEN's, WALLACE's and the residents'). Emerald's cutscene (`src/rayquaza_scene.c`, played only
  here) shows KYOGRE and GROUDON with their Primal palettes. RAYQUAZA takes flight from Sky Pillar
  green, and comes down to chase them away black and gold. Then a line says it Mega Evolved and the two
  lost their primal forms. `tools/hoenn/legend_palettes.py` makes the palettes.
- Not done: the cutscene's close-ups of RAYQUAZA coming down (scenes 3 and 4) share their palettes with
  the background, so RAYQUAZA is still green there, and it isn't MEGA RAYQUAZA's shape anywhere. The
  ORBS don't glow with the Primal colors in Seafloor Cavern (Emerald's `DoOrbEffect`).

Checked in an emulator with test builds: the Sootopolis clash (cutscene and overworld) and RAYQUAZA's
arrival, from the POKéMON CENTER, and BIRCH taking the PRIMAL NOTES. The new lines were measured
against the text box (205 pixels at most, Emerald's go to 212). Not played in-game: the AQUA HIDEOUT and
Sky Pillar scenes.

Phase 7 is implemented: the theft, the split PC and the returns (section 3b, D9-D11), and Emerald's
levels back. Checked in an emulator with three test builds (a party of two with a held MEGA STONE, a
MEGA RING, RARE CANDIES, POTIONS, ¥50000, and Kanto POKéMON in boxes 1, 9 and 14), reading the save data
from memory after each:

- **The crossing** (`SlateportCity_Harbor`, state 3): the night narration on a black screen, the
  SAILOR, then BIRCH, who lends ¥3000 and takes the player to the lab for the starter. After it: an
  empty party and BAG, ¥3000, the Kanto POKéMON of boxes 9 and 14 moved after the one in box 1, boxes
  8-14 named HOENN 1 to HOENN 7, and everything in the stash. Then the lab gives the starter.
- **The PC**: it opens on HOENN 1, and the box arrows go round HOENN 1 to HOENN 7 only.
- **The return** (`LittlerootTown`, state 6): ARCHIE and MAXIE outside the guest room give everything
  back. The two POKéMON went to box 1, the BAG merged with Hoenn's (3 + 2 POTIONS, a single MEGA RING),
  the money added (¥53000), boxes 8-14 renamed as before, the stash empty.
- **Code**: `src/hoenn_story.c` (the specials `CheckKantoBoxesFit`, `StealPlayerTeamAndBag`,
  `TakeStolenMegaRing`, `RestoreKantoLink`, `ScriptIsKantoLinkCut`), the stash
  `struct HoennStolenTeam` at the end of `struct PokemonStorage` (a `STATIC_ASSERT` checks it fits),
  `GetFirstUsableBox`/`ClampToUsableBox` in the PC's box menu, arrows and current box, in
  `CopyMonToPC` (catches and gifts), `IsPokemonStorageFull`, `CheckFreePokemonStorageSpace` and
  `IsDestinationBoxFull`. The player's PC closes its item storage while the link is cut, and hands
  over the stash's leftovers after.
- **Scripts**: the Vermilion sailor's check (210 boxed POKéMON at most), the Slateport and Lilycove
  sailors ("the line to Kanto is cut"), the Route 117 DAY CARE closed (it shares Four Island's
  POKéMON), the MEGA RING in the Magma Hideout, MATT's taunt, ARCHIE and MAXIE's promise in
  Sootopolis, the Hall of Fame setting state 6, and the scene in LITTLEROOT (state 7 after it).
  `FLAG_HOENN_TEAM_STOLEN`, `FLAG_HOENN_MEGA_RING_RETURNED`, `FLAG_HOENN_KANTO_LINK` and
  `FLAG_HIDE_LITTLEROOT_ARCHIE_MAXIE` are `KANTO_EXTRA_FLAGS_START + 0x191`-`0x194`.
- **Levels**: commit `6b0697cd` reverted, keeping the later Sky Pillar scene: Emerald's trainers, wild
  POKéMON and static encounters (KYOGRE, GROUDON and RAYQUAZA at 70).
- Not played in-game: the Magma Hideout and Sootopolis lines, a catch going to HOENN 1, the item
  storage message. The POKéNAV's condition and ribbon lists still list the Kanto boxes' POKéMON (they
  only show them).

Phase 8 is implemented: Act 4's legendaries and Act 5. Checked in an emulator with test builds (after
Hoenn's Hall of Fame, the link mended, a TORCHIC starter): BIRCH's stone, the ORBS, LATIAS on Southern
Island and JIRACHI.

- **The ORBS** (`MtPyre_Summit`, `HoennStory_EventScript_TryGivePrimalOrbs`): after Hoenn's Hall of Fame,
  once ARCHIE and MAXIE have brought the ORBS back (`VAR_MT_PYRE_STATE` 3), the old lady gives the BLUE
  and the RED ORB: "The one RAYQUAZA trusts may keep them". KYOGRE and GROUDON are where Emerald puts
  them after its Hall of Fame: Marine Cave and Terra Cave, found from the Weather Institute's reports
  (already on Hoenn's flags since phase 1).
- **Southern Island**: LILYCOVE's SEAGALLOP sailor offers it after Hoenn's Hall of Fame, before the
  ride to Kanto (`HoennStory_EventScript_LilycoveSailor`); Emerald's EON TICKET isn't needed. LATIAS is
  there first, then LATIOS once she's caught (`FLAG_CAUGHT_SOUTHERN_ISLAND_LATIAS`/`_LATIOS`), at
  level 50 with the SOUL DEW, as in Emerald. A defeated LATI flies away and is back on the next visit.
  The LATIASITE and LATIOSITE are item balls on either side of the pond.
- **JIRACHI** (`MossdeepCity_SpaceCenter_1F`, the scientist on the left as the director): with the ORBS
  and both LATI, the MILLENNIUM COMET has left a JIRACHI at level 5.
- **BIRCH** (Act 5): after Hoenn's Hall of Fame, the MEGA STONE of the starter he gave
  (`VAR_HOENN_STARTER`: SCEPTILITE, BLAZIKENITE or SWAMPERTITE), "sent by BLAINE".
- **BLAINE** (Act 5, his gym's rematch): once the link is mended, he thanks the player, for the file
  if BIRCH sent it.
- **STEVEN** (Act 5, MR. M's tournament, story and replays): after Hoenn's Hall of Fame he remembers
  the player from SOOTOPOLIS.
- New flags `KANTO_EXTRA_FLAGS_START + 0x195`-`0x19C`.
- Not played in-game: LATIOS (the same script), the Mega Stone item balls, BLAINE's and STEVEN's lines.

## Hoenn's tilesets

The port set the map engine to FRLG's tileset layout (`include/fieldmap.h`: 640 tiles and metatiles
in the primary tileset, 7 palettes), and Hoenn's tilesets use Emerald's (512, 512, 6). So on every
Hoenn map the secondary tileset's tiles, metatiles and first palette are read from the wrong place.
Converting the data isn't possible: 32 of the 70 secondary tilesets Hoenn uses need more than the
384 tiles FRLG's layout leaves them, 15 more than 384 metatiles.

Fixed in the engine: the layout follows the map's primary tileset. `Tileset.isHoennLayout` is set on
Emerald's primaries (`gTileset_General`, `gTileset_Building`, `gTileset_SecretBase`), and
`GetNumTilesInPrimary`, `GetNumMetatilesInPrimary` and `GetNumPalsInPrimary` (`src/fieldmap.c`)
replace the old constants where the map matters: loading tiles and palettes, metatile attributes,
drawing (`field_camera.c`), the day/night tint, the shop's map background, the connection reload.
Code that only runs on Hoenn maps uses `HOENN_NUM_*`: Hoenn's tileset animations, decorations,
Trainer Hill. Checked in the emulator: SLATEPORT's harbor, BIRCH's lab and LITTLEROOT draw right, and
so does the player's room in PALLET TOWN.

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
  start of the post-game levels (they're in Kanto, so they keep them).
- Vermilion's pier: the two teams' boats leave for Hoenn as the player arrives. A sailor
  offers the ship to Slateport (`FLAG_HOENN_OPEN`). BLAINE asks the player to get the file
  back: "If those fools force that power out of Kyogre and Groudon, nothing will stop it."

### Act 1: the arrival, and the theft

- **The crossing**: in the night, the SEAGALLOP is stopped at sea. An Aqua submarine comes alongside
  and Magma and Aqua grunts, together for once, climb aboard while the player sleeps. When the player
  wakes, the POKé BALLS and the BAG are gone, and the ship's radio is smashed. SHELLY and TABITHA knew
  BLAINE would send the KANTO CHAMPION after the file; with the team gone, the player is no threat.
- **Slateport Harbor**: the sailor docks and can't sail back ("the teams cut the cable to KANTO too:
  no radio, no PC link"). BIRCH, warned by OAK before the line went dead, meets the player, hears what
  happened, and takes them to his lab. Hoenn's story starts there (section 3), with BIRCH's starter
  and ¥3000 he lends. BIRCH has heard of the ORBS, kept at MT. PYRE.
- From here until after Hoenn's Hall of Fame: the SEAGALLOP doesn't sail to Kanto, the PC shows only
  the seven HOENN boxes, and the player's item storage is closed (section 3b).

### Act 2: the race, at Emerald's own beats

Emerald's scenes stay as they are; a few lines and one item are added where the teams talk
about their plans.

| Where (Emerald's beat) | Added |
|------------------------|-------|
| Rustboro, Devon Corporation (the stolen DEVON GOODS) | MR. STONE: the thief spoke of "the Kanto file" |
| Mt. Chimney (Maxie and the METEORITE) | Maxie: the file says the METEORITE's energy is what Mega Stones are made of; it will wake Groudon, and the RED ORB will make it Primal |
| Weather Institute (Shelly's raid) | Shelly: Aqua's half of the file says the BLUE ORB awakens Kyogre's "true form" |
| Mt. Pyre summit (both ORBS stolen) | Archie and Maxie name Primal Reversion. The old lady: the ORBS were never meant to make them stronger, only to calm them |
| Magma Hideout (MAXIE awakens GROUDON) | **New:** after MAXIE's battle, a grunt's storeroom on 4F holds only the MEGA RING, taken from the BAG to study (TABITHA's half of the file speaks of it): the player gets it back. The rest of the BAG went with the grunts (section 3b) |
| Aqua Hideout (MATT, the submarine) | **New:** MATT taunts the player: the POKé BALLS stolen on the SEAGALLOP left with ARCHIE in the submarine. The team isn't here |
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
- **A promise**: when ARCHIE and MAXIE leave the player in Sootopolis (Emerald's scene before
  WALLACE's gym opens), they admit the storm scattered their grunts, and the POKé BALLS with them.
  They'll gather them and have the cable to Kanto mended, but it will take time. Nothing comes back
  here: the player faces WALLACE and the League with the Hoenn team.
- **After**: Archie and Maxie give the ORBS back at MT. PYRE, as in Emerald. The player hands
  the PRIMAL NOTES to BIRCH, who sends them to BLAINE (the mail still goes by sea; only the cable is
  cut). Hoenn's story goes on to the eighth gym,
  Victory Road and the Hoenn League.
- **After the Hall of Fame**: on the first morning after Hoenn's Elite Four, ARCHIE and MAXIE wait in
  LITTLEROOT, outside the guest room. They give back the POKé BALLS and the BAG their grunts took,
  and the cable is mended: the Kanto team goes to the PC (the party is the new Champion's), the
  Kanto items and money join the player's, the PC's 14 boxes are merged, and the item
  storage are back, and the SEAGALLOP sails home (`FLAG_HOENN_KANTO_LINK`).

### Act 4: the legendaries

| Legendary | Where | When | Reward |
|-----------|-------|------|--------|
| RAYQUAZA | Sky Pillar (Emerald's, at level 70 like the others: Emerald's levels, D5) | After Sootopolis | MEGA RAYQUAZA, with DRAGON ASCENT and the player's MEGA RING (already in the game) |
| KYOGRE | Marine Cave (Emerald's roaming cave, found from the Weather Institute's reports) | After Hoenn's Hall of Fame | The old lady of MT. PYRE gives the BLUE ORB: held, **PRIMAL KYOGRE** in battle (new: phase 6 of `docs/mega_evolution_plan.md`) |
| GROUDON | Terra Cave (same) | After Hoenn's Hall of Fame | The RED ORB, from the same old lady: **PRIMAL GROUDON** (new, same phase) |
| LATIAS and LATIOS | Southern Island, today reachable only with the EON TICKET event. A sailor in Lilycove takes the player there | After Hoenn's Hall of Fame | Both can be caught; LATIASITE and LATIOSITE on the island |
| REGIROCK, REGICE, REGISTEEL | Emerald's Braille puzzles | Any time after Dive | As in Emerald |
| JIRACHI (optional) | New scene: the Millennium Comet, seen from the Mossdeep Space Center | After all of the above | A gift from the Space Center's director |

The old lady gives the ORBS after Hoenn's Hall of Fame (phase 8; RAYQUAZA came at the MEGA RING's
call in Sootopolis, so she trusts the player): "The one RAYQUAZA trusts may keep them." DEOXYS stays on
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
| D5 | Levels | **Changed:** Emerald's own levels (3-58 in the story, up to 78 in the rematches), for trainers, wild and static Pokémon. Phase 4's rescale is undone. (It was: raise everything once to a post-game band, 54-80) | The player plays Hoenn's story with BIRCH's starter, not with the Kanto team (D9), so Emerald's curve fits again. The Kanto team, once back, is for Hoenn's post-game: the rematches and the Battle Frontier |
| D6 | The player's home in Hoenn | None: the house of the player's gender in Littleroot becomes a guest room BIRCH lends them (no MOM, a bed that heals). The rival is the other house's child, chosen by `checkplayergender` as now | The player comes from Pallet Town; six characters can be picked, so Brendan's or May's own home can't be the player's |
| D7 | Hoenn's Navel Rock and Birth Island | Taken out of Lilycove Harbor's ferry menu. Kanto's are the ones with the event | Two copies of Lugia, Ho-Oh and Deoxys behind the same flags |
| D8 | Hoenn's Hall of Fame | Records the team and sets `FLAG_HOENN_GAME_CLEAR`, with Hoenn's credits but no `GameClear` reset of the Kanto story; Continue puts the player in the Littleroot guest room | `FLAG_SYS_GAME_CLEAR` and `FLAG_SYS_CAN_LINK_WITH_RS` already mean Kanto's progress |
| D9 | The Kanto team in Hoenn | **New:** on the first arrival, Team Aqua and Team Magma steal the player's party and BAG (money included) and cut the link with Kanto. BIRCH gives the starter and lends ¥3000 | Hoenn's story is played again from the start, at its own pace and levels; the teams have a reason to fear the player (BLAINE sent them) |
| D10 | The PC while the link is cut | **Changed:** the 14 boxes are split in two halves: boxes 1-7 are Kanto's, out of reach; boxes 8-14 are Hoenn's (HOENN 1 to HOENN 7), the only ones the PC shows, emptied into boxes 1-7 on arrival. The item storage is closed. Catches and gifts go to the Hoenn boxes. After Hoenn's Hall of Fame the halves merge into 14 boxes again. (It was: two boxes, 13 and 14, for Hoenn) | The player can't fetch the Kanto team from a box. 210 places are what a Hoenn playthrough needs (Emerald's 202 species), and both regions get the same. The storage's 14 boxes can't grow (save space). The cost: the Kanto POKéMON must fit in 7 boxes, at most 210 of them, checked before sailing |
| D11 | When things come back | **Changed:** only the MEGA RING after beating MAXIE in the Magma Hideout; the stolen POKéMON, the rest of the BAG, the money and the link (all 14 boxes, the item storage, the ship home) after Hoenn's Hall of Fame, from ARCHIE and MAXIE in LITTLEROOT. (It was: the BAG after MAXIE, the team after MATT, the link in Sootopolis) | The team that beat Kanto's Elite Four, or its RARE CANDIES, MASTER BALL and TMs, would make Hoenn's League easy, so they come back as late as the story allows: the teams promise them in Sootopolis and deliver after the League. The MEGA RING is needed before, at Sky Pillar. The team goes to the PC, since the party is the one that just won |

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

## 3b. The theft and the cut link (D9-D11)

**State.** `FLAG_HOENN_TEAM_STOLEN` (set by the crossing, cleared when everything is back),
`FLAG_HOENN_MEGA_RING_RETURNED` and `FLAG_HOENN_KANTO_LINK`, in the Hoenn block of section 1
(`KANTO_EXTRA_FLAGS_START + 0x191`-`0x193`).

**The stash.** What the teams took is kept in the save, in `struct PokemonStorage`'s free space
(after the Kanto port's vars and flags: about 1650 bytes of the storage's 9 sectors are unused;
SaveBlock1 has only 24 left):

| Field | Size |
|-------|-----:|
| `stolenParty[PARTY_SIZE]` (`struct Pokemon`) and `stolenPartyCount` | 601 |
| `stolenBag`: the five pockets' `struct ItemSlot`s (30 + 30 + 16 + 64 + 46), quantities decrypted | 744 |
| `stolenMoney` (decrypted) | 4 |
| The old names of boxes 8-14 | 63 |

About 1410 bytes, so about 240 are left.

A `STATIC_ASSERT` checks that the storage still fits its sectors.

**The theft** (a special, `StealPlayerTeamAndBag`, run by the SEAGALLOP's arrival when
`VAR_HOENN_STORY_STATE` is 3): copies the party, the pockets and the money to the stash; empties the
party and the pockets; sets the money to 3000; moves the POKéMON in boxes 8-14 to free places in
boxes 1-7, keeping their order; names boxes 8-14 HOENN 1 to HOENN 7 (their old names are kept in the
stash, and come back when the halves merge; the wallpapers stay). What stays: the flags (POKéDEX, RUNNING SHOES,
badges, Kanto progress), the PC's boxes 1-7 and the item storage, out of reach.

**Before sailing**: the Vermilion sailor checks that the PC's POKéMON fit in 7 boxes (at most 210). If
not, he explains that HOENN's PC borrows half of the boxes and asks the player to make room; the
SEAGALLOP doesn't sail until then. Only a player with more than 210 boxed POKéMON meets this.

**While the link is cut** (`FLAG_HOENN_TEAM_STOLEN` set, `FLAG_HOENN_KANTO_LINK` clear):

- `src/pokemon_storage_system.c`: the first usable box (`GetFirstUsableBox`: 7, counting from 0,
  while the link is cut, 0 otherwise) bounds the box list, the arrows and the jump menu, and the PC
  opens on HOENN 1.
- Catches and gifts (`SendMonToPC`, `IsPlayerPartyAndPokemonStorageFull` in `src/pokemon.c`): only
  the HOENN boxes count; with all 7 full, the player can't throw balls, as with a full PC in Emerald.
- The player's PC: the item storage says the link with Kanto is down. The Mailbox and Decoration stay.
- The SEAGALLOP at Slateport and Lilycove: "the line to Kanto is cut" instead of the sailing menu.
- Whiting out with no POKéMON is impossible from here: the player is never without BIRCH's starter
  outside the lab's scripted moments.

**Coming back**, each a special run by the map's script after the battle:

- The MEGA RING (Magma Hideout 4F, after MAXIE): taken out of the stash's KEY ITEMS
  (`TakeStolenItem`) and given with `giveitem`; `FLAG_HOENN_MEGA_RING_RETURNED`. Nothing else.
- `RestoreKantoLink` (LITTLEROOT, ARCHIE and MAXIE, after Hoenn's Hall of Fame): sets
  `FLAG_HOENN_KANTO_LINK`, so all 14 boxes show again, Kanto's 1-7 then Hoenn's 8-14 (boxes 8-14 get back their old names; the HOENN POKéMON stay in
  them). The stolen POKéMON go to the first free places, in
  their order ("sent to BOX n" lines). Each stolen item is added to the BAG (Hoenn's items stay);
  what doesn't fit its pocket goes to the item storage, and what doesn't fit there stays in the
  stash, handed over by the player's PC as room is made. The money is added to the player's. Then it
  clears `FLAG_HOENN_TEAM_STOLEN`.
- The scene: a frame script in `LittlerootTown` when `FLAG_HOENN_GAME_CLEAR` is set and
  `FLAG_HOENN_KANTO_LINK` clear. Hoenn's Hall of Fame sends the player to the guest room (phase 5),
  so the next step outside meets them. ARCHIE and MAXIE are new objects there, hidden otherwise.
- No gaps: the Magma Hideout comes before Sky Pillar in Emerald's story, and the Hall of Fame can't be
  skipped. A player with all 210 HOENN places full before the League catches nothing more until
  then, as with a full PC.

**Text.** BIRCH's welcome and lab (Act 1), the Slateport and Lilycove sailors, the PC's messages, the
MEGA RING's return, MATT's taunt, ARCHIE and MAXIE's promise in Sootopolis and their visit to LITTLEROOT. The Route 103 rival greeting (phase 3) says the player is
the KANTO CHAMPION; it can stay, with "...even without your team".

## 4. Levels

**Changed (D5): Hoenn keeps Emerald's levels.** Phase 7 undoes phase 4: the data of commit `6b0697cd`
(`src/data/trainer_parties.h`, `src/data/wild_encounters.json`, the `setwildbattle` levels of the
Hoenn maps) goes back to Emerald's, keeping the later edits to those maps' scripts (Sky Pillar's
MEGA RING scene). `tools/hoenn/rescale_levels.py` and its stray `__pycache__` are removed. The
Mansion's raiders (Act 0, in Kanto) keep their levels. What phase 4 was:

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
6. Phase 7 (new, after phase 6): the theft and the cut link (Act 1's crossing, section 3b, D9-D11)
   and Emerald's levels back (D5, undoing phase 4), with the 7 and 7 box split. Then the returns: the
   MEGA RING in the Magma Hideout,
   MATT's and ARCHIE and MAXIE's lines, and the team and the link in LITTLEROOT after the Hall of Fame.
7. Phase 8: Act 4 (the ORBS, Southern Island, then Jirachi) and Act 5. Done.
8. The other features (section 6), each on its own.

## 9. Tests

- The reachability check, `tools/hoenn/check_reachability.py` (already written: it made the
  table at the top): from the player's room, the warps, connections, script warps, Seagallop
  destinations and heal locations. Phase 2 adds `SEAGALLOP_SLATEPORT`'s harbor. Today it finds
  2 of the 518 non-Kanto maps (the link rooms); after phase 2 it should find Hoenn's 369 story
  maps, the Battle Frontier and the event islands that Hoenn's own story opens, and still not
  Hoenn's Navel Rock and Birth Island.
- A check that no Hoenn map reads `FLAG_BADGE0n_GET`, `FLAG_SYS_GAME_CLEAR` or
  `VAR_STARTER_MON`, and no Kanto map reads the Hoenn flags.
- The theft and the returns (phase 7), with test builds: a party of six with held items and a full
  BAG, a Kanto PC with POKéMON in boxes 8-14; after the crossing, an empty party and BAG, ¥3000,
  boxes 1-7 out of reach with every Kanto POKéMON in them; the sailor refusing with 211 boxed
  POKéMON; a catch with a full party going to HOENN 1; only the MEGA RING back after MAXIE; still the
  HOENN boxes only after Sootopolis; the team, the BAG, the money and all 14 boxes back in LITTLEROOT
  after the Hall of Fame, and nothing lost or doubled (count the POKéMON and items before and after).
  A save and reload at each step.
- In-game: a save after the first Kanto Hall of Fame; the ship both ways; Birch's lab to the
  first gym; that Surf does nothing in Hoenn before Norman and works in Kanto; Fly between the
  regions; whiting out in Hoenn; the Hoenn Hall of Fame and Continue after it.
