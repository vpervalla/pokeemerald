# 22.1 Lavender Pokémon Center

**Arrival.** Door (6,5).

## Map notes

- Heals and saves: a nurse, a gentleman ("I saw CUBONE's mother trying to escape from TEAM ROCKET. She was killed trying to get away..."), a lass, a youngster, a balding man ("TEAM ROCKET will do anything for the sake of money!") and the **Channeler at (10,7)**, a **double-exchange trader**. The second floor is the cable club (not usable alone).

## Story

1. **The PC** (BILL'S PC: WITHDRAW, DEPOSIT, MOVE POKéMON, MOVE ITEMS, SEE YA!): PRIMEAPE deposited ("Deposit in which BOX?", "Continue BOX operations?": NO leaves) and the Modest GASTLY withdrawn (see the [Pokémon Tower](22_lavender_town--pokemon_tower.md)).
2. **The double exchange: DR VENKMAN becomes GENGAR.** The Lavender Center has its own **double-exchange trader**: the **Channeler at (10,7)** (the script `LavenderTown_PokemonCenter_1F_EventScript_DoubleExchangeTrader`, flag `FLAG_DID_DOUBLE_EXCHANGE_LAVENDER_TOWN`; the first visit's sweep talked to him without accepting). Each Center has one and each exchanges only once: Cerulean, Vermilion (used for ALAKAZAM), Route 10, Lavender, Celadon, Saffron, Fuchsia, Cinnabar and four of the Sevii Islands; the unused ones were read from the save (Vermilion's flag set, all others clear). The trader asks "Ready to exchange?" ("Remember, I can only do this once."), offers his **GASTLY "WISP"** (OT AGNES), "That was the first exchange. Now let's trade them back!", and "Oh, your DR VENKMAN! It'll evolve on its way back!": **"Congratulations! Your DR VENKMAN evolved into GENGAR!"** (L25, 71 HP, Sp. Atk 82; SHADOW PUNCH, NIGHT SHADE, SPITE, MEAN LOOK). An Escape Rope took the party out of the Tower to the town door first. `saves/ethan_gengar.sav`.
