# 33.8 Safari Zone

**Arrival.** The gate at (24,5) north of Fuchsia: the attendant at (7,3) charges ¥500 for 30 SAFARI BALLs and 500 steps. The Kanto Safari Zone is four areas (CENTER, EAST, NORTH, WEST) with rest houses and the SECRET HOUSE.

## Map notes

- **Entrance** (map 44.0): door (4,1) into the CENTER area; leaving early through the Center's south mat (25..27,30) asks "Are you going to leave the SAFARI ZONE early?" (YES).
- **CENTER** (34.63): the pond (18..33,13..18) with an island (NUGGET (24,15), hidden LEAF STONE); the fishing spot (33,19); one-way ledges cut it in two (the west door (8,17..19) is not reachable from the entrance on foot).
- **EAST** (34.64), **NORTH** (34.65), **WEST** (34.66): grass and items; the WEST area holds the **GOLD TEETH (28,14)**, **TM32 (17,13)**, a hidden **REVIVE (13,11)**, the **SECRET HOUSE** (12,7) and, in its southern part (not reachable from the north), a MAX POTION (16,27) and a MAX REVIVE (27,24).
- **DRATINI** (L15-25) only by fishing (slot 8 of the rod table, 15% with the SUPER ROD). Catch chances per SAFARI BALL are low (catch factor 3, about 7%; 15% flee chance each turn); one GO NEAR raises the catch factor to 7 and the flee chance to 35%.

## Story

1. **Safari Zone** (door (24,5), ¥500, 30 SAFARI BALLS; walking out through the south mat of the Kanto Center map is the safe exit). The Kanto part (CENTER, EAST, NORTH, WEST) has **DRATINI L15-25 by fishing only** (slot 8 of the rod table: 15% on the SUPER ROD, which the bag already had). The pond is at (18..33,13..18); stand at **(33,19)** facing up (the ledge row at y=20 can only be passed at x 31 to 33 from the east side). The rod was registered to SELECT and a script cast it, pressed A when the "Oh! A bite!" text jumped (it comes more than once, the dots game repeats) and ran from every GOLDEEN, SEAKING and the like. Several SAFARI BALLs were wasted by an unfinished script (A pressed on the menu).
2. **The DRATINI was not caught.** A DRATINI L22 did bite and I threw SAFARI BALLs at it, but my script took the battle for finished after the first throw and the DRATINI got away. The script's blind A presses had also thrown balls at wild Pokémon earlier: **two GOLDEENs (L19 and L22) were caught** that way (they are in Box 2; GOLDEEN was the 49th species in the Pokédex, 48 before). SAFARI BALLS left 22. The DRATINI was still to catch after this first visit.
3. **Bug 4: RETIRE sends you to Hoenn.** Choosing RETIRE in the START menu ("Would you like to exit the SAFARI ZONE right now?") warps to **Route 121's Safari entrance (Hoenn)**, not to Fuchsia. FLY does not help yet (no new badge). The Kanto Safari maps (`SAFARI_ZONE_CENTER` and friends) have no way back from there. A savestate from before the retire fixed it. Always leave on foot through the south mat (25..27,30) of the Safari Center and answer YES to "leave early?".


Save: `saves/ethan_safari.sav` (no DRATINI yet).

