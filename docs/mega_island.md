# The tournament island

The island where Mewtwo holds its Mega Evolution tournament. "APEX ISLAND" is a working
name: it is only the map section's name in `src/data/region_map/region_map_sections.json`
(`MAPSEC_MEGA_ISLAND`), so it can change in one place. The code calls everything
`MegaIsland`.

## Maps

All of them are in `gMapGroup_KantoSpecialArea`, after Birth Island.

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_MEGA_ISLAND_HARBOR` | `LAYOUT_KANTO_ISLAND_HARBOR` (shared) | The Seagallop dock. The sailor sails back to Vermilion. |
| `MAP_MEGA_ISLAND` | `LAYOUT_MEGA_ISLAND`, 30x58, `kanto_general` + `kanto_sevii_islands_45` | The route: harbor building and pier (copied from Five Island), a beach, a meadow with tall grass across three cliff ledges whose stairs alternate sides, and a forest corridor north. Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 60x42, `kanto_general` + `kanto_mega_castle` | The castle grounds, in blackstone: near-black stone with a teal tint, indigo-blue spires, pale weathered stone and lawns. The castle is gigantic: its facade spans the whole map and its top is higher than the camera ever reaches, so only the lower walls show (towering stained-glass windows, turrets with blue spires running out of sight, violet banners down the great towers, a pointed gate with a pale stone frame and black iron doors). The wings run into the forest on both sides. In front, a raised bastion carries a pale paved forecourt with two lawns, gargoyles by the gate and spired obelisks; its front wall is a blind gothic arcade, with round corner towers under blue spires. A pale grand staircase goes down the bastion to a bridge over the moat, which runs the whole width and on into the forest. Below: a courtyard with gargoyles, an iron fence, lantern pillars and lamp posts, then a grass path south through the forest. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SUNNY`. Connected below to the route (offset 15). |
| `MAP_MEGA_ISLAND_CASTLE_1F` | `LAYOUT_MEGA_ISLAND_CASTLE_1F`, 26x16, `kanto_building` + `kanto_mega_hall` | The entrance hall, behind the castle gate: dark marble, a violet runner from the entrance to the door of the court, a short stone wall with arched windows and candelabras, and pillars. Diagonal staircases in both top corners go up to the guest floor. The receptionist stands behind a wooden desk in front of the court door. Music `MUS_RG_POKE_MANSION`. |
| `MAP_MEGA_ISLAND_CASTLE_2F` | `LAYOUT_MEGA_ISLAND_CASTLE_2F`, 26x9, `kanto_building` + `kanto_mega_hall` | The guest corridor: wooden floor and a violet runner, six guest-room doors along the north wall with candelabras between them, and diagonal stairs down in both bottom corners. |
| `MAP_MEGA_ISLAND_CASTLE_ROOM1`-`6` | `LAYOUT_MEGA_ISLAND_CASTLE_GUEST_ROOM` (shared), 11x9 | The six guest rooms, one map each (so each can hold its own guest) on the same layout: a bed, a nightstand with a candle, a wardrobe, a rug and a round table, with a doormat back to the corridor. |
| `MAP_MEGA_ISLAND_ARENA` | `LAYOUT_MEGA_ISLAND_ARENA`, 32x30, `kanto_general` + `kanto_mega_arena` | The castle's inner courtyard, used as the tournament stadium. North: the inner face of the keep (stained glass, turrets with blue spires, violet banners) with the host's balcony in the middle: a throne under violet drapes behind a pale balustrade. Tiered stands run along the north side and down both sides. A paved walkway with a blue-flame brazier at each corner surrounds the battlefield: pale sand with white lines, trainer boxes at both ends and a blue-and-gold Mega Evolution emblem in the centre circle. South: the wall-walk with the entrance passage from the hall. Music `MUS_RG_TRAINER_TOWER`. |

The moat is the sea's animated water, but its metatiles have no water behavior and block
movement, so it can't be surfed. The castle gate's two door tiles (`MB_NON_ANIMATED_DOOR`) lead
into the entrance hall, whose doormat (`MB_SOUTH_ARROW_WARP`) leads back out. The court door
in the hall (`MB_NON_ANIMATED_DOOR`) leads to the stadium's entrance passage, whose last tiles
lead back to the hall. The bottom step of each hall staircase (`MB_NORTH_ARROW_WARP`) goes up to
the guest corridor, and the top step of each corridor staircase (`MB_SOUTH_ARROW_WARP`) comes
back down. The corridor's doors (`MB_NON_ANIMATED_DOOR`) lead to the guest rooms, whose doormats
lead back.

The route has tall grass but no wild encounter table yet, so it has no encounters.

## Getting there

- `SEAGALLOP_MEGA_ISLAND` (11) is a Seagallop destination: `src/seagallop.c` has its
  harbor, its direction from Vermilion (east) and its ferry number (11).
- Nothing in the story sends the player there yet. For testing, a sailor in TestArea
  (west of Pallet Town) sails there.

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
  gargoyles, obelisks, paving, arcades, balustrades, fence, lamps) and the two palettes.
- `castle_facade.py`: the castle, bastion and moat drawing. `arena.py`: the stadium drawing.
  `hall.py`: the interiors, drawn for the game's high camera: walls are a pale top edge and a
  two-cell face, floors fill the view, stairs are diagonal flights in the corners. Windows, banners and walls repeat
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
- `build_route.py`: writes `data/layouts/MegaIsland` from primary and Sevii metatiles.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `build_castle.py`, `build_arena.py`, `build_hall.py` and `build_route.py` after changing the art or the
layouts. The output is deterministic. The castle tileset uses 298 of the 384 secondary tiles
and 340 of the 384 metatiles, the stadium tileset 234 and 118, the interior tileset 149 and 94. In both maps, rows 0-4 are above
anything the camera can show, so they reuse row 5's metatiles.
