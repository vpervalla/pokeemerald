# Plan: the Johto starters in Hoenn

This is a plan for making CHIKORITA, CYNDAQUIL and TOTODILE gifts from three side quests in Hoenn,
the way the Kanto starters are gifts from three side quests in Kanto (`docs/catch_them_all.md`,
"Kanto starters"). It is implemented: see "Status" at the end.

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
| TOTODILE | ROUTE 109's SEASHORE HOUSE, then ROUTE 109's, SLATEPORT's and DEWFORD's beaches | MR. BRINEY's boat to SLATEPORT (`FLAG_DELIVERED_STEVEN_LETTER`) | 15 |

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
  CYNDAQUIL is cornered by wild ZUBAT. Talking to it or to a ZUBAT starts the scene: two wild battles
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
- TOTODILE hides on three beaches, one after the other, all reachable on foot or by MR. BRINEY's
  boat (no SURF): ROUTE 109's (28, 19), where BRINEY docks, then SLATEPORT's (25, 55), then DEWFORD's
  (18, 10) (BRINEY sails between ROUTE 109 and DEWFORD). Each beach has its own TOTODILE object, and
  each map's transition shows it only when `VAR_TOTODILE_TIMES_FOUND` is that beach's number.
- Each time the player finds it, it hops, dives and swims to the next beach, and the text says
  where. The third time, on DEWFORD's beach, it's out of breath and stays: it asks to come along
  (YES/NO, the offer stays open), and drinks a SODA POP if the player has one. The SEASHORE HOUSE
  owner, glad it's gone, gives a SODA POP the next time the player talks to him.
- No daily reset: with no FLY yet, a spot that changed overnight could mean another boat ride for
  nothing.

## 2. Code and data

- **Flags** (`include/constants/flags.h`, `KANTO_EXTRA_FLAGS_START + 0x19F`-`0x1A9`): `FLAG_GOT_JOHTO_*`
  for each gift, `FLAG_FED_FLOWER_SHOP_CHIKORITA`, `FLAG_DEFEATED_GRANITE_CAVE_ZUBAT_1`/`_2` and
  `FLAG_RESCUED_GRANITE_CAVE_CYNDAQUIL` (which also hide their objects), `FLAG_HIDE_DEWFORD_CENTER_CYNDAQUIL`
  and `FLAG_HIDE_TOTODILE` (kept by the maps' transitions), `FLAG_HEARD_ABOUT_TOTODILE` and
  `FLAG_GOT_SODA_POP_FOR_TOTODILE`.
- **Var** (`include/constants/vars.h`): `VAR_TOTODILE_TIMES_FOUND`, `KANTO_VARS_START + 0x72`, which is
  also the beach TOTODILE hides on.
- **Objects** (`src/data/object_events/starter_object_events.h`): `OBJ_EVENT_GFX_STARTER_CHIKORITA`,
  `_CYNDAQUIL`, `_TOTODILE` and `OBJ_EVENT_GFX_ICON_ZUBAT`, from their box icons, in a new range after
  the Hoenn legends' (`OBJ_EVENT_GFX_JOHTO_STARTER_START`, so no older graphics id moves). TOTODILE and
  ZUBAT use icon palette 2 (`OBJ_EVENT_PAL_TAG_STARTER_ICON_2`). The ZUBAT are on screen with
  CYNDAQUIL, so they take the special NPC's reflection slot, as the Primal legends do in SOOTOPOLIS.
- **Scripts**: the shared parts in `data/scripts/starter_quests.inc` (`JohtoStarters_EventScript_ReceivedMon`
  for the fanfare, nickname and PC message; TOTODILE, on all three beaches), the rest in the maps:
  `Route104_PrettyPetalFlowerShop`, `DewfordTown_PokemonCenter_1F`, `GraniteCave_1F`,
  `Route109_SeashoreHouse`, and the transitions of `Route109` (new), `SlateportCity` and `DewfordTown`.
- **J1 and J2**: `data/scripts/hall_of_fame.inc` sets `VAR_DEX_UPGRADE_JOHTO_STARTER_STATE` to 3 instead
  of 1, with what the skipped scene would have set (BIRCH and the rival hidden in LITTLEROOT, SCOTT's
  Battle Frontier call). `LittlerootTown_ProfessorBirchsLab`'s `CheckReadyForJohtoStarter` no longer
  moves to state 4. The lab's three ball scripts stay in the file, unreachable.

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

## Status

Implemented as above. Checked in mGBA with test builds that start a new game at each quest:

- CHIKORITA: the three BERRIES taken, CHIKORITA given at level 5, the shop drawn with the visitor and
  CHIKORITA.
- CYNDAQUIL: both ZUBAT battles in GRANITE CAVE 1F, CYNDAQUIL's flame and its leaving; the HIKER's
  gift at level 10, with CYNDAQUIL beside him.
- TOTODILE: the first find on ROUTE 109 (it dives away), TOTODILE shown on SLATEPORT's beach after it,
  the last find on DEWFORD's beach (the SODA POP, the gift at level 15), and the SEASHORE HOUSE
  owner's SODA POP.
- The new text measured against the text box.

Not played: the Hall of Fame without the National Dex scene (J1, J2), running or blacking out against
a ZUBAT, a gift with no room, the YES/NO refusals, and the playthrough to WATTSON.
