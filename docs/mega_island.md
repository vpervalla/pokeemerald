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
| `MAP_MEGA_ISLAND` | `LAYOUT_MEGA_ISLAND`, 30x58, `kanto_general` + `kanto_sevii_islands_45` | The route, after a reference map: from the castle in the north, a grass plateau leads to two plank bridges over a rock band and a river (with wooden posts along its bank). Below, a meadow with tall grass, the memorial pillar, a rock outcrop with the cave's mouth, the rest house, two rows of ledges and woods; at the bottom, the north side of the gate to the port (as on Route 5). Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_REST_HOUSE` | `LAYOUT_KANTO_SAFARI_ZONE_REST_HOUSE` (shared) | The rest house on the route: a woman restores the player's team, and an old man remembers when the castle appeared. |
| `MAP_MEGA_ISLAND_CAVE` | `LAYOUT_MEGA_ISLAND_CAVE` (the Lost Cave's first room without its ladder) | A small cave in the outcrop: a MAX REVIVE (`FLAG_ITEM_MEGA_ISLAND_CAVE_MAX_REVIVE`) and a hiker who has seen the walls glow like the champions' stones. Music `MUS_RG_SEVII_CAVE`. |
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 60x42, `kanto_general` + `kanto_mega_castle` | The castle grounds, in blackstone: near-black stone with a teal tint, indigo-blue spires, pale weathered stone and lawns. The castle is gigantic: its facade spans the whole map and its wings run into the forest on both sides. It is drawn for the game's high camera, so every face is short and the tops show: slate roofs, the crenellated wall-walk, squat round towers under blue cones, bartizans with blue spires, and a taller gatehouse with violet banners either side of a pointed gate (pale stone frame, black iron doors). From the forecourt the camera sees about 4.5 rows above the player, and the whole facade, cones included, fits in that. In front, a raised bastion carries a pale paved forecourt with two lawns, gargoyles by the gate and spired obelisks; its short front wall is a blind gothic arcade, with round corner towers under blue cones. A pale grand staircase goes down the bastion to a bridge over the moat, which runs the whole width and on into the forest. Below: a courtyard with gargoyles, an iron fence, lantern pillars and lamp posts, then a grass path south through the forest. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SUNNY`. Connected below to the route (offset 15). |
| `MAP_MEGA_ISLAND_CASTLE_1F` | `LAYOUT_MEGA_ISLAND_CASTLE_1F`, 26x16, `kanto_building` + `kanto_mega_hall` | The entrance hall, behind the castle gate: dark marble, a violet runner from the entrance to the door of the court, a short stone wall with arched windows and candelabras, and pillars. Straight staircases in both top corners go up to the guest floor. The receptionist stands behind a wooden desk in front of the court door. Music `MUS_RG_POKE_MANSION`. |
| `MAP_MEGA_ISLAND_CASTLE_2F` | `LAYOUT_MEGA_ISLAND_CASTLE_2F`, 26x9, `kanto_building` + `kanto_mega_hall` | The guest corridor: wooden floor and a violet runner, six guest-room doors along the north wall with candelabras between them, and a flight of stairs down at each end. |
| `MAP_MEGA_ISLAND_CASTLE_ROOM1`-`6` | `LAYOUT_MEGA_ISLAND_CASTLE_GUEST_ROOM` (shared), 11x9 | The six guest rooms, one map each (so each can hold its own guest) on the same layout: a bed, a nightstand with a candle, a wardrobe, a rug and a round table, with a doormat back to the corridor. |
| `MAP_MEGA_ISLAND_ARENA` | `LAYOUT_MEGA_ISLAND_ARENA`, 32x30, `kanto_general` + `kanto_mega_arena` | The castle's inner courtyard, used as the tournament stadium. North: the keep's roofs, wall-walk and short inner face (lancet windows, bartizans with blue spires, violet banners) with the host's box in the middle, seen from above: a pale platform with a throne under a violet canopy and a rail along its front. Tiered stands (two along the north side) run down both sides. A paved walkway with a blue-flame brazier at each corner surrounds the battlefield: pale sand with white lines, trainer boxes at both ends and a blue-and-gold Mega Evolution emblem in the centre circle. South: the wall-walk with the entrance passage from the hall. Music `MUS_RG_TRAINER_TOWER`. |

The moat is the sea's animated water, but its metatiles have no water behavior and block
movement, so it can't be surfed. The castle gate's two door tiles (`MB_NON_ANIMATED_DOOR`) lead
into the entrance hall, whose doormat (`MB_SOUTH_ARROW_WARP`) leads back out. The court door
in the hall (`MB_NON_ANIMATED_DOOR`) leads to the stadium's entrance passage, whose last tiles
lead back to the hall. The top step of each hall staircase (`MB_NORTH_ARROW_WARP`) goes up to
the guest corridor, and the outer step of each corridor staircase (`MB_WEST_ARROW_WARP`, `MB_EAST_ARROW_WARP`) comes
back down. The corridor's doors (`MB_NON_ANIMATED_DOOR`) lead to the guest rooms, whose doormats
lead back.

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
   loose and destroyed the lab, the notes burned and the MEGA STONES were lost. He doesn't
   know that MEWTWO itself can Mega Evolve: there is no record of it. He gives the player the
   MEGA RING (`FLAG_RECEIVED_MEGA_RING`) and explains how to use it (hold the stone, press
   START when choosing a move, once per battle). A player who beat him before this change
   gets the ring by talking to him again.
2. **Hall of Fame.** `PokemonLeague_HallOfFame_EventScript_GameClear` sets
   `VAR_MEGA_STORY_STATE` to 1 and calls `GameClear` (the Hall of Fame record, the save and
   the credits). The port of FRLG's script had lost this step, so the game used to stop on a
   black screen. After the credits, Continue puts the player in their room in Pallet Town.
3. **The invitation.** When the player comes downstairs, MOM gives them a letter that a tall,
   quiet gentleman in a long coat brought that morning. It's an invitation to a tournament
   on an island east of Vermilion, signed "MR. M.". MR. M. is MEWTWO, which uses its powers
   to be seen as a man; nothing in the letter says so. The MEGA STONE of the player's starter
   is inside: VENUSAURITE or BLASTOISINITE, or both CHARIZARDITE X and Y for CHARMANDER (from
   `VAR_STARTER_MON`). `VAR_MEGA_STORY_STATE` becomes 2, and the Vermilion sailor, who works
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

`VAR_MEGA_STORY_STATE` goes on counting: 3 registered (quarterfinal next), 4 semifinal next, 5 final
next, 6 tournament won.

1. **Registration.** The receptionist in the hall gives the quarterfinal pairings and registers the
   player. Before each round she heals the player's team.
2. **The matches** (`MegaIsland_Arena`). With a match to play, entering the court starts it: the
   player walks to the south trainer box, the camera rises to MR. M. (a gentleman, for now) in his box
   for the announcement, then shows both trainers, and the opponent battles from the north box.
   MR. M. then gives the other results, and the player goes back to the hall.
3. **Losing** heals the team (`trainerbattle_earlyrival` with `RIVAL_BATTLE_HEAL_AFTER`, no
   whiteout), and the player can try the same round again.

The bracket is fixed:

| Round | The player's match | The other results |
|-------|--------------------|-------------------|
| Quarterfinals | vs Alder (Mega Heracross) | the rival beats Cynthia, Diantha beats Lance, Leon beats Steven |
| Semifinals | vs the rival (Mega Pidgeot; his team follows the player's starter) | Leon beats Diantha |
| Final | vs Leon (Mega Charizard Y) | |

After the final MR. M. tells the player to rest and come to him: the story goes on from there.

## The castle tilesets

`data/tilesets/secondary/kanto_mega_castle` (the castle grounds), `kanto_mega_arena` (the
stadium) and `kanto_mega_hall` (the hall, the guest corridor and the guest rooms) are new art, drawn for this
project. The interiors add a third palette, slot 9, for wood, linen, rugs and candle light.
All three use the same two
palettes: slot 7 for the blackstone, spires, banners, glass and gold, and slot 8 for the pale
stone and moss (with the outline, dark stone, gold and spire blues shared). Each 8x8 tile uses
one of them. The glass colours (13 and 14 of slot 7, also the lanterns' glow) are lit in the
evening and at night (`sLitPalettes` in `src/day_night.c`), in both tilesets.

## Tools

`tools/mega_island/` generates the two castle tilesets and the three layouts:

- `art.py`: the pixel-art primitives (bricks, towers, spires, windows, banners, gates,
  gargoyles, obelisks, paving, arcades, balustrades, fence, lamps) and the palettes. The
  ones for the high camera (`wall_top`, `short_wall`, `roof_top`, `tower3q`, `bartizan`,
  `cone_roof3q`) draw short faces and the tops seen from above: a tower's rim, base and
  cone eave are ellipses.
- `castle_facade.py`: the castle, bastion and moat drawing. `arena.py`: the stadium drawing.
  `hall.py`: the interiors, drawn for the game's high camera: walls are a pale top edge and a
  two-cell face, floors fill the view, stairs are straight flights. Windows, banners and walls repeat
  on an 8-pixel grid so the 60-wide facade fits the tile budget.
- `tilesetgen.py`: cuts the art into 8x8 tiles (merging flipped duplicates, and giving each
  tile the palette that has its colours), builds metatiles, and writes a tileset (`tiles.png`,
  palettes, metatiles, attributes) and a layout.
- `build_castle.py`: composes the castle grounds and writes `kanto_mega_castle` and
  `data/layouts/MegaIsland_Castle`. Where a tree's tip or canopy stands in
  front of the castle, the metatile puts the castle on the bottom layer and the tree's top
  layer above it. `PREVIEW=out.png` renders the whole map instead of writing it.
- The bottom rows of the castle grounds, which the route draws with its own tileset across
  the connection, use only primary metatiles, so they show correctly from the route.
- `build_arena.py`: writes `kanto_mega_arena` and `data/layouts/MegaIsland_Arena`. It also takes
  `PREVIEW=out.png`.
- `build_hall.py`: writes `kanto_mega_hall` and the hall, corridor and guest room layouts
  (`PREVIEW=out.png` too).
- `build_route.py`: writes `data/layouts/MegaIsland` from a character design of the map, and the port
  and cave layouts. Rock and water
  are autotiled by `autotile.py`, which learns from FRLG's own maps (Routes 3, 4, 9, 10, 22-25, Kindle
  Road and others) which metatile each cell gets, given which of its neighbours are the same terrain.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `build_castle.py`, `build_arena.py`, `build_hall.py` and `build_route.py` after changing the art or the
layouts. The output is deterministic. The castle tileset uses 345 of the 384 secondary tiles
and 287 of the 384 metatiles, the stadium tileset 221 and 99, the interior tileset 139 and 91. Rows above
anything the camera can show reuse the first visible row's metatiles (rows 0-5 in the castle grounds, 0-4 in the stadium).
