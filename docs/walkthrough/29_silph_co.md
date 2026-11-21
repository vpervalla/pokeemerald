# 29 Silph Co. (Saffron City)

**Arrival.** The Silph Co. door at (33,30) in Saffron. The 11 floors are linked by stairs, an **elevator** (floor panel at (0,2), face up from (0,3); the elevator exit mat only works after stepping off and on again), **teleport pads** (a warp tile that sends you to another floor and lands on a pad there) and **Card Key doors** (two-by-two panels: stand next to one and press A: "Bingo! The CARD KEY opened the door!", or "The door is open..." afterwards).

## Map notes

- **1F:** a worker; the stairs (31,2) and the elevator (22,3). Everything starts here: the lift reaches every floor.
- **5F: the CARD KEY** lies at (22,21) in a corridor that nothing on foot reaches: only the **9F pad (22,18)** lands next to it (5F (10,20)). The 9F lobby side of that pad is reached from the elevator (9F lobby (24,4)). A text box left open or a trainer's dialogue makes every move look blocked: clear it first.
- Pads used (arrival in brackets): 3F (13,14) to 7F (5,4); 7F (5,8) to 11F (2,5); 3F (2,14) to 9F (9,4); 9F (22,18) to 5F (10,20); 5F (10,20) to 9F (22,18).
- Doors per floor: 2F (5..6,8..9) and (5..6,15..16); 3F (9..10,12..13) and (20..21,12..13); 4F (3..4,16..17) and (14..15,11..12); 5F three; 7F (11..12,8..9), (24..25,7..8), (25..26,13..14); 9F four; 11F (5..6,16..17) closing the board room.
- **9F** has a **nurse-like woman at (2,16)** ("You look tired. You should take a quick nap.": the whole party is healed, PP too). Reached from the 3F pad (2,14) then the door (2..3,10..11).
- **7F, Blue (the rival)** waits in the west room (2,6), reached by the 3F pad (13,14): "Well, ETHAN! I'm moving on up and ahead!..." Team: PIDGEOT L37 first, then EXEGGCUTE L35 and others (KADABRA, GROWLITHE-line, a water starter...); ¥ 4 100 range. It went to the party's limit: two fainted.
- **11F: Giovanni** (board room (6,11), door (5..6,16..17)): NIDORINO L37, then RHYHORN and others; ¥4 100: "Blast it all! You ruined our plans for SILPH! But, TEAM ROCKET will never fall! ETHAN! Never forget that all POKéMON exist for TEAM ROCKET! I must go, but I shall return!". Afterwards the **President** (9,9): "I will never forget you saved us in our moment of peril... I can give you anything. You can't buy that anywhere. It's our secret prototype **MASTER BALL**. It will catch any POKéMON without fail!" (received, key to the Legendaries: keep it).

## Items and trainers

All 3 grunts of 2F to 11F, the scientists (Jerry, Connor, Jose, Rodney, Beau, Taylor, Joshua, Parker, Travis, Ed, Dalton the juggler) and the rival and Giovanni are listed on the right; **TM41 (Softboiled), TM01 (Focus Punch), TM08 (Body Slam... see the item list), the Card Key, MAX REVIVE, FULL HEAL, ESCAPE ROPE, HYPER POTION, X SPECIAL, HP UP, PROTEIN, ZINC, CALCIUM, IRON x2, NUGGET, ELIXIR, PP UP, REVIVE (11F hidden)** were taken (hidden: Ultra Ball 2F, Protein 3F, Iron 4F, Elixir 5F, PP Up 5F, Carbos 6F, Zinc 7F, Nugget 8F, Calcium 9F, Revive 11F).
**Still left (reached only through pads that were not found in time):** the 9F grunt (15,18) and the hidden **MAX POTION (11,14)**, and on 10F the grunt (0,13), **CARBOS (5,15), ULTRA BALL (2,15), RARE CANDY (3,17)** and the hidden **HP UP (9,9)**.

## Story

1. Silph's first floors by stairs (clearmap): the first fights (Rocket grunts, scientists) with the lead switching by type. The lead DR VENKMAN kept fainting to NORMAL-type teams (his Ghost moves do nothing to them), so `lead 0 2` put CHARIZARD first.
2. 5F: the CARD KEY via the elevator (9F) and the pad.
3. Cards opened every door on each floor; 6F: the workers; 7F: Blue; 9F: the healer; 11F: Giovanni and the MASTER BALL.
4. CHARIZARD learned **SLASH at L44 (you chose to forget EMBER)**; ALAKAZAM got **PSYCHIC** and lost RECOVER at some point in the fights **without a prompt I saw** (to check, see bugs/notes); CHARIZARD reached L45 and ALAKAZAM L39.
5. Save `saves/ethan_silph.sav`.

## Party after

CHARIZARD L45 (WING ATTACK, FLAMETHROWER, SLASH, SMOKESCREEN), WARTORTLE L34, DR VENKMAN L35 (fainted), VENUSAUR L37, ALAKAZAM L39 (PSYCHIC, CONFUSION, CALM MIND, PSYBEAM; fainted), SNORLAX L30. Money ¥86 713. Master Ball in the bag.

## Bugs

- The move-learn guard in `g.py` looked for the word "forget" and blocked ordinary text ("Never forget that all POKéMON..."): fixed to the real prompts.
- The map's teleport/door puzzle makes the static planner useless: `rbfs.py`/`rbfs_g.py` explore the real game (see the guide).
