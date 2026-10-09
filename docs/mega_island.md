# The tournament island

The island where Mewtwo holds its Mega Evolution tournament. "APEX ISLAND" is a working
name: it is only the map section's name in `src/data/region_map/region_map_sections.json`
(`MAPSEC_MEGA_ISLAND`), so it can change in one place. The code calls everything
`MegaIsland`.

## Maps

All four are in `gMapGroup_KantoSpecialArea`, after Birth Island.

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_MEGA_ISLAND_HARBOR` | `LAYOUT_KANTO_ISLAND_HARBOR` (shared) | The Seagallop dock. The sailor sails back to Vermilion. |
| `MAP_MEGA_ISLAND` | `LAYOUT_MEGA_ISLAND`, 30x58, `kanto_general` + `kanto_sevii_islands_45` | The route: harbor building and pier (copied from Five Island), a beach, a meadow with tall grass across three cliff ledges whose stairs alternate sides, and a forest corridor north. Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 60x42, `kanto_general` + `kanto_mega_castle` | The castle grounds, in blackstone: near-black stone with a teal tint, indigo-blue spires, pale weathered stone and lawns. The castle is gigantic: its facade spans the whole map and its top is higher than the camera ever reaches, so only the lower walls show (towering stained-glass windows, turrets with blue spires running out of sight, violet banners down the great towers, a pointed gate with a pale stone frame and black iron doors). The wings run into the forest on both sides. In front, a raised bastion carries a pale paved forecourt with two lawns, gargoyles by the gate and spired obelisks; its front wall is a blind gothic arcade, with round corner towers under blue spires. A pale grand staircase goes down the bastion to a bridge over the moat, which runs the whole width and on into the forest. Below: a courtyard with gargoyles, an iron fence, lantern pillars and lamp posts, then a grass path south through the forest. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SUNNY`. Connected below to the route (offset 15). |
| `MAP_MEGA_ISLAND_ARENA` | `LAYOUT_MEGA_ISLAND_ARENA`, 32x30, `kanto_general` + `kanto_mega_arena` | The castle's inner courtyard, used as the tournament stadium. North: the inner face of the keep (stained glass, turrets with blue spires, violet banners) with the host's balcony in the middle: a throne under violet drapes behind a pale balustrade. Tiered stands run along the north side and down both sides. A paved walkway with a blue-flame brazier at each corner surrounds the battlefield: pale sand with white lines, trainer boxes at both ends and a blue-and-gold Mega Evolution emblem in the centre circle. South: the wall-walk with the entrance passage. Music `MUS_RG_TRAINER_TOWER`. |

The moat is the sea's animated water, but its metatiles have no water behavior and block
movement, so it can't be surfed. The castle gate's two door tiles (`MB_NON_ANIMATED_DOOR`) lead
into the stadium's entrance passage; its last two tiles (`MB_SOUTH_ARROW_WARP`) lead back out.

The route has tall grass but no wild encounter table yet, so it has no encounters.

## Getting there

- `SEAGALLOP_MEGA_ISLAND` (11) is a Seagallop destination: `src/seagallop.c` has its
  harbor, its direction from Vermilion (east) and its ferry number (11).
- Nothing in the story sends the player there yet. For testing, a sailor in TestArea
  (west of Pallet Town) sails there.

## The castle tilesets

`data/tilesets/secondary/kanto_mega_castle` (the castle grounds) and `kanto_mega_arena` (the
stadium) are new art, drawn for this project. Both use the same two
palettes: slot 7 for the blackstone, spires, banners, glass and gold, and slot 8 for the pale
stone and moss (with the outline, dark stone, gold and spire blues shared). Each 8x8 tile uses
one of them. The glass colours (13 and 14 of slot 7, also the lanterns' glow) are lit in the
evening and at night (`sLitPalettes` in `src/day_night.c`), in both tilesets.

## Tools

`tools/mega_island/` generates the two castle tilesets and the three layouts:

- `art.py`: the pixel-art primitives (bricks, towers, spires, windows, banners, gates,
  gargoyles, obelisks, paving, arcades, balustrades, fence, lamps) and the two palettes.
- `castle_facade.py`: the castle, bastion and moat drawing. `arena.py`: the stadium drawing. Windows, banners and walls repeat
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
- `build_route.py`: writes `data/layouts/MegaIsland` from primary and Sevii metatiles.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `build_castle.py`, `build_arena.py` and `build_route.py` after changing the art or the
layouts. The output is deterministic. The castle tileset uses 298 of the 384 secondary tiles
and 340 of the 384 metatiles, the stadium tileset 234 and 118. In both maps, rows 0-4 are above
anything the camera can show, so they reuse row 5's metatiles.
