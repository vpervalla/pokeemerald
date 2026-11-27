# Playthrough bugs

Found while playing the build headless (see `playthrough_guide.md`, "Goal"). One row per bug; the details
(steps, evidence, workaround) go under the table. Status: open, workaround, fixed (commit), won't fix.

| # | Where (map, x,y) | Kind | Summary | Status |
| --- | --- | --- | --- | --- |
| 1 | PalletTown_PlayersHouse_1F (5,8) | Missing interaction | The right tile of the exit does not leave the house | open |
| 2 | CeladonCity_GameCorner (10,13), (11,13); RocketHideout_Elevator (1,5) | Dead warp tiles | Two of the Game Corner's three exit tiles and one of the elevator's two leave nothing (same cause as #1) | open |
| 3 | RocketHideout_B2F, B3F (spinner mazes) | Camera/position desync | After a spinner slide the save-block position (camera) stays behind the player; walls and map edges then block moves that the map allows | open |
| 4 | SafariZone_Center (Kanto Safari Zone), START > RETIRE | Wrong destination | Retiring, running out of SAFARI BALLs or of steps in the Kanto Safari Zone warps to Route 121's Hoenn Safari entrance, with no way back until a later badge (FLY) | **fixed** (commit f2a9d4e0d, merged; timeout verified: back to the Fuchsia entrance) |
| 5 | FuchsiaCity_WardensHouse (11,6) and every Kanto boulder | Strength can't push | The boulder push check (`src/field_player_avatar.c:850`, also `src/fldeff_strength.c:20`) only accepts `OBJ_EVENT_GFX_PUSHABLE_BOULDER`; Kanto boulders use `OBJ_EVENT_GFX_KANTO_PUSHABLE_BOULDER`, so STRENGTH activates but they never move | **fixed** (both checks accept the Kanto boulder; tested in the Warden's house) |
| 6 | Build (`src/region_map.c`), kanto-hoenn commit d26e26329 | Doesn't compile | "Allow using Fly from the Town Map" uses `FIELD_MOVE_FLY` without including `constants/party_menu.h`: `make modern` stops with "'FIELD_MOVE_FLY' undeclared" | **fixed** (include added; Town Map Fly tested) |
| 7 | TwoIsland (28,4), the market clerk | Empty shop lists | All four item lists of the Two Island shop (`TwoIsland_Items_ShopInitial`, `..._ShopExpanded1..3` in `data/maps/TwoIsland/scripts.inc`) are placeholders (`release` / `end`), so the POKé MART shows "????????" items at ¥0 and offers to sell one | open |

## Details

### 1. The house's exit at (5,8) does nothing

- **Where:** PalletTown_PlayersHouse_1F, tile (5,8). `map.json` lists warps at (5,8), (4,8) and (3,9), all to PALLET_TOWN.
- **Steps:** from the start, go downstairs, walk to (5,8) and press down. Nothing happens, twice. From (4,8) pressing down leaves.
- **Expected:** both tiles of the doorway leave the house.
- **Workaround:** use (4,8).
- **Seen in:** [Pallet Town](walkthrough/01_pallet_town.md).

### 2. Exit tiles that are warp events but not arrow warps

- **Where:** `CeladonCity_GameCorner` warps (9,13), (10,13), (11,13); `RocketHideout_Elevator` warps (1,5), (2,5). The metatile behaviour of (9,13) and (2,5) is `MB_SOUTH_ARROW_WARP`; (10,13), (11,13) and (1,5) are `MB_NORMAL`.
- **Steps:** enter the Game Corner, walk to (10,13) (or step onto it from (10,12)) and press down: nothing. Step left to (9,13) and press down: Celadon City. In the elevator, (1,5) does nothing and (2,5) leaves.
- **Expected:** the whole doormat leaves. Same cause as #1: a warp event on a tile whose behaviour is not an arrow/door warp can never fire.
- **Workaround:** use (9,13) and (2,5).
- **Seen in:** [Game Corner and Rocket Hideout](walkthrough/25_celadon_city--game_corner.md).

### 3. Spinner slides leave the camera position behind (Rocket Hideout B2F, B3F)

