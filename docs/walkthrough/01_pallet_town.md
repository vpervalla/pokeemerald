# 01 Pallet Town

**Arrival.** The new game starts in the player's bedroom (PalletTown_PlayersHouse_2F, (6,6)), money ¥3000, no
POKéMON, no badges. See [Start](00_start.md).

## Map notes

- The player's house is at the west, the rival's house east of it, OAK's LAB south-east (door at (16,13)).
- The way out is the one-tile corridor at the top, x 12 (the grass starts at y 1): it leads to
  [Route 1](02_route_1.md). There is no other exit: the sea to the south is closed.
- The player's house, 1F: the exit warps are (4,8), (5,8) and (3,9); **(5,8) does nothing, use (4,8)**
  ([bug 1](../playthrough_bugs.md)). The stairs up are an arrow warp at (10,2) (press left).
- The bedroom's PC is at (1,1): stand at (1,2) facing up.

## Story

1. In the bedroom, use the PC (ITEM STORAGE, WITHDRAW ITEM): there is a **POTION** to take. ![PC](img/01_pc_potion.png)
2. Downstairs, MOM at (8,4): "...Right. All boys leave home someday. It said so on TV." ![Mom](img/01_mom.png)
3. Out of the house, walk to (12,2) and step up: OAK stops the player ("Hey! Wait! Don't go out! It's unsafe!
   Wild POKéMON live in tall grass!") and walks the player to his lab. ![OAK](img/01_oak_stops_you.png)
4. In the lab BLUE is waiting: he has already taken SQUIRTLE; RED took CHARMANDER and LEAF BULBASAUR early
   in the morning. OAK has one more POKéMON, a wild one that chewed on the power cables: **PIKACHU, L5**
   (THUNDERSHOCK, GROWL). ![Lab](img/01_lab_entry.png) ![Pikachu](img/01_pikachu_received.png)
5. The nickname prompt (YES/NO) appears: answered NO. ![Nickname](img/01_nickname_prompt.png)
6. Leave the lab. **A girl (the sign lady) now stands in the corridor at (12,2) and blocks the way north.** She says
   "Look, look! I copied what it said on one of those TRAINER TIPS signs! ... Press START to open the MENU!"
   ![Sign lady](img/01_sign_lady.png)
   She only moves after the START menu has been opened **and the map has been re-entered**: open START, close it,
   walk into any building and out again, and the corridor is free. This is the original FireRed tutorial; it is
   not a bug, but a player who talks to her and does nothing else can be stuck.

7. **Second visit, with OAK'S PARCEL** (from the Viridian Mart, see [Viridian City](03_viridian_city.md)): talk to OAK at
   (6,3) from (6,4). He says "Oh, ETHAN! How is my old POKéMON?", takes the parcel (a custom POKé BALL) and BLUE
   arrives. ![Parcel](img/01_parcel_delivered.png)
   - OAK gives the **POKéDEX** to ETHAN and BLUE ("a high-tech encyclopedia, it records data on POKéMON you've seen or
     caught"; the START menu now has POKéDEX). ![Dex](img/01_pokedex_received.png)
   - He gives **five POKé BALLS** ("you must catch them to obtain complete data"). ![Balls](img/01_pokeballs_received.png)
   - He explains catching: throw a ball at a wild POKéMON; it won't always work, a healthy one can escape. He wants the
     two of them to finish his POKéDEX. BLUE: "I'll borrow a TOWN MAP from my sis!" and leaves. Setting
     `VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN` to 1 here is what lets the old man in Viridian teach catching.

## Items

- POTION in the bedroom PC (the PC has only this).
- POKéDEX and 5 POKé BALLS from OAK, after the parcel.
- No item balls, no hidden items found.

## Pokémon

- No wild POKéMON here. Gift: **PIKACHU L5**, Lonely, 19 HP, stats 12/7/11/10/14.

## Trainers and shops

- No trainers. No shop (the POKéMON MART opens after the parcel, in Viridian).

## Party after

After the first visit: PIKACHU L5, ¥3000, no POKéDEX (the START menu: POKéMON, BAG, ETHAN, SAVE, OPTION, EXIT).
After the parcel: POKéDEX, 5 POKé BALLS.

## Bugs

- [Bug 1](../playthrough_bugs.md): the right half of the house's exit does nothing.
- Interactions tried: the PC, MOM, OAK's scene, BLUE, the sign lady. The sign lady's blocking is intended (see step 6).
- Not tried yet: the fat man and the signs, the other house.

**Save.** None here. See [Route 1](02_route_1.md).
