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

RED (or ETHAN) waits deep in CERULEAN CAVE (B1F), where MEWTWO used to be, once the player is back
from Hoenn (`FLAG_HOENN_KANTO_LINK`, after Hoenn's Hall of Fame; see `docs/hoenn_plan.md`). The
post-game LEAF waits for that too.
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

## Hoenn starters

BIRCH gives one Hoenn starter when the player arrives in Hoenn (`docs/hoenn_plan.md`, Act 1). After
Hoenn's Hall of Fame, right after the starter's MEGA STONE, he gives the other two at level 5
(`HoennStory_EventScript_GiveOtherHoennStarters` in `data/scripts/hoenn_story.inc`).

## Johto starters

Each Johto starter has a side quest early in Hoenn, before WATTSON's gym (`docs/johto_starters_plan.md`;
BIRCH's lab no longer offers one of the three after a complete Hoenn Dex):

| POKéMON   | Where                                                         | Level |
|-----------|---------------------------------------------------------------|-------|
| CHIKORITA | ROUTE 104's flower shop: bring an ORAN, a PECHA and a CHERI BERRY | 5  |
| CYNDAQUIL | DEWFORD's POKéMON CENTER: save the HIKER's CYNDAQUIL from two ZUBAT in GRANITE CAVE 1F | 10 |
| TOTODILE  | ROUTE 109's SEASHORE HOUSE: find the SODA POP thief on ROUTE 109's, SLATEPORT's and DEWFORD's beaches | 15 |

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

In FireRed, the SAFFRON CITY DOJO gives either HITMONLEE or HITMONCHAN. Here, once the player is back
from Hoenn (`FLAG_HOENN_KANTO_LINK`, after Hoenn's Hall of Fame), the DOJO's trainers are swapped on transition for stronger ones
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

Before then, nothing changes. Breeding a HITMON with DITTO for a TYROGUE is still another
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

then `tools/hoenn/add_missing_wild.py --write` and `tools/day_night_encounters/apply.py --write`,
which rebuilds the day and night tables (`docs/day_night_encounters_plan.md`).

## Trade evolutions

Twelve double-exchange traders wait in Pokémon Centers along the way, using the mechanism tested
in `TestArea_House`. The player gives a POKéMON, receives the trader's, and the two are traded
straight back, so the player's POKéMON evolves on its way back, with its held item if its
evolution needs one (METAL COAT, KING'S ROCK, DRAGON SCALE, UP-GRADE…).

Each trader exchanges only once (`FLAG_DID_DOUBLE_EXCHANGE_*`). So that no exchange is wasted,
they first check the chosen POKéMON with the `WouldMonEvolveThroughTrade` special: if it wouldn't
evolve they refuse, and if it needs an item it isn't holding they name the item. Fourteen traders
cover the ten Kanto trade evolutions (ALAKAZAM, MACHAMP, GOLEM, GENGAR, STEELIX, SCIZOR, POLITOED,
SLOWKING, KINGDRA, PORYGON2) and Hoenn's two (HUNTAIL and GOREBYSS), with two to spare.

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
| Mossdeep City | MARIO (Sailor) | WINGULL |
| Lilycove City | NELSON (Fisherman) | LUVDISC |

The Kanto ones stand at (10, 7), next to the table; in Hoenn's Pokémon Centers, MARIO stands at
(10, 7) and NELSON at (10, 6). The shared script is
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
  FireRed. The other two follow (see "Johto and Hoenn" below).
- **Smaller things:** Vermilion Gym's trash cans, DAISY's grooming, the Name Rater, the move
  deleter, the diploma, SELPHY's requests at RESORT GORGEOUS, the MAGIKARP and HERACROSS size
  records, the BERRY POWDER vendor (it now takes the powder), Cycling Road's forced bike, the
  POKéMON marked as seen by signs and binoculars (`SetSeenMon`), and screen shakes.

Still missing (they don't affect which POKéMON can be caught): the Fan Club's rankings, the
museum's fossil pictures, the S.S. ANNE departure cutscene, LORELEI's dolls, the help system,
Trainer Tower and the e-Reader trainer. The Birth Island DEOXYS puzzle is back (see "Johto and
Hoenn" below).

## Johto and Hoenn

A check of every Johto and Hoenn species (#152-386): the wild tables of the maps the player can reach
(`tools/hoenn/check_reachability.py`), every `givemon`, `giveegg`, `setwildbattle` and `seteventmon`,
the NPC trades, then what evolving and breeding add (the BABY POKéMON, AZURILL and WYNAUT with the
SEA and LAX INCENSE from LOST CAVE, a DITTO for the female-only ones), plus the cases scripts and code
decide (FEEBAS's fishing spots, the roamer, the Game Corner, the DOJO, Altering Cave's tables). After
the Johto starters' quests and BIRCH's other two starters, 18 were still missing:

| POKéMON | Why | Now |
|---|---|---|
| SURSKIT, MASQUERAIN, MEDITITE, MEDICHAM, ROSELIA, ZANGOOSE, LUNATONE | Emerald has no wild slot for them | Wild in Hoenn, where Ruby and Sapphire had them |
| HUNTAIL or GOREBYSS | CAPT. STERN trades the SCANNER for one deep-sea item only, and nothing else gives the other. With both, the 12 traders would all be needed | STERN gives both, and two more traders wait in Hoenn |
| RAIKOU, ENTEI | Only one beast roams, picked from the starter (always SUICUNE here) | The other two roam after it |
| LUGIA, HO-OH | NAVEL ROCK needs the MYSTIC TICKET, from Mystery Gift only | CELIO gives the ticket |
| DEOXYS | BIRTH ISLAND needs the AURORA TICKET, from Mystery Gift only, and the port had lost its puzzle | The Space Center gives the ticket; the puzzle is back |
| CELEBI | Nothing gave it | In the BERRY FOREST, after LUGIA and HO-OH |

Altering Cave only has ZUBAT without Mystery Gift, but every species of its other tables is also in
Hoenn's Safari Zone (its north-east and south-east areas, open after Hoenn's Hall of Fame) or Artisan
Cave.

### Two more double-exchange traders

The trade evolutions take 12 exchanges (Kanto's 10 and two CLAMPERL), as many as there were traders,
so one exchange spent on a POKéMON the player already had (a second ALAKAZAM) would have left a gap
in the POKéDEX, and the traders were all in Kanto. Two more wait in Hoenn's Pokémon Centers, where
CLAMPERL lives (see "Trade evolutions" above): MARIO in MOSSDEEP CITY and NELSON in LILYCOVE CITY
(`INGAME_TRADE_EXCHANGE_MOSSDEEP_CITY`/`_LILYCOVE_CITY`, `FLAG_DID_DOUBLE_EXCHANGE_MOSSDEEP_CITY`/
`_LILYCOVE_CITY`).

### The wild tables

`tools/hoenn/add_missing_wild.py` gives each species slots of the one it shares its area with in Ruby
and Sapphire; the slots keep their levels:

| POKéMON | Where | Slots |
|---|---|---|
| LUNATONE | METEOR FALLS, every room | Every other SOLROCK slot, land and water (which one gets the first slot alternates by room) |
| ZANGOOSE | ROUTE 114 | The first SEVIPER slot (4%) |
| ROSELIA | ROUTE 117 | Two ODDISH slots (20%) |
| MEDITITE | MT. PYRE, outside | Two VULPIX slots (15%) |
| SURSKIT | ROUTE 102, 111, 114, 117 and 120's water | The 30% surfing slot; also ROUTE 102's 1% grass slot, as in Ruby |

### CAPT. STERN's deep-sea items

Right after the SCANNER trade (`SlateportCity_Harbor`), whichever item the player picked, STERN gives
the other: the SCANNER showed where the ABANDONED SHIP's cargo went down. Each item has its own flag
(`FLAG_RECEIVED_DEEP_SEA_TOOTH`/`_SCALE`), so with a full BAG he keeps it for the next talk, and a save
that traded before this gets the other one too.

### The legendary beasts

The beasts roam one after the other, in the order ENTEI, SUICUNE, RAIKOU, from the one FireRed gives
for the starter (`src/roamer.c`). When the roaming beast is caught, the next one the player hasn't
caught starts roaming at once (`TryStartNextRoamer`, from the end of the battle in
`src/battle_main.c`). A beast that is knocked out stops roaming, as in FireRed, and roams again after
the next Kanto Hall of Fame (the same special, in `PokemonLeague_HallOfFame`).

### The tickets, NAVEL ROCK and BIRTH ISLAND

- **MYSTIC TICKET**: CELIO, in ONE ISLAND's POKéMON CENTER, once the link with Hoenn is mended
  (`FLAG_HOENN_KANTO_LINK`) and his quest is done: his network machine printed it. VERMILION's
  SEAGALLOP then sails to NAVEL ROCK (`FLAG_ENABLE_SHIP_NAVEL_ROCK`, as the Mystery Gift sets).
- **AURORA TICKET**: the Mossdeep Space Center's director, after JIRACHI
  (`HoennStory_EventScript_TryGiveAuroraTicket`): the meteorite that fell with the MILLENNIUM COMET came
  down on BIRTH ISLAND. Then `FLAG_ENABLE_SHIP_BIRTH_ISLAND`.
- **BIRTH ISLAND's puzzle**: the port had dropped FRLG's `DoDeoxysTriangleInteraction`. It is back, as
  Emerald's `DoDeoxysRockInteraction` (`src/field_specials.c`) with Kanto's vars and flag
  (`VAR_DEOXYS_INTERACTION_NUM`, `VAR_DEOXYS_INTERACTION_STEP_COUNTER`, `FLAG_SYS_DEOXYS_AWAKENED`),
  for the island the player is on, and so is `SetDeoxysTrianglePalette`. Hoenn's copies of the two
  islands stay closed (`docs/hoenn_plan.md`, D7).
- In FireRed, LUGIA, HO-OH and DEOXYS fly away for good when knocked out. They are back after the next
  Kanto Hall of Fame (`FLAG_LUGIA_FLEW_AWAY`, `FLAG_HO_OH_FLEW_AWAY` and `FLAG_DEOXYS_FLEW_AWAY` cleared
  there).

### CELEBI

Once LUGIA and HO-OH are caught (`FLAG_FOUGHT_LUGIA` and `FLAG_FOUGHT_HO_OH`, which FireRed sets only on
a catch), CELEBI floats in the BERRY FOREST's north-west corner (THREE ISLAND), at level 50. It is drawn
from its PC box icon, like the Johto starters (`OBJ_EVENT_GFX_ICON_CELEBI`). CELIO tells the player
about it until it's caught. Beaten or run from, it vanishes, and is back the next time the player
enters the forest (`FLAG_CAUGHT_BERRY_FOREST_CELEBI`, `FLAG_HIDE_BERRY_FOREST_CELEBI`).

The new flags are `KANTO_EXTRA_FLAGS_START + 0x1AA`-`0x1AF`.

### Status

Implemented. Checked in mGBA with test builds:

- STERN: the SCANNER traded for the DEEPSEATOOTH, then the DEEPSEASCALE given, and his thanks after.
- MARIO in MOSSDEEP: a CLAMPERL holding the DEEPSEATOOTH exchanged and back as HUNTAIL. NELSON in
  LILYCOVE: his lines.
- The beasts: SUICUNE roaming, then RAIKOU in the next roamer battle once SUICUNE was caught; and the
  order of `TryStartNextRoamer` through every case (SUICUNE, then RAIKOU, RAIKOU again after a
  knock-out, ENTEI, then none).
- CELIO's MYSTIC TICKET, then his CELEBI rumor; the director's AURORA TICKET; VERMILION's SEAGALLOP
  offering NAVEL ROCK and BIRTH ISLAND, and the ride to NAVEL ROCK.
- BIRTH ISLAND: the puzzle solved (the triangle's colors change), DEOXYS appearing, caught, and gone
  after.
- CELEBI: shown in the BERRY FOREST, the battle, and vanishing after a knock-out.
- The new text measured against the text box (205 pixels at most, `{PLAYER}` counted as seven Ws).

Not played: the wild encounters themselves, a full BAG at STERN's and CELIO's, the Hall of Fame bringing
back a knocked-out legendary, CELEBI caught and coming back after a knock-out, LUGIA and HO-OH inside
NAVEL ROCK (FireRed's scripts, unchanged).

