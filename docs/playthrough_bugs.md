# Playthrough bugs

Found while playing the build headless (see `playthrough_guide.md`, "Goal"). One row per bug; the details
(steps, evidence, workaround) go under the table. Status: open, workaround, fixed (commit), won't fix.

| # | Where (map, x,y) | Kind | Summary | Status |
| --- | --- | --- | --- | --- |
| 1 | PalletTown_PlayersHouse_1F (5,8) | Missing interaction | The right tile of the exit does not leave the house | open |

## Details

### 1. The house's exit at (5,8) does nothing

- **Where:** PalletTown_PlayersHouse_1F, tile (5,8). `map.json` lists warps at (5,8), (4,8) and (3,9), all to PALLET_TOWN.
- **Steps:** from the start, go downstairs, walk to (5,8) and press down. Nothing happens, twice. From (4,8) pressing down leaves.
- **Expected:** both tiles of the doorway leave the house.
- **Workaround:** use (4,8).
- **Seen in:** [Pallet Town](walkthrough/01_pallet_town.md).
