# Plan: the Johto starters in Hoenn

This is a plan for making CHIKORITA, CYNDAQUIL and TOTODILE gifts from three side quests in Hoenn,
the way the Kanto starters are gifts from three side quests in Kanto (`docs/catch_them_all.md`,
"Kanto starters"). Nothing is implemented yet.

## Where things are now

- **Emerald's pick in BIRCH's lab** is still live. After Hoenn's Hall of Fame,
  `EverGrandeCity_HallOfFame_EventScript_ReadyDexUpgradeEvent` sets
  `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` to 1, so the next step into LITTLEROOT plays Emerald's
  National Dex scene (`LittlerootTown_EventScript_BeginDexUpgradeScene`), even though the player
  already has OAK's National Dex. Then, once `HasAllHoennMons` is true (every Hoenn Dex species but
  JIRACHI and DEOXYS caught), the lab's table shows the three POKé BALLS and the player picks **one**
  Johto starter at level 5 (`LittlerootTown_ProfessorBirchsLab_EventScript_GiveCyndaquil`, ...,
  state 6). The other two can only be traded for.
- **Nothing else gives them**: no wild table (`src/data/wild_encounters.json` has none of the
  three), no trade, no egg. Only the BATTLE FRONTIER's dolls and the card game name them.
- **The pattern to follow**: `data/scripts/starter_quests.inc` and the five Kanto maps it names;
  CHARMANDER and SQUIRTLE as overworld objects made from their box icons
  (`src/data/object_events/starter_object_events.h`, `OBJ_EVENT_GFX_STARTER_*`).
- **Room**: the Hoenn flag block has `KANTO_EXTRA_FLAGS_START + 0x19F`-`0x1BF` free (33 flags),
  right after Act 5's and BIRCH's other Hoenn starters. `KANTO_VARS_START` has free vars after `0x71`. Emerald's spare trainer ids are
  only 861-863 and the Kanto trainers' flag block has 2 left, so the quests use no new trainers.

## 0. Decisions

| # | Question | Recommendation | Why |
|---|----------|----------------|-----|
| J1 | What happens to BIRCH's pick | Removed: the lab's table never shows the three balls (`FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_POKEBALL_*` stay set, state 4 is never entered). Each Johto starter has exactly one source, its quest | As in Kanto, "each starter always has its own quest". Keeping the pick would make one quest's POKéMON a duplicate, and its gate (the whole Hoenn Dex) is far later than the quests |
| J2 | Emerald's National Dex scene after Hoenn's Hall of Fame | Skipped: the Hall of Fame no longer sets `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` to 1; it goes straight to 3 (the state after the scene). The rival's and BIRCH's later lines that read the var stay valid | The player has had the National Dex since OAK. The scene also runs on the same `LittlerootTown` frame table as ARCHIE and MAXIE's (state 6), one right after the other |
| J3 | When the quests open | Early in Hoenn's story, in the first three places MR. BRINEY's boat opens: ROUTE 104, DEWFORD and SLATEPORT. All three are done before WATTSON's gym, and none needs a field move, the Kanto team or the link | The player starts Hoenn with only BIRCH's starter (D9 of `docs/hoenn_plan.md`); the Johto starters help most while the team is still being built |
| J4 | Levels | CHIKORITA 5, CYNDAQUIL 10, TOTODILE 15 | Each at about the level of the wild POKéMON where its quest is (ROUTE 104: 3-5, GRANITE CAVE: 6-11, ROUTE 110: 12-13), so it joins the team at once without being stronger than the region at that point (D5) |
| J5 | Kinds of quest | One of each kind the Kanto quests use, so the three don't feel alike: a fetch (berries), a rescue with battles, and a search | The Kanto ones are a rescue, a lost child and a friendship check |
| J6 | The overworld objects | The box icon objects, as CHARMANDER's and SQUIRTLE's: `OBJ_EVENT_GFX_STARTER_CHIKORITA`, `_CYNDAQUIL`, `_TOTODILE` | No new art. CHIKORITA and CYNDAQUIL use icon palette 1, TOTODILE icon palette 2, which needs a third palette tag |

## 1. The quests