- **Where:** the spinner mazes of `RocketHideout_B2F` and `RocketHideout_B3F` (the arrow tiles use `MB_SPIN_*`; `MovementAction_Spin_*` in `src/event_object_movement.c` moves the object with `InitNpcForMovement` at `MOVE_SPEED_FAST_1`).
- **Symptoms:** after a few slides `gSaveBlock1Ptr->pos` (which the field camera keeps) differs from the player's ObjectEvent tile by 1 to 9 tiles. The screen scrolls wrongly (duplicated or black tiles, the sprite outside the map) and `CanCameraMoveInDirection` (called for the camera-tracked player in `ObjectEventCanMoveInDirection`) then refuses moves near the map edge: on B3F, standing at (2,14) with the camera at x=0, LEFT into the open (1,14) did nothing, which closed the only way to the south-west rooms. Items are also not spawned while the camera window is elsewhere.
- **Steps:** B3F from the stairs at (18,2): walk through the maze to (2,14), press left.
- **Evidence:** the ObjectEvent said (2,14) and the save block (0,14); copying the ObjectEvent tile into `pos` made the move work.
- **Workaround:** tools copy the object's tile into `pos` after every step (`tools/playthrough/emu.py: fix_pos`), and `rbfs.py` explores the real game with savestates because the static planner cannot predict slides. Not fixed in the ROM (a rebuild invalidates the savestates): the cause is in the spin movement not feeding `CameraObject`'s per-frame deltas.
- **Seen in:** [Game Corner and Rocket Hideout](walkthrough/25_celadon_city--game_corner.md).

### Observations (not bugs)

