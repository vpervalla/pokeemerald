# 24 Route 12

**Arrival.** South of Lavender Town: the town's south wall is rock except for two tiles, **(11,19) and (12,19)**, which lead onto Route 12 (map 36.30) at (11,3). The route is a long wooden-bridge road (120 rows). A **gatehouse crosses the road at y 15 to 21**: Route 12 north is cut off from the rest of the road until you walk through it (door (14,15), stairs, door (14,21)). The road south ends at a sleeping **SNORLAX** on the bridge at (14,70): it needs the POKé FLUTE (Mr. Fuji, after the Pokémon Tower).

## Map notes

- **Gatehouse 1F/2F** (doors (14,15), (15,15), (14,21)): a policeman ("There's a lookout spot upstairs. The view is magnificent.") and, upstairs, a **little girl** who gives **TM27 (RETURN)** ("My POKéMON's ashes are stored in POKéMON TOWER. You can have this TM. I don't need it anymore...": "ETHAN put away the TM27") and two **binoculars** ("It's POKéMON TOWER!" and "There's a man fishing!").
- The Route 12 bridges are one tile wide in places: a trainer who walked up to the player stays where he stopped. **FISHERMAN HANK** ended on the junction at (13,47) and shut the way back north, but the bridge there is two tiles wide: go round by row 48 (from (14,47): down, left twice, then up along x=12).
- The route's **wild grass is south of the Snorlax** (BELLSPROUT, WEEPINBELL, GLOOM, ODDISH, VENONAT, PIDGEY; night HOOTHOOT): not reachable yet, so VENONAT, GLOOM and WEEPINBELL are still missing. **TM48** at (18,36) lies on a part of the bridge network that no walk from the gate reaches (it needs SURF or a path that opens later).

## Everyone and every building

- **Trainers north of the Snorlax (all beaten):** fishermen **NED** (GOLDEEN, POLIWAG, GOLDEEN L22), **CHIP** (TENTACOOL, GOLDEEN L24), **HANK** (GOLDEEN L27) and **ELLIOT** (POLIWAG, SHELLDER, GOLDEEN, HORSEA L21), and the **young couple GIA & JES** (NIDORAN♂ and NIDORAN♀ L24, a **double battle**): about ¥5 300 in all. South of the Snorlax stand LUCA (VOLTORB, ELECTRODE L29), JUSTIN (NIDORAN♂, NIDORINO L29) and ANDREW (MAGIKARP x2 L24).
- Hidden: **HYPER POTION** (18,57) found. Under the Snorlax: **LEFTOVERS** (14,70). South: RARE CANDY (9,116) and an IRON (9,101) behind a Cut tree.
- Reaching the Snorlax: "A POKéMON is sprawled out in a deep and comfortable slumber." (no action without the POKé FLUTE).

## Story

1. **Lavender to the gate.** The party was healed at the Lavender Center; GENGAR (L26) led so that it could level on the fishermen's water types. The first sweep found everything "unreachable" until the **gatehouse** was crossed (the planner treats the building as a wall).
2. **The fishermen** fell to the type-chosen leads (RAZOR LEAF from VENUSAUR on NED, THUNDERBOLT from PIKACHU on the others; DR VENKMAN reached L27). **Gia & Jes** stalled the sweep tool (double battle again): played by hand with GENGAR (SHADOW PUNCH) and WARTORTLE (BUBBLE).
3. The Snorlax: one step from the bridge's end: no battle, only the message above. The walk back north was blocked by Hank at the bridge corner (see map notes); ESCAPE ROPE does not work outdoors ("OAK's words echoed... there's a time and place for everything").
4. **TM27 (RETURN)** came from the lookout's little girl only on the second try: the first visit's sweep presses skipped the gift. Not a game bug (the text says "put away the TM27").
5. Saved at the lookout: `saves/ethan_route12.sav`.

## Party after

DR VENKMAN (GENGAR) L27, WARTORTLE L32, CHARMELEON L35, VENUSAUR L34, ALAKAZAM L32, PIKACHU L33. Money ¥19 827.

## Bugs

None found.

## Later: waking and catching the SNORLAX (with the POKé FLUTE)

1. At (14,69) facing down (the SNORLAX is at (14,70)): "Want to use the POKé FLUTE?" YES, "SNORLAX woke up!" and a wild **SNORLAX L30** (140 HP, Quiet, holding LEFTOVERS; moves HEADBUTT, YAWN, REST, BODY SLAM-class) fights. **A mistake first**: my menu script selected RUN (a move index chosen while the screen still showed text) and the game ended the fight: "SNORLAX calmed down. It gave a huge yawn... And returned to the mountains" (a win, run or teleport removes it for good). A savestate taken at the start of the fight brought it back.
2. **What the SNORLAX does:** it uses **YAWN** almost every turn (whoever is out falls asleep a turn later: CHARIZARD and PIKACHU and VENUSAUR all slept, and a sleeping benched Pokémon keeps its sleep until it is out again), and **REST** whenever it is under about 40% HP (full HP, asleep, paralysis cured), and it recovers 8 HP a turn from the LEFTOVERS. DR VENKMAN (Ghost) takes nothing from its Normal moves.
3. **What worked:** CHARIZARD led and used SMOKESCREEN (the lowered accuracy kept its HEADBUTTs off); PIKACHU's THUNDER WAVE paralysed it; then, since REST always cures that, VENUSAUR's SLEEP POWDER and RAZOR LEAF (about 35 a hit) brought it to 27/140 while it slept (its own REST gave a two-turn window each time), and the first GREAT BALL then caught it. Boxed in **Box 1**.
4. Hidden **LEFTOVERS** (14,70) found where it slept.

Saves: `saves/ethan_snorlax.sav`.
