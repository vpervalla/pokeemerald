# 33 Fuchsia City

**Arrival.** The west door of Route 15's gate ([page 32](32_route_15.md)) opens on Fuchsia City (map 36.7). Everything here is done except the Gym (not entered), the Safari Zone's far areas and the Rare Candy behind the Warden's boulder.

## Map notes

- **Fuchsia City** (map 36.7): the Pokémon Center (25,31) (a man at (10,7) offers the DOUBLE EXCHANGE, declined), the Mart (11,15), the Safari Zone office (28,16) (exit through the middle mat (6,9)), the **Warden's house (33,31)** (a **RARE CANDY (11,5)** sits behind a boulder at (11,6) that needs STRENGTH), three houses ((14,31), (19,31), (38,31)) and the **Gym (9,32)** (not entered yet; the Safari Zone gate at (24,5) neither). The fenced yard at (31..40,23..29) is reached only through **House 2's north door (3,1)** (arrive (39,28)); the **hidden MAX REVIVE (33,26)** is there: stand at (32,26), face right and press A. The first try failed with "The BAG is full..." (the bag's 30 item slots were full), so the NUGGETs and the STAR PIECE were sold at the Mart and two LEFTOVERS given to ALAKAZAM and CHARIZARD to make room.
- **Mart** (clerk at (2,3), counter): ULTRA BALL ¥1 200, GREAT BALL ¥600, SUPER POTION ¥700, REVIVE ¥1 500, FULL HEAL ¥600, MAX REPEL ¥700. Bought: **10 ULTRA BALLs, 6 REVIVEs, 5 FULL HEALs**; sold the 5 NUGGETs (¥25 000 for the five) and the STAR PIECE (¥4 900).
- Trainers in the **Gym**: Juggler Kayden (HYPNO L38), Shawn (DROWZEE, HYPNO L34), Kirk (DROWZEE x3, KADABRA L31), Nate (DROWZEE, KADABRA L34), Tamer Edgar (ARBOK, SANDSLASH, ARBOK L33), Phil (SANDSLASH, ARBOK L34) and **KOGA** (KOFFING L37, MUK L39, KOFFING L37, WEEZING L43). Not fought yet.

## Story

4. Fuchsia: heal, the buildings, shop, the hidden MAX REVIVE.
2. Saved: `saves/ethan_fuchsia.sav`.

## Party after

ALAKAZAM L42 (LEFTOVERS), WARTORTLE L34, DR VENKMAN (GENGAR) L37, VENUSAUR L37, CHARIZARD L47 (LEFTOVERS), SNORLAX L31. All healed at the Fuchsia Center. Money ¥120 889. Bag: **6 REVIVE, 1 MAX REVIVE, 5 FULL HEAL, 10 ULTRA BALL**.

## Not done here

- The Gym (Koga), the Safari Zone's other areas, the Rare Candy at the Warden's (STRENGTH).

## Bugs

- The Safari Zone office's middle mat (6,9) is the working exit; (5,9) and (7,9) did not warp (bug 2 family).
- **Bug 4**, see below.

## The PC and the Safari Zone's fishing (DRATINI)

1. **PC (Fuchsia Center, the machine at (11,2), face up from (11,3)):** ETHAN's PC > ITEM STORAGE > DEPOSIT: SOOTHE BELL, BLACKGLASSES, X ACCURACY, CLEANSE TAG, BIG MUSHROOM, PEARL, X DEFEND, AWAKENING, ICE HEAL, BURN HEAL, MOON STONE, PARLYZ HEAL and ANTIDOTE (the bag's Items pocket holds 30 slots).
2. **Safari Zone** (door (24,5), ¥500, 30 SAFARI BALLS; walking out through the south mat of the Kanto Center map is the safe exit). The Kanto part (CENTER, EAST, NORTH, WEST) has **DRATINI L15-25 by fishing only** (slot 8 of the rod table: 15% on the SUPER ROD, which the bag already had). The pond is at (18..33,13..18); stand at **(33,19)** facing up (the ledge row at y=20 can only be passed at x 31 to 33 from the east side). The rod was registered to SELECT and a script cast it, pressed A when the "Oh! A bite!" text jumped (it comes more than once, the dots game repeats) and ran from every GOLDEEN, SEAKING and the like. Several SAFARI BALLs were wasted by an unfinished script (A pressed on the menu).
3. **The DRATINI was not caught.** A DRATINI L22 did bite and I threw SAFARI BALLs at it, but my script took the battle for finished after the first throw and the DRATINI got away. The script's blind A presses had also thrown balls at wild Pokémon earlier: **two GOLDEENs (L19 and L22) were caught** that way (they are in Box 2; GOLDEEN was the 49th species in the Pokédex, 48 before). SAFARI BALLS left 22. The DRATINI was still to catch after this first visit.
4. **Bug 4: RETIRE sends you to Hoenn.** Choosing RETIRE in the START menu ("Would you like to exit the SAFARI ZONE right now?") warps to **Route 121's Safari entrance (Hoenn)**, not to Fuchsia. FLY does not help yet (no new badge). The Kanto Safari maps (`SAFARI_ZONE_CENTER` and friends) have no way back from there. A savestate from before the retire fixed it. Always leave on foot through the south mat (25..27,30) of the Safari Center and answer YES to "leave early?".

