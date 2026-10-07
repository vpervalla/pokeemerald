# Plan: Mega Evolution

This is a plan for adding Mega Evolution (and optionally Primal Reversion) to this
project. The project is vanilla pokeemerald (Gen 3 battle engine, 386 species, 78
abilities, no Fairy type) with the Kanto port on top, not pokeemerald-expansion. So
every mechanic below has to be built by hand.

## 0. Decisions to make first

| # | Decision | Recommendation | Why it matters |
|---|----------|----------------|----------------|
| D1 | Which Megas | All 40 Megas of Gen 1–3 species (plus Primal Kyogre and Groudon as phase 6) | Gen 4+ Megas (Lucario, Garchomp, …) have no base species here |
| D2 | Add the Fairy type? | **No** for v1. Keep Gen 3 typings: Mega Altaria is Dragon/Flying, Mega Mawile is Steel, Mega Gardevoir is Psychic. Pixilate becomes "Normal moves get 1.3x and become Psychic", or Pixilate is swapped for an existing ability | A Fairy type touches the type chart, the type icons, the summary and pokédex UI, the Hidden Power math, contests and the move data. That is its own project |
| D3 | Turn-order rules | Gen 7: Mega Evolve at the start of the turn, and turn order uses the Mega's speed | Gen 6 used the pre-Mega speed. Gen 7 is simpler, because the order is computed after evolving |
| D4 | Link and recorded battles | Turn Megas off in `BATTLE_TYPE_LINK` and `BATTLE_TYPE_RECORDED*` for v1 | The action protocol and the Frontier records would otherwise need a new field. Re-enable in phase 7 |
| D5 | Key item gating | A new key item, `ITEM_MEGA_RING`, given by an NPC (for example after Steven, or in the Kanto post-game) | It decides where the story hook goes |
| D6 | New abilities | Add only the abilities the chosen Megas need (about 17). For a cheaper v1, give a Mega an existing ability where the real one is complex | Each new ability is battle-engine work |

## 1. Data: Mega species

The Mega species work like the Unown letter forms. They come after `NUM_SPECIES`, so
they never show up in the pokédex and never break the `NUM_SPECIES - 1` sized tables.

- `include/constants/species.h`: add `#define FORMS_MEGA_START (SPECIES_UNOWN_QMARK + 1)`
  and then `SPECIES_VENUSAUR_MEGA`, `SPECIES_CHARIZARD_MEGA_X`, … End with
  `#define NUM_SPECIES_WITH_FORMS`.
- Audit every table and bounds check that uses `NUM_SPECIES` or `SPECIES_UNOWN_QMARK`
  as its upper bound, for example `src/pokemon.c:4647` (`species > NUM_SPECIES`). Tables
  that are indexed by any battle species must grow to the new count:
  - `src/data/pokemon/species_info.h` (gSpeciesInfo: base stats, types, abilities,
    catch and exp data). Set `abilities[0] == abilities[1]` for a Mega.
  - `src/data/text/species_names.h`. A Mega shows its base name in battle, so the
    entry only has to exist.
  - Graphics tables in `src/data/pokemon_graphics/`: front, back, palette, shiny
    palette, `front_pic_coordinates.h`, `back_pic_coordinates.h`,
    `enemy_mon_elevation.h` (sized `[NUM_SPECIES]`, so it must grow), `front_pic_anims.h`.
  - Icons: `gMonIconTable` and `gMonIconPaletteIndices` (the party menu shows the base
    form, so the Mega entries can point at the base icon).
  - `sMonFrontAnimIdsTable` and `sMonAnimationDelayTable` in `src/pokemon.c` are sized
    `NUM_SPECIES - 1`. Either grow them or guard the lookups.
- New table `src/data/pokemon/mega_evolutions.h`:
  ```c
  struct MegaEvolution { u16 baseSpecies; u16 item; u16 megaSpecies; };
  ```
  Add lookup helpers in `src/pokemon.c`: `GetMegaSpecies(species, heldItem)` and
  `IsMegaSpecies(species)`. Mega Rayquaza is keyed on "knows Dragon Ascent" instead of
  a held item. Without Dragon Ascent, key it on knowing a chosen existing move or on a
  flag.

## 2. Data: Mega Stones and the Mega Ring

