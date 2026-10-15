# Plan: a Pokémon card game ("Poké Duel")

This is a plan for adding a small, self-contained card game to the project, in the spirit of
Gwent in The Witcher 3: a two-player, best-of-three duel that NPCs all over Hoenn and Kanto
will play with you, with a deck you build from cards you collect while adventuring. "POKé
DUEL" is a working name; it should live in one string so it can change in one place.

The game runs on its own screen (like the slot machine or roulette), launched from an
event script with a `special`, and returns to the field when it ends. It does not touch
the battle engine.

## 0. Decisions to make first

| # | Decision | Recommendation | Why it matters |
|---|----------|----------------|----------------|
| D1 | Rules model | Gwent's: three rows, best of three rounds, no card draw between rounds except through card effects. Simpler than the real TCG (no energy, no HP) | It fits one screen and a short session, and the AI stays tractable |
| D2 | Where cards come from | Each species the player has **caught** unlocks its card (read from the pokédex "owned" flags), plus ~30 special cards (items, weather, trainers) won from opponents or bought | Ties the game to "catch them all" and costs no save space for 386 cards |
| D3 | Card stats | Generated from base stats by a host-side script into a table, then hand-tuned for the ~60 cards in the v1 set | 386 hand-written cards is weeks of balancing; a formula gets 90% of the way |
| D4 | Art | Reuse the 32x32 party icons on the board and the 64x64 front pic in the card detail view. Only new art: card frames, row backgrounds, row/ability icons | Icons already share 3 palettes (`gMonIconPaletteIndices`), which keeps VRAM simple |
| D5 | Stakes | No money or coin bets in v1; win = the opponent's reward card (once) | Gwent's "win their unique card" loop is the fun part, and betting adds UI |
| D6 | Link play | Out of scope for v1. Keep the game state in one struct with a deterministic RNG so a link mode can be added later | Link is the most expensive and least used part of every minigame here |
| D7 | Where it unlocks | Lilycove Department Store sells a starter deck after the 4th badge; a "Card Club" in Mauville (Hoenn) and Celadon (Kanto); a card tournament once a day on Mega Island | Gives the hook a home in both regions |

## 1. Rules

### Board and flow

- Each side has three rows. Pokémon cards have a **row** decided by their best stat:
  - **Front** row: physical attackers (Atk is the highest offensive stat).
  - **Back** row: special attackers (SpA highest).
  - **Support** row: walls and supporters (Def/SpD/HP dominate, or a support role).
  - **Agile**: Flying-types with a higher Speed than both attacks may go in Front or Back.
- Deck: 22 to 30 cards, at most 10 special (non-Pokémon) cards, one **Leader** card.
- Start: draw 10. Before round 1 the player may redraw (mulligan) up to 2 cards.
- Turns alternate: play one card, or **pass**. Once you pass you play nothing more this
  round. The round ends when both have passed. Higher total power wins the round; a tie
  costs both a life. Two lives each; lose both and you lose the match.
- Board is cleared after each round (cards go to the discard pile, which "Revive" reads).
- The coin flip decides who starts round 1; the round's loser starts the next one.

### Abilities (Gwent → Pokémon)

| Gwent | Here | Effect |
|-------|------|--------|
| Hero | **Legendary** | Immune to all effects (weather, boosts, Explosion, Revive) |
| Tight Bond | **Evolution line** | Each card of the same evolution line in that row multiplies the others' power (x2 for 2, x3 for 3) |
| Morale Boost | **Helping Hand** | +1 to every other card in its row |
| Muster | **Swarm** | Playing it pulls every copy of the same species from the deck (Wurmple, Zubat, Magikarp…) |
| Spy | **Ghost** (Ghost-types / Pokémon with a "trick" role) | Played on the opponent's side (adds to their score) and you draw 2 cards |
| Medic | **Revive** | Play one Pokémon from your discard pile |
| Scorch | **Explosion** | Destroys the strongest non-legendary card(s) on the board, both sides |
| Commander's Horn | **Cheer** | Doubles one row (special card, or a Pokémon with the ability that only boosts its row) |
| Decoy | **Switch** | Return a card from your board to your hand |
| Agile | **Agile** | See above |