- **Card duel NPCs.** The rocker in the Route 2 east building starts the ROM's card game (`cardduel CARD_OPPONENT_ROUTE2_ROCKER`) straight after his FLASH hint, with no yes/no question: talking to him always begins a duel. The duel screen draws and takes input; Forfeit (last action) then A ends it with "You lost the duel..." and the overworld returns. A full duel has not been played: still to test (win and lose paths, the reward).
- **Rocket Hideout stair warps** (B1F (12,2), (17,2), B2F (21,2), (28,2), (23,12), B3F (18,2), (15,18), B4F (11,15)) fire when a direction is pressed on the tile (the stair's own direction: the B2F stair at (21,2) needs RIGHT, the B3F one at (18,2) needs RIGHT); walking onto the tile does nothing.
- **Hideout barriers:** B1F's barrier (20..21,19..20) opens when the grunt at (21,27) is beaten, who can only be reached by the elevator (LIFT KEY from the B4F grunt); B4F's barrier (17..18,12..14) opens when both grunts at (16,14) and (19,14) are beaten (one is on each side; the east side is reached by the elevator too). The elevator rooms' panel is at (0,2): face up from (0,3).
- **Cerulean's east road** is only reachable from the west plateau through the robbed house (hedge at x=34 and a one-way ledge row at y=20). That matches the layout and is the intended way, but it makes the walk from the Center to Route 9 long.

### 4. RETIRE in the Kanto Safari Zone warps to Hoenn

START menu > RETIRE > YES in the Fuchsia Safari Zone (maps 34.63 and the other Kanto Safari maps) moves the player to `ROUTE121_SAFARI_ZONE_ENTRANCE` (Hoenn, 23.0), the Hoenn Safari's own exit warp, with the Safari state ended. The Hoenn and Kanto Safari maps are not connected and FLY needs a badge, so the player is stranded until a later badge. The same exit script runs when the SAFARI BALLs run out or the steps run out (`SafariZone_EventScript_OutOfBalls`, `SafariZone_EventScript_TimesUp` and `OutOfBallsMidBattle` in `data/scripts/safari_zone.inc` all warp to `MAP_ROUTE121_SAFARI_ZONE_ENTRANCE`), so keep at least one ball and some steps. Walk out through the Center's south door (25..27,30) instead: the attendant then asks "Are you going to leave the SAFARI ZONE early?" and returns to Fuchsia.

**Fixed (2026-11-25):** commit f2a9d4e0d branches the three exit scripts on region (`special IsInKantoSafariZone`) and warps Kanto players to `MAP_FUCHSIA_CITY_SAFARI_ZONE_ENTRANCE`. Tested on a rebuilt ROM by letting the steps run out in the West area: the player arrived in the Fuchsia entrance (4,4).

### 5. Kanto boulders can't be pushed with STRENGTH

In the Warden's house (FuchsiaCity_WardensHouse), the boulder at (11,6) guards a RARE CANDY at (11,5). With HM04 in the bag the game offers "Would you like to use STRENGTH?" and a borrowed MACHOP activates it ("Its STRENGTH made it possible to move boulders around!"; talking to the boulder again: "STRENGTH made it possible to move boulders around."), but pushing it from the right ((12,6), LEFT) or the left ((10,6), RIGHT) never moves it, although both destination tiles are free. Cause: `TryPushBoulder` in `src/field_player_avatar.c` (line 850) compares the object's graphics id with `OBJ_EVENT_GFX_PUSHABLE_BOULDER` only, and the Kanto maps' boulders are `OBJ_EVENT_GFX_KANTO_PUSHABLE_BOULDER` (same check in `src/fldeff_strength.c:20`). Likely affects Victory Road, Seafoam Islands and every other Kanto boulder.

**Fixed (2026-11-25):** `TryPushBoulder` (`src/field_player_avatar.c`) and `SetUpFieldMove_Strength` (`src/fldeff_strength.c`) now accept `OBJ_EVENT_GFX_KANTO_PUSHABLE_BOULDER` as well as `OBJ_EVENT_GFX_PUSHABLE_BOULDER` (both functions take a u16 graphics id, so the Kanto value 343 fits). Tested on the rebuilt ROM from `ethan_hm03_hm04.sav`: the Warden's boulder moved left to (10,6) (the RARE CANDY at (11,5) was then taken), and after leaving and re-entering the house (STRENGTH off, boulder back at (11,6)) "Would you like to use STRENGTH?" YES and a push from (10,6) moved it right to (12,6).

### 6. The Town Map Fly commit doesn't compile

Merging `origin/kanto-hoenn` (d26e26329, "Allow using Fly from the Town Map") and running `make modern` failed: `src/region_map.c:2274: error: 'FIELD_MOVE_FLY' undeclared`. The new `TryPrepareFlyFromTownMap` uses the constant, which lives in `include/constants/party_menu.h`; `region_map.c` only includes `party_menu.h` (the functions). **Fixed:** `#include "constants/party_menu.h"` added to `src/region_map.c`; the ROM builds.

**The feature, tested on the rebuilt ROM from `saves/ethan_route21.sav`** (6 badges, nobody knows FLY, HM02 in the bag): START > BAG > KEY ITEMS > TOWN MAP > USE opens the map.
- On Route 21 (outdoors), A on Route 21 itself (a route) and on CINNABAR ISLAND (not visited yet) does nothing.
- A on PALLET TOWN closes the map, the field returns, the player raises an arm, the PIDGEOT flight scene plays and the player lands at Pallet Town (6,8), in front of the house. It works with the borrowed PIDGEOT (no party member knows FLY). At first there was no "A wild PIDGEOT appeared! It looks willing to lend a hand!" message, which FLY from the HM in the bag shows. **Added (your request):** when FLY is chosen on the TOWN MAP and the user is a borrowed POKéMON, `CB_ExitFlyMap` (`src/region_map.c`) calls the new `SetFlyShowBorrowedMessage()`, and `Task_UseFly` (`src/field_effect.c`) shows "A wild PIDGEOT appeared! / It looks willing to lend a hand! / The wild PIDGEOT used FLY!" in a field message box before the flight. Tested: Route 21 → TOWN MAP → PALLET TOWN shows both boxes on the field, then the flight. HM02 from the bag (Pallet → Viridian) still shows the message once, in the bag, and nothing more on the field.
- Indoors (the player's house 1F), A on VIRIDIAN CITY does nothing (FLY isn't allowed indoors), and B closes the map back to the bag as before.

### 7. The Two Island shop sells "????????" for ¥0

Talking to the clerk at the outdoor stall (28,4) (after the free card pack) opens the POKé MART with a list of "????????" entries, all at ¥0; pressing A on one asks "????????? And you wanted 1? That will be ¥0." (answered NO). In `data/maps/TwoIsland/scripts.inc` the four lists the clerk can use (`TwoIsland_Items_ShopInitial`, `TwoIsland_Items_ShopExpanded1`, `2`, `3`: the shop grows as the Sevii story goes on, LOSTELLE's rescue first) contain only `release` and `end`: the FireRed lists (`.2byte ITEM_...` after `.align 2`, ending with `ITEM_NONE`) were lost in the port, so the mart reads the script bytes as item ids. Fix: restore the four lists from pokefirered (`data/maps/TwoIsland/scripts.inc` there).

