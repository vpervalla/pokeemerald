# Catching every Pokémon without trading

The goal is for every species to be obtainable in a single game, with no trades.

## Kanto starters

The player arrives at Oak's lab too late: RED took CHARMANDER, LEAF took BULBASAUR and the rival
took SQUIRTLE (ETHAN stands in for RED, or KRIS for LEAF, when the player is dressed as them). There
is no rival battle in the lab, and OAK gives the player a PIKACHU instead, at level 5.
`VAR_STARTER_MON` is set to 2, as if the player had picked CHARMANDER, so the rival's teams always
follow his SQUIRTLE.

The trainers who took the starters meet the player again (`data/scripts/lab_trainers.inc`). Talking
to them starts the battle; afterwards they leave. LEAF (or KRIS) battles the player four times, and
each time she's beaten she also hides the earlier ones the player skipped:

| Battle    | Where                                      | Team                                                                      |
|-----------|--------------------------------------------|---------------------------------------------------------------------------|
| Early     | ROUTE 24, past NUGGET BRIDGE               | NIDORAN♀ 13, CLEFAIRY 13, BULBASAUR 15                                    |
| Middle    | CELADON CITY, outside the DEPT. STORE      | NIDORINA 30, CLEFAIRY 30, PONYTA 31, PSYDUCK 30, IVYSAUR 33              |
| Late      | ROUTE 23, by VICTORY ROAD                  | NIDOQUEEN 50, CLEFABLE 49, RAPIDASH 50, GOLDUCK 50, DRAGONAIR 51, VENUSAUR 53 |
| Post-game | CERULEAN CITY, by CERULEAN CAVE (Surf)     | NIDOQUEEN 74, CLEFABLE 74, RAPIDASH 75, STARMIE 75, DRAGONITE 76, VENUSAUR 78 (VENUSAURITE) |

RED (or ETHAN) waits deep in CERULEAN CAVE (B1F), where MEWTWO used to be, after the Hall of Fame.
MEWTWO isn't there anymore: it's MR. M., on APEX ISLAND. His team: SNORLAX 77, LAPRAS 77,
ESPEON 76, AERODACTYL 76, GENGAR 76 and CHARIZARD 80 with CHARIZARDITE X.

After the post-game battle, LEAF gives the player her VENUSAURITE, and RED his CHARIZARDITE X
(see "The MEGA STONES before the tournament" in `docs/mega_island.md`).

The NPCs use `OBJ_EVENT_GFX_RED_OR_ETHAN` and `OBJ_EVENT_GFX_LEAF_OR_KRIS`, which pick the sprite
from the player's costume. LEAF and KRIS, and RED and ETHAN, are separate trainers with the same
parties (`src/data/lab_trainers.h` and `lab_trainer_parties.h`).

Each Kanto starter then has its own side quest, always in the same place, a little like in Pokémon
Yellow, but they take more effort:

| POKéMON    | Where                                   | Level |
|------------|-----------------------------------------|-------|
| BULBASAUR  | VIRIDIAN FOREST, then VERMILION CITY    | 20    |
| CHARMANDER | MT. MOON                                | 10    |
| SQUIRTLE   | CERULEAN CITY                           | 12    |

CHARMANDER and SQUIRTLE are shown on the map with their PC box icon as an overworld sprite
(`OBJ_EVENT_GFX_STARTER_*`, in `src/data/object_events/starter_object_events.h`). BULBASAUR and
the BEEDRILL use pokeemerald-expansion's follower sprites (`OBJ_EVENT_GFX_BULBASAUR` and
`OBJ_EVENT_GFX_BEEDRILL`, in `src/data/object_events/story_mon_object_events.h`, made by
`tools/viridian_bulbasaur/sprites.py`).

### VIRIDIAN FOREST: the lost BULBASAUR

- In VERMILION CITY (`VermilionCity_House2`, with ELYSSA), a TRAINER lost their BULBASAUR in
  VIRIDIAN FOREST when a swarm of BEEDRILL attacked them. It ran into the trees to the north-east.
- Behind two trees that need CUT, in the forest's north-east corner, a path leads to a small
  clearing where two BEEDRILL have cornered the BULBASAUR against a tree. Stepping into the
  clearing starts the scene: two wild battles against the BEEDRILL at level 20, one after the
  other. Each BEEDRILL beaten or caught stays gone; running or blacking out leaves the rest there.
- Once both are gone, the BULBASAUR thanks the player, shows the tag with its address in
  VERMILION CITY and runs home. Its TRAINER then gives the player one of their own BULBASAUR.

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
  after POKéMON that were hurt or abandoned. Her SQUIRTLE was left on NUGGET BRIDGE by its
  TRAINER after it lost a battle, and it doesn't trust TRAINERS anymore.
- Each time the player talks to her, they show their lead POKéMON to it. It comes out only if
  that POKéMON's friendship is 150 or more (`GetLeadMonFriendshipScore`). At 100 to 149, she says
  they're almost there. Then the POKéMON joins the player at level 12, after a YES/NO question.

The scripts are in `data/scripts/starter_quests.inc` (which species) and after the generated
Kanto NPCs in the scripts of `ViridianForest`, `VermilionCity_House2`, `MtMoon_B2F`,
`Route4_PokemonCenter_1F` and `CeruleanCity_House5`. The flags are at the end of the Kanto extra flags in
`include/constants/flags.h`.

## The other MT. MOON fossil

