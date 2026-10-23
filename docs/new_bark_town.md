# New Bark Town

Johto's starting town, imported from the user's reference map
(`tools/johto_town/reference/new_bark_town.jpg`). For now it stands on its own: no route leads
to it yet, and it's reached from the Test Area.

## Maps

All of them are in `gMapGroup_Custom`, after the Test Area, in `MAPSEC_NEW_BARK_TOWN` (on the
Kanto region map, west of the Test Area).

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_NEW_BARK_TOWN` | `LAYOUT_NEW_BARK_TOWN`, 22x17, `kanto_general` + `kanto_new_bark_town` | The town, as in the reference: woods all round, the lab and the blue house on the upper terrace, two houses below, a pond in the east and the road out west. Music `MUS_RG_PALLET`, weather `WEATHER_SUNNY`. |
| `MAP_NEW_BARK_TOWN_LAB` | `LAYOUT_PALLET_TOWN_PROFESSOR_OAKS_LAB` (shared) | The lab's inside (door at (13,8)). Music `MUS_RG_OAK_LAB`. |
| `MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_1F`, `_2F` | Pallet Town's player's house (shared) | The blue house (door at (16,8)), with its upstairs. |
| `MAP_NEW_BARK_TOWN_HOUSE1` | `LAYOUT_PALLET_TOWN_RIVALS_HOUSE` (shared) | The west house (door at (11,12)). |
| `MAP_NEW_BARK_TOWN_HOUSE2` | `LAYOUT_KANTO_HOUSE1` (shared) | The east house (door at (16,13)), ELM'S HOUSE. |

The interiors are empty so far. The town has three signs (the town's, the lab's and ELM'S HOUSE)
and an old man by the west road who takes the player back to the Test Area
(`NewBarkTown_EventScript_Guide`). In the Test Area, an old man at (16,6) takes the player to the
town (`TestArea_EventScript_JohtoGuide`).

## The import

`python3 tools/johto_town/build_new_bark.py` writes the tileset
(`data/tilesets/secondary/kanto_new_bark_town`) and the layout (`data/layouts/NewBarkTown`)
from the reference; `PREVIEW=out.png` also renders the map.

- The reference is a JPEG of 16 px tile art scaled up about 1.24x. It is scaled back down to
  352x272 and cut on that grid (22x17 cells). The black void above the map is filled with forest.
- Colours are reduced to six 15-colour palettes (slots 7-12); the buildings' pixels count more,
  so the lab roof's stripes survive. The 8x8 tiles are then merged with their closest neighbours
  until they fit the 384 secondary tiles; merging a building's tile costs more.
- Every cell is its own metatile (about 300 of 384), drawn below the player.
- What blocks is set by hand in the script's `MAP`: `#` blocks, `.` is walkable, `D` is a door
  (`MB_NON_ANIMATED_DOOR`), `W` the pond (`MB_POND_WATER`, surfable), `S` a sign. The ledges'
  edges block and their stairs are walkable; everything is at elevation 3. The woods in the
  north-west (behind the ledges) are closed off.