- `include/constants/items.h`: there is no room in the middle of the enum (the
  `ITEM_034` style placeholders are save-compatible but scattered). Add the stones
  after `ITEM_OLD_SEA_MAP` and before `ITEMS_COUNT`. Item IDs are u16, so space is not
  a problem. Check `LAST_BERRY_INDEX`, the bag pocket sizes (`BAG_ITEMS_COUNT`) and the
  save layout.
- `src/data/items.h`: name, price, description, `POCKET_ITEMS`, and a new hold effect
  `HOLD_EFFECT_MEGA_STONE` in `include/constants/hold_effects.h` (after
  `HOLD_EFFECT_SHELL_BELL`).
- Item icons and palettes go in `graphics/items/` and `src/data/item_icon_table.h`.
- Mega Stones can't be taken or swapped in battle: Trick, Thief, Covet and Knock Off
  (which in Gen 3 removes the item until the battle ends) must fail. Add this to the
  item-steal checks in `src/battle_script_commands.c`.
- `ITEM_MEGA_RING` is a key item with no field use.

## 3. Battle state

- In `struct BattleStruct` (`include/battle.h`), add:
  - `u8 megaUsedBySide[2]` or per trainer, as a bitfield over `GetBattlerPosition`. In
    double battles against two trainers, or in multi battles, each trainer gets one Mega.
  - `u8 toMegaEvolve`, a bitfield over battlers that queued a Mega this turn.
  - `u16 megaBaseSpecies[MAX_BATTLERS_COUNT]`, to revert, and for the illusion and
    transform edge cases.
- Clear all of it in `BattleStartClearSetData` and per turn where needed.
- Because `gBattleMons[b].species` is the only thing that changes, the party struct
  keeps the base species. Make sure nothing writes `REQUEST_SPECIES_BATTLE` or
  `REQUEST_ALL_BATTLE` back to the party with the Mega species. Check the
  `EmitSetMonData` callers and `src/battle_controller_*`. Also apply the "revert at
  battle end" rule explicitly for safety.

## 4. Player input (move menu)

- `src/battle_controller_player.c`, `HandleInputChooseMove`: A, B and SELECT (swap
  moves) are already taken, so **START** toggles the Mega trigger. This only works when:
  the player has the Mega Ring flag, the battler holds a matching stone, the side
  hasn't used a Mega yet, and the battle type allows it (D4).
- UI: a small "trigger" sprite next to the move window (a new graphic in
  `graphics/battle_interface/`), which animates or changes palette when it is on. It
  is created in `InitMoveSelectionsVarsAndStrings`, destroyed on leaving the move menu,
  and refreshed per battler in doubles.
- Return the choice to the engine. The move-choice reply is
  `BtlController_EmitTwoReturnValues(B_COMM_TO_ENGINE, B_ACTION_EXEC_SCRIPT, moveIndex | (target << 8))`.
  Put a flag in the spare bit of the move-index byte (`moveIndex | RET_MEGA_EVOLUTION`) and
  decode it in `HandleTurnActionSelectionState` (`src/battle_main.c`) into
  `gBattleStruct->toMegaEvolve`.
- In doubles, if the first battler queued a Mega, the second battler's trigger is
  disabled.

## 5. Engine: performing the Mega Evolution

- **When:** in `src/battle_main.c`, add a pass right after the turn order is set up
  (`SetActionsAndBattlersTurnOrder`, before `RunTurnActionsFunctions`). For each
  battler with the bit set in `toMegaEvolve` (in speed order), run
  `BattleScript_MegaEvolution`. Then **recompute the turn order** (D3).
- **What:** a new battle script command, `handlemegaevo` (see `data/battle_scripts_1.s`,
  `asm/macros/battle_script.inc`, and the command table in
  `src/battle_script_commands.c`), with three stages:
  1. Message: "{TRAINER}'s {ITEM} is reacting to {MON}'s {STONE}!". New strings go in
     `src/battle_message.c` and `include/constants/battle_string_ids.h`.
  2. Animation: reuse the Castform path. `BattleScript_DoCastformChange` uses
     `playanimation B_ANIM_CASTFORM_CHANGE` and swaps the sprite via
     `HandleSpeciesGfxDataChange` (`src/battle_gfx_sfx_util.c`). Add `B_ANIM_MEGA_EVOLUTION` with a simple
     rainbow-orb effect, built from existing particle tasks.
  3. Data: set `gBattleMons[b].species = megaSpecies`. Recompute
     Atk/Def/Spe/SpA/SpD from the party mon's IVs, EVs and nature with the Mega base
     stats. Keep HP and max HP, as Megas never change base HP. Set the types and the
     ability. Copy the stat-calc pattern from `CalculateMonStats`, or factor out a
     helper that takes a species override.
