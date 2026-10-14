# The tournament island

The island where Mewtwo holds its Mega Evolution tournament. "APEX ISLAND" is a working
name: it is only the map section's name in `src/data/region_map/region_map_sections.json`
(`MAPSEC_MEGA_ISLAND`), so it can change in one place. The code calls everything
`MegaIsland`.

## Maps

All of them are in `gMapGroup_KantoSpecialArea`, after Birth Island.

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_MEGA_ISLAND_HARBOR` | `LAYOUT_KANTO_ISLAND_HARBOR` (shared) | The Seagallop dock, inside the harbor building. The sailor sails back to Vermilion. Its door leads out to the port. |
| `MAP_MEGA_ISLAND_PORT` | `LAYOUT_MEGA_ISLAND_PORT`, 30x20, `kanto_general` + `kanto_sevii_islands_45` | The port: the harbor building and pier (copied from Five Island) on the coast, and the south side of the gate to the route (as on Route 6), between woods. A sailor. Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_GATE` | `LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE` (shared) | The gate between the port (south door) and the route (north door), with a guard. |
| `MAP_MEGA_ISLAND` | `LAYOUT_MEGA_ISLAND`, 30x50, `kanto_general` + `kanto_sevii_islands_45` | The route, after a reference map: from the castle in the north, a grass plateau leads to two plank bridges over a rock band and a river (with wooden posts along its bank). Below, a meadow with tall grass, the memorial pillar, a rock outcrop with the cave's mouth, the rest house and woods, on terraces that step down from the river to the gate: rock walls one tile thick (as in Five Isle Meadow) with stairs between the levels; at the bottom, the north side of the gate to the port (as on Route 5). Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_REST_HOUSE` | `LAYOUT_KANTO_SAFARI_ZONE_REST_HOUSE` (shared) | The rest house on the route: a woman restores the player's team, and an old man remembers when the castle appeared. |
| `MAP_MEGA_ISLAND_CAVE` | `LAYOUT_MEGA_ISLAND_CAVE` (the Lost Cave's first room without its ladder) | A small cave in the outcrop: a MAX REVIVE (`FLAG_ITEM_MEGA_ISLAND_CAVE_MAX_REVIVE`) and a hiker who has seen the walls glow like the champions' stones. Music `MUS_RG_SEVII_CAVE`. |
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 38x37, `kanto_general` + `kanto_mega_castle` | The castle grounds, built by hand from the pieces imported from the reference map: the castle at the top (its two-tile door at (19,15), with the warps) between rock walls, lawns with a pond, trees, flowers and two smaller buildings, a cobbled approach with Mewtwo statues by the door and banners down to the torii in the low wall (with a statue at the foot of each pillar), and below it a road through the forest to the route. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SUNNY`. Connected below to the route (offset 4). |
| `MAP_MEGA_ISLAND_CASTLE_1F` | `LAYOUT_MEGA_ISLAND_CASTLE_1F`, 23x16, `kanto_building` + `kanto_mega_hall` | The entrance hall, on the ground floor, in the style of the Pokémon Mansion: its walls, windows and checked floor, and its pink carpet from the entrance (a red mat and a three-wide door mat) up to the registration desk, between two pillars. The desk is the Celadon Hotel's counter (`MB_COUNTER`), so the receptionist is spoken to across it; two Mewtwo statues stand behind it. Two waiting areas with a table and chairs (one with an open book), plants, and the mansion's stairs in the top left corner, up to the guest floor. Music `MUS_RG_POKE_MANSION`. |
| `MAP_MEGA_ISLAND_CASTLE_2F` | `LAYOUT_MEGA_ISLAND_CASTLE_2F`, 26x9, `kanto_building` + `kanto_mega_hall` | The guest floor: a corridor with windows, plants and two Mewtwo statues, the mansion's stairs down to the hall at the west end and up to the arena at the east end, and the six guest rooms' doors (door mats) in the south wall. |
| `MAP_MEGA_ISLAND_CASTLE_ROOM1`-`6` | `LAYOUT_MEGA_ISLAND_CASTLE_GUEST_ROOM` (shared), 11x9 | The six guest rooms, one map each (so each can hold its own guest) on the same layout: the mansion's walls and floor, a bed (from the S.S. Anne's cabins), a table with a chair, plants, and the way out (a red mat and a door mat) in the south wall. |
| `MAP_MEGA_ISLAND_ARENA` | `LAYOUT_MEGA_ISLAND_ARENA`, 23x22, `kanto_building` + `kanto_mega_arena` | The arena, on the castle's top floor: a hall in the mansion's style with four windows. MR. M. stands on a red mat between two Mewtwo statues with their eyes lit, at the north end. The battlefield is Agatha's (the Elite Four's lilac floor with white lines and a Poké Ball in the middle), lengthened. Rows of chairs for the spectators down both sides, pillars in the corners, plants, and the way down to the guest floor (a red mat and a door mat) in the middle of the south wall. Music `MUS_RG_TRAINER_TOWER`. |

The castle door's two tiles (`MB_NON_ANIMATED_DOOR`) lead into the entrance hall, whose
entrance (`MB_SOUTH_ARROW_WARP`, three cells on the carpet's red mat) leads back out. The hall's
stairs go up to the guest floor: as in the mansion, the player steps onto the lowest step and walks
east (`MB_EAST_ARROW_WARP`). The guest floor's stairs down are entered from the east, on the top step
(`MB_WEST_ARROW_WARP`), and its stairs up to the arena like the hall's. The arena's way out
(`MB_SOUTH_ARROW_WARP`) goes back down to the guest floor. The guest floor's doors
(`MB_SOUTH_ARROW_WARP` above each door mat) lead to the guest rooms, whose ways out lead back.

The route has tall grass but no wild encounter table yet, so it has no encounters.

## Getting there

- `SEAGALLOP_MEGA_ISLAND` (11) is a Seagallop destination: `src/seagallop.c` has its
  harbor, its direction from Vermilion (east) and its ferry number (11).
- After the invitation (below), a sailor on the east pier of Vermilion Harbor sails there
  (`VermilionCity_EventScript_ApexSailor`). He is hidden by `FLAG_HIDE_VERMILION_APEX_SAILOR`,
  which a new game sets. For testing, a sailor in TestArea (west of Pallet Town) also sails there.

## The story so far

Nobody can Mega Evolve during the first run through the League: no trainer holds a MEGA
STONE, and the player has none.

1. **Cinnabar Gym.** After the battle (and TM38), BLAINE tells the player that Mega Evolution
   was discovered at the POKéMON MANSION while studying MEWTWO, and that when MEWTWO broke
   loose and destroyed the lab, the notes burned and every stone, KEY STONE included, was lost.
   He doesn't know that MEWTWO itself can Mega Evolve: there is no record of it. He gives
   nothing Mega-related.
2. **Hall of Fame.** `PokemonLeague_HallOfFame_EventScript_GameClear` sets
   `VAR_MEGA_STORY_STATE` to 1 and calls `GameClear` (the Hall of Fame record, the save and
   the credits). The port of FRLG's script had lost this step, so the game used to stop on a
   black screen. After the credits, Continue puts the player in their room in Pallet Town.
3. **The invitation.** When the player comes downstairs, MOM gives them a letter that a tall,
   quiet gentleman in a long coat brought that morning. It's an invitation to a tournament
   on an island east of Vermilion, signed "MR. M.". MR. M. is MEWTWO, which uses its powers
   to be seen as a man; nothing in the letter says so. The MEGA RING
   (`FLAG_RECEIVED_MEGA_RING`) is inside, and the letter explains how to use it (the POKéMON holds
   its stone, press START when choosing a move, once per battle). No MEGA STONE comes with it:
   the letter says they are hidden, waiting for worthy trainers. A save that already got the
   ring from BLAINE (before this change) doesn't get a second one. `VAR_MEGA_STORY_STATE` becomes 2, and the Vermilion sailor, who works
   for MR. M., appears.

## The tournament entrants

Seven champions answered MR. M.'s invitation. With the player, that makes an 8-trainer bracket.
Their trainers are `TRAINER_TOURNAMENT_*` in `include/constants/opponents_kanto.h` (after the
generated Kanto trainers); the teams are in `src/data/tournament_trainer_parties.h`. They are lv
70-75, and each one's ace, sent out last, holds its MEGA STONE.

| Champion | Region | Mega | Where they wait |
|----------|--------|------|-----------------|
| The rival ({RIVAL}) | Kanto | Pidgeot (the rest of his team follows the player's starter, as in FRLG) | The entrance hall |
| Lance | Johto | Gyarados | Guest room 2 |
| Steven | Hoenn | Metagross | Guest room 3 |
| Cynthia | Sinnoh | Salamence | Guest room 4 |
| Alder | Unova | Heracross | Guest room 5 |
| Diantha | Kalos | Gardevoir | Guest room 6 |
| Leon | Galar | Charizard (Y) | Lost in the guest corridor |

Guest room 1 is the player's.

Alain (`TRAINER_ALAIN`, class "PKMN TRAINER", trainer pic `TRAINER_PIC_ALAIN`, overworld
`OBJ_EVENT_GFX_ALAIN`) is ready but not placed yet: his part in the story is still to be decided.
His team: Swellow, Absol, Scizor, Tyranitar, Metagross and his ace, Charizard with CHARIZARDITE X
(Swellow, Absol and Scizor stand in for his Unfezant, Weavile and Bisharp). The teams use only Gen 1-3 POKéMON, standing in for the ones
those champions use in their own games (Salamence for Garchomp, for example).

Cynthia, Alder, Diantha and Leon are new characters. Their sprites come from the images in
`tools/mega_island/champions/source/` (at their native pixel size, backgrounds removed), and
`sprites.py` turns them into GBA sprites: the trainer pics are scaled to fit 64x64
(`graphics/trainers/front_pics/champion_*.png`, `TRAINER_PIC_CHAMPION_*`), the overworld frames
are cut from the walking sheets (32x32, Alder's 16x32; `OBJ_EVENT_GFX_CHAMPION_*`), and each gets
its own 15-colour palette. On the maps they use the special NPC palette slot, so there can be only
one of them on a map at a time. The rival, Lance and Steven use their existing sprites.

In battle, the Kanto rival (TERRY in the trainer data) now shows the name the player gave him.

## The tournament

`VAR_MEGA_STORY_STATE` goes on counting: 3 registered (asleep tonight), 4 quarterfinal next,
5 semifinal next, 6 final next, 7 tournament won, 8 waking up from a lost match, 9 MEWTWO
waiting in the island's cave, 10 MEWTWO caught.

1. **Registration.** The receptionist in the hall gives the quarterfinal pairings and registers the
   player: the tournament begins tomorrow morning, and their room is upstairs (guest room 1, the
   first door on the left). Until they sleep she only repeats that.
2. **The night.** The bed in guest room 1 (`MegaIsland_Castle_Room1_EventScript_Bed`) asks
   whether to sleep: the screen fades, the healing jingle plays, the team is healed, and it's
   morning (state 4). Before each later round the receptionist heals the team.
3. **The matches** (`MegaIsland_Arena`). With a match to play, coming up to the arena starts it:
   the player walks from the stairs to the south trainer box, the camera rises to MR. M. (a
   gentleman, for now) on his dais for the announcement, then shows both trainers, and the opponent
   battles from the north box. MR. M. then gives the other results, and the player goes back down
   to the desk in the hall.
4. **Losing** was a dream. The battle ends without a whiteout (`trainerbattle_earlyrival` with
   `RIVAL_BATTLE_HEAL_AFTER`), the screen fades to white and the player wakes up in their bed
   (state 8). On the room's first frame they wake with a start: it was all a dream, it's morning,
   and the tournament begins again from the quarterfinal (state 4), with the team healed.

The bracket is fixed:

| Round | The player's match | The other results |
|-------|--------------------|-------------------|
| Quarterfinals | vs Alder (Mega Heracross) | the rival beats Cynthia, Diantha beats Lance, Leon beats Steven |
| Semifinals | vs the rival (Mega Pidgeot; his team follows the player's starter) | Leon beats Diantha |
| Final | vs Leon (Mega Charizard Y) | |

After the final MR. M. tells the player to rest and come to him, and the receptionist reminds
them that he waits in the arena.

## MEWTWO

MEWTWO is no longer in CERULEAN CAVE: RED (or ETHAN) waits there instead (see
`docs/catch_them_all.md`). After the tournament (state 7), MR. M. stands on his dais in the
arena. Talking to him, he reveals himself: in a flash he becomes MEWTWO, tells the player how it
broke free and took the stones, and why it gathered the champions. It won't battle before an
audience: it invites the player to the cave on the island's route, and vanishes (state 9).

In the cave (`MegaIsland_Cave`) MEWTWO waits at the back, and the hiker has left. Talking to it
starts a legendary battle at lv 80. MEWTWO holds MEWTWONITE X and Mega Evolves on its first
turn: wild POKéMON don't Mega Evolve, except a legendary holding its stone
(`OpponentHandleChooseMove`). Catching it (it keeps the stone) ends the story (state 10). If it's
beaten it says strength alone won't bind it and vanishes, and if the player runs it stays: either
way it waits in the cave again, so it can always be caught.

## The castle tilesets

`data/tilesets/secondary/kanto_mega_castle` (the castle grounds), `kanto_mega_hall` (the hall, the
guest floor and the guest rooms) and `kanto_mega_arena` (the arena) are the castle's tilesets.

The castle grounds' tiles are imported from the reference map (`build_castle.py`), with six
palettes of their own (slots 7-12) fitted to the art.

The interiors have no new art: they are built after the Pokémon Mansion from Kanto's own tilesets
(`build_interiors.py`). Every cell is an existing metatile, or a piece of furniture from another
tileset standing on the mansion's floor: its top layer, and its bottom layer where that isn't its
own floor. The pieces come from `kanto_pokemon_mansion` (walls, windows, the floor and its shadows,
the pink carpet and red mats, stairs, pillars, the Mewtwo statues, plants, tables and chairs),
`kanto_restaurant_hotel` (the desk), `kanto_ss_anne` (the bed), `kanto_pokemon_league` (Agatha's
battlefield) and `kanto_building` (the black edge and the door mats). Their tiles are copied (merging
identical and flipped ones), and so are their palettes, each source palette into the secondary slot
that already holds the same colours, its own slot if it's free, or the first free one.

## Tools

`tools/mega_island/` generates the castle tilesets and their layouts:

- `tilesetgen.py`: cuts the art into 8x8 tiles (merging flipped duplicates, and giving each
  tile the palette that has its colours; the palettes are a parameter), builds metatiles, and writes a tileset (`tiles.png`,
  palettes, metatiles, attributes) and a layout.
- `build_castle.py`: imports the castle grounds' tiles from `reference/castle_grounds.png`, the
  reference map. It is a smoothed upscale (about 1.4x) of 16 px tile art, so it is scaled back to
  its native 640x420 and the pieces are cut out on its own grid: the castle (made symmetric from
  its right half, without the banners and komainu in front of its ground floor, and its door
  widened to fill its two cells, which it straddles in the reference), the gate (the torii as it
  is, a statue at the foot of each pillar; the wall rebuilt on a 16 px period from its clean
  panels, without the plants in front), the banners and cobbles, and the Mewtwo statues. Grass
  around them is transparent, over Kanto's grass. The colours are fitted to six 15-colour palettes
  (each 8x8 tile picks the one that draws it best) and the closest tiles are merged if they don't
  fit the 384 secondary tiles. It writes `kanto_mega_castle`.
  The castle grounds are built by hand from the pieces in Porymap, and re-running it keeps that
  layout: `reference/castle_pieces.bin` records which metatile each piece cell got, and the
  hand-built layout's metatiles are translated cell by cell to the new ones. `--pieces` writes the
  pieces laid out on grass as the layout instead. `PREVIEW=out.png` renders the map.
- `statues.py`: the Mewtwo statue (16x32, a stone figure on a pedestal), drawn by hand after the
  user's reference sprite, and its red-eyed variant (a piece of its own in the tileset, not placed).
- Across a connection the engine keeps the rows it has already drawn when it swaps in the other
  map's secondary tileset: 7 rows above the player and 8 below. So the castle grounds' last 7 rows
  and the route's first 8 must use only primary (`kanto_general`) metatiles, or the cells drawn with
  one map's tileset show as garbage in the other. Edit the maps with that in mind.
- `build_interiors.py`: builds the hall, the guest floor, the guest room and the arena from existing
  metatiles (see above): a small vocabulary of the mansion's pieces (its room shell, stairs, statues,
  pillars, tables, plants, carpet and exits) placed on each map, with their collision and behaviors.
  It writes `kanto_mega_hall`, `kanto_mega_arena` and the four layouts. `PREVIEW=out.png` also
  renders the four maps.
- `build_route.py`: writes `data/layouts/MegaIsland` from a character design of the map, and the port
  and cave layouts. Rock and water
  are autotiled by `autotile.py`, which learns from FRLG's own maps (Routes 3, 4, 9, 10, 22-25, Kindle
  Road and others) which metatile each cell gets, given which of its neighbours are the same terrain.
  The terraces come from a height map: a cell on the high side of a drop becomes a wall, picked
  by where the lower ground is.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `build_castle.py`, `build_interiors.py` and `build_route.py` after changing the art or the
layouts. The output is deterministic. The castle tileset uses all 384 secondary tiles (none merged)
and 207 of the 384 metatiles, the hall tileset 90 and 91, the arena tileset 64
and 69.
