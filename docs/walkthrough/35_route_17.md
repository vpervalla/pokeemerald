# 35 Route 17 (the Cycling Road)

**Arrival.** From Route 16's west half ([page 34](34_route_16.md)), riding south; the road ends at Route 18 ([page 36](36_route_18.md)).

## Getting the BICYCLE (the long walk)

FLY needs a new badge, so it was done on foot from Fuchsia: Route 15 north lane (see [page 32](32_route_15.md)), Routes 14, 13 and 12, the Route 12 west strip at y=70 (where the SNORLAX was) to **Route 11** and its gate, **Vermilion City**: in the **Pokémon Fan Club** (12,17) the chairman (5,4) tells his story ("YES") and gives the **BIKE VOUCHER**; then Route 6, the **Underground Path** (south entrance stairs (7,4) press LEFT, north end (4,3) press RIGHT), Route 5 and **Cerulean City**, where the Cut tree at **(26,32)** had to be cut to get back south. The **Bike Shop** clerk (9,3) swaps the voucher for the **BICYCLE** ("Okay! Here you go!"). Back south: Route 5's gate, Saffron, Route 7's gate (Route 7's west exit is the strip at y 1..4, reached through the ledge gap at x 6..7), Celadon, Route 16. The BICYCLE was registered to SELECT.

## Map notes

- **Almost every tile of the road pulls the bike south** (metatile `MB_CYCLING_ROAD_PULL_DOWN`): standing still means sliding down to the next wall or water. Holding UP rides uphill. The middle of the road (x 8..16 on most rows) is **water**, not ground. The only opening between Route 17 and Route 18 is **column 12** (row 158).
- Neither the static planner nor `rbfs` handle it, so a small driver (`ride.py` in the scratchpad) did a grid search over the road with water and trainers as walls and held the next direction every 6 frames; to talk to a trainer it rides to the tile below and holds UP+A.
- A first `clearmap` run simply slid the whole road from top to bottom (beating only Jamal on the way).
- **Trainers (all beaten):** Bikers **Virgil** (WEEZING, KOFFING, WEEZING L28), **Nikolas** (VOLTORB x2 L29), **Billy** (MUK L33), **William** (KOFFING, WEEZING, KOFFING, KOFFING, WEEZING L25), **Jaxon** (WEEZING, MUK L29); Cue Balls **Isaiah** (MACHOP, MACHAMP L29), **Raul** (MANKEY, PRIMEAPE L29), **Jamal** (MANKEY, MANKEY, MACHAMP, MACHOP L26), **Zeek** (MACHOKE L33), **Corey** (PRIMEAPE, MACHOKE L29).
- **Hidden items, not taken:** RARE CANDY (18,83), FULL RESTORE (10,53), PP UP (4,68), MAX REVIVE (4,125), MAX ELIXIR (14,157): they lie on pull-down tiles, and facing one while standing still is not possible on the bike.
- The tall grass at the top (around (15,19..25)) has wild Pokémon; several fights there were wild ones.

## Story

1. DRATINI led every fight and switched out on the first turn (`SWITCH=3 RESHARE=1`, CHARIZARD coming in); a Super Potion was needed after William. At the top, a wild battle in the grass took DRATINI to L25.
2. CHARIZARD reached L50.

## Party after

DRATINI L25, DR VENKMAN L39, VENUSAUR L37, CHARIZARD L50, WARTORTLE L34, SNORLAX L32.

## Bugs

None (the pull-down tiles are the game's design).
