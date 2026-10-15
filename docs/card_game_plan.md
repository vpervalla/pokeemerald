# Plan: a Pokémon card game ("Poké Duel")

This is a plan for adding a small card game to the project, a game inside the game (as
Gwent is in The Witcher 3). It is single-player only: the player duels NPCs scattered
across Hoenn, Kanto and the Sevii Islands, in towns, Pokémon Centers, houses, routes and
gyms, each with their own deck and a card to win. The
rules are those of **Pokémon TCG Pocket**: 20-card decks, no Energy cards (an Energy Zone
makes one Energy a turn), up to 3 benched Pokémon, and the first player to earn
**3 trophies** wins. "POKé DUEL" is a working name; it should live in one string so it
can change in one place.

The game runs on its own screen (like the slot machine or roulette), launched from an
event script with a `special`, and returns to the field when it ends. It does not touch
the battle engine, though its card data is generated from the battle data.

## 0. Decisions to make first

| # | Decision | Recommendation | Why it matters |
|---|----------|----------------|----------------|
| D1 | Which Pocket rules | All the core rules in section 1. Leave out Pocket's later additions (Fossil Items, Ultra Beasts, Pokémon Tools beyond 2-3, Special Conditions beyond the 5 classic ones) | The core is complete and fits a GBA screen; the rest is content, not rules |
| D2 | Where cards come from | Catching a species unlocks its Pokémon card. **ex** cards and Trainer cards come from daily booster packs, shop packs and opponents' rewards | Ties the game to "catch them all", and costs no save space for ~380 cards |
| D3 | What the ex cards are | The 48 Mega forms already in the game (`src/data/pokemon/mega_evolutions.h`), as Basic or evolved "Mega ex" cards worth 2 trophies, plus a few legendaries ex | Reuses existing species, graphics and names |
| D4 | Card stats | Generated from base stats, types, evolutions and level-up moves by a host-side script, then hand-tuned for the cards in the v1 set | ~430 hand-designed cards is months of balancing; a generator gets most of the way |
| D5 | Supporters | Gym leaders, Elite Four and rivals of both regions (as Pocket uses the Kanto gym leaders). Beating a leader in a duel gives their Supporter card | Natural story hook and reward |
| D6 | Art | Reuse the 32x32 party icons on the board and the 64x64 front pic in the card detail view. New art: board, card frames, Energy symbols, trophies, status icons | Icons share 3 palettes (`gMonIconPaletteIndices`), which keeps VRAM simple |
| D7 | Opponents | NPCs only, never another human player: no link or wireless mode | Removes all link code, and lets every duel be designed around a specific NPC and place |
| D8 | How many NPCs | ~60 for the full game (~40 Hoenn, ~20 Kanto/Sevii), ~15 in the first playable version | Enough that one is never far, as Gwent players are in every inn |
| D9 | New NPCs or existing ones | Mostly existing townsfolk who also play cards (an extra line and a choice in their script); new objects only where a town has nobody suitable | Map edits are the slowest part; reusing NPCs keeps them small |

## 1. Rules (Pokémon TCG Pocket)

### Deck and setup

- **20 cards**, at most **2 copies** of cards with the same name. At least one Basic
  Pokémon.
- The deck chooses **1 to 3 Energy types** for its Energy Zone (deck builder field).
- Coin flip decides who goes first.
- Each player draws **5 cards**, always with at least one Basic Pokémon (the deck is
  reshuffled until the opening hand has one: no mulligans).
- Each player puts a Basic Pokémon face down as **Active** and up to 3 as **Bench**, then
  both are revealed.
- Hand limit: **10 cards** (no more draws, by any effect, once at 10).

### A turn

1. **Draw** a card (if the deck is empty, skip; running out of cards is not a loss).
2. The **Energy Zone** produces 1 Energy of one of the deck's types, at random; the next
   one is shown in advance. The **first player gets no Energy on their first turn**.
3. In any order, as many times as allowed:
   - Attach the zone's Energy to one of your Pokémon (once a turn).
   - Put Basic Pokémon on the Bench (max 3).
   - Evolve a Pokémon (not on either player's first turn, and not one played or evolved
     this turn).
   - Play any number of **Item** cards, and **one Supporter** per turn (allowed on the
     first turn too; only Energy and attacking are limited then).
   - Attach a **Pokémon Tool** (one per Pokémon).
   - **Retreat** once: discard Energy equal to the retreat cost, swap with a Bench Pokémon.
   - Use **Abilities**.
