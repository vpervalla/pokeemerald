# 21 Rock Tunnel

**Arrival.** The north door (8,19) on [Route 10](21b_route_10.md)'s north part; the south door (18,37) comes out on Route 10's south part.

## Map notes

- **Rock Tunnel is dark.** FLASH lights a small circle (see below); the tools walk it from the map data either way. **The floors are split in pieces, joined by ladders:** 1F north (door (17,2), REPEL (37,5), ladder (45,2) to B1F (38,28)); B1F east (ladder (33,3) leads to 1F west at (4,2)); 1F west (ladder (20,13) to B1F west (27,12)); B1F west (ladder (2,3) to 1F south at (45,21)); 1F south has the south door (18,37).

## Everyone

- **All tunnel trainers are beaten and every item taken:** ASHTON, LENNY (4 HIKER/MANIAC teams of GEODUDE, MACHOP, ONIX, GRAVELER), LUCAS, OLIVER, DUDLEY, COOPER (three SLOWPOKE L20), SOFIA, DANA, ARIANA, LEAH, STEVE, ALLEN, ERIC, MARTHA, WINSTON; items REPEL, ESCAPE ROPE, PEARL, MAX ETHER (9,2), the REVIVE (2,37), plus a talk with the youngster at B1F (2,29). The group was fought with the lowest-level POKéMON in front (WARTORTLE, PIKACHU) and the type-switch helper bringing in IVYSAUR, MANKEY or CHARMELEON when the foe resisted; Pikachu fainted once (Geodude).
- **Caught in the tunnel:** GEODUDE L16, ONIX L15 (a Gust and a ball each). **MACHOP** L17 (wild on B1F: THUNDER WAVE from PIKACHU, then the first ball; it went to Box 1). **GRAVELER** is missing: in this ROM it only appears from the smashable rocks of B1F (ROCK SMASH, L25 to L40), and no POKéMON of the team has ROCK SMASH.

## Story

1. **Rock Tunnel, first try (dark):** 1F north (ASHTON, the REPEL) and B1F east (STEVE, ALLEN, ERIC, the REVIVE). The leveling loop that followed killed the whole party on a wild GEODUDE (see the whiteout below).
2. **MACHOP L17 caught on B1F:** THUNDER WAVE from PIKACHU, then the first ball. Throwing balls with the tool left a stray nickname screen: B erases, START moves to OK, A keeps the default name (the catch tool presses A blindly: a bug of mine).
3. The tunnel needed light and **HM05 FLASH** was fetched from Route 2's Aide (see [Route 2](05_route_2.md), second section of its log): PIKACHU learned it, the tunnel was resumed with FLASH lit from the party menu.
4. **The rest of the tunnel,** floor by floor through the ladders: 1F west (LUCAS, OLIVER, LENNY, the ESCAPE ROPE), B1F west (COOPER, DUDLEY, SOFIA, MAX ETHER; the youngster talked to), 1F south (DANA, ARIANA, LEAH, the PEARL). Super Potions on PIKACHU and WARTORTLE between fights.

## Whiteout (a bug of mine, not the game's)

A leveling loop (`train` with `MINHP=0` and `SWITCH=0`) kept Pikachu fighting GEODUDE (Ground, immune to its Electric moves) until the whole party fainted: **"ETHAN is out of useable POKéMON… ETHAN panicked and lost ¥19 905"**, the game's normal penalty (half of ¥39 810), and the party woke up at the Route 10 Center. The first-priority rule (never faint the whole team) was broken by switching the safety off.

## FLASH (HM05) and the trip to Route 2

The tunnel's NPCs and the Aide's rocker say the same thing: teach FLASH. **HM05 is given by the Aide in the Route 2 east building (needs 10 POKéMON seen)**, so the leg went back through Rock Tunnel's north door to Route 9 (the Cut tree at (2,8) had grown back), Cerulean (via the robbed house's back door), Route 5, the Underground Path, Route 6, Vermilion (Center: heal and `saves/ethan_flash.sav`), Route 11, Diglett's Cave and Route 2 (see [05](05_route_2.md)). PIKACHU learned **FLASH** in place of TAIL WHIP; it was used from the party menu on the way into the tunnel (the circle of light is larger, but still small: the first FLASH use only reaches level 1 of the dark). The trip also gave CHARMELEON **FLAMETHROWER** (L34, in place of SCARY FACE) and ALAKAZAM L30 (FUTURE SIGHT declined).

## Bugs

None found in the game.
