# 22.6 Pokémon Tower

**Arrival.** Door (18,6). A seven-floor graveyard (7F is closed without the SILPH SCOPE) full of Channelers with GASTLY; its wild encounters are GASTLY, CUBONE and, rarely, HAUNTER.

## Map notes

- **Warps in the Tower:** 1F (18,9) up; 2F (4,10) up / (18,10) down; 3F (18,10) up / (4,10) down; 4F (4,10) up; 5F (18,10) up; 6F stairs at (11,16), blocked by the MAROWAK ghost. The up and down stairs sit on the same tiles in alternating floors: do not walk back and forth on a stair tile (an automatic walk loop on 5F fell up to 6F).
- The 2F rival, the Channeler trainers and the 4F Elixir behind a gravestone (reach it from (13,11), facing west) are listed below.
- **Escape Rope** works from inside the Tower (it returns to the door at (18,7) of Lavender).
- **1F:** a worker, a Channeler ("There are spirits up to mischief. I sense them high in the TOWER."), two women and a balding man mourning ("My GROWLITHE... Why did you die?").

## Story

1. **2F: the rival.** BLUE stands at (16,5): "{RIVAL}: Hey, {PLAYER}! What brings you here? Is your POKéMON dead? Hey! It's alive! I can at least make them faint!" A full-team battle (won with PIKACHU, WARTORTLE and the others, ¥900); afterwards "What? You stinker! I took it easy on you, too!" and "I just caught a CUBONE! I can't find the bigger MAROWAK. Where could they be?" The Channeler at (3,7): "Even we could not identify the wayward ghosts. A SILPH SCOPE might be able to unmask them."
2. **3F to 6F: the Channelers** (GASTLY L22 to L24 each, HAUNTER L23 for Tammy on 5F; Angelica has three GASTLY): Patricia, Carly, Hope (3F); Laurel, Jody, Paula (4F); Ruth, Tammy, Karina, Janae (5F); Angelica, Jennifer, Emilia (6F). They use **CURSE and MEAN LOOK**: after MEAN LOOK a POKéMON cannot be switched out ("PRIMEAPE can't be switched out!"); PRIMEAPE (Fighting and Ground moves do nothing to a GHOST/POISON GASTLY with LEVITATE) fainted twice. ALAKAZAM's PSYBEAM (x2) and PIKACHU's THUNDERBOLT do the work. (A tool slip: the type chooser picked DIG against a levitating GASTLY and looped for half an hour on the move menu; the loop is now stopped when the same enemy repeats.)
3. **Items:** ESCAPE ROPE (3F, (13,2)), AWAKENING (8,11), GREAT BALL (11,16) and the ELIXIR (12,11) on 4F (behind a gravestone), the NUGGET (6,16), the **CLEANSE TAG** (11,9) and a hidden BIG MUSHROOM (7,3) on 5F, the RARE CANDY (5,15) and X ACCURACY (15,15) on 6F (the Rare Candy is kept in the bag by the no-Rare-Candy rule).
4. **Wild catches:** **GASTLY L17** (3F; PIKACHU's THUNDERBOLT to 19/37, a ball: "Gotcha!") and **CUBONE L17** (4F; WARTORTLE's BUBBLE to 16/48, a ball). A **HAUNTER L20** appeared twice (4F): the first was knocked out by one THUNDERBOLT (54 HP), the second was weakened by a looping tool and left the battle: **HAUNTER is still missing**. Encounters are rare (about one per 15 steps on 3F to 5F). The PP of WARTORTLE's BUBBLE and RAPID SPIN ran out in the hunt; it learned **PROTECT** at L31 (replacing TAIL WHIP).
5. **6F: the ghost.** Walking onto the stairs (11,16): "Be gone... Intruders..." and the player is pushed back one tile: the MAROWAK ghost cannot be fought without the **SILPH SCOPE** (Celadon's Rocket Hideout), so the top floor, MR. FUJI and the POKé FLUTE wait for later. The wild "GHOST" battles on 6F and 7F also appear as normal GASTLY here (the ROM's note: Emerald's battles have no unidentified GHOST).
6. **Bad hour:** a hunt loop standing on 5F's up-stairs walked onto 6F and wild GASTLY wore the party down to CHARMELEON only; the party was saved by an ESCAPE ROPE and the Lavender Center. The saves: `saves/ethan_lavender.sav`, then `saves/ethan_tower.sav` (healed at Lavender).

## The SILPH SCOPE, the ghost and MR. FUJI (7F)

1. With the SILPH SCOPE (Rocket Hideout) back on **6F**: walking to (11,15) starts the ghost scene ("Be gone... Intruders...") and the **MAROWAK ghost L30** (84 HP) is a normal battle now; won in five turns. "The ghost was the restless spirit of CUBONE's mother! The mother's spirit was calmed. It departed to the afterlife...". The stairs (11,16) up to 7F work (hold a direction on them: LEFT here).
2. **7F:** three Rocket grunts (ZUBAT L25 x2 + GOLBAT L25; KOFFING L26 + DROWZEE L26; ZUBAT, RATTATA, RATICATE, ZUBAT L23) fell to THUNDERBOLT and PSYBEAM (¥2 400 together). Talking to **MR. FUJI at (11,4)** ends the scene and he sends you to the Volunteer House.
4. Back on 7F the hidden **SOOTHE BELL (11,4)**, where he stood, was found from (11,3) facing down. Save `saves/ethan_poke_flute.sav`.

## Hunting a MODEST GASTLY (for the GENGAR goal)

1. The GASTLY caught first is **Relaxed** (nature = personality mod 25, read from the PC box), so a **Modest** one (+Sp. Atk, -Atk: the best for a special attacker like GENGAR) was hunted on Tower 5F, on a strip of floor away from the stairs (a first try on the purified-zone tile did nothing: the text box "Entered the purified and protected zone." swallowed the walking keys).
2. A script walked and, on every wild encounter, read the enemy's nature from the battle; anything but a Modest GASTLY/HAUNTER was beaten with the lead. The first attempt tried to RUN with a key sequence that picked a move instead (THUNDER WAVE), then the script switched to fighting. The **7th encounter** was a **Modest GASTLY L15**: THUNDER WAVE, then GREAT BALLS ("Gotcha!"); stray A presses in the nickname screen named it "BBBCCCDDDE" (later renamed, see below). The nature was confirmed in the PC data (`Modest`).
3. At the Center's PC the new GASTLY replaced PRIMEAPE, and its typo name was fixed at the [Name Rater](22_lavender_town--name_rater.md).

## Training DR VENKMAN with simple battles (no swap-ins)

1. DR VENKMAN (GASTLY, moves HYPNOSIS, LICK, SPITE, MEAN LOOK) was put in front and fought wild GASTLY, HAUNTER and CUBONE on the 5F strip alone, healing with POTIONS between fights; a script drove it (walk back and forth, fight, drink, repeat).
2. **Results:** L15 to **L18** (4351 exp) in about ten fights, but **three faintings** (REVIVE each time) and a stuck loop: the wild GASTLYs use **MEAN LOOK** (he cannot be switched out or run: "Can't escape!"), **HYPNOSIS** (he sleeps while the foe attacks) and **CURSE**; a wild **HAUNTER L20** (49 HP) beat him outright. LICK is his only attack and its 30 PP ran out after about ten fights: an ELIXIR restored 10 PP for each move.
3. **Learned:** CURSE was offered at L16 and refused (it would replace a move). A move-learn prompt stopped the first script: NO to deleting, YES to stop learning; a "stuck" fight must be dismissed with those keys, not A. The script's health check failed silently once because the POTION at the top of the bag ran out (the top entry became an ANTIDOTE).
4. The grind was paused at L18 (`saves/ethan_venkman18.sav`), then resumed with **no items** and a trick: Tower **5F's purified zone** (tiles x 10-12, y 8-10: `special HealPlayerParty`, "ETHAN's POKéMON were fully healed.") heals everything, fainted POKéMON included. The loop stepped up into the zone tile (11,10) and back down to (11,11) after every fight; wild battles only happen outside it. If DR VENKMAN fainted, the next POKéMON finished the fight and the next zone step revived him.
5. **L18 to L25 in 24 more fights** with no faint stopping the run: NIGHT SHADE learned at L21, then **evolved into HAUNTER at L25** ("Modest": 74 Sp. Atk) and learned SHADOW PUNCH. A tool slip again taught the new moves in place of LICK and HYPNOSIS (the move-learn prompt has two text boxes before the YES/NO; the key sequence answered YES and forgot the first move). Moves now: SHADOW PUNCH, NIGHT SHADE, SPITE, MEAN LOOK. `saves/ethan_haunter.sav`.

## Bugs

None found in the game: the tower's rival fight and the Mean Look "can't be switched out" message work as in the original.