- After the ability changes, call `AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, …)`
  so that Drought, Sand Stream, Intimidate and the rest activate. Also re-run the
  `ABILITYEFFECT_INTIMIDATE*` and `ABILITYEFFECT_TRACE` checks.
- Set the side or trainer's `megaUsed` bit.
- Health box: mark the Mega with an indicator icon in `src/battle_interface.c` (it is
  optional and can come later).
- **Interactions to handle:**
  - Transform or Imposter on a Mega copies the Mega species, which is fine. A
    transformed mon can't Mega Evolve.
  - Baton Pass, switching and fainting: the mon stays a Mega for the rest of the
    battle. On switch-in, `gBattleMons` is reloaded from the party (`CopyPlayerPartyMonToBattleData`,
    `src/pokemon.c:4683`), which would revert it. Re-apply the Mega form on switch-in
    if the battler's party slot is in a `megaEvolvedPartySlots` bitfield.
  - Skill Swap and Role Play must not take or overwrite ability-dependent
    "permanent" Mega abilities. In Gen 6 they still worked on most Megas. Decide
    per ability.
  - `reshow_battle_screen.c` (after the bag or party menu) must reload the Mega sprite.
    It already handles `transformSpecies` and Castform, so follow that.
  - The Deoxys stat hack in `src/pokemon.c` (around 2713) and in
    `src/battle_util.c:3903` shows other places where species-based stats are
    re-derived. Audit them.
- **Revert:** at the end of the battle, nothing needs to happen if the party species
  was never written. Also check the post-battle evolution scene and the pokédex
  "seen" update (`HandleSetPokedexFlag`) so they use the base species, not
  `gBattleMons[].species`.

## 6. Abilities

New `ABILITY_*` constants go after `ABILITY_AIR_LOCK` (77). The ability is a u8, so
there is room up to 255. They also need names and descriptions in `src/data/text/abilities.h`.
These are the abilities the Gen 1–3 Megas need, ordered from cheapest to most work:

- **Damage modifiers** in `CalculateBaseDamage` (`src/pokemon.c`) and in the attacker
  checks: Tough Claws (contact moves get 1.3x; this needs a "makes contact" flag, and
  Gen 3 has `FLAG_MAKES_CONTACT`), Strong Jaw (biting moves, which need a new move
  flag), Mega Launcher (pulse moves, a new flag), Technician, Sand Force, Sheer Force
  (also suppresses secondary effects in `seteffectwithchance`), Adaptability, Solar Power.
- **-ate abilities**: Aerilate and Refrigerate, plus Pixilate if D2 changes. These
  change a Normal-type move's type and give 1.3x power. Add a `dynamicMoveType`
  override in `src/battle_script_commands.c` where `gBattleStruct->dynamicMoveType` is set.
- **Accuracy and flinch**: No Guard (in `accuracycheck`), Steadfast, Skill Link (in the
  multi-hit setup).
- **Filter** (super-effective damage x0.75), **Mold Breaker** (ignore the target's
  defensive abilities, which means a guard on every ability check against the target;
  this is the most invasive one).
- **Magic Bounce** reflects status moves. It is similar to Magic Coat, so reuse
  `BattleScript_MagicCoatBounce`.
- **Prankster** gives +1 priority to status moves, in `GetWhoStrikesFirst`.
- **Parental Bond** (Kangaskhan) makes a second hit at 0.5x (0.25x in Gen 6). This is
  a lot of script work, so put it last. A cheap alternative is Scrappy or Huge Power.
- **Primal or weather abilities** (phase 6): Primordial Sea, Desolate Land and Delta
  Stream need new weather states in `gBattleWeather`.

## 7. Trainers and AI

- Opponent trainers: in `src/data/trainer_parties.h`, give a Mega Stone to selected
  mons (Steven, the Champion, the Kanto rematches, the Frontier Brains). Opponents
  don't need a Mega Ring check. Use a trainer-level flag in `struct Trainer`, or just
  "holding a stone is enough".