| POKéMON | Where | Opens with | Level |
|---------|-------|------------|-------|
| CHIKORITA | ROUTE 104, the PRETTY PETAL FLOWER SHOP | Reaching ROUTE 104, after PETALBURG (the shop's first visit gives the WAILMER PAIL) | 5 |
| CYNDAQUIL | DEWFORD TOWN's POKéMON CENTER, then GRANITE CAVE 1F | MR. BRINEY's boat to DEWFORD (after the DEVON GOODS) | 10 |
| TOTODILE | ROUTE 109's SEASHORE HOUSE, then SLATEPORT's beach, ROUTE 109's beach and DEWFORD's beach | MR. BRINEY's boat to SLATEPORT (`FLAG_DELIVERED_STEVEN_LETTER`) | 15 |

### ROUTE 104: the CHIKORITA that won't eat

- In the PRETTY PETAL FLOWER SHOP (`Route104_PrettyPetalFlowerShop`), a new NPC, a visitor from
  JOHTO, brought her CHIKORITA to the sisters: it hasn't eaten since it left home. The sisters think
  it misses the BERRIES of JOHTO's gardens, and name three that grow in Hoenn too: an ORAN BERRY, a
  PECHA BERRY and a CHERI BERRY.
- The player brings one of each (`checkitem`). ROUTE 104's trees have all three from the start of the
  game (`data/scripts/new_game.inc` sets `BERRY_TREE_ROUTE_104_ORAN_1`, `_PECHA`, `_CHERI_1` and the
  others to `BERRY_STAGE_BERRIES`), and Hoenn is closed until after Kanto's Hall of Fame, so they're
  still there when the player arrives. No new item is needed.
- With all three, she takes them (`removeitem`), CHIKORITA eats, and it won't leave the player's
  side: she asks (YES/NO) if the player will take it, since it has chosen them. A NO keeps the
  berries taken and the offer open.
- CHIKORITA stands next to her as `OBJ_EVENT_GFX_STARTER_CHIKORITA` until it's given.

### GRANITE CAVE: the CYNDAQUIL whose flame went out

- In the DEWFORD TOWN POKéMON CENTER (`DewfordTown_PokemonCenter_1F`), a HIKER from JOHTO came to
  see GRANITE CAVE's old wall, and his CYNDAQUIL ran off into the cave when the ZUBAT swarmed. In the
  dark its flame went out, and he can't find it without one.
- In GRANITE CAVE 1F, by the ladder down to B1F (past the HIKER who gives FLASH; FLASH isn't needed),
  CYNDAQUIL is cornered by wild ZUBAT. Stepping in front of it starts the scene: two wild battles
  against ZUBAT at level 8, as with VIRIDIAN FOREST's BEEDRILL (each one beaten or caught stays gone;
  running or blacking out leaves the rest). GRANITE CAVE's own ZUBAT (7-10) keep it a scene of the
  place.
- Then CYNDAQUIL's flame lights again and it runs back to the POKéMON CENTER. The HIKER says it
  would rather stay with the TRAINER who saved it, and gives it (YES/NO, the offer stays open).
- The scripts copy `ViridianForest`'s BEEDRILL scene (`setwildbattle`, a var for how many are
  left, `FLAG_UNCATCHABLE_WILD_BATTLE` not set so they can be caught).

### SLATEPORT and ROUTE 109: the TOTODILE thief

- In ROUTE 109's SEASHORE HOUSE (`Route109_SeashoreHouse`), the owner says a little blue POKéMON
  has been stealing his SODA POP and diving back into the sea. Nobody can catch it: it's always
  somewhere else.
- TOTODILE hides at one of four places by the water, all reachable on foot or by MR. BRINEY's boat
  (no SURF): SLATEPORT's beach, two places on ROUTE 109's beach, and DEWFORD's beach (MR. BRINEY
  sails between ROUTE 109 and DEWFORD). The spot is kept in `VAR_TOTODILE_HIDING_SPOT`, and each
  map's transition script moves TOTODILE there (`setobjectxyperm`) or hides it.
- Each time the player finds it, it splashes away to the next spot, in a fixed order
  (`VAR_TOTODILE_TIMES_FOUND`). The third time, it's out of breath and stays: it takes the player's
  SODA POP if they have one, or else just follows them. The SEASHORE HOUSE owner, glad it's gone,
  says it can go with the player (YES/NO) and adds a SODA POP for its trouble.
- No daily reset: with no FLY yet, a spot that changed overnight could mean another boat ride for
  nothing.

