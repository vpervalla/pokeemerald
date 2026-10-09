# Catching every Pokémon without trading

The goal is for every species to be obtainable in a single game, with no trades.

## Kanto starters

The player picks one of BULBASAUR, CHARMANDER and SQUIRTLE in Oak's lab. The other two come from
two side quests early in the game, a little like in Pokémon Yellow, but they take more effort.
The species of each quest depends on the player's starter (`VAR_STARTER_MON`):

| Starter    | MT. MOON   | CERULEAN CITY |
|------------|------------|---------------|
| BULBASAUR  | CHARMANDER | SQUIRTLE      |
| SQUIRTLE   | CHARMANDER | BULBASAUR     |
| CHARMANDER | SQUIRTLE   | BULBASAUR     |

The POKéMON are shown on the map with their PC box icon as an overworld sprite
(`OBJ_EVENT_GFX_STARTER_*`, in `src/data/object_events/starter_object_events.h`), through
`VAR_OBJ_GFX_ID_0`, which each map sets on transition.

### MT. MOON: LUNA, lost in the cave

- In the ROUTE 4 POKéMON CENTER, at the foot of MT. MOON, LUNA's mom asks the player to find her
  daughter, who went to see the CLEFAIRY and never came back. Her hint: LUNA loves quiet places
  deep underground, where she once saw a STAR shining.
- LUNA is on MT. MOON B2F, at the end of the passage below the STAR PIECE (reached from 1F through
  the B1F corridor), cornered by a wild CHARMANDER. Walking up to them starts the scene, and the
  POKéMON attacks: a wild battle at level 10.
- Whatever happens (caught, defeated or run from), LUNA gets away and goes home. If the POKéMON
  wasn't caught, it stays there, and talking to it starts the battle again. If the player blacks
  out, the scene starts again next time.
- Back at the POKéMON CENTER, LUNA waits with her mom, who gives the player a MOON STONE.

### CERULEAN CITY: the abandoned POKéMON

- In the house full of plants (`CeruleanCity_House5`, with the BERRY POWDER man), a woman looks
  after POKéMON that were hurt or abandoned. Her BULBASAUR was left on NUGGET BRIDGE by its
  TRAINER after it lost a battle, and it doesn't trust TRAINERS anymore.
- Each time the player talks to her, they show their lead POKéMON to it. It comes out only if
  that POKéMON's friendship is 150 or more (`GetLeadMonFriendshipScore`). At 100 to 149, she says
  they're almost there. Then the POKéMON joins the player at level 12, after a YES/NO question.

The scripts are in `data/scripts/starter_quests.inc` (which species, which sprite) and after the
generated Kanto NPCs in the scripts of `MtMoon_B2F`, `Route4_PokemonCenter_1F` and
`CeruleanCity_House5`. The flags are at the end of the Kanto extra flags in
`include/constants/flags.h`.

## Version exclusives

The Kanto wild encounters were ported from FireRed (`port_kanto.py encounters` skips LeafGreen's
tables), so LeafGreen's exclusives were missing. `tools/kanto_port/add_leafgreen_exclusives.py`
puts each of them in the areas where FireRed has its counterpart, comparing the two versions'
tables slot by slot (pokefirered has both):

- If the counterpart has other slots in that table, the LeafGreen species takes all of its
  LeafGreen slots (MAGMAR keeps its 5% on MT. EMBER, since SPEAROW has other slots there).
- If they share several slots, they split them and their rates as evenly as possible. Which of
  the two gets the most common slot alternates from one area to the next.
- If they share a single slot, the LeafGreen species takes it in every other area, unless it's
  already in that area through another method (surfing, fishing…).

Fishing slots are only exchanged within the same rod, and no other species loses a slot. Every
FireRed species is still in at least one area.

