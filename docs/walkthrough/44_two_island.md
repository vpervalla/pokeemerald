# 44 Two Island

**Arrival.** By the SEAGALLOP from One Island, arriving at the harbour (8,5) and out at (10,8).

## Map notes

- The POKéMON Center (21,7), a house (33,9), the **Joyful Game Corner** (39,9), and an outdoor stall with a clerk (28,4). People: a woman (the shopkeeper's brother works at the Game Corner), a beauty, a sailor, a POKé MANIAC ("Boing! Boing! Have you tried the jumping game?"), a little boy and a boy. A REVIVE (39,16) behind a Cut tree (30,16).
- North: [Cape Brink](44_two_island--cape_brink.md).

## Story

1. The sweep: everyone talked to; the Cut tree cut ("The wild PARAS used CUT!") and the **REVIVE** taken.
2. **The stall clerk** gives today's free card pack, then opens a shop whose list is all "????????" at ¥0 (bug 7: the shop's item lists are empty in the script); the ¥0 purchase was declined.
3. **The Joyful Game Corner:** walking in starts a scene: LOSTELLE's father paces ("Now, where's LOSTELLE gotten to today? She always brings me lunch every day right about now..."), asks if the player has seen her, and a biker of the gang that "said they'd be waiting on the island" stumbles in by mistake ("These islands are confusing..."). The METEORITE is handed over only after LOSTELLE is found (the script `GiveDaddyMeteorite`).
4. Saved: `saves/ethan_two_island.sav`.
5. **The stall's shop fixed** (bug 7): after the rebuild it sells GREAT BALL ¥600 and FRESH WATER ¥200.
6. After LOSTELLE's rescue on Three Island she walks the player back here: "So you rescued LOSTELLE? How can I thank you?" Then the **METEORITE**: "Oh, what's that? You're saying that this is for me? ... Oh, I see, this is from BILL!": a **MOON STONE** in return. "All right, I'll get the GAME CORNER running right away." He also offers a card duel (played out by my key presses; no penalty).
7. Healed at the Center and saved: `saves/ethan_meteorite.sav`.

## Bugs

- [Bug 7](../playthrough_bugs.md): the stall's shop sold "????????" for ¥0 (fixed).
