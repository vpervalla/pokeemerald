# Playthrough bugs

Found while playing the build headless (see `playthrough_guide.md`, "Goal"). One row per bug; the details
(steps, evidence, workaround) go under the table. Status: open, workaround, fixed (commit), won't fix.

| # | Where (map, x,y) | Kind | Summary | Status |
| --- | --- | --- | --- | --- |
| 1 | PalletTown_PlayersHouse_1F (5,8) | Missing interaction | The right tile of the exit does not leave the house | open |
| 2 | CeladonCity_GameCorner (10,13), (11,13); RocketHideout_Elevator (1,5) | Dead warp tiles | Two of the Game Corner's three exit tiles and one of the elevator's two leave nothing (same cause as #1) | open |
| 3 | RocketHideout_B2F, B3F (spinner mazes) | Camera/position desync | After a spinner slide the save-block position (camera) stays behind the player; walls and map edges then block moves that the map allows | open |
| 4 | SafariZone_Center (Kanto Safari Zone), START > RETIRE | Wrong destination | Retiring from the Kanto Safari Zone warps to Route 121's Hoenn Safari entrance, with no way back until a later badge (FLY) | workaround: walk out through the south door |

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
- **Seen in:** [Game Corner and Rocket Hideout](walkthrough/26_game_corner_rocket_hideout.md).

### 3. Spinner slides leave the camera position behind (Rocket Hideout B2F, B3F)

- **Where:** the spinner mazes of `RocketHideout_B2F` and `RocketHideout_B3F` (the arrow tiles use `MB_SPIN_*`; `MovementAction_Spin_*` in `src/event_object_movement.c` moves the object with `InitNpcForMovement` at `MOVE_SPEED_FAST_1`).
- **Symptoms:** after a few slides `gSaveBlock1Ptr->pos` (which the field camera keeps) differs from the player's ObjectEvent tile by 1 to 9 tiles. The screen scrolls wrongly (duplicated or black tiles, the sprite outside the map) and `CanCameraMoveInDirection` (called for the camera-tracked player in `ObjectEventCanMoveInDirection`) then refuses moves near the map edge: on B3F, standing at (2,14) with the camera at x=0, LEFT into the open (1,14) did nothing, which closed the only way to the south-west rooms. Items are also not spawned while the camera window is elsewhere.
- **Steps:** B3F from the stairs at (18,2): walk through the maze to (2,14), press left.
- **Evidence:** the ObjectEvent said (2,14) and the save block (0,14); copying the ObjectEvent tile into `pos` made the move work.
- **Workaround:** tools copy the object's tile into `pos` after every step (`tools/playthrough/emu.py: fix_pos`), and `rbfs.py` explores the real game with savestates because the static planner cannot predict slides. Not fixed in the ROM (a rebuild invalidates the savestates): the cause is in the spin movement not feeding `CameraObject`'s per-frame deltas.
- **Seen in:** [Game Corner and Rocket Hideout](walkthrough/26_game_corner_rocket_hideout.md).

### Observations (not bugs)

- **Card duel NPCs.** The rocker in the Route 2 east building starts the ROM's card game (`cardduel CARD_OPPONENT_ROUTE2_ROCKER`) straight after his FLASH hint, with no yes/no question: talking to him always begins a duel. The duel screen draws and takes input; Forfeit (last action) then A ends it with "You lost the duel..." and the overworld returns. A full duel has not been played: still to test (win and lose paths, the reward).
- **Rocket Hideout stair warps** (B1F (12,2), (17,2), B2F (21,2), (28,2), (23,12), B3F (18,2), (15,18), B4F (11,15)) fire when a direction is pressed on the tile (the stair's own direction: the B2F stair at (21,2) needs RIGHT, the B3F one at (18,2) needs RIGHT); walking onto the tile does nothing.
- **Hideout barriers:** B1F's barrier (20..21,19..20) opens when the grunt at (21,27) is beaten, who can only be reached by the elevator (LIFT KEY from the B4F grunt); B4F's barrier (17..18,12..14) opens when both grunts at (16,14) and (19,14) are beaten (one is on each side; the east side is reached by the elevator too). The elevator rooms' panel is at (0,2): face up from (0,3).
- **Cerulean's east road** is only reachable from the west plateau through the robbed house (hedge at x=34 and a one-way ledge row at y=20). That matches the layout and is the intended way, but it makes the walk from the Center to Route 9 long.

### 4. RETIRE in the Kanto Safari Zone warps to Hoenn

START menu > RETIRE > YES in the Fuchsia Safari Zone (maps 34.63 and the other Kanto Safari maps) moves the player to `ROUTE121_SAFARI_ZONE_ENTRANCE` (Hoenn, 23.0), the Hoenn Safari's own exit warp, with the Safari state ended. The Hoenn and Kanto Safari maps are not connected and FLY needs a badge, so the player is stranded until a later badge. Walk out through the Center's south door (25..27,30) instead: the attendant then asks "Are you going to leave the SAFARI ZONE early?" and returns to Fuchsia.