4. **Second visit (¥500, 30 new SAFARI BALLs): DRATINI caught.** Same spot (33,19). The script now throws one ball at a time and checks the result after each, answers the text with B (so no nickname is typed) and stops at 1 ball (running out of balls also warps to Hoenn, see bug 4). Four DRATINIs fled first (after 3, 5, 1 and 2 throws; the game's numbers: catch factor 3, so about 7% per SAFARI BALL, and a 15% flee chance each turn). One GO NEAR at the start of a fight raises the catch factor to 7 and the flee chance to 35%, a better deal per ball. **DRATINI L21 was caught on the fifth try** with 3 balls left (Box 2, named DRATINI).
5. A **PSYDUCK L23** (new species) was caught on the way, but the old script's A presses typed its nickname: it is called **AAAAAAAAAA** (the Name Rater can fix it, like the EEVEE).
6. Pokédex: 51 species, so OAK's AIDE on Route 15's gate 2F now had a prize for us (it turned out to be the EXP. SHARE, see [Route 15](32_route_15.md)). Walked out through the south door (YES to "leave early?"). Save: `saves/ethan_dratini.sav`.

## GOLD TEETH and HM03 (SURF)

1. **Where things are** (from the ROM's maps): the **GOLD TEETH** are an item ball at **(28,14) in the Safari Zone West** area; **HM03 (SURF)** is given by the attendant in the **SECRET HOUSE** (West area door (12,7)), as the prize for being "the first person to reach the SECRET HOUSE!"; the **WARDEN** (Warden's house (33,31), the old man at (3,5)) swaps the GOLD TEETH for **HM04 (STRENGTH)**.
2. **Getting there:** the static planner finds no way from the entrance to the West door (one-way ledges), so the real-game search (`rbfs_g`) found the route through the **East** and **North** areas (about 285 steps of the 500). A wrapper handled the wild encounters on the way: new species got SAFARI BALLs (**NIDORAN♂ L30** and **NIDORINO L33** caught; EXEGGCUTE, VENOMOTH and RHYHORN fled), the rest were run from.
3. In the West area: **GOLD TEETH**, **TM32 (17,13)** and the hidden **REVIVE (13,11)** taken; the Secret House attendant gave **HM03**. The MAX POTION (16,27) and MAX REVIVE (27,24) lie in the West area's southern part, not reachable from the north part. The West rest house was not entered.
4. **No way back on foot** from the West area: the real-game search explored everything reachable (about 2 000 states) without finding the exit. So the trip ends with the step counter.
5. **The ROM was rebuilt in the middle** (you asked for the Safari exit fix, commit f2a9d4e0d on `kanto-hoenn`, merged): you cannot save inside the Safari Zone and the old savestate does not survive a rebuild (75 000 code addresses moved), so the first trip's GOLD TEETH and HM03 were lost and the trip was **redone from `ethan_soul_badge.sav` on the new ROM**.
6. **Timeout test:** the steps were walked down to 0 on purpose: "Ding-dong! Time's up! Your SAFARI Game is over. Did you catch your fair share? Come again!" and the player landed in the **Fuchsia** Safari Zone entrance (4,4). **Bug 4 is fixed.**
7. **WARDEN:** "ETHAN gave the GOLD TEETH to the WARDEN. The WARDEN popped in his teeth! ... Let me give you something for your trouble." **HM04** received ("Inside HM04 you'll find STRENGTH. It lets POKéMON move boulders when you're outside of battle.").
8. **The RARE CANDY behind his boulder still can't be reached:** STRENGTH activates ("Its STRENGTH made it possible to move boulders around!", a borrowed MACHOP) but the boulder never moves, pushed from either side: **bug 5**. **Fixed** in the ROM at your request (the push check now accepts the Kanto boulder): after a rebuild the boulder moved and the **RARE CANDY** was taken (not used). Saved: `saves/ethan_strength_fix.sav`.
9. Saved: `saves/ethan_hm03_hm04.sav` (5 badges, Pokédex 58, 193:40).

## SURF on the pond

At your choice **BLASTOISE learned SURF (HM03) in place of BUBBLE**. A third Safari trip (¥500) used it on the Center pond (face the water from (33,19)): the island's **NUGGET (24,15)** and the hidden **LEAF STONE** (found facing left from (27,15)) were taken; out through the south door ("leave early?" YES).

## Bugs

- **Bug 4** (RETIRE, out of balls and out of steps warped to Hoenn): fixed by commit f2a9d4e0d and verified here (the timeout put us in the Fuchsia entrance).