In FireRed, the player and MIGUEL each take one of the two fossils on MT. MOON B2F, so either
KABUTO or OMANYTE needs a trade. Here MIGUEL leaves MT. MOON once the fossils are taken (his
object is hidden by `FLAG_GOT_FOSSIL_FROM_MT_MOON`) and studies his fossil at the PEWTER CITY
MUSEUM (1F, next to the fossil cases; `FLAG_HIDE_PEWTER_MUSEUM_MIGUEL` is set on transition).

- Until the player's own fossil has been revived at the CINNABAR LAB, he asks to see the POKéMON
  it becomes.
- Once it has been revived, he wants to see it (or its evolution) in the party. Then he gives the
  player his fossil (`FLAG_GOT_FOSSIL_FROM_MIGUEL`, and the fossil's `FLAG_GOT_*_FOSSIL`).
- The CINNABAR LAB revives it like the first one. The doctor only says he has revived every
  fossil once each MT. MOON fossil the player got has been revived.

The scripts are after the generated NPCs in `PewterCity_Museum_1F` and
`CinnabarIsland_PokemonLab_ExperimentRoom`.

## The other DOJO POKéMON

In FireRed, the SAFFRON CITY DOJO gives either HITMONLEE or HITMONCHAN. Here, after the ELITE FOUR
(`FLAG_SYS_GAME_CLEAR`), the DOJO's trainers are swapped on transition for stronger ones
(`FLAG_HIDE_DOJO_TRAINERS` / `FLAG_HIDE_DOJO_REMATCH_TRAINERS`, trainers `TRAINER_DOJO_*` in
`src/data/dojo_trainers.h` and `dojo_trainer_parties.h`):

- The four BLACK BELTS (levels 60 to 63) battle on sight, as the first time.
- MASTER KOICHI only fights once all four are beaten. His team ends with a level 70 HERACROSS that
  holds a HERACRONITE and Mega Evolves.
- Beating him lets the player take the POKé BALL still on the altar
  (`SaffronCity_Dojo_EventScript_CheckCanTakeHitmon`), and he tells them so: the other POKéMON,
  or both if they never took one. He also gives them his HERACRONITE
  (`FLAG_GOT_HERACRONITE_FROM_KOICHI`; with a full BAG he gives it the next time they talk). Until
  the altar is empty, he reminds them of the POKéMON waiting there.

Before the ELITE FOUR, nothing changes. Breeding a HITMON with DITTO for a TYROGUE is still another
way.

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

## FireRed's in-game trades

FireRed's nine NPC trades are the only way to get some POKéMON: MR. MIME (Route 2), JYNX
(Cerulean City), FARFETCH'D (Vermilion City), LICKITUNG (Route 18), and also NIDORAN♀ (Underground
Path), NIDORINA (Route 11), ELECTRODE, TANGELA and SEEL (Cinnabar Lab). The port had dropped their
`setvar VAR_0x8008, INGAME_TRADE_*`, because Emerald had no such constants, so every one of them
ran Emerald's trade 0. Their trades are now in `src/data/trade.h` as `INGAME_TRADE_KANTO_*`
(FireRed's versions, with ZYNX's mail), and `port_trainers.py` maps FRLG's `INGAME_TRADE_*`
names to them, so porting the NPCs again keeps them.

## Scripts the port had dropped

`port_trainers.py` leaves out every line that uses a special, command or constant Emerald lacks,
plus the lines testing its result. Some of those lines blocked POKéMON or broke whole events, so
the missing FRLG specials were added (`src/field_specials.c`, `data/specials.inc`; they keep
their FRLG names, so porting again keeps the lines), or mapped to Emerald's equivalents in
`SPECIAL_RENAMES`. The scripts were then restored by hand inside the generated blocks.

- **POKéMON TOWER 6F (MAROWAK):** without the SILPH SCOPE the GHOST can't be identified and the
  player is pushed back. With it, the player battles MAROWAK, which dodges every BALL
  (`FLAG_UNCATCHABLE_WILD_BATTLE`, checked in `Cmd_handleballthrow`), and winning calms its spirit.
  Before this, the scene softlocked and blocked MR. FUJI and the POKé FLUTE (SNORLAX).
- **Menus:** the Game Corner (coins and prizes, including ABRA, CLEFAIRY, DRATINI, SCYTHER and
  PORYGON), the CINNABAR LAB fossils (OMANYTE, KABUTO, AERODACTYL), the Bike Shop, the Celadon
  roof vending machines and the other `MULTI_KANTO_*` menus.
- **Sevii Islands:** CELIO and the RUBY/SAPPHIRE quest (National POKéDEX checks), the WATER
  LABYRINTH TOGEPI EGG (friendship check), Icefall Cave's cracked ice, the Seafoam currents and
  the layouts the scripts switch to, the Dunsparce Tunnel, and the braille rooms.
- **Roamer:** RAIKOU, ENTEI or SUICUNE (depending on the starter) roams the Kanto routes, as in
  FireRed.
- **Smaller things:** Vermilion Gym's trash cans, DAISY's grooming, the Name Rater, the move
  deleter, the diploma, SELPHY's requests at RESORT GORGEOUS, the MAGIKARP and HERACROSS size
  records, the BERRY POWDER vendor (it now takes the powder), Cycling Road's forced bike, the
  POKéMON marked as seen by signs and binoculars (`SetSeenMon`), and screen shakes.

Still missing (they don't affect which POKéMON can be caught): the Fan Club's rankings, the
museum's fossil pictures, the S.S. ANNE departure cutscene, LORELEI's dolls, the help system,
Trainer Tower, the e-Reader trainer, and the Birth Island DEOXYS puzzle (an event-only POKéMON).