## 2. Code and data

- **Flags** (`include/constants/flags.h`, `KANTO_EXTRA_FLAGS_START + 0x19F` on, under a
  "Johto starters" comment): `FLAG_GOT_JOHTO_CHIKORITA`, `FLAG_GOT_JOHTO_CYNDAQUIL`,
  `FLAG_GOT_JOHTO_TOTODILE`, `FLAG_HIDE_FLOWER_SHOP_JOHTO_VISITOR`, `FLAG_HIDE_DEWFORD_JOHTO_HIKER`,
  `FLAG_HIDE_GRANITE_CAVE_CYNDAQUIL`, `FLAG_HIDE_TOTODILE` (each map's transition keeps it), and the
  quest steps (berries asked, CYNDAQUIL rescued, ...). About 12 flags.
- **Vars** (`include/constants/vars.h`, after `VAR_HOENN_STORY_STATE`): `VAR_GRANITE_CAVE_ZUBAT_LEFT`,
  `VAR_TOTODILE_HIDING_SPOT`, `VAR_TOTODILE_TIMES_FOUND`. Two of them could be temp vars if each
  scene stays on one map; to be decided when writing them.
- **Objects**: three entries in `starter_object_events.h` (`STARTER_PIC_TABLE(gMonIcon_Chikorita)`,
  ...), `OBJ_EVENT_PAL_TAG_STARTER_ICON_2` for `gMonIconPalettes[2]`, the three
  `OBJ_EVENT_GFX_STARTER_*` constants in `include/constants/event_objects.h` and their graphics info,
  as CHARMANDER's were added. They share the special NPC palette slot, so no two on one map (none is).
- **Scripts**: the three quests appended to `data/scripts/starter_quests.inc` (its header comment
  grows a Hoenn section), with the maps' own events in their `scripts.inc` and new objects in their
  `map.json`: `Route104_PrettyPetalFlowerShop`, `DewfordTown_PokemonCenter_1F`, `GraniteCave_1F`,
  `Route109_SeashoreHouse`, `SlateportCity`, `Route109`, `DewfordTown`. Every gift with the same
  shape as the Kanto ones: `givemon`, the "sent to the PC" branch (it goes to a HOENN box while the
  link is cut, `docs/hoenn_plan.md` 3b), the nickname prompt.
- **J1 and J2**: `data/scripts/hall_of_fame.inc` sets `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` to 3
  instead of 1; `LittlerootTown_ProfessorBirchsLab`'s `CheckReadyForJohtoStarter` no longer moves to
  state 4. The lab's three ball scripts and texts stay in the file, unreachable, so Emerald's diffs
  stay small (or are removed, if unreachable code is unwanted).
- **Hoenn's shared-state check** (`tools/hoenn/check_shared_state.py`): nothing new to check: the
  flags are Hoenn's own and only Hoenn maps read them.
- **Docs**: `docs/catch_them_all.md` gets a "Johto starters" section like "Kanto starters", and
  `docs/hoenn_plan.md` a line in Act 4 or 5 pointing here.

## 3. Phases

1. The objects (J6) and the flags and vars; J1 and J2. Nothing visible changes but the dex scene
   and the lab's table.
2. CHIKORITA (the simplest: one map, no battles).
3. CYNDAQUIL (two maps, the BEEDRILL pattern).
4. TOTODILE (four places, the moving object).
5. The docs.

## 4. Tests

- A playthrough from BIRCH's lab to WATTSON with all three quests on the way, at Emerald's
  levels, to check that the three gifts aren't too strong for the first gyms.
- A test build per quest that starts a new game at its gate (the Test Area's ride to SLATEPORT with
  `VAR_HOENN_STORY_STATE` 3, then flags set by a debug script): each quest to the end, with a full
  party (the gift goes to HOENN 1), with NO at the offer, and a save and reload halfway.
- CYNDAQUIL: run from a ZUBAT, black out, catch one: the count stays right.
- TOTODILE: its spot after each find and after a reload; never shown on two maps.
- J1 and J2: after Hoenn's Hall of Fame, LITTLEROOT plays only ARCHIE and MAXIE's scene, and the
  lab's table never shows the balls, even with the Hoenn Dex complete.
- The text measured against the text box (205 pixels at most), as in phase 6 of
  `docs/hoenn_plan.md`.