- `src/battle_controller_opponent.c`, `OpponentHandleChooseMove`: set
  `RET_MEGA_EVOLUTION` whenever the battler can Mega Evolve. That is always the right
  play.
- The partner AI (`battle_controller_player_partner.c`) does the same.
- AI scripts (`data/battle_ai_scripts.s`) work as before. Damage scoring reads
  `gBattleMons`, so it sees the Mega stats after the evolution.

## 8. Phases and order of work

1. **Skeleton**: species slots, one Mega (Mega Blaziken: Speed Boost already exists,
   and the type doesn't change), one stone, the START toggle, the engine pass, a
   placeholder animation. Test it in a wild battle.
2. **Correctness pass**: switch-out and switch-in, Baton Pass, Transform, the bag
   reshow, doubles (one per trainer), turn-order recompute, the end-of-battle party
   integrity check.
3. **Content**: the remaining Megas whose abilities already exist (Venusaur, Charizard Y,
   Alakazam, Slowbro, Gengar, Mewtwo Y, Tyranitar, Sceptile, Swampert, Mawile,
   Medicham, Manectric, Latias, Latios). This is data and graphics only.
4. **New abilities**, in the order listed in section 6, unlocking the matching Megas.
5. **Integration**: the Mega Ring story event, stone placement on the Hoenn and Kanto
   maps and in shops, trainer parties, the opponent and partner AI.
6. **Optional**: Primal Kyogre and Groudon (auto-trigger on switch-in when holding
   the Blue or Red Orb), Mega Rayquaza, Delta Stream.
7. **Optional**: link and recorded battle support. Bump the link protocol and encode
   the flag in the recorded action stream. The Battle Frontier rules could also ban
   Megas.

### Phase 1 status: done

What exists now:

- `SPECIES_BLAZIKEN_MEGA` (after the Unown letters, `FORMS_MEGA_START`), with Gen 6 base
  stats and Speed Boost. Its sprite is Blaziken's with a recoloured **placeholder**
  palette (`graphics/pokemon/blaziken/mega/`).
- `ITEM_BLAZIKENITE` (hold effect `HOLD_EFFECT_MEGA_STONE`) and the key item
  `ITEM_MEGA_RING`, both with placeholder icons. Nothing in the game gives them out yet.
- The Mega table in `src/data/pokemon/mega_evolutions.h`, with the lookups
  `GetMegaEvolutionSpecies` and `GetMegaBaseSpecies` in `src/pokemon.c`.
- In the move menu, START toggles a trigger icon next to the healthbox (placeholder
  graphic in `graphics/battle_interface/mega_trigger*.{png,pal}`). It only shows when
  `CanMegaEvolve` (`src/battle_util.c`) allows it: the player has the Mega Ring, the
  Pokémon holds its stone, and its trainer hasn't Mega Evolved yet this battle.
- `TryDoMegaEvolutions` (`src/battle_main.c`) runs `BattleScript_MegaEvolution`
  for each battler that chose to, after everyone has picked an action and **before**
  the turn order is set. That is simpler than recomputing the order and gives the same
  Gen 7 result. The new `handlemegaevolution` command updates the battle data, plays
  `B_ANIM_MEGA_EVOLUTION` and then runs switch-in abilities.
- Sprite reloads (the bag, the party menu, a substitute fading) keep the Mega sprite.

Tested in mGBA with a throwaway boot-to-battle hook:
- The Mega Evolution happens, and its stats, ability and turn order are right.
- Speed Boost activates.
- There is no trigger without the Mega Ring, or on later turns.
- In doubles, only one battler per trainer can Mega Evolve.
- The Mega sprite survives the bag reshow.
- Switching out and back in doesn't break anything.

### Phase 2 status: done

- **Switching:** a Mega that switches out comes back in Mega Evolved.
  `gBattleStruct->megaEvolvedPartySlots` remembers it per party slot,
  `TryRestoreMegaEvolution` re-applies it in `Cmd_switchindataupdate`, and the sprite
  loaders read it too. A Mega that faints loses it (`ClearMegaEvolutionOnFaint` in
  `Cmd_cleareffectsonfaint`), so a revived Pokémon comes back in base form.
- **Speed order:** when several Pokémon Mega Evolve in one turn, the fastest goes first
  (`TryDoMegaEvolutions` uses `GetWhoStrikesFirst`).
- **Transform:** transforming into a Mega copies its sprite as well as its data.
- **Mega Stones:** a stone that its holder can use (`IsMegaStoneUsableBy`) can't be
  stolen with Thief or Covet, knocked off with Knock Off, or swapped with Trick. Trick
  also fails if either item is a stone that the other Pokémon could use.
- **Baton Pass and the bag:** nothing special was needed; both keep working.

Tested in mGBA, with the same throwaway hook plus a test-only opponent that Mega Evolves:
- Switching out and back in.
- A wild Ditto transforming into Mega Blaziken.
- Knock Off, Trick and Thief against a stone.
- A faster opponent Mega Evolving before the player in the same turn.
- The phase 1 cases again.

The faint-then-revive path was not exercised in the emulator.

Remaining gaps:
- An Encored Pokémon skips the move menu, so it can't Mega Evolve. Fixing that needs a
  way to show the trigger without the move menu.
- Opponents never Mega Evolve (phase 5).
- Link, recorded and Battle Frontier battles have Megas turned off (`CanMegaEvolve`).
- The Mega's exp yield stays 209, because Gen 6's 284 doesn't fit in a u8.
- Wild Pokémon can't hold Mega Stones in normal play, because vanilla
  `SetWildMonHeldItem` replaces a wild Pokémon's item with its species' `itemCommon`
  or `itemRare`.

### Phase 3 status: done

- **New Megas:** 14, plus real art for Mega Blaziken: Venusaur, Charizard Y, Alakazam,
  Slowbro, Gengar, Mewtwo Y, Tyranitar, Sceptile, Swampert, Mawile, Medicham, Manectric,
  Latias and Latios. Each has Gen 6/7 base stats, its Mega ability and its Mega Stone.
  Mega Mawile stays pure Steel (D2).
- **Sprites:** the Gen 3 style sprites and stone and Mega Ring icons come from
  pokeemerald-expansion; see `docs/mega_evolution_credits.md`.
- **Import tool:** `tools/mega_evolutions/import_megas.py` copies the assets and
  generates all the data from one table (`MEGAS`). Adding a phase 4 Mega is a new row
  and a re-run. Its generated code sits between `<mega-evolutions>` markers, and
  re-running it changes nothing.
- **Sprite positions:** sprite offsets and elevation in battle now use the Mega's own
  values (`GetBattlerPartySpriteSpecies`), not the base form's.
- **Switch-in abilities:** Mega Evolution now also triggers Intimidate and Trace, not
  just weather abilities.
- **Stone name:** item names fit 13 characters, so Charizardite Y is spelled
  "CHARZARDITE Y".
- **Palette tags:** a static assert keeps the species ids below `SPECIES_SHINY_TAG`
  (500), which the palette tags rely on. The Megas so far reach 454.

Tested in mGBA, for all 15 Megas, as the player's Pokémon and as a (test-only) Mega
Evolving opponent:
- Species, stats and abilities are right.
- Front and back sprites look right, and Mega Latias and Mega Latios float with
  shadows.
- Mega Charizard Y's Drought, Mega Manectric's Intimidate (Attack −1) and Mega
  Alakazam's Trace activate.
- Mega Tyranitar shows no Sand Stream message, because base Tyranitar's Sand Stream
  already started the sandstorm.
- The phase 1 and 2 scenarios still pass.

## 9. Testing

- `make` and `make modern` must both build. Watch the ROM size, because each Mega adds
  about 6–8 KB of graphics. Check the `ld_script.ld` free space.
- There is no automated test harness in vanilla pokeemerald. Check the behavior in
  mGBA with a debug script that gives a stone and the ring and starts a wild battle,
  and use save states for each interaction in phase 2.
- Regression: no change to `rom.sha1` is expected to hold any more, so check the
  vanilla battles (the Castform, Transform and Deoxys paths) by hand.

## Assets needed (outside the code)

For each Mega: front sprite, back sprite, normal and shiny palettes (64×64, 16 colours,
in the Gen 3 style), the stone icons, the Mega trigger graphic and the health-box
indicator. The sprites and icons come from pokeemerald-expansion (see
`docs/mega_evolution_credits.md`), which has Gen 3 style art for every Mega. The Mega
trigger is still a placeholder.