4. **Attack** (optional; the first player cannot attack on their first turn). The turn
   ends after attacking.
5. **Pokémon Checkup** (between turns): Poison (10 damage), Burn (20 damage, then flip:
   heads cures), Sleep (flip: heads wakes), Paralysis (wears off at the end of its
   owner's next turn).

### Damage and trophies

- Damage = attack's damage, **+20 if the defender is weak** to the attacker's type (each
  type has one weakness, no resistances), plus/minus effects (Giovanni +10, …).
- A Pokémon whose damage reaches its HP is **Knocked Out**: its owner moves a Bench
  Pokémon up. The attacker's player earns **1 trophy**, or **2 trophies for an ex**.
- **Win**: first to **3 trophies**, or the opponent has no Pokémon in play after a
  knockout. Both reaching 3 at once (or both losing their last Pokémon), or reaching the
  turn limit (**30 turns**, Pocket's sudden-death rule replaced by a draw), is a draw.

### Energy types

Pocket's 10 types, mapped from the game's 18:

| Card type | Pokémon types | Weak to |
|-----------|---------------|---------|
| Grass | Grass, Bug | Fire |
| Fire | Fire | Water |
| Water | Water, Ice | Lightning |
| Lightning | Electric | Fighting |
| Psychic | Psychic, Ghost, Fairy | Darkness |
| Fighting | Fighting, Rock, Ground | Psychic or Grass (by species, as in Pocket) |
| Darkness | Dark, Poison | Fighting |
| Metal | Steel | Fire |
| Dragon | Dragon (no Energy of its own: attacks cost two other types) | none |
| Colorless | Normal, Flying | Lightning or Fighting |

A dual-typed species uses its first type that isn't Normal/Flying (Gyarados → Water),
unless the overrides say otherwise.

### Special Conditions

Asleep, Burned, Confused (flip when attacking: tails, the attack does nothing),
Paralyzed, Poisoned. Retreating or
evolving cures them all.

## 2. Card data

### Card ids: `include/constants/card_game.h`

Cards have their own ids, because the Mega species sit after `NUM_SPECIES`
(`include/constants/species.h:497`) and several cards can share a species.

```c
#define CARD_NONE 0
// Pokémon cards: generated, one per species that can be caught, then the ex cards
#define CARD_BULBASAUR      1
// ...
#define CARD_POKEMON_END    ...
#define CARD_VENUSAUR_MEGA_EX ...
// ...
// Trainer cards
#define CARD_POTION         ...
#define CARD_POKE_BALL      ...
#define CARD_X_SPEED        ...
#define CARD_PROF_RESEARCH  ...   // Supporter: draw 2
#define CARD_ROXANNE        ...   // Supporter
// ...
#define CARD_COUNT          ...

#define CARD_TYPE_GRASS ... CARD_TYPE_COLORLESS   // 10 types
#define CARD_STAGE_BASIC 0
#define CARD_STAGE_1     1
#define CARD_STAGE_2     2
```

### Pokémon cards: `src/data/card_game/pokemon_cards.h`

```c
struct CardAttack
{
    const u8 *name;
    u8 cost[4];        // CARD_TYPE_* per Energy, CARD_TYPE_NONE padded; Colorless = any
    u8 damage;         // in 10s: 6 = 60
    u8 effect;         // CARD_EFFECT_*
    u8 effectArg;
};

struct PokemonCard
{
    u16 species;            // for icons, pics, names, and the pokédex ownership check
    u16 evolvesFrom;        // CARD_* or CARD_NONE
    u8 hp;                  // in 10s
    u8 type:4;
    u8 weakness:4;
    u8 stage:2;
    u8 retreatCost:3;
    u8 isEx:1;
    u8 ability;             // CARD_ABILITY_* or NONE
    struct CardAttack attacks[2];
};
```

### Generator: `tools/card_game/gen_cards.py`

Reads the game's own data and writes `pokemon_cards.h`, which is checked in:

- **Stage and evolvesFrom** from `src/data/pokemon/evolution.h`. Baby Pokémon
  (Pichu, Azurill…) become Basic cards and their evolutions stay Basic too, as in Pocket.
- **HP**: `round10(base HP + BST / 8)`, clamped to 40-180; ex +30-50.
- **Type, weakness** from `species_info.h` with the table in section 1.
- **Retreat cost** from weight in `pokedex_entries.h` (0-4).
- **Attacks** from `level_up_learnsets.h` and `src/data/battle_moves.h`: one cheap
  attack (an early damaging move) and, for evolved or strong Pokémon, one big attack
  (the best STAB move it learns). Damage ≈ move power × 0.6, rounded to 10; cost by
  damage (≤30: 1 Energy, ≤60: 2, ≤100: 3, more: 4), part of it Colorless.
- **Effects** from the move's battle effect: `EFFECT_PARALYZE_HIT` → "flip: heads,
  Paralyzed", `EFFECT_POISON_HIT` → Poisoned, `EFFECT_BURN_HIT` → Burned,
  `EFFECT_SLEEP` → Asleep, `EFFECT_CONFUSE_HIT` → Confused, `EFFECT_MULTI_HIT` →
  "flip 2 coins, 30× heads", `EFFECT_ABSORB` → heal, `EFFECT_RECOIL` → self damage,
  and so on (`include/constants/battle_move_effects.h`).
- A hand-written **overrides** file (`pokemon_cards_overrides.h`, applied by the
  script) for the v1 set, the ex cards and every ability, so rerunning the script never
  loses balancing work.

### Effects: `src/card_game_effects.c`

One function per `CARD_EFFECT_*`, called by the rules engine. Expected v1 list (~35):
flip for status (5), flip for extra damage, flip-until-tails, damage per Energy, damage to
a Bench Pokémon, damage to all opponent's Bench, heal self, heal all, discard Energy from
self, discard Energy from the defender, self damage, can't attack next turn, switch the
defender, draw a card, search the deck for a Basic, attach extra Energy from the zone
(Misty-style), prevent damage next turn. Abilities use the same table (~10 to start).

### Trainer cards: `src/data/card_game/trainer_cards.h`

| Kind | v1 cards |
|------|----------|
| Item | Potion (heal 20), Poké Ball (random Basic from the deck), X Speed (retreat −1), Red Card (opponent shuffles hand, draws 3), Hand Scope (see opponent's hand), Rare Candy (Basic → Stage 2) |
| Tool | Giant Cape (+20 HP), Rocky Helmet (20 damage to the attacker) |
| Supporter | Professor's Research → **Prof. Birch's Research** (draw 2), Giovanni-style +10 damage (**Brawly**), Misty-style Water Energy flips (**Juan**), Erika-style heal 50 to Grass (**Erika** from Kanto), Sabrina-style switch the opponent's Active (**Tate & Liza**), Koga-style return Active to hand (**Koga**), Brock-style Fighting Energy (**Roxanne**), Lt. Surge-style free retreat with Lightning (**Wattson**), Blaine-style Fire boost (**Flannery**)… one per gym leader of both regions |

### Opponents: `src/data/card_game/opponents.h`

```c
struct CardOpponent
{
    const u8 *name;
    u16 trainerPic;          // TRAINER_PIC_*, drawn with CreateTrainerPicSprite
    u8 aiLevel;              // 0 easy .. 2 hard
    u8 energyTypes[3];       // the deck's Energy Zone
    u16 rewardCard;          // given on the first win, CARD_NONE for none
    const u16 *deck;         // 20 CARD_* ids
};
```

`CARD_OPPONENT_*` ids are in `include/constants/card_game.h`; the table also has the
NPC's lines (challenge, win, loss), so a map script needs no text of its own. Who they are
and where they stand is section 5.

## 3. Save data

All in `SaveBlock1.unused_3598[0x180]` (384 free bytes, `include/global.h:1069`), turned
into a struct so the offsets stay readable:

```c
struct CardGameSave
{
    u8 exCards[NUM_EX_CARDS];             // copies owned of each ex card (0..2)
    u8 trainerCards[NUM_TRAINER_CARDS];   // copies owned of each Trainer card (0..2)
    u16 decks[3][20];                     // 3 saved decks
    u16 deckEnergy[3];                    // up to 3 CARD_TYPE_* per deck, 4 bits each
    u8 activeDeck;
    u8 opponentsBeaten[10];               // bitfield over CARD_OPPONENT_* (80 max)
    u16 wins;
    u16 losses;
    u8 unused[...];                       // pad back to 0x180
};
STATIC_ASSERT(sizeof(struct CardGameSave) == 0x180, CardGameSaveSize);
```

Rough size: ~60 ex + ~40 trainer counts (100), 3 decks (120), energy (6), the rest (~10):
~240 bytes, with room left for a v2. Regular Pokémon cards need no storage: owning one =
`GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT)`, and the deck
limit of 2 copies applies as usual. Old saves read zeros there ("no cards, no decks"):
fine, since the starter deck is given by script.

Flags:
- `FLAG_RECEIVED_CARD_CASE` (reuse an `FLAG_UNUSED_*` from `include/constants/flags.h`).
- Daily flags, next to `FLAG_DAILY_APEX_TOURNAMENT` (`include/constants/flags.h:1576`):
  `FLAG_DAILY_CARD_PACK` (Pocket's free daily pack) and `FLAG_DAILY_CARD_TOURNAMENT`.

## 4. Code layout

Split the rules from the GBA screen so the rules can be tested on the host.

| File | Contents |
|------|----------|
| `include/card_game.h` | Public API: `PlayCardGame` (special), `OpenCardDeckBuilder`, `OpenCardPack`, `GiveCard`, `HasCard` |
| `src/card_game_rules.c` | `struct CardDuel` (decks, hands, Active/Bench slots with damage, Energy, Tool, status, turn played; discard piles; zone's current and next Energy; trophies; turn number; supporter/retreat/energy-used flags). `Duel_Init`, `Duel_StartTurn`, `Duel_GetLegalActions`, `Duel_DoAction`, `Duel_Checkup`. No GBA headers besides `global.h` types; takes an RNG callback. Every change also pushes an **event** (damage, KO, coin flip, draw…) for the screen to animate |
| `src/card_game_effects.c` | Attack, ability and Trainer effects |
| `src/card_game_ai.c` | `Ai_ChooseAction(const struct CardDuel *, u8 side)` |
| `src/card_game.c` | The duel screen: CB2, BG/window setup, sprites, input, animations of the event queue, entry/exit |
| `src/card_game_deck.c` | The deck builder (collection list + deck list on `ListMenu`, Energy types) |
| `src/card_game_pack.c` | Opening a pack (5 cards revealed one by one) |
| `src/data/card_game/*.h` | Tables above |
| `graphics/card_game/` | Board tilemap, card frames, Energy symbols, trophies, status icons |
| `tools/card_game/` | `gen_cards.py`, and `test_rules.c` (host test harness, see phase 1) |

`struct CardDuel` is allocated with `AllocZeroed` on entry and freed on exit (like
`sRoulette`), so it costs no permanent EWRAM.

### Entry and exit

```
special PlayCardGame        @ gSpecialVar_0x8004 = CARD_OPPONENT_*
waitstate
@ gSpecialVar_Result = 0 lost, 1 won, 2 draw, 3 forfeited
```

- `data/specials.inc`: `def_special PlayCardGame, waitstate=1` (as `PlayRoulette`, line 182).
- A macro in `asm/macros/event.inc`, `cardduel CARD_OPPONENT_X`, sets `VAR_0x8004` and
  calls the shared script `Common_EventScript_CardDuel` (new file
  `data/scripts/card_duel.inc`), which does everything around the duel: the NPC's
  challenge line, "Play? YES/NO", the checks (has the Card Case, has a legal deck), the
  duel, and the win/loss line and reward. Turning an NPC into a card player is then one
  line in their script.
- `PlayCardGame` fades out, sets `CB2_CardGame`, and on exit
  `SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic)`, the same exit the slot
  machine uses (`src/scrcmd.c:1919`).
- The reward card and `opponentsBeaten` bit are set in C on a win, so scripts only branch
  on the result for dialogue. With no link mode, `struct CardDuel` never has to be
  serialized or kept in sync, and the RNG is just `Random()`.

### Screen layout (240x160)

```
+---------------------------------------------+
| ★★☆ OPP    [b1 60] [b2 40] [b3 --]   deck 12 |   opponent Bench
|               [ ACTIVE  icon ]  HP 70/120    |   opponent Active
|               ●● ⚡ PSN                       |   Energy, status
|---------------------------------------------|
|               [ ACTIVE  icon ]  HP 90/90     |   player Active
|               ●●●              zone: 🔥 next 💧|
| ★☆☆ YOU    [b1 50] [b2 80] [b3 70]   deck 9  |   player Bench
+---------------------------------------------+
  hand: < [card][card][card][card][card] >  (slides up over the board)
```

- BG0: text windows (HP, names, prompts). BG1: board frame. BG2: hand panel / card detail
  panel / action menu (Attack, Retreat, Ability, End turn). BG3: background.
- Pokémon in play: `CreateMonIcon` sprites, 8 at most. Energy pips (8x8, up to 6 per
  Pokémon), status icons, trophies, zone Energy, cursor, 5 visible hand cards, 2 trainer
  pics (`CreateTrainerPicSprite`, at the start and end): ≈ 90 OAM at the very worst,
  under 128. Energy pips beyond 6 show as "×N".
- Card detail (Select, or hovering a card): `CreateMonPicSprite_Affine` front pic, HP,
  type, weakness, retreat, attacks with costs and effect text.
- Icon palettes loaded once with `LoadMonIconPalettes`, as the party menu does.
- Coin flips: a small spinning Poké Ball coin sprite.

### AI (`src/card_game_ai.c`)

Rule-based, over the legal actions the rules engine lists:

1. Play Basics to the Bench while there is room (an empty Bench loses the game on a
   single knockout).
2. Evolve whenever possible.
3. Attach the Energy to the Active if it brings it to an attack's cost; otherwise to the
   Pokémon closest to attacking (prefer evolved ones and ex).
4. Items and Supporters by simple conditions per card (Potion if it prevents a KO,
   Supporter "draw" when the hand is small, the +10 Supporter when it makes a KO…).
5. Retreat if the Active will be knocked out next turn and a Bench Pokémon can attack,
   and the cost is affordable.
6. Attack with the attack that knocks out, else the most damage (counting weakness and
   coin flips at their average).

`aiLevel` 0 plays a random legal action half the time; level 2 also simulates the
player's best attack next turn to decide steps 3 and 5, and plays around ex (2 trophies).

## 5. NPC opponents in the world

### How a card player works

- Any NPC can be a card player: their usual line, then the shared
  `Common_EventScript_CardDuel` (section 4). Before the Card Case they just say they play
  cards, which is the hook ("Come back when you have a deck!").
- They can be replayed any time. The first win gives their **reward card** (often a card
  that can't be found elsewhere, as Gwent's unique cards); later wins give nothing, or one
  random card at most once a day for the stronger ones (a daily flag each, from the
  daily block).
- Deck strength and `aiLevel` follow the story: early-route NPCs play weak, single-type
  decks of local Pokémon; late ones play ex and two-type decks.
- A **Card Player list** in the Card Case shows every opponent met: name, place, beaten
  or not (from `opponentsBeaten`), so the player can hunt the ones still to beat, like
  Gwent's quest log.

### Where they are (Hoenn)

| Place | Opponent (example) | Deck / reward |
|-------|--------------------|---------------|
| Littleroot, Oldale | A Littleroot neighbor, a kid in the Oldale Pokémon Center | Starter-level decks; teach the rules |
| Petalburg, Route 104 | A Youngster in the Pokémon Center, the flower shop girl | Normal, Grass; Potion, Erika |
| Rustboro | Devon employee, Trainers' School student | Steel, mixed; X Speed. Card Case given here (see below) |
| Dewford, Slateport | Surfer at the hall, Slateport market vendor | Water, Fighting; Poké Ball |
| Mauville | Game Corner regular, Wattson's assistant | Lightning; Red Card |
| Lavaridge, Fallarbor | Hot spring old lady, a Fallarbor contest fan | Fire, Psychic |
| Fortree, Lilycove | Tree-house resident, Department Store regular | Flying/Colorless, Water; Hand Scope, shop packs here |
| Mossdeep, Sootopolis | Space Center scientist, Sootopolis elder | Psychic, Water ex |
| Pacifidlog, Ever Grande | Raft resident, Victory Road veteran | Dragon, ex decks |
| Routes | 1-2 per long route, standing near a rest spot (Route 111 rest house, Route 119 weather institute, Route 123 Berry Master's house) | Local Pokémon |
| Gyms | Each gym leader, after their badge | Their type; their Supporter card. All 8 → an ex card |

### Where they are (Kanto and Sevii)

| Place | Opponent (example) | Deck / reward |
|-------|--------------------|---------------|
| Pallet, Viridian, Pewter, Cerulean | Locals and Pokémon Center visitors | Kanto species, Kanto gym leaders' Supporters through the gyms |
| Vermilion, Celadon, Saffron | Fan club, Game Corner, Fighting Dojo | Stronger decks; Celadon Department Store sells packs |
| Fuchsia, Cinnabar | Safari Zone warden's friend, a lab scientist | Rare Candy, fossil Pokémon |
| Sevii Islands | One per island | ex cards |
| Mega Island | The castle's guests (the 6 guest rooms) and a daily card tournament in the entrance hall (3 duels in a row, `FLAG_DAILY_CARD_TOURNAMENT`) | Mega ex cards; the strongest opponent of the game is the tournament's champion |

Gym leaders' card duels hook into the existing gym leader rematch scripts
(`data/scripts/gym_rematches.inc`): after the battle-rematch question, a second choice
"Or a card duel?".

### Getting started

- **Card Case** key item (`ITEM_CARD_CASE`, after the existing key items in
  `include/constants/items.h`), used from the bag to open the deck builder and the Card
  Player list. Given in Rustboro after the first badge (a card fan in the Pokémon
  Center), with a starter deck: 2 copies each of common early species the player is likely
  to have caught, plus Potion, Poké Ball and Prof. Birch's Research ×2. Early enough that
  most of the world's card players are met with a deck in hand.
- **Daily pack**: the card fan in Rustboro (and a second one in Celadon, for Kanto) gives
  one free pack a day (`FLAG_DAILY_CARD_PACK`): 5 cards, mostly Trainers, rarely an ex.
- **Shop packs**: sold at the Lilycove and Celadon Department Stores.

## 6. Phases

| Phase | Deliverable | Rough size |
|-------|-------------|------------|
| 1 | Rules engine `card_game_rules.c` + host test harness (`tools/card_game/test_rules.c`, compiled with host `cc` against stubbed `global.h` types): setup, turn flow, Energy Zone, retreat, evolution, weakness, Special Conditions, KOs, trophies, win/draw | 1000-1300 lines |
| 2 | Effects for the v1 set, card generator + overrides, ~60 Pokémon (incl. 6 ex), ~15 Trainer cards, 3 NPC decks | 1 script, 600 lines, data |
| 3 | Minimal screen: entry/exit special, board with text placeholders, menus for every action, event queue shown as messages. Playable against a random AI from a debug script | 1200 lines |
| 4 | Real graphics: board, icons, Energy pips, card detail, coin flip and damage animations | art + 500 lines |
| 5 | AI as in section 4 | 500 lines |
| 6 | Save struct, deck builder, Card Case item, pack opening | 800 lines |
| 7 | World, first wave: `cardduel` macro and shared script, Card Case and starter deck in Rustboro, daily pack, ~15 NPC card players from Littleroot to Lilycove, Hoenn gym leaders, Card Player list | scripts, a few map objects |
| 8 | World, second wave: the rest of Hoenn, Kanto, Sevii, the Mega Island guests and daily tournament (~45 more NPCs) | scripts, a few map objects |
| 9 | Content: the full generated card pool, more Trainers and ex | data |

Phases 1-3 are the core and should land first, behind a debug script so the main game is
untouched until phase 7. There is no link phase: the game is played only against NPCs.

## 7. Risks and checks

- **Rules engine size**: Pocket is far richer than a Gwent-style game (attacks, effects,
  statuses, evolution). Keeping the rules pure and host-tested is what makes it
  manageable.
- **ROM space**: ~430 cards × ~24 bytes ≈ 10 KB, plus attack names (shared strings with
  `gMoveNames` where the attack is a move), code and art. Check `make` still fits the
  ROM after phases 4 and 9.
- **Save layout**: `STATIC_ASSERT` the struct size; never move other SaveBlock1 fields.
- **Pokédex dependency**: a player who has caught few species has few cards. The starter
  deck covers that, and Trainer cards come from packs, not catches.
- **Balance**: a "show opponent's hand" debug toggle, and a self-play mode on the host
  harness (AI vs AI, 1000 games per deck pair) to find dominant decks before tuning.
- **NPC decks**: ~60 hand-built 20-card decks is real work. A small script can build a
  first draft per NPC from a type, a level band and the local route's encounters
  (`src/data/wild_encounters.json`), to be tuned by hand.
- **Readability**: HP, Energy and trophies must be readable at a glance at 240x160;
  playtest on mGBA at 1x before adding more information.