Save: `saves/ethan_safari.sav` (no DRATINI yet).

5. **Second visit (¥500, 30 new SAFARI BALLs): DRATINI caught.** Same spot (33,19). The script now throws one ball at a time and checks the result after each, answers the text with B (so no nickname is typed) and stops at 1 ball (running out of balls also warps to Hoenn, see bug 4). Four DRATINIs fled first (after 3, 5, 1 and 2 throws; the game's numbers: catch factor 3, so about 7% per SAFARI BALL, and a 15% flee chance each turn). One GO NEAR at the start of a fight raises the catch factor to 7 and the flee chance to 35%, a better deal per ball. **DRATINI L21 was caught on the fifth try** with 3 balls left (Box 2, named DRATINI).
6. A **PSYDUCK L23** (new species) was caught on the way, but the old script's A presses typed its nickname: it is called **AAAAAAAAAA** (the Name Rater can fix it, like the EEVEE).
7. Pokédex: 51 species, so OAK's AIDE on Route 15's gate 2F now has the EVERSTONE for us. Walked out through the south door (YES to "leave early?"). Save: `saves/ethan_dratini.sav`.
## Team change and save edits

At your request: **ALAKAZAM went to the PC (Box 2) and DRATINI joined the party** (Fuchsia Center PC). With a save editor (`edit.py` in the scratchpad, on the running game, then an in-game save): **DRATINI's nature was changed to ADAMANT** (a new personality value with the same gender byte, not shiny, the four data blocks re-ordered and re-encrypted, the checksum and the stats recomputed: the editor reproduced the game's own stats before the change), and the three Pokémon named AAAAAAAAAA got their species names back: **MAGIKARP, EEVEE, PSYDUCK**. The summary screen shows "ADAMANT nature".

## Safari Zone West: GOLD TEETH, HM03 (SURF) and HM04 (STRENGTH)

1. **Where things are** (from the ROM's maps): the **GOLD TEETH** are an item ball at **(28,14) in the Safari Zone West** area; **HM03 (SURF)** is given by the attendant in the **SECRET HOUSE** (West area door (12,7)), as the prize for being "the first person to reach the SECRET HOUSE!"; the **WARDEN** (Warden's house (33,31), the old man at (3,5)) swaps the GOLD TEETH for **HM04 (STRENGTH)**.
2. **Getting there:** the static planner finds no way from the entrance to the West door (one-way ledges), so the real-game search (`rbfs_g`) found the route through the **East** and **North** areas (about 285 steps of the 500). A wrapper handled the wild encounters on the way: new species got SAFARI BALLs (**NIDORAN♂ L30** and **NIDORINO L33** caught; EXEGGCUTE, VENOMOTH and RHYHORN fled), the rest were run from.
3. In the West area: **GOLD TEETH**, **TM32 (17,13)** and the hidden **REVIVE (13,11)** taken; the Secret House attendant gave **HM03**. The MAX POTION (16,27) and MAX REVIVE (27,24) lie in the West area's southern part, not reachable from the north part. The West rest house was not entered.
4. **No way back on foot** from the West area: the real-game search explored everything reachable (about 2 000 states) without finding the exit. So the trip ends with the step counter.
5. **The ROM was rebuilt in the middle** (you asked for the Safari exit fix, commit f2a9d4e0d on `kanto-hoenn`, merged): you cannot save inside the Safari Zone and the old savestate does not survive a rebuild (75 000 code addresses moved), so the first trip's GOLD TEETH and HM03 were lost and the trip was **redone from `ethan_soul_badge.sav` on the new ROM**.
6. **Timeout test:** the steps were walked down to 0 on purpose: "Ding-dong! Time's up! Your SAFARI Game is over. Did you catch your fair share? Come again!" and the player landed in the **Fuchsia** Safari Zone entrance (4,4). **Bug 4 is fixed.**
7. **WARDEN:** "ETHAN gave the GOLD TEETH to the WARDEN. The WARDEN popped in his teeth! ... Let me give you something for your trouble." **HM04** received ("Inside HM04 you'll find STRENGTH. It lets POKéMON move boulders when you're outside of battle.").
8. **The RARE CANDY behind his boulder still can't be reached:** STRENGTH activates ("Its STRENGTH made it possible to move boulders around!", a borrowed MACHOP) but the boulder never moves, pushed from either side: **bug 5**.
9. Saved: `saves/ethan_hm03_hm04.sav` (5 badges, Pokédex 58, 193:40).
