# 38 Saffron Gym (Sabrina)

**Arrival.** FLY from Route 15 (the borrowed PIDGEOT works now that there are five badges) to Saffron City ([page 28](28_saffron_city.md)); the Gym door is (46,12), entered from (46,13).

## Map notes

- Map 47.3: nine rooms joined only by **teleport pads** (32 pads, each sends you to a fixed partner; the entrance room's pad is (18,20)). The real-game search (`rbfs_g`) does not follow pads inside one map, so a small planner (`tools/playthrough/padplan.py`: BFS over the pads, the static planner inside each room) chose the hops; e.g. Sabrina's room from the entrance: (18,20), then (20,7), then (0,7), arriving at (18,15).
- **Trainers (all beaten):** Psychics **Cameron** (SLOWPOKE, SLOWPOKE, SLOWBRO L33), **Tyron** (MR. MIME, KADABRA L34), **Johan** (KADABRA, SLOWPOKE, MR. MIME, KADABRA L31), **Preston** (SLOWBRO L38); Channelers **Amanda** (GASTLY, HAUNTER L34), **Stacy** (HAUNTER L38), **Tasha** (GASTLY, GASTLY, HAUNTER L33).
- **SABRINA** (14,11): KADABRA L38, MR. MIME L37, VENOMOTH L38, ALAKAZAM L43. After the battle: the **MARSH BADGE** ("Stronger POKéMON will become wild, ignoring your orders in battle. Just don't raise your POKéMON too much..."), then **TM04 (CALM MIND)** ("It makes the POKéMON concentrate to raise both SP. ATK and SP. DEF.").
- Talking to her again: "I foresaw this. A card duel... Shall we begin?" (the card game; forfeited: "As I foresaw. You lose.").

## Story

1. The fights used `SWITCH=auto` and the type rules: GENGAR's SHADOW PUNCH against the Psychics, VENUSAUR's RAZOR LEAF against the SLOWPOKE/SLOWBRO lines, CHARIZARD against Tasha's Ghosts after GENGAR fainted (SNORLAX's Normal moves can't touch Ghosts).
2. Before Sabrina the party was healed at the Saffron Center (out through the pads and back).
3. **Sabrina** fell in 5 turns, mostly to GENGAR's SHADOW PUNCH; GENGAR reached L47.
4. Healed and saved: `saves/ethan_marsh_badge.sav` (6 badges, Pokédex 58, 194:15).

## Party after

DRAGONAIR L46, GENGAR L47, VENUSAUR L46, CHARIZARD L51, BLASTOISE L45 (WATER GUN, PROTECT, SURF, RAIN DANCE), SNORLAX L45. Money ¥157 753.

## Bugs

None.
