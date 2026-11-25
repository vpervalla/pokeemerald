# 33.9 Fuchsia Gym (Koga)

**Arrival.** The Gym door (9,32) in Fuchsia City ([page 33](33_fuchsia_city.md)). The party was the L45 team (Charizard L51), healed.

## Map notes

- Map 44.3: a room of **invisible walls** (they are ordinary collision tiles for the planner, so the paths are known); Koga stands on the platform at (7,13), the gym guide at (11,19), the exit mat (7,21).
- **Trainers (all beaten):** Jugglers **Kirk** (DROWZEE, DROWZEE, KADABRA, DROWZEE L31), **Kayden** (HYPNO L38), **Nate** (DROWZEE, KADABRA L34), **Shawn** (DROWZEE, HYPNO L34); Tamers **Edgar** (ARBOK, SANDSLASH, ARBOK L33) and **Phil** (SANDSLASH, ARBOK L34).
- **KOGA:** KOFFING L37, MUK L39, KOFFING L37, WEEZING L43. Prize ¥4 300. After the battle he gives the **SOUL BADGE** ("It also lets you SURF outside of battle") and **TM06 (TOXIC)** ("Sealed within that TM06 lies TOXIC! ... It will surely terrorize foes!").
- Talking to Koga again starts the ROM's **card game** ("Fwahaha! A ninja's cards are full of tricks. Duel me!"): it was forfeited (choose an Active card, "Done, start the duel", then on the first turn's menu the last entry **Forfeit**, A, A). "Fwahaha! Poison wins slowly!"

## Story

1. The sweep (`clearmap` with `SWITCH=auto`) beat Kirk, Edgar and Phil (GENGAR's PSYCHIC and VENUSAUR's RAZOR LEAF against the Tamers), then went to **Koga** before the other three Jugglers, which it could not engage: walking next to them did not start a battle; facing them and pressing A by hand did.
2. **Koga:** DRAGONAIR led; GENGAR (switched in for its PSYCHIC) used up its attack PP and **fainted**, VENUSAUR was paralysed; SNORLAX and CHARIZARD finished the second KOFFING, MUK and WEEZING. GENGAR reached L46.
3. The three Jugglers: **Kayden**, **Nate**, **Shawn**, one at a time (face them, A).
4. Healed at the Fuchsia Center and saved: `saves/ethan_soul_badge.sav` (5 badges, Pokédex 56, time 193:25).

## Party after

DRAGONAIR L45, GENGAR L46, VENUSAUR L45, CHARIZARD L51, BLASTOISE L45, SNORLAX L45. Money ¥147 829. New: SOUL BADGE (SURF in the field), TM06.

## Bugs

- The `trainer` tool's walk-up does not start these Jugglers' battles (turns 0); talking to them while facing them does. Not a game bug as far as seen.
