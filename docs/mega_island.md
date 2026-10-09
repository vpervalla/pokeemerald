# The tournament island

The island where Mewtwo holds its Mega Evolution tournament. "APEX ISLAND" is a working
name: it is only the map section's name in `src/data/region_map/region_map_sections.json`
(`MAPSEC_MEGA_ISLAND`), so it can change in one place. The code calls everything
`MegaIsland`.

## Maps

All three are in `gMapGroup_KantoSpecialArea`, after Birth Island.

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_MEGA_ISLAND_HARBOR` | `LAYOUT_KANTO_ISLAND_HARBOR` (shared) | The Seagallop dock. The sailor sails back to Vermilion. |
| `MAP_MEGA_ISLAND` | `LAYOUT_MEGA_ISLAND`, 30x58, `kanto_general` + `kanto_sevii_islands_45` | The route: harbor building and pier (copied from Five Island), a beach, a meadow with tall grass across three cliff ledges whose stairs alternate sides, and a forest corridor north. Music `MUS_RG_SEVII_ROUTE`. |
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 60x38, `kanto_general` + `kanto_mega_castle` | The castle grounds. The castle is gigantic: its facade spans the whole map and its top is higher than the camera ever reaches, so only the lower walls show (towering stained-glass windows and banners running out of sight, great tower bases, the gate with drapes). The wings run into the forest on both sides. The castle stands on a stone rampart: a flagstone terrace with two gargoyles by the gate, a retaining wall of ashlar blocks with a coping and pilasters, and a grand staircase with balustrades. A moat runs along the foot of the rampart across the whole width and on into the forest on both sides; a stone bridge carries the staircase over it, with two more gargoyles at its far end. Below: a courtyard with an iron fence, lantern pillars and lamp posts, then a grass path south through the forest. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SHADE`. Connected below to the route (offset 15). |

The moat is the sea's animated water, but its metatiles have no water behavior and block
movement, so it can't be surfed. The terrace runs the width of the castle, between the forests; the walkway goes around the
balustrades through the row in front of the gate. The castle gate is a sign for now ("The great doors are shut tight…"); it becomes the
entrance when the interior exists.

The route has tall grass but no wild encounter table yet, so it has no encounters.

## Getting there

- `SEAGALLOP_MEGA_ISLAND` (11) is a Seagallop destination: `src/seagallop.c` has its
  harbor, its direction from Vermilion (east) and its ferry number (11).
- Nothing in the story sends the player there yet. For testing, a sailor in TestArea
  (west of Pallet Town) sails there.

## The castle tileset

`data/tilesets/secondary/kanto_mega_castle` is new art, drawn for this project. It uses
one palette (slot 7): dark stone, purple slate, crimson, gold, stained glass and wood.
The stained glass colours (13 and 14) are lit in the evening and at night
(`sLitPalettes` in `src/day_night.c`).

## Tools

`tools/mega_island/` generates the castle tileset and both layouts:

- `art.py`: the pixel-art primitives (bricks, roofs, towers, windows, banners, gate,
  gargoyles, fence, lamps) and the palette.
- `castle_facade.py`: the castle and rampart drawing. Windows, banners and walls repeat
  on an 8-pixel grid so the 60-wide facade fits the tile budget.
- `build_castle.py`: composes the castle grounds, cuts them into 8x8 tiles (merging
  flipped duplicates), and writes the tileset (`tiles.png`, palettes, metatiles,
  attributes) and `data/layouts/MegaIsland_Castle`. Where a tree's tip or canopy stands in
  front of the castle, the metatile puts the castle on the bottom layer and the tree's top
  layer above it. `PREVIEW=out.png` renders the whole map instead of writing it.
- The bottom rows of the castle grounds, which the route draws with its own tileset across
  the connection, use only primary metatiles, so they show correctly from the route.
- `build_route.py`: writes `data/layouts/MegaIsland` from primary and Sevii metatiles.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `python3 tools/mega_island/build_castle.py` and `python3 tools/mega_island/build_route.py`
after changing the art or the layouts. The output is deterministic. The castle tileset
uses 272 of the 384 secondary tiles and 325 of the 384 metatiles.