### Weather (special cards, affect both sides)

| Card | Row hit | Exception |
|------|---------|-----------|
| **Hail** | Front: every card's power becomes 1 | Ice-types keep their power |
| **Rain** | Back | Water-types |
| **Sandstorm** | Support | Rock, Ground and Steel-types |
| **Sunny Day** | Clears all weather (Gwent's Clear Weather) | Fire-types in the Back row get +1 while it lasts |

Weather type exceptions are the one place where Pokémon typing matters, which gives each
type a reason to exist without a full type chart. A later phase could add a light type
effect (e.g. +1 when a card is played opposite a row containing a type it is
super-effective against), but it's not in v1: it makes the score hard to read.

### Leaders

Each trainer's deck has a Leader with one use per match, shown on the left of the board.
Examples: Brawly (Cheer on Front row), Winona (clear all weather), Wattson
(draw 1 card), Juan (Rain from the deck), Steven (look at 3 of the opponent's hand cards),
the player (choose one of 4 unlocked leaders: Birch, Mom, May/Brendan, Wally).

## 2. Card data

### Constants: `include/constants/card_game.h`

```c
#define CARD_ROW_FRONT    0
#define CARD_ROW_BACK     1
#define CARD_ROW_SUPPORT  2
#define CARD_ROW_COUNT    3

#define CARD_ABILITY_NONE         0
#define CARD_ABILITY_LEGENDARY    1
#define CARD_ABILITY_BOND         2
// ... one per row in the table in section 1

// Special cards, after the species range so the card id of a Pokémon IS its species id
#define CARD_SPECIAL_START  NUM_SPECIES
#define CARD_HAIL           (CARD_SPECIAL_START + 0)
#define CARD_RAIN           (CARD_SPECIAL_START + 1)
// ...
#define CARD_COUNT          (CARD_SPECIAL_START + NUM_SPECIAL_CARDS)
```

Using the species id as the card id means the collection is just the pokédex plus a small
bitfield, and the board can call `CreateMonIcon(cardId, ...)` directly.

### Table: `src/data/card_game/cards.h`

```c
struct CardInfo
{
    u8 power;       // 0..15
    u8 row:2;       // CARD_ROW_*, or CARD_ROW_AGILE
    u8 ability:5;
    u8 bondGroup;   // evolution line id for CARD_ABILITY_BOND, 0 otherwise
    const u8 *description;
};
extern const struct CardInfo gCardInfo[CARD_COUNT];
```

- Generated by `tools/card_game/gen_cards.py` from `src/data/pokemon/species_info.h`
  (base stats, types) and `src/data/pokemon/evolution.h` (bond groups), output checked in.
  Formula proposal: `power = clamp((BST - 180) / 45, 1, 10)`, legendaries 10-15 with
  `CARD_ABILITY_LEGENDARY`; row by the highest of Atk, SpA and (Def+SpD+HP)/3.
- Then a hand-written override file (`cards_overrides.h`, included after) for the v1 set,
  so rerunning the script never loses balancing work.
- Special cards' names and descriptions in `src/data/card_game/card_text.h`.

### Opponents: `src/data/card_game/opponents.h`

```c
struct CardOpponent
{
    const u8 *name;
    u16 trainerPic;          // TRAINER_PIC_*, drawn with CreateTrainerPicSprite
    u8 leader;               // LEADER_*
    u8 aiLevel;              // 0 easy .. 2 hard
    u16 rewardCard;          // given on the first win, CARD_NONE for none
    const u16 *deck;         // CARD_* list, terminated by CARD_NONE
};
```

Start with ~12: two Card Club members per region, the 8 Hoenn gym leaders as optional
rematch-style duels (spoken to after their badge), the Mega Island card champion, and
a secret one (Mr. M.?).

## 3. Save data

All in `SaveBlock1.unused_3598[0x180]` (384 free bytes, `include/global.h:1069`), turned
into a struct so offsets stay readable:

```c
struct CardGameSave
{
    u8 specialCards[NUM_SPECIAL_CARDS];   // count owned of each special card (0..3)
    u16 deck[30];                         // CARD_* ids, CARD_NONE padded
    u8 deckLeader;
    u8 opponentsBeaten[4];                // bitfield over CARD_OPPONENT_* (32 max)
    u16 wins;
    u16 losses;
    u8 unused[...];                       // pad back to 0x180
};
STATIC_ASSERT(sizeof(struct CardGameSave) == 0x180, CardGameSaveSize);
```

About 32 + 60 + 1 + 4 + 4 = ~101 bytes, with room left for a v2. The Pokémon cards need no
storage: owning a species' card =
`GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT)`. Old saves
read zeros there, which is "no cards, empty deck": fine, since the starter deck is given by
script.

Flags/vars:
- `FLAG_RECEIVED_CARD_CASE` (reuse an `FLAG_UNUSED_*` from `include/constants/flags.h`).
- `FLAG_DAILY_CARD_TOURNAMENT` in a free slot of the daily block (next to
  `FLAG_DAILY_APEX_TOURNAMENT`, `include/constants/flags.h:1576`).

## 4. Code layout

Split the pure rules from the GBA screen so the rules can be tested on the host.

| File | Contents |
|------|----------|
| `include/card_game.h` | Public API: `PlayCardGame` (special), `OpenCardDeckBuilder`, `GiveCard`, `HasCard` |
| `src/card_game_rules.c` | `struct CardDuel` (hands, decks, rows, discard, lives, weather, passed flags, leader used), `Duel_Init`, `Duel_PlayCard`, `Duel_Pass`, `Duel_RowPower`, `Duel_EndRound`. No GBA headers besides `global.h` types; takes an RNG callback |
| `src/card_game_ai.c` | `Ai_ChooseMove(const struct CardDuel *, u8 side)` |
| `src/card_game.c` | The screen: CB2, BG/window setup, sprites, input task, animations, and the entry/exit |
| `src/card_game_deck.c` | The deck builder screen (collection list + deck list, `ListMenu`) |
| `src/data/card_game/*.h` | Tables above |
| `graphics/card_game/` | Board tilemap, card frames, row/ability/weather icons, palettes |
| `tools/card_game/` | `gen_cards.py`, and `test_rules.c` (host test harness, see phase 1) |

`struct CardDuel` is allocated with `AllocZeroed` on entry and freed on exit (like
`sRoulette`), so it costs no permanent EWRAM.

### Entry and exit

```
special PlayCardGame        @ gSpecialVar_0x8004 = CARD_OPPONENT_*
waitstate
@ gSpecialVar_Result = 0 lost, 1 won, 2 forfeited
```

- `data/specials.inc`: `def_special PlayCardGame, waitstate=1` (as `PlayRoulette`, line 182).
- `PlayCardGame` fades out, sets `CB2_CardGame`, and on exit
  `SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic)`, the same exit the slot
  machine uses (`src/scrcmd.c:1919`).
- The reward card and `opponentsBeaten` bit are set in C on a win, so scripts only branch
  on the result for dialogue.

### Screen layout (240x160)

```
+----+--------------------------------------+
| OP |  SUPPORT  [ic][ic][ic]          [ 7] |   opponent rows, 24px each
| ♥♥ |  BACK     [ic][ic]              [12] |
| 23 |  FRONT    [ic][ic][ic][ic]      [15] |
+----+------------- weather ---------------+
| ME |  FRONT    [ic][ic]              [ 9] |   player rows
| ♥♥ |  BACK     [ic][ic][ic]          [18] |
| 31 |  SUPPORT  [ic]                  [ 4] |
+----+--------------------------------------+
  hand: < [card][card][card][card][card] >  (slides up over the board on L/A)
```

- BG0: text windows (scores, card names, prompts). BG1: row tilemap, redrawn on weather.
  BG2: hand panel / card detail panel. BG3: board background.
- Board cards: `CreateMonIcon` sprites, max 8 per row (overflow shows "+N"), so at most
  48 board sprites + 5 visible hand cards + cursor + leader + 2 trainer pics (64x64,
  `CreateTrainerPicSprite`) ≈ 60 OAM, well under 128.
- Card detail (Select, or hovering a hand card): `CreateMonPicSprite_Affine` front pic,
  name, power, row and ability text.
- Icon palettes: load the 3 icon palettes once (`LoadMonIconPalettes`), as the party menu
  does.

### AI (`src/card_game_ai.c`)

Gwent's AI is mostly about **when to pass**, so the AI evaluates that first:

1. If it is ahead and the opponent has passed: pass.
2. If it is ahead by more than the opponent's estimated best play and has fewer than or
   equal cards: pass (save cards for the next round).
3. If this round is hopeless (behind by more than its hand can add while keeping 2+ cards
   for later) and it has a life to spare: pass.
4. Otherwise score every legal play with a one-ply simulation (`Duel_PlayCard` on a copy
   of the state): delta in score, +value for card draw (Ghost), −value for spending
   legendaries in round 1, weather only if it hurts the opponent more. Pick the best.

`aiLevel` 0 skips rules 2-3 and picks randomly among the top 3; level 2 also tracks the
cards the player has played (Swarm and Bond sets) to predict them.

## 5. Story integration

- **Card Case** key item (`ITEM_CARD_CASE`, after the existing key items in
  `include/constants/items.h`), used from the bag to open the deck builder. Given by the
  Lilycove Department Store clerk with a 22-card starter deck (common Hoenn species the
  player is likely to have caught, plus 2 Rain, 1 Sunny Day, 1 Revive), after badge 4.
- **Mauville Card Club**: a room behind the Game Corner, 3 opponents of rising difficulty.
- **Celadon Card Club**: in the Celadon Game Corner or the Condominiums, 3 Kanto
  opponents.
- **Gym leaders**: after the badge, talking to a leader again offers a duel (their deck is
  their type, leader ability themed). Beating all 8 gives a legendary special card.
- **Mega Island**: a daily card tournament (3 duels in a row) on a table in the
  castle's entrance hall, gated by `FLAG_DAILY_CARD_TOURNAMENT`.
- **Shops**: "booster" special cards for sale at Lilycove (random 1 of 3 special cards per
  purchase, through a `special` that rolls and gives it).

## 6. Phases

| Phase | Deliverable | Rough size |
|-------|-------------|------------|
| 1 | Rules engine `card_game_rules.c` + host test harness (`tools/card_game/test_rules.c`, compiled with host `cc` against stubbed `global.h` types): round scoring, weather, bond, swarm, ghost, revive, explosion, lives, ties | 600-800 lines |
| 2 | Card table: generator script, overrides for the v1 set (~60 Pokémon + 12 special cards), opponent table with 3 decks | 1 script + data |
| 3 | Minimal screen: entry/exit special, board drawn with text-only placeholders, hand cursor, play/pass, round/match end. Playable against a random AI from a debug script | 800-1000 lines |
| 4 | Real graphics: board tilemap, card frames, icons, detail panel, weather overlays, score animations | art + 400 lines |
| 5 | AI as in section 4 | 300 lines |
| 6 | Save struct, deck builder screen, Card Case item | 600 lines |
| 7 | Story: Card Clubs, gym leader duels, starter deck, rewards, Mega Island daily tournament | maps + scripts |
| 8 (later) | Link duels over the cable club / wireless, using the deterministic state | large |

Each phase builds and is playable on its own; phases 1-3 are the core and should land
first, behind a debug script so the main game is untouched until phase 7.

## 7. Risks and checks

- **ROM space**: the game, AI and data are small (tens of KB); new graphics are the main
  cost. Check `make` still fits the ROM after phase 4.
- **Save layout**: `STATIC_ASSERT` the struct size; never move other SaveBlock1 fields.
- **Pokédex dependency**: a player who has caught few species has few cards. The starter
  deck's species should be ones every route gives early, and special cards fill the gaps.
- **Balance**: a "Show opponent's hand" debug toggle and a self-play mode (AI vs AI on the
  host harness, 1000 games per deck) to find dominant decks before tuning by hand.
- **Readability on a GBA screen**: row totals and lives must be readable at a glance;
  playtest the layout on real hardware or mGBA at 1x before adding more information.
