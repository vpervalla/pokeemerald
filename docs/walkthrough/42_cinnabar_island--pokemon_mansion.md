# 42.4 Pokémon Mansion

**Arrival.** Door (8,3), north-west of the Center. A burned-out four-floor house (1F, 2F, 3F, B1F) whose doors are opened and closed by **secret switches** in the statues (one switch flag for the whole house: each press flips every door).

## Map notes

- The switches: 1F (5,5), 2F (2,16), 3F (12,5), B1F (24,29) and (27,5); each one is pressed from the tile below it, facing up ("A secret switch! Press it?" YES, "Who wouldn't?").
- Stairs are arrow tiles, taken by walking in the arrow's direction; the cracked floor holes on 3F (18..20,18) drop to 1F and (23..24,18) to 2F; their landing spots can't be climbed back.
- Wild: RATTATA, RATICATE, GROWLITHE, VULPIX, GRIMER, KOFFING everywhere; MUK on 1F and 3F (5%), WEEZING on 2F and B1F (5%), DITTO on B1F (10%). The KOFFINGs and WEEZINGs use SELFDESTRUCT: they knocked SNORLAX out twice.
- **1F:** youngster JOHNSON (8,8) and scientist TED (22,24); ESCAPE ROPE (22,6), PROTEIN (27,15), CARBOS (29,32), hidden MOON STONE (2,21). The Magma (8,29) and Aqua (26,27) grunts are hidden until after the first Hall of Fame.
- **2F:** burglar ARNIE (5,23); ZINC (18,23), CALCIUM (36,8), HP UP (35,17).
- **3F:** burglar SIMON (7,13) and scientist BRAYDON (22,13), the latter in the middle room with the holes; MAX POTION (9,19), IRON (32,6), hidden RARE CANDY (36,13) in the east room.
- **B1F:** burglar LEWIS (20,25) and scientist IVAN (34,13); TM22 SOLARBEAM (6,21), TM14 BLIZZARD (23,4), FULL RESTORE (2,29), the **SECRET KEY** (5,7) (opens the Gym), the **MEWTWONITE Y** (36,4) and a hidden ELIXIR (35,5); the diary ("MEWTWO is far too powerful."). SHELLY, TABITHA, their grunts and BLAINE appear here only after the first Hall of Fame (they raid the lab's safe for the PRIMAL NOTES).

## Story

1. **1F:** the sweep beat JOHNSON (BRICK BREAK) and took the ESCAPE ROPE and the MOON STONE; the 1F switch opened the PROTEIN's room.
2. **The switch puzzle.** The doors are scripted (`Kanto_PokemonMansion_EventScript_PressSwitch_xF`), so the static planner and the real-game search both got lost. A planner over (floor, tile, switch state) was written (`tools/playthrough/mansion_plan.py`, with `mansion_go`/`mansion_act` to follow it in the game): it found every route, including pressing 2F's and 3F's statues in turn and dropping through 3F's hole.
3. **2F:** ARNIE (CHARMANDER, CHARMELEON) beaten; ZINC, CALCIUM; the HP UP from the 3F stairs on the east side.
4. **3F:** SIMON (NINETALES) and BRAYDON (MAGNEMITE, MAGNETON, VOLTORB) beaten; MAX POTION, IRON, the hidden RARE CANDY (kept, not used).
5. **1F again:** TED (ELECTRODE, WEEZING: the ELECTRODE's explosion left SNORLAX at 62 HP) and the CARBOS. SNORLAX reached **L50**; healed at the Center; OMANYTE collected at the Lab.
6. **B1F:** LEWIS (GROWLITHE, PONYTA) and IVAN (MAGNEMITE, ELECTRODE) beaten; TM22, TM14, the SECRET KEY, the MEWTWONITE Y, the ELIXIR and, through the B1F switch, the FULL RESTORE. Move prompts on the way: GENGAR L48 was offered DESTINY BOND and SNORLAX L51 HYPER BEAM: both declined (your choice).
7. **Catches** (first-visit rule): GRIMER and KOFFING on the way; **DITTO** (VENUSAUR's SLEEP POWDER, then an ULTRA BALL; the first DITTO copied SNORLAX, took ten balls and knocked it out), **WEEZING** (one BRICK BREAK, then balls) and **MUK** (1F hall). Two Center trips and an ESCAPE ROPE on the way; saved at the Center (`saves/ethan_mansion.sav`).
