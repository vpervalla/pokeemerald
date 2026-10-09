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
