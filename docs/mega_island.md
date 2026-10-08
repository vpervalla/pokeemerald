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
| `MAP_MEGA_ISLAND_CASTLE` | `LAYOUT_MEGA_ISLAND_CASTLE`, 30x30, `kanto_general` + `kanto_mega_castle` | The castle grounds: the gothic castle (towers, slate spires, buttresses, stained glass, banners, drapes around the gate, two gargoyles), a flagstone plaza, an iron fence with lantern pillars, lamp posts and dead trees. Music `MUS_RG_POKE_MANSION`, weather `WEATHER_SHADE`. Connected below to the route. |

The castle gate is a sign for now ("The great doors are shut tight…"); it becomes the
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
  gargoyles, fence, lamps, dead trees) and the palette.
- `castle_facade.py`: the castle drawing.
- `build_castle.py`: composes the castle grounds, cuts them into 8x8 tiles (merging
  flipped duplicates), and writes the tileset (`tiles.png`, palettes, metatiles,
  attributes) and `data/layouts/MegaIsland_Castle`.
- `build_route.py`: writes `data/layouts/MegaIsland` from primary and Sevii metatiles.
- `render.py`: renders a tileset or a layout to PNG, for previews.

Run `python3 tools/mega_island/build_castle.py` and `python3 tools/mega_island/build_route.py`
after changing the art or the layouts. The output is deterministic. The castle tileset
uses 344 of the 384 secondary tiles and 275 of the 384 metatiles.
