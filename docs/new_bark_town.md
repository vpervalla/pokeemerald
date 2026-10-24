# New Bark Town

Johto's starting town, imported from the user's reference map
(`tools/johto_town/reference/new_bark_town.png`). For now it stands on its own: no route leads
to it yet, and it's reached from the Test Area.

## Maps

All of them are in `gMapGroup_Custom`, after the Test Area, in `MAPSEC_NEW_BARK_TOWN` (on the
Kanto region map, west of the Test Area).

| Map | Layout | What it is |
|-----|--------|------------|
| `MAP_NEW_BARK_TOWN` | `LAYOUT_NEW_BARK_TOWN`, 25x23, `kanto_general` + `kanto_new_bark_town` | The town, as in the reference: woods all round, the lab in the north-west, three houses around a wide paved square, and a lake in the east (Kanto's animated sea, surfable). The road out is the square's west edge. Music `MUS_RG_PALLET`, weather `WEATHER_SUNNY`. |
| `MAP_NEW_BARK_TOWN_LAB` | `LAYOUT_PALLET_TOWN_PROFESSOR_OAKS_LAB` (shared) | The lab's inside (door at (4,6)). Music `MUS_RG_OAK_LAB`. |
| `MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_1F`, `_2F` | Pallet Town's player's house (shared) | The north house (door at (12,7)), with its upstairs. |
| `MAP_NEW_BARK_TOWN_HOUSE1` | `LAYOUT_PALLET_TOWN_RIVALS_HOUSE` (shared) | The south-west house (door at (2,15)). |
| `MAP_NEW_BARK_TOWN_HOUSE2` | `LAYOUT_KANTO_HOUSE1` (shared) | The south house (door at (10,16)). |

The interiors are empty so far. The town has an old man by the west road who takes the player
back to the Test Area (`NewBarkTown_EventScript_Guide`). In the Test Area, an old man at (16,6) takes the player to the
town (`TestArea_EventScript_JohtoGuide`).

## The import

`python3 tools/johto_town/build_new_bark.py` writes the tileset
(`data/tilesets/secondary/kanto_new_bark_town`) and the layout (`data/layouts/NewBarkTown`)
from the reference; `PREVIEW=out.png` also renders the map.

- The reference is 16 px tile art (slightly smoothed), 400x373. Its tile grid starts 3 px above
  the picture, so it is moved down 3 px and cut into 25x23 cells: each door then fills its own cell
  (15 of 16 px), and the warps are on those cells. The NPC on the jetty ("Porter") is painted over.
- Lake cells that are all water use Kanto's animated sea (kanto_general's metatile 0x12B,
  `MB_OCEAN_WATER`), at elevation 1; only the banks are imported.
- Colours are reduced to six 15-colour palettes (slots 7-12); the buildings' and the banks' pixels
  count more, so their shades survive. The 8x8 tiles are then merged with their closest
  neighbours until they fit the 384 secondary tiles; merging a building's tile costs more.
- Cells drawn alike share a metatile (about 240 of 384), drawn below the player; if there were too
  many, the closest would be merged.
- What blocks is set by hand in the script's `MAP`: `#` blocks, `.` is walkable, `D` is a door
  (`MB_NON_ANIMATED_DOOR`), `W` the lake (surfable). The flowers are walkable.
