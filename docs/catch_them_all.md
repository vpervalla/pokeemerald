# Catching every Pokémon without trading

The goal is for every species to be obtainable in a single game, with no trades.

## Starters

In the Kanto story, the player picks one of BULBASAUR, CHARMANDER and SQUIRTLE, and no other
starter of KANTO, JOHTO or HOENN appears anywhere else (Hoenn can't be reached, so Prof. Birch's
JOHTO starters are gone too).

Once the POKéDEX is upgraded to the National Dex, at the end of that scene in Oak's lab, PROF. OAK
gives the player the 8 starters they don't have, at level 5:

- the KANTO starter left in the last POKé BALL on his table (the ball disappears),
- the one like the rival's,
- CHIKORITA, CYNDAQUIL and TOTODILE, sent by PROF. ELM,
- TREECKO, TORCHIC and MUDKIP, sent by PROF. BIRCH.

They go to the party, then to the PC. If the boxes fill up, Oak keeps the rest and gives them the
next time the player talks to him. Each starter has its own flag (`FLAG_RECEIVED_STARTER_*`, with
the player's own starter marked at once), so nothing is given twice. A save that is already past
the National Dex scene gets them the next time the player talks to Oak.

The scripts are at the end of `data/maps/PalletTown_ProfessorOaksLab/scripts.inc`, after the
generated Kanto NPCs. The generated block has two calls into them: one at the end of the
National Dex scene and one when the player talks to Oak.
