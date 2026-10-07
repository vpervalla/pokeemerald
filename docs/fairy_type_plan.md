# Plan: Fairy type

This is a plan for adding the Fairy type to this project, with the Gen 6 type chart
(including Steel's Gen 6 resistances) and a set of new Fairy moves. Nothing here is
implemented yet.

## 0. Decisions

| # | Decision | Choice |
|---|----------|--------|
| D1 | Type ID | `TYPE_FAIRY = 18`, appended after Dark. Existing type IDs keep their values |
| D2 | Physical or special | Special. This engine splits by type (`include/battle.h:484`: types above `TYPE_MYSTERY` are special), so every Fairy move is special, Play Rough included |
| D3 | Steel resistances | Gen 6: Steel no longer resists Ghost or Dark |
| D4 | New moves | Yes, see section 7 |
| D5 | Hidden Power | Stays on the original 16 types; it can never be Fairy, as in Gen 6+ |
| D6 | Type-boosting item (Pixie Plate) | Not in this project. Can be added later |

Note on D2: Mawile, Azumarill, Granbull and Mega Mawile are physical attackers, so
their Fairy STAB moves will use Sp. Atk. That is the accepted cost of the type split.

## 1. Constant

`include/constants/pokemon.h`:

```c
#define TYPE_DARK             17
#define TYPE_FAIRY            18
#define NUMBER_OF_MON_TYPES   19
```

These follow on their own: `MENU_INFO_ICON_*` in `include/menu.h:17-23` and the
contest-category offsets in `src/pokemon_summary_screen.c` (`NUMBER_OF_MON_TYPES + x`).

## 2. Type chart

`src/battle_main.c:337`, `gTypeEffectiveness`.

- Change the size from `[336]` to `[]` (or the new count) so it can't drift.
- **Remove** (Gen 6 Steel, D3):
  - `TYPE_GHOST, TYPE_STEEL, TYPE_MUL_NOT_EFFECTIVE` (line 432)
  - `TYPE_DARK, TYPE_STEEL, TYPE_MUL_NOT_EFFECTIVE` (line 440)
- **Add**, before the `TYPE_FORESIGHT, TYPE_FORESIGHT` separator (rows after it are
  the Ghost immunities that Foresight ignores):

| Attacker | Defender | Multiplier |
|---|---|---|
| Fairy | Fighting, Dragon, Dark | `TYPE_MUL_SUPER_EFFECTIVE` |
| Fairy | Fire, Poison, Steel | `TYPE_MUL_NOT_EFFECTIVE` |
| Poison, Steel | Fairy | `TYPE_MUL_SUPER_EFFECTIVE` |
| Fighting, Bug, Dark | Fairy | `TYPE_MUL_NOT_EFFECTIVE` |
| Dragon | Fairy | `TYPE_MUL_NO_EFFECT` |

Net change: +12 rows, -2 rows, so 112 to 122 rows (366 bytes).

## 3. Battle mechanics that read the type list

- **Hidden Power** (`src/battle_script_commands.c:9082`): the formula uses
  `NUMBER_OF_MON_TYPES - 3`. Replace it with a fixed 15 (the 16 old types minus
  Normal) so the result never reaches Fairy and existing Hidden Powers keep their type.
- **Conversion 2** (`battle_script_commands.c:8267`): `Random() % 128 >
  sizeof(gTypeEffectiveness) / 3` still fits at 122 rows, but replace it with the
  real row count and `>=` (the current `>` is off by one). The fallback loop below it
  also mixes up `i`, `j` and `rands`; fix it while there.
- **Conversion** (`battle_script_commands.c:7577`): no change needed. It only special-
  cases `TYPE_MYSTERY` (Curse).
- **Battle Factory**:
  - `src/battle_factory.c:606` already loops over `NUMBER_OF_MON_TYPES`.
  - `data/maps/BattleFrontier_BattleFactoryPreBattleRoom/scripts.inc:207-223`: add a
    `call_if_eq VAR_0x8005, TYPE_FAIRY, ..._OpponentUsesFairy` branch and its text.
- **AI** (`data/battle_ai_scripts.s`, the special-type lists at lines 1183, 1393 and
  2181): add `.byte TYPE_FAIRY`.
- **Battle Dome** (`src/battle_dome.c`): reads `gTypeEffectiveness` directly; check
  it still works with the new rows.

## 4. Text and UI tables

| File | Change |
|---|---|
| `src/battle_main.c:464` `gTypeNames` | `[TYPE_FAIRY] = _("FAIRY")` |
| `src/battle_message.c:1346` `sATypeMove_Table` | `[TYPE_FAIRY] = _("a FAIRY move")` |
| `src/pokedex.c:1379` `sDexSearchTypeOptions` | add `gTypeNames[TYPE_FAIRY]` before the terminator |
| `src/pokedex.c:1413` `sDexSearchTypeIds` | add `TYPE_FAIRY` |
| `src/data/union_room.h:870` `sTradingBoardTypes` | add a Fairy row before Exit |
| `src/pokemon_summary_screen.c:810-931` | add an `ANIMCMD_FRAME(TYPE_FAIRY * 8, ...)` anim and an entry in `sSpriteAnimTable_MoveTypes` and `sMoveTypeToOamPaletteNum` |
| `src/menu.c:113` `sMenuInfoIcons` | `[TYPE_FAIRY + 1] = { 32, 12, <offset> }` |

## 5. Graphics

- **Type icon**: new `graphics/types/fairy.png`, 32x16, same format as `dark.png`.
  Add `fairy` after `dark` in `types :=` in `graphics_file_rules.mk:16`. The list
  order must match the type IDs, because `move_types.4bpp` is these files
  concatenated.
- **Palette**: the icons share `move_types_{1,2,3}.pal` (OAM palette slots 13-15).
  Fit the Fairy pink into one of them if there are free colours; otherwise add a
  fourth palette, which needs a free OAM palette slot on the summary screen.
- **Move-info icon**: `graphics/interface/menu_info.png` (128x128) needs a free 32x12
  spot for the Fairy label used by the move-info windows, or the sheet has to grow.

## 6. Retyping existing data

**Moves** (`src/data/battle_moves.h`): Charm, Sweet Kiss and Moonlight change from
Normal to Fairy.

**Species** (`src/data/pokemon/species_info.h`), Gen 6 typings:

| Species | New types |
|---|---|
| Cleffa, Clefairy, Clefable | Fairy |
| Togepi | Fairy |
| Togetic | Fairy / Flying |
| Snubbull, Granbull | Fairy |
| Igglybuff, Jigglypuff, Wigglytuff | Normal / Fairy |
| Azurill | Normal / Fairy |
| Marill, Azumarill | Water / Fairy |
| Mr. Mime | Psychic / Fairy |
| Ralts, Kirlia, Gardevoir | Psychic / Fairy |
| Mawile, `SPECIES_MAWILE_MEGA` | Steel / Fairy |

A Pokémon's type isn't stored in the save (it's read from `gSpeciesInfo`), so
existing saves keep working.

## 7. New Fairy moves

All are Fairy type and therefore special (D2). IDs follow `MOVE_DRAGON_ASCENT` (355),
so `MOVES_COUNT` goes from 356 to 363. Names fit `MOVE_NAME_LENGTH` (12).

| ID | Constant | Name | Power | Acc | PP | Effect | Target | Notes |
|---|---|---|---|---|---|---|---|---|
| 356 | `MOVE_FAIRY_WIND` | FAIRY WIND | 40 | 100 | 30 | `EFFECT_HIT` | selected | |
| 357 | `MOVE_DISARMING_VOICE` | DISARM VOICE | 40 | – | 15 | `EFFECT_ALWAYS_HIT` | both foes | sound move (Soundproof blocks it) |
| 358 | `MOVE_DRAINING_KISS` | DRAINING KISS | 50 | 100 | 10 | drain 75% | selected | contact. New effect, or reuse `EFFECT_ABSORB` (50%) for v1 |
| 359 | `MOVE_DAZZLING_GLEAM` | DAZZLINGLEAM | 80 | 100 | 10 | `EFFECT_HIT` | both foes | |
| 360 | `MOVE_MOONBLAST` | MOONBLAST | 95 | 100 | 15 | `EFFECT_SPECIAL_ATTACK_DOWN_HIT`, 30% | selected | |
| 361 | `MOVE_PLAY_ROUGH` | PLAY ROUGH | 90 | 90 | 10 | `EFFECT_ATTACK_DOWN_HIT`, 10% | selected | contact; special here (D2) |
| 362 | `MOVE_BABY_DOLL_EYES` | BABYDOLLEYES | – | 100 | 30 | `EFFECT_ATTACK_DOWN` | selected | priority +1, status move |

"DRAINING KISS" is 13 characters, so use "DRAININGKISS".

Gen 6 Fairy status moves that need new engine work (Misty Terrain, Crafty Shield,
Flower Shield, Fairy Lock, Aromatic Mist, Geomancy) are out of scope.

### Files to touch per move

Follow commit `f01cd799` (Dragon Ascent), which added a later-generation move:

- `include/constants/moves.h`: the constant, and bump `MOVES_COUNT`.
- `src/data/battle_moves.h`: the `gBattleMoves` entry (effect, power, type, accuracy,
  PP, secondary chance, target, priority, flags such as `FLAG_MAKES_CONTACT`,
  `FLAG_PROTECT_AFFECTED`, `FLAG_MIRROR_MOVE_AFFECTED`, `FLAG_KINGS_ROCK_AFFECTED`).
- `src/data/text/move_names.h` and `src/data/text/move_descriptions.h` (indexed
  `[MOVE_X - 1]`).
- `src/data/contest_moves.h`: contest category and effect (Cute for most).
- `data/battle_anim_scripts.s`: add a `.4byte Move_X` entry to the table and a script.
  Reuse existing animations to start (for example Sweet Kiss / Moonlight sparkles for
  Moonblast and Dazzling Gleam, Gust for Fairy Wind, Absorb for Draining Kiss, Charm
  for Baby-Doll Eyes).
- Sound-move lists: add Disarming Voice wherever Soundproof checks its list
  (`sSoundMovesTable`, `src/battle_util.c:693`).
- Draining Kiss at 75%: a new `EFFECT_DRAINING_KISS` (or a drain-percent field) in
  `include/constants/battle_move_effects.h`, `data/battle_scripts_1.s` and the absorb
  heal command in `src/battle_script_commands.c`.
- Check other tables sized by `MOVES_COUNT`: easy chat move groups, bard music,
  apprentice data, `src/battle_arena.c`, `src/battle_dome.c`, `src/party_menu.c`,
  `src/pokemon_storage_system.c`. Dragon Ascent skipped some of these; do the same
  unless a table is indexed by move ID without a bounds check.

### Who learns them (level-up, `src/data/pokemon/level_up_learnsets.h`)

Use Gen 6 (ORAS) levels at implementation time. Level-up moves can also be taught by
the Move Relearner.

| Move | Learners |
|---|---|
| Fairy Wind | Togepi line, Mawile |
| Disarming Voice | Clefairy line, Jigglypuff line, Ralts line, Togepi line |
| Draining Kiss | Ralts line, Snubbull line, Togepi line |
| Dazzling Gleam | Ralts line (Gardevoir), Togetic |
| Moonblast | Clefairy line, Gardevoir |
| Play Rough | Snubbull line, Marill line, Mawile, Jigglypuff line |
| Baby-Doll Eyes | Snubbull line, Mawile, Azurill |

TM and tutor tables are bitfields with no free slots for new moves; leave them alone.

## 8. Follow-ups this unlocks

- `docs/mega_evolution_plan.md` D2: Mega Gardevoir (Psychic/Fairy) and Mega Altaria
  (Dragon/Fairy) can keep their real typings, and Pixilate can be the real ability
  (Normal moves become Fairy with 1.3x). Mega Mawile becomes Steel/Fairy here.
- Pixie Plate (D6).
- Gen 6 base-stat changes (for example Clefable, Wigglytuff).

## 9. Order of work

1. Sections 1-2 (constant and type chart), then build.
2. Section 3 (mechanics), then build.
3. Sections 4-5 (UI and graphics), then build and check every type screen.
4. Section 6 (retyping).
5. Section 7 (moves), one move at a time, building after each.
6. Testing (section 10).

## 10. Testing

- `make` builds and `make compare` is expected to fail (the ROM changes on purpose).
- Battle checks:
  - Dragon move into Clefairy: "It doesn't affect…".
  - Fairy move into a Fighting, Dragon or Dark Pokémon: super effective.
  - Poison or Steel move into a Fairy Pokémon: super effective.
  - Shadow Ball or Crunch into Steelix: neutral (Gen 6 Steel).
  - Charm and every new move show FAIRY and do special damage.
  - Disarming Voice never misses and is blocked by Soundproof; Draining Kiss heals
    the right amount; Moonblast and Play Rough lower stats at their chances;
    Baby-Doll Eyes goes first.
- Screens: summary type icon and palette, move-info window icon, Pokédex Fairy
  search, Union Room trading board, Battle Factory opponent-type hint.
- An existing save loads, and its Hidden Power types are unchanged.
- Linking with unmodified Emerald won't match; that was already true because of the
  Mega Evolution changes.