| LeafGreen species | Where (FireRed counterpart) |
|---|---|
| SANDSHREW | Routes 4, 8, 9, 10, 11, 23 (EKANS) |
| SANDSLASH | Victory Road 1F, 3F (ARBOK) |
| VULPIX | Routes 7, 8, Pokémon Mansion (GROWLITHE) |
| BELLSPROUT | Routes 5, 6, 7, 12–15, 24, 25, Berry Forest, Cape Brink, Water Path (ODDISH) |
| WEEPINBELL | Routes 12, 13, 15, Berry Forest, Cape Brink, Water Path (GLOOM) |
| SLOWPOKE | Most water: surfing and fishing in 41 areas, Seafoam Islands (PSYDUCK) |
| SLOWBRO | Seafoam Islands, Cerulean Cave, Berry Forest, Cape Brink, Cinnabar (GOLDUCK, SEADRA) |
| STARYU | Super Rod at Pallet Town, Cinnabar Island, S.S. Anne, Five Island (SHELLDER) |
| KINGLER | Super Rod at Routes 20, 21 and eight Sevii areas (SEADRA) |
| MUK | Pokémon Mansion 1F, 3F (WEEZING) |
| MAGMAR | Mt. Ember (SPEAROW) |
| PINSIR | Safari Zone Center (SCYTHER, still in Safari Zone East and the Game Corner) |
| MARILL | Surfing at Four Island, Icefall Cave, Ruin Valley (WOOPER) |
| MISDREAVUS | Lost Cave, all rooms (MURKROW) |
| SNEASEL | Icefall Cave 1F (DELIBIRD, still on B1F) |
| REMORAID | Super Rod at Resort Gorgeous, Five Isle Meadow, Outcast Island, Water Path, Tanoby Ruins (QWILFISH) |
| MANTINE | Surfing at Trainer Tower, Tanoby Ruins (TENTACOOL) |

After running `port_kanto.py encounters` again, run the script again:

    python3 tools/kanto_port/add_leafgreen_exclusives.py --frlg ../pokefirered --write

## Trade evolutions

Twelve double-exchange traders wait in Pokémon Centers along the way, using the mechanism tested
in `TestArea_House`. The player gives a POKéMON, receives the trader's, and the two are traded
straight back, so the player's POKéMON evolves on its way back, with its held item if its
evolution needs one (METAL COAT, KING'S ROCK, DRAGON SCALE, UP-GRADE…).

Each trader exchanges only once (`FLAG_DID_DOUBLE_EXCHANGE_*`). So that no exchange is wasted,
they first check the chosen POKéMON with the `WouldMonEvolveThroughTrade` special: if it wouldn't
evolve they refuse, and if it needs an item it isn't holding they name the item. Twelve traders
cover the ten Kanto trade evolutions (ALAKAZAM, MACHAMP, GOLEM, GENGAR, STEELIX, SCIZOR, POLITOED,
SLOWKING, KINGDRA, PORYGON2) with two to spare.

| Pokémon Center | Trader | Their POKéMON |
|---|---|---|
| Cerulean City | DANTE (Cooltrainer) | PSYDUCK |
| Vermilion City | MORGAN (Sailor) | SHELLDER |
| Route 10 | BRUNO (Hiker) | GEODUDE |
| Lavender Town | AGNES (Channeler) | GASTLY |
| Celadon City | LILY (Beauty) | ODDISH |
| Saffron City | OTTO (Scientist) | DROWZEE |
| Fuchsia City | JOEY (Camper) | VENONAT |
| Cinnabar Island | IGOR (Poké Maniac) | GROWLITHE |
| One Island | WADE (Fisher) | TENTACOOL |
| Three Island | RICK (Biker) | GRIMER |
| Four Island | SUZY (Little girl) | SWINUB |
| Seven Island | EDNA (Old woman) | PONYTA |

They all stand at (10, 7), next to the table. The shared script is
`data/scripts/double_exchange.inc`; each map has a short script that sets its
`INGAME_TRADE_EXCHANGE_*` and checks its flag. Their POKéMON (in `src/data/trade.h`) never evolve
through trade, so the first half of the exchange doesn't evolve them. Like any trade, the exchange
registers the trader's POKéMON in the POKéDEX.
