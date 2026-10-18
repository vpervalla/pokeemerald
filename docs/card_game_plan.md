# Plan: a Pokémon card game ("Poké Duel")

This is a plan for adding a small card game to the project, a game inside the game (as
Gwent is in The Witcher 3). It is single-player only: the player duels NPCs scattered
across Kanto, the Sevii Islands and Mega Island, in towns, Pokémon Centers, houses, routes
and gyms, each with their own deck and a card to win. Hoenn is never reachable in this
project, so nothing is placed there. The rules are those of **Pokémon TCG Pocket**: 20-card decks, no Energy cards (an Energy Zone
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
| D2 | Where cards come from | Catching a species unlocks its Pokémon card. **ex** cards and Trainer cards come from daily booster packs, shop packs and opponents' rewards | Ties the game to "catch them all", and costs no save space for the Pokémon cards |
| D2b | Which species get a card | Only those the player can obtain: the generator reads the Kanto, Sevii and Mega Island encounters in `src/data/wild_encounters.json`, adds their evolutions and the gift/trade/static Pokémon, and skips species only found in Hoenn | A card that can never be owned is wasted ROM and an empty slot in the collection |
| D3 | What the ex cards are | The Mega forms already in the game (`src/data/pokemon/mega_evolutions.h`), as Basic or evolved "Mega ex" cards worth **3 trophies** (as in Pocket's Mega Rising set), plus maybe a few legendaries ex worth 2 later | Reuses existing species, graphics and names |
| D4 | Card stats | Generated from base stats, types, evolutions and level-up moves by a host-side script, then hand-tuned for the cards in the v1 set | Hundreds of hand-designed cards is months of balancing; a generator gets most of the way |
| D5 | Supporters | The Kanto gym leaders (as in Pocket itself), Prof. Oak, Bill, the rival and the Elite Four. Beating a leader in a duel gives their Supporter card | Natural story hook and reward, and the cards already exist in Pocket to copy |
| D6 | Art | Reuse the 32x32 party icons on the board; the cards' 64x64 pictures are the Pokémon Gold front sprites, and Gold-style sprites for the Megas (the game's front pic for Hoenn species and Mega Dragonite). New art: board, card frames, Energy symbols, trophies, status icons | Icons share 3 palettes (`gMonIconPaletteIndices`), which keeps VRAM simple |
| D7 | Opponents | NPCs only, never another human player: no link or wireless mode | Removes all link code, and lets every duel be designed around a specific NPC and place |
| D8 | How many NPCs | ~50 for the full game (Kanto ~30, Sevii ~12, Mega Island ~8), ~15 in the first playable version | Enough that one is never far, as Gwent players are in every inn |
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
  Pokémon up. The attacker's player earns **1 trophy**, **2 for an ex**, or **3 for a Mega ex**.
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
- A hand-written **overrides** file (`src/data/card_game/pokemon_card_overrides.json`, applied
  by the script) for the v1 set, the ex cards and every ability, so rerunning the script never
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
| Supporter | As in Pocket: **Prof. Oak's Research** (draw 2), **Brock** (Fighting Energy to Onix/Golem), **Misty** (flip for Water Energy), **Lt. Surge** (free retreat with Lightning), **Erika** (heal 50 to Grass), **Koga** (Poisons the opponent's Active, see phase 8), **Sabrina** (switch the opponent's Active), **Blaine** (Fire boost), **Giovanni** (+10 damage). Later: **Bill** (draw), the rival, the Elite Four (all added in phase 9) |

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
| `src/card_game_rules.c` | `struct CardDuel` (decks, hands, Active/Bench slots with damage, Energy, Tool, status, turn played; discard piles; zone's current and next Energy; trophies; turn number; supporter/retreat/energy-used flags). `Duel_Init`, `Duel_GetLegalActions`, `Duel_IsLegalAction`, `Duel_DoAction`, `Duel_Forfeit`. No GBA headers besides `global.h` types; the duel carries its own seeded RNG (and a forced-flips hook for tests and debugging). Every change also pushes an **event** (damage, KO, coin flip, draw…) for the screen to animate |
| `src/card_game_effects.c` | Attack, ability and Trainer effects |
| `src/card_game_ai.c` | `Ai_ChooseAction(const struct CardDuel *, u8 side)` |
| `src/card_game.c` | The duel screen: CB2, BG/window setup, sprites, input, animations of the event queue, entry/exit |
| `src/card_game_deck.c` | The deck builder (collection list + deck list on `ListMenu`, Energy types) |
| `src/card_game_pack.c` | Opening a pack (5 cards revealed one by one) |
| `src/data/card_game/*.h` | Tables above |
| `graphics/card_game/` | Board tilemap, card frames, Energy symbols, trophies, status icons |
| `src/card_game_data.c` | The card tables and their lookups (`CardGame_GetPokemonInfo`, `CardGame_GetTrainerInfo`, `CardGame_GetCardKind`), which the engine only reaches through those functions |
| `tools/card_game/` | `gen_cards.py`, and `test_rules.c` + `run_tests.sh` (host test harness, see phase 1) |

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

### Where they are (Kanto)

| Place | Opponent (example) | Deck / reward |
|-------|--------------------|---------------|
| Pallet, Viridian | A Pallet neighbor, the old man's grandson in Viridian | Starter-level decks that teach the rules |
| Route 2, Pewter | Route 2 house resident, Museum guide, a Pokémon Center visitor | Bug, Normal; Card Case given in Pewter (see below) |
| Mt. Moon, Cerulean, Route 25 | Hiker at the Pokémon Center by Mt. Moon, Bike Shop owner, Bill's neighbor | Fighting, Water; Poké Ball |
| Vermilion | Pokémon Fan Club members, an S.S. Anne passenger | Colorless, Lightning; Red Card |
| Lavender, Route 12 | Name Rater, the Fishing Guru's brother | Psychic, Water |
| Celadon | Game Corner regulars, Department Store regular, Celadon Mansion residents | Mixed decks; packs sold at the Department Store |
| Saffron | Fighting Dojo karate master, Copycat's house | Fighting, Psychic ex |
| Fuchsia, Routes 12-18 | Safari Zone warden, the Fishing house, Cycling Road biker | Poison, Grass; Hand Scope |
| Cinnabar | Lab scientist, Pokémon Mansion visitor | Fire; Rare Candy, fossil Pokémon |
| Indigo Plateau | A veteran in the Pokémon Center | ex decks, after the Hall of Fame |
| Gyms | Each Kanto gym leader, after their badge | Their type; their Supporter card. All 8 → an ex card |

### Where they are (Sevii Islands and Mega Island)

| Place | Opponent (example) | Deck / reward |
|-------|--------------------|---------------|
| One Island | Pokémon Net Center staff, Kindle Road hot spring guests | Fire, Water |
| Two Island | The Joyful Game Corner, a natural home for a card table: 2-3 players | Mixed; Bill |
| Three, Four Island | Bond Bridge fisherman, Lorelei at her house (after the Hall of Fame) | Water/Ice ex |
| Five, Six, Seven Island | Resort Gorgeous Selphy, Ruin Valley explorer, Trainer Tower receptionist | Strong mixed decks, ex |
| Mega Island | The castle's guests (the 6 guest rooms), and a daily card tournament in the entrance hall (3 duels in a row, `FLAG_DAILY_CARD_TOURNAMENT`) | Mega ex cards; the tournament's champion is the strongest opponent in the game |

Gym leader card duels are offered after the badge, during the story, from each gym's own
post-badge script (one more choice: "How about a card duel?"). After the Hall of Fame the
same choice is added to the leaders' rematch scripts (`data/scripts/gym_rematches.inc`),
next to the rematch battle.

### Getting started

- **Card Case** key item (`ITEM_CARD_CASE`, after the existing key items in
  `include/constants/items.h`), used from the bag to open the deck builder and the Card
  Player list. Given in Pewter City after the Boulder Badge by PROF. OAK's AIDE, the one who
  brings the Running Shoes in FRLG (the Running Shoes are on from the start of the game
  instead), with a starter deck and a first pack. The starter deck: early Kanto species the
  player is likely to have met (Caterpie, Pidgey, Rattata, Spearow lines), plus Potion, Poké
  Ball and Prof. Oak's Research ×2. Early enough that most of the world's card players are
  met with a deck in hand.
- **Daily pack**: the first POKé MART clerk the player talks to each day gives one free pack
  (`FLAG_DAILY_CARD_PACK`): 5 cards, mostly Pokémon, rarely an ex. A second fan in the Two
  Island Game Corner does the same, so the pack stays reachable late in the game.
- **Shop packs**: sold at the Celadon Department Store.

## 6. Phases

| Phase | Deliverable | Rough size |
|-------|-------------|------------|
| 1 ✅ | Rules engine `card_game_rules.c` + host test harness (`tools/card_game/test_rules.c`, compiled with host `cc` against stubbed `global.h` types): setup, turn flow, Energy Zone, retreat, evolution, weakness, Special Conditions, KOs, trophies, win/draw | 1000-1300 lines |
| 2 ✅ | Effects for the v1 set, card generator + overrides, ~60 Pokémon (incl. 6 ex), ~15 Trainer cards, 3 NPC decks | 1 script, 600 lines, data |
| 3 ✅ | Minimal screen: entry/exit special, board with text placeholders, menus for every action, event queue shown as messages. Playable against a random AI from a debug script | 1200 lines |
| 4 ✅ | Real graphics: board, icons, Energy pips, card detail, coin flip and damage animations | art + 500 lines |
| 5 ✅ | AI as in section 4 | 500 lines |
| 6 ✅ | Save struct, deck builder, Card Case item, pack opening | 800 lines |
| 7 ✅ | World, first wave: `cardduel` macro and shared script, Card Case and starter deck in Pewter, daily pack, ~15 NPC card players from Pallet to Celadon, the first 4 gym leaders, Card Player list | scripts, a few map objects |
| 8 ✅ | World, second wave: the rest of Kanto and its gym leaders, the Sevii Islands, the Mega Island guests and daily tournament (~35 more NPCs) | scripts, a few map objects |
| 9 ✅ | Content: the full generated card pool, more Trainers and ex | data |

**Phase 1 status (done).** `src/card_game_rules.c`, `src/card_game_effects.c`,
`include/card_game.h`, `include/constants/card_game.h`, and the host tests
(`tools/card_game/run_tests.sh`: scripted tests of each rule, plus 3000 random games checking
that every game ends, no card is ever lost and every kind of event happens). Notes for the next
phases:

- The engine already has the first 8 attack effects and 5 Trainer effects; phase 2 adds the rest.
- `src/card_game_data.c` holds 11 hand-written Pokémon (the Kanto starters' lines, Pikachu and
  Raichu) and 5 Trainers, so the game links and the screen has cards to show. Phase 2 replaces
  it with the generated pool.
- Retreating pays with Energy chosen by the engine (other types first, the Pokémon's own type
  last). The screen can let the player choose later, if needed.
- Abilities are not in yet (phase 2, with the first cards that have one).
- The engine adds about 8 KB of ROM and no static RAM: `struct CardDuel` is allocated for the
  duel.

**Phase 2 status (done).**

- `tools/card_game/gen_cards.py` writes `include/constants/card_game_cards.h` and
  `src/data/card_game/pokemon_cards.h`: **255 Pokémon cards**, every species the player can
  meet in Kanto, the Sevii Islands and Mega Island (wild, gifts, trades, statics, prize room)
  with their whole evolution families, plus **28 Mega ex** (the Megas whose line has cards).
  Rerun it after changing the overrides or the game's data.
- `src/data/card_game/pokemon_card_overrides.json` hand-tunes 52 cards: the starters, Pikachu
  and Raichu, the Pokémon of the first decks, 6 Mega ex (Venusaur, Charizard Y, Blastoise,
  Gengar, Alakazam, Pidgeot) and the 10 cards with an ability.
- The engine has 18 attack effects, 9 abilities (Powder Heal, Psy Shadow, Gas Leak, Sleep
  Pendulum, Drive Off, Water Shuriken-like, damage reduction, Rough Skin-like counter, free
  retreat; used with `CARD_ACTION_USE_ABILITY`) and **16 Trainer cards**: Potion, Poké Ball,
  X Speed, Hand Scope, Red Card, Rare Candy, Giant Cape, Rocky Helmet, Prof. Oak's Research,
  Brock, Misty, Lt. Surge, Erika, Sabrina, Blaine, Giovanni. Supporters that Pocket limits to
  named Pokémon work here for a type (Brock: Fighting, Lt. Surge: Lightning…).
- A Pokémon knocked out during its opponent's turn (by an ability) is replaced right away and
  the turn goes on (`promoteMidTurn`).
- Where Pocket lets a player choose (Sabrina, Drive Off, Red Card's shuffle), a random Bench
  Pokémon is used for now; the screen can make it a choice later.
- `src/data/card_game/decks.h`: the starter deck (Grass, Kanto bugs and birds) and three NPC
  decks (Bug Catcher: Grass, Hiker: Fighting, Swimmer: Water).
- `tools/card_game/run_tests.sh` now also builds `test_data.c`, which checks every real card
  and deck and plays random games between the decks. Random play only catches broken decks;
  balancing them waits for the AI (phase 5).
- The card game now adds about 22 KB of ROM, still no static RAM.

**Phase 3 status (done).**

- `src/card_game.c`: the duel screen. A text board (each side's trophies, deck and hand
  counts, Active and Bench with HP, Energy letters, Special Conditions and Tools, the Energy
  Zone), the engine's events played back one message at a time (A or B skips), and a menu of
  every legal action of the player, plus "Look at your hand" and "Forfeit".
- `special PlayCardGame` (`VAR_0x8004`: the opponent's `CARD_DECK_*`) fades out of the field,
  plays a duel with the starter deck and returns to the script with `VAR_RESULT` set to a
  `CARD_GAME_RESULT_*`.
- `src/card_game_ai.c`: a placeholder opponent that plays random legal actions (and its best
  attack) until phase 5.
- **To try it**: uncomment `#define CARD_GAME_DEBUG` in `include/config.h`. The START menu then
  has CARD DUEL, which plays `Debug_EventScript_CardDuel` (`data/scripts/card_duel.inc`): a duel
  against one of the three NPC decks, at random. With the define commented out the game is
  unchanged.
- Checked in mGBA: the START menu entry, the setup, many full duels with random menu choices,
  the hand view, forfeiting, the result message and the return to the field.
- Events now carry the card they were about (`CardDuelEvent.cardId`), so a message can name a
  Pokémon that has just been knocked out.

**Phase 4 status (done).**

- The table: a green board drawn from `graphics/card_game/board.png` (the opponent's half
  darker), with a slot frame for each Active and Bench place. Each half shows, from the edge
  in: the info panel (name, deck and hand counts, trophies as stars) and the Bench; then the
  Energy Zone (this turn's and the next Energy as pips), the Active and its details (name,
  HP, conditions, its Energy as pips).
- The Pokémon in play are their party icons (Mega ex use their base species' icon), the
  Active ones animated. Bench slots show HP and Energy count.
- The board follows the events as they are shown (`sScreen->display`), instead of jumping to
  the end of the action: damage, healing, knockouts, switches and trophies happen in order.
- Animations: the attacker lunges, a damaged Pokémon shakes, a knocked out (or newly played,
  evolved or switched in) one blinks, and coin flips spin a coin (heads: a Poké Ball, tails: a
  gold coin) before showing the result. A few sound effects.
- The menu is a 2-line list at the bottom with scroll arrows. SELECT (or "Look at the cards")
  opens the card viewer: the hand, then every Pokémon in play on both sides, with the front
  picture (or an item icon for Trainers that are items), stage and what it evolves from, HP,
  type, weakness, retreat cost, the Tool, the ability and the attacks with their cost and
  effect spelled out. LEFT/RIGHT to browse, B to go back.
- `tools/card_game/gen_graphics.py` draws the board tiles, the pips (an Energy of each type,
  trophies), the coin and the Energy icons; rerun it to change them.
- Attack costs, the Pokémon's type, its weakness, its retreat cost (Colorless icons, 0 when
  free) and a Trainer's type are shown as 9x9 Energy icons instead of letters and names, in the card viewer, the pack screen and the CARD CASE: a
  colored circle with its symbol, after the Energy cards of the Game Boy TCG (Darkness and
  Metal drawn to match; Dragon after the TCG's Dragon symbol, a black claw on gold). `CardGame_AppendTypeIcon`
  leaves a blank for the icon, after a `{PALETTE type}` code that prints nothing, and
  `CardGame_DrawEnergyIcons` draws the icons there once the text is printed. Their colors are
  4-15 of the text palette.
- **Vintage card test** (`src/card_game_vintage.c`): a card drawn like the Base Set's (1999), as
  a test of that look. The Test Area's card tester offers it first (special
  `OpenVintageCardTest`): it opens on Pidgeotto, LEFT/RIGHT go through the other Pokémon cards.
  A 14x20-tile portrait card: yellow border, a panel in the type's color, stage and "Evolves
  from" (when it fits), name, HP and type, the picture in a frame over a sky and ground (tall
  enough for the biggest sprites), the attacks with their damage on the right and their
  effects (left out when they don't all fit), then weakness and retreat cost. No resistance:
  this game has none. The small print is a tiny font of its own (capitals 6 pixels tall, a
  pixel less than FONT_SMALL), drawn straight into the window's pixels; its glyphs are in
  `tools/card_game/gen_vintage_font.py`, which writes `src/data/card_game/vintage_font.h`. The
  name and damage are in FONT_SMALL. The frame is drawn with rectangles on BG1, the text and
  icons on BG0, the picture between them, clipped by the hardware window 0.
- The card pictures (card viewer and pack screen) are the Pokémon Gold front sprites, Bulbasaur
  to Celebi, at their size in Gold (40 to 56 pixels, on the bottom of a 56x56 box), in
  `graphics/card_game/retro/`. `tools/card_game/gen_retro_pics.py` makes them from a checkout
  of pret/pokegold (the white around the Pokémon becomes transparent, the white inside stays).
  The Megas' pictures come from `tools/card_game/mega_sprites.png`, Megas drawn in the Gold style
  (the script names its cells in `MEGA_SHEET`). Hoenn species and Mega Dragonite, which have
  no such sprite, keep the game's front pic (`CardGame_CreatePokemonPic`). About 240 KB of ROM.
- Checked in mGBA: setup, many full duels, the coin flips (Misty's repeated flips too),
  knockouts and trophies, the card viewer on hand, Bench and opponent's cards, and a Mega ex
  card's picture and text.

**Phase 5 status (done).**

- `src/card_game_ai.c` decides one action at a time from the board alone, in a player's order:
  draw (Prof. Oak's Research, Poké Ball), Rare Candy and evolutions (the Active only if it
  keeps an attack as good), Basics to the Bench, free abilities (Powder Heal when there is
  damage, Sharp Shot on a knockout), retreat, the Energy where it brings an attack closest,
  Supporters and Items that help now (Energy Supporters, healing, Tools, Red Card), then the
  best attack: a sure knockout first (more for ex), else damage and effects, with drawbacks
  (self-damage, discards) counted. Coin flips count at their average or, for a "sure"
  knockout, as tails; it never reads the duel's random numbers.
- Levels, per deck in `CardDeckData.aiLevel` (`CARD_AI_*`): EASY plays a random action half of
  the time; NORMAL follows the rules; HARD also retreats a stuck Active or one about to be
  knocked out, plays Giovanni/Blaine/Sabrina when they make a knockout, prefers matchups on
  weaknesses, and doesn't feed Energy to an Active about to be knocked out. The Bug Catcher
  is EASY, the Hiker NORMAL, the Swimmer HARD.
- `tools/card_game/test_data.c` checks the decisions in set-up situations (knockout, evolving,
  Energy, Giovanni, retreat, promotion, setup) and plays the levels against each other with
  every deck: HARD and NORMAL beat EASY about 75% of the time. HARD and NORMAL are even with
  the current decks, which don't have the cards HARD plays better (Giovanni, Blaine, Sabrina);
  the NPC decks of phase 7-8 should use them.
- Duels against the AI now end in a few minutes, instead of reaching the turn limit.

**Phase 6 status (done).**

- Save: `struct CardGameSave` (`include/global.h`) takes the place of `SaveBlock1.unused_3598`,
  0x180 bytes, so nothing else moves. It differs a little from section 3: the Pokémon cards of
  species not caught can be received too (from packs and the starter deck), 2 bits each.
  Caught species still count as 2 copies for free. Also 3 decks with their Energy types, the
  deck in use, a flag for the starter deck, and the wins, losses and draws.
- `src/card_game_collection.c`: owned copies, giving cards, the saved decks, checking a deck
  (rules and owned copies), the results, and the pack rolls (40% Pokémon, 55% Trainers, 5% ex).
  Specials: `CardGame_GiveStarterDeck`, `CardGame_CheckActiveDeck` (VAR_RESULT),
  `OpenCardPack` and `OpenCardDeckBuilderFromScript`.
- **CARD CASE** (`ITEM_CARD_CASE`, the TM Case's icon): USE from the Key Items pocket, or
  registered to SELECT, opens the deck builder.
- `src/card_game_deck.c`: the deck builder (your cards, A adds; RIGHT: the deck, A takes out;
  SELECT: Energy types, 3 at most; L/R: decks 1-3; START: duel with this deck, if it follows the
  rules; the highlighted card described with its icon) and the pack screen (each card with its
  picture and description, and how many you have).
- Duels use the player's deck in use (the starter deck if it can't be played, for the debug
  duel) and record the result.
- The debug START menu entry (`CARD_GAME_DEBUG`) gives the CARD CASE and the starter deck the
  first time, then offers a duel, a pack or the deck builder.
- Fixed: the START menu faded to black before the debug script.
- Checked in mGBA: getting the CARD CASE, a pack, the deck builder from the bag (adding,
  removing, the rules check, the deck in use, Energy types, the other decks, back to the bag),
  and a duel with the saved deck.

**Phase 7 status (done).**

- The Running Shoes are on from the start of a new game (`NewGameInitData`). PROF. OAK's AIDE,
  who brought them in FRLG, now catches up with the player in Pewter City after the Boulder
  Badge with the CARD CASE, the starter deck and a first pack
  (`CardGame_EventScript_AideGiveCardCase`, called from the AIDE's script in
  `data/maps/PewterCity/scripts.inc`; the letter from Mom and the shoes' texts are gone). Back in
  PROF. OAK's LAB, the same AIDE (the scientist by the door) gives a pack a day
  (`FLAG_DAILY_CARD_PACK`, one of the daily flags).
- `cardduel CARD_OPPONENT_*` (`asm/macros/event.inc`) calls `Common_EventScript_CardDuel`
  (`data/scripts/card_duel.inc`) after an NPC's usual line. Without a CARD CASE: "I play
  cards too, come back with a deck". With one: the player's name is added to the Card Player
  list, the NPC's challenge (yes/no), the deck check, the duel, then the NPC's line for a win or
  a loss; the first win gives their reward card, shown on the pack screen. Draws and later wins
  give nothing. The NPC's lines are in the table, not the map scripts.
- `gCardOpponents` (`src/data/card_game/opponents.h`, `CARD_OPPONENT_*`): name, lines, deck,
  reward card and place. 16 NPCs and 4 gym leaders, each with their own deck (17 new decks in
  `decks.h`):

  | Place | NPC (their script) | Deck, level | Reward |
  |-------|--------------------|-------------|--------|
  | Pallet Town | GUS, the fat man | Normal, EASY | X Speed |
  | Viridian City | TOMMY, the boy | Nidoran, EASY | Nidoking |
  | Pewter City | RICK, the Bug Catcher with the Repel | Bugs, EASY | Scyther |
  | Pewter City Pokémon Center | DANE, the youngster | Fossils, NORMAL | Aerodactyl |
  | Route 2 (east building) | WES, the rocker | Psychic, NORMAL | Mr. Mime |
  | Route 4 Pokémon Center | ANSEL, the gentleman | Mt. Moon rocks, NORMAL | Giant Cape |
  | Cerulean City Pokémon Center | CORAL, the lass; ZACK, the rocker | Water / Lightning, NORMAL | Lapras / Electabuzz |
  | Vermilion City | DUNCAN, the sailor; MORT, the Fan Club's fat man | Sailor / Fan Club, NORMAL | Red Card / Chansey |
  | Lavender Town | IVAN, the boy | Ghosts, NORMAL | Hand Scope |
  | Celadon City | LOU, Poliwrath's owner; FLORA, the woman | Poliwag / Garden, NORMAL | Politoed / Rare Candy |
  | Celadon Game Corner | BERT, the balding man; VERA, a woman; OTTO, the old man | Rocket / Tech / Fire, HARD | Giovanni / Porygon / Mega Charizard Y ex |
  | Gyms | BROCK, MISTY, LT. SURGE, ERIKA, after their badge (and in their post-game rematch scripts, when the battle is declined or done for the day) | Their type: NORMAL, NORMAL, HARD, HARD | Their Supporter |

  The map scripts are edited inside the generated Kanto sections, each line marked
  `@ added: card game`.
- Card Player list: LEFT from the player's cards in the CARD CASE. Each player met, WON once
  beaten; on the right, the place, their Energy, beaten or not, and the prize card once won.
  The save struct takes `opponentsMet` (10 bytes) from its unused space.
- `special PlayCardGame` takes a card player (`CARD_OPPONENT_*`) instead of a deck; the duel
  shows their name.
- AI: HARD did worse than NORMAL with the new decks (44% against NORMAL). Two of its
  extras cost more games than they saved: retreating a Pokémon about to be knocked out, and
  holding back all Energy from it. HARD now retreats only a stuck Active, and gives Energy to
  one about to be knocked out when it lets it attack first: 50-51% against NORMAL.
- Balance: `tools/card_game/test_data.c` now plays the starter deck (NORMAL, for the player)
  against every card player at their level. The EASY ones must lose to it (they do, 71-100%);
  the first leaders are beatable (BROCK about 30%, MISTY 50%), the Celadon players are meant
  for decks improved with caught Pokémon and packs. The starter deck lost its Weedle line for
  Spearow/Fearow and a second Pidgeotto (it was the weakest deck of all); Geodude's attack
  costs 2 Energy (it did 40 for 1, which made Brock beat the starter 85% of the time); the
  Swimmer deck is NORMAL. Every line of the card players is checked to fit the message box.
- Fixed: Blaine's card read "...30 more damage.FIRE." (now "...with POKéMON of this type:
  FIRE.").
- Checked in mGBA (with a test build): running from the start; the AIDE in Pewter (the CARD
  CASE, the starter deck, the first pack, the AIDE leaving); a card player (usual line,
  challenge, duel, win line, reward card and its screen; a second win without a reward); the
  Card Player list; the daily pack and "come back tomorrow"; BROCK's card duel after the badge.

**Phase 8 status (done).**

- 31 more card players, all through `cardduel` (51 in all; `opponentsBeaten` has room for 80),
  each with their own deck (31 new decks) and a reward found nowhere else but packs:

  | Place | Card player (NPC) | Deck, level | Reward |
  |-------|-------------------|-------------|--------|
  | Saffron City | NIGEL (Pokémon Center gentleman); KOICHI (the Dojo's master, after his battle); the COPYCAT | Psychic ex, HARD; Fighting, HARD; **a copy of the player's own deck**, NORMAL | Mega Alakazam ex; Hitmontop; Ditto |
  | Fuchsia City | GRAMPS (Bill's grandfather); PAIGE (Pokémon Center) | Eeveelutions, NORMAL; Safari Zone, HARD | Umbreon; Mega Kangaskhan ex |
  | Route 17 | BILLY, a biker (after his battle) | Poison, NORMAL | Rocky Helmet |
  | Cinnabar Island | GIDEON (Lab lounge); NED (Pokémon Center) | Fossil ex, HARD; Seafoam ice, HARD | Mega Aerodactyl ex; Articuno |
  | Indigo Plateau | TAKEO, the black belt AGATHA beat (after the Hall of Fame) | Ghosts, HARD | Mega Gengar ex |
  | Gyms | KOGA, SABRINA, BLAINE after their badge; GIOVANNI in the talk where he leaves his gym; all four in their post-game rematch scripts too | Their type, HARD | Their Supporter (KOGA is a new card) |
  | One Island, Kindle Road | ROCKO (Pokémon Center hiker); HANAKO (Ember Spa) | Mt. Ember, HARD; Spa, NORMAL | Moltres; Blissey |
  | Two Island Game Corner | GORDON, Lostelle's father (once she's home); LOSTELLE | Lightning, HARD; Cute, NORMAL | Zapdos; Togetic |
  | Three to Six Island | MARLON (Three Island Pokémon Center); LORELEI at her house; WALDO (Four Island Pokémon Center); DAHLIA (Five Island Pokémon Center); ELDON (Six Island Pokémon Center) | Deep sea, HARD; Ice, HARD; Babies, NORMAL; Bloom, HARD; Ruins, HARD | Mega Blastoise ex; Mega Slowbro ex; Togepi; Mega Venusaur ex; Mega Tyranitar ex |
  | Seven Island | RODETTE, by the Trainer Tower (after her battle) | Dragons, HARD | Mega Rayquaza ex |
  | Mega Island castle | {RIVAL}, LEON, LANCE, STEVEN, CYNTHIA, ALDER, DIANTHA (the guests); MR. M. (the tournament's final only) | Their teams, as far as the card pool goes, HARD | Their Mega ex (Pidgeot, Charizard X, Dragonite, Steelix, Latias, Heracross, Latios); Mega Mewtwo X ex |

  The Game Corner rocket (BERT) now gives Houndour: the Giovanni card is Giovanni's.
- **KOGA**: a new Supporter (`CARD_TRAINER_POISON_OPPONENT`): the opponent's Active is
  Poisoned. The AI plays it on an Active it can't knock out this turn. Tested in `test_rules`.
- **All 8 leaders**: the last leader beaten at cards also gives `CARD_LEADERS_BONUS` (Mega
  Gyarados ex), shown with their own card (`CARD_SAVE_FLAG_ALL_LEADERS`).
- **Mega Island's card tournament**: a card dealer at the table of the castle's entrance hall
  (a new object in `MegaIsland_Castle_1F/map.json`), once a day (`FLAG_DAILY_CARD_TOURNAMENT`):
  2 guests at random, then MR. M., in a row. Each duel counts as one with that player (met,
  beaten, reward); the champion gets a pack.
- **More packs**: the Two Island Game Corner's info man is a card fan with a pack a day
  (`FLAG_DAILY_CARD_PACK_2`), and the Celadon Dept. Store's 4F clerk sells packs, ¥500
  (`CARD_PACK_PRICE`), before the usual shop.
- Opponents' names can have placeholders (`{RIVAL}`): the duel and the Card Player list fill
  them in (`CardGame_GetOpponentName`).
- Mega Charizard and Mega Mewtwo cards are named with their X or Y.
- Balance (`tools/card_game/test_data.c` and a deck-against-all-decks rating): the late decks
  rate 50-80% against every deck with the AI on both sides. Giovanni, {RIVAL}, Cynthia and Lance
  were first among the weakest (32-41%), with 3 Energy types or slow lines; they were reworked to
  50-58%. MR. M. is the strongest deck (79%). The AI-against-AI games were halved to keep the
  tests near 2 minutes with 52 decks.
- Checked in mGBA (with a test build): the card tournament from the dealer to the champion's
  pack (with LEON's and ALDER's rewards and MR. M.'s Mega Mewtwo), the 8-leaders bonus (7
  leaders set as beaten, then BROCK: both cards and their screen), the Celadon shop (two packs
  bought), and the COPYCAT playing the starter deck back.

**Phase 9 status (done).**

- The generated pool was already complete in phase 2: every species met in Kanto, the Sevii
  Islands and Mega Island, with their families (227 cards). Phase 9 adds the rest of the plan's
  content: **15 ex cards of Pokémon without a Mega** (as in Pocket: Pikachu, Mewtwo, Articuno,
  Zapdos, Moltres, Lapras, Starmie, Arcanine, Marowak, Wigglytuff, Exeggutor, Machamp, Dragonite,
  Lugia, Ho-Oh) and **11 Trainer cards**. 270 Pokémon cards (43 ex) and 28 Trainers in all.
- The ex cards (`EXTRA_EX` in `gen_cards.py`) are the Pokémon's card with 30 more HP and 20
  more damage per attack; Pikachu ex and Starmie ex are hand-tuned in the overrides (their plain
  cards are hand-tuned weaker ones). They come after the Megas, so no saved card moves. A first
  try at Pocket-like ex designs came out weaker than the generated legendaries' plain cards and
  cost the decks that used them (Moltres ex: 39% against 45% for the plain card), so the ex keep
  the plain card's attacks, stronger.
- New Trainers, with 7 new effects (`CARD_TRAINER_*` 16-23; Koga's became the general
  `STATUS_OPPONENT`):

  | Card | Kind | Effect |
  |------|------|--------|
  | BILL | Supporter | A random Pokémon (any stage) from your deck into your hand |
  | BLUE | Supporter | During your opponent's next turn, your Pokémon take 10 less damage from attacks |
  | LORELEI | Supporter | The opponent's Active is Asleep |
  | BRUNO | Supporter | This turn, Fighting attacks do 20 more damage |
  | AGATHA | Supporter | The opponent's Active is Confused |
  | LANCE | Supporter | This turn, Dragon attacks do 30 more damage |
  | DAISY | Supporter | Heal 20 from each of your Pokémon |
  | CELIO | Supporter | Draw until you have 5 cards in your hand |
  | FULL HEAL | Item | Your Active recovers from all Special Conditions |
  | LEFTOVERS | Tool | At the end of your turn, as your Active, heal 10 |
  | CHOICE BAND | Tool | Its attacks do 10 more damage to the opponent's Active |

  Each is tested in `test_rules`; the AI plays them (Celio with a small hand, Bill with no
  Pokémon in hand, Blue when the opponent's Active can attack, Daisy with 2 damaged Pokémon, Full
  Heal on any Special Condition, the Tools on the best Pokémon, the conditions on an Active it
  can't knock out), with decision scenarios in `test_data`. They come from packs, and the late
  card players use them (BRUNO at the Dojo, AGATHA with the Indigo veteran and MR. M., LORELEI and
  Lapras ex at her house, LANCE with LANCE and RODETTE, BLUE with the rival, BILL with Bill's
  grandfather, DAISY at the Ember Spa, CELIO on Five Island, the Tools and the bird ex on the
  Sevii Islands, Mewtwo ex with MR. M.).
- **Trainer ids**: the Trainers now start at a fixed `CARD_TRAINER_START` (300) instead of right
  after the Pokémon cards, so more Pokémon cards will never move them again. Saved decks hold card
  ids: `CardGameSave.version` (from the unused bytes) updates older saves once, moving their
  decks' Trainers from the old ids (checked in mGBA: an old Potion stays a Potion).
- Card texts: Supporters read "SUPPORTER (1 a turn)" on two lines, which no longer runs into the
  card's name in the Card Case.
- Balance: the late decks that took the new cards stay within a few points of before, apart from
  the Game Corner's Zapdos ex deck (53% to 63%).

**After phase 9: the pack screen and the difficulty curve.**

- Pack screen: the card's text no longer runs past the bottom of its window (Mega Charizard X
  ex). The window goes down to the bottom of the screen with the help in the header, the stage,
  type and HP share a line, and the lines move closer together only for a card with too many. It
  now shows the ability's text (it only said ABILITY) and the ex rule.
- Difficulty along the story, from the player's side (`PlayTheStory` in
  `tools/card_game/test_data.c`): every card player against the decks a player is likely to have
  where they are met (`sProxyDecks`: for each stage, a Grass, a Fire and a Water starter's deck
  with the species caught by then and the rewards of the card players before; `sOpponentStages`),
  the player's deck played by NORMAL. It replaces the starter-deck-only report, which made the
  later players look impossible.
- What it showed and what changed:
  - The Fighting players around Pewter (DANE, ANSEL, BROCK) beat an early deck 70-85% of the
    time: early decks are mostly Normal Pokémon, weak to Fighting. Between EASY (a random action
    half of the time) and NORMAL there was nothing, a 60-point gap, so there is a new level,
    **CASUAL** (a random action an eighth of the time). BROCK, ANSEL and DANE play CASUAL: 44%,
    42%, 65% for the player.
  - Too strong for where they stand: ZACK and MORT (20%), LOU (27%), OTTO (13%), NED (14%, the
    Articuno ex of phase 9), GORDON and MARLON (21-23%), WES (36%), DUNCAN (37%). Lighter decks
    (fewer evolved Pokémon, no Rare Candy or Mega there), plain Articuno and Zapdos instead of the
    ex, NORMAL instead of HARD for OTTO, NED, MARLON and LT. SURGE.
  - Leaders: BROCK 44%, MISTY 38%, LT. SURGE 40%, ERIKA 52%, KOGA 51%, SABRINA 43%, BLAINE 37%
    (a lighter deck), GIOVANNI 56% (with 2 Mega Tyranitar, 2 Mega Aerodactyl and Marowak ex he is
    still the 8th leader of a Grass player's weakness; the differences under ~10 points are noise
    at these game counts).
  - Now: EASY players 71-94% for the player, the others mostly 40-65%, OTTO (who gives a Mega)
    29%, the champions 52-65%, LORELEI 31%, MR. M. 25%.
- A first Fire player's deck made Fire and Lightning Energy and lost to almost everyone, even
  Grass decks: the Fire decks of the report make Fire only. Proxy decks are rough, so the report
  is a guide, not a rule; the checks only keep the EASY players beatable by the starter deck.

**After phase 9: one copy per catch, and the daily pack at the POKé MARTs.**

- A caught species is **one** copy of its card (it was two, the most a deck can hold); the
  second comes from packs, the starter deck or an opponent's reward. `CardGame_GiveCard` counts
  the copies received apart from the catch, so a Pokémon reward is no longer lost on a caught
  species.
- Saves: `CARD_SAVE_VERSION` 2. Each Pokémon card a saved deck holds more copies of than the
  player now owns is received until it fits, so no deck stops being playable. The version 1
  Trainer id move now only runs on version 0 saves.
- Packs: 60% Pokémon, 35% Trainers, 5% ex (40/55/5 before; `CARD_PACK_*_CHANCE`). Half of the
  Pokémon are the second copy of a caught species the player has one copy of, while there is one
  (`CARD_PACK_SECOND_COPY_CHANCE`); the others are any Pokémon card, as before.
- The daily pack moved from PROF. OAK's AIDE in the LAB to the POKé MART clerks of Kanto and
  the Sevii Islands (the town marts, the Celadon Dept. Store's clerks, the Indigo Plateau, Two
  Island's shop and the Trainer Tower): the first clerk talked to that day gives it, before the
  shop. Once it's taken, the clerks say nothing about it. The AIDE's lines in Pewter say so.

Phases 1-3 are the core and should land first, behind a debug script so the main game is
untouched until phase 7. There is no link phase: the game is played only against NPCs.

## 7. Risks and checks

- **Rules engine size**: Pocket is far richer than a Gwent-style game (attacks, effects,
  statuses, evolution). Keeping the rules pure and host-tested is what makes it
  manageable.
- **ROM space**: ~300 cards × ~24 bytes ≈ 7 KB, plus attack names (shared strings with
  `gMoveNames` where the attack is a move), code and art. Check `make` still fits the
  ROM after phases 4 and 9.
- **Save layout**: `STATIC_ASSERT` the struct size; never move other SaveBlock1 fields.
- **Pokédex dependency**: a player who has caught few species has few cards, and Hoenn-only
  species are left out of the pool entirely (D2b). The starter
  deck covers that, and Trainer cards come from packs, not catches.
- **Balance**: a "show opponent's hand" debug toggle, and a self-play mode on the host
  harness (AI vs AI, 1000 games per deck pair) to find dominant decks before tuning.
- **NPC decks**: ~60 hand-built 20-card decks is real work. A small script can build a
  first draft per NPC from a type, a level band and the local route's encounters
  (`src/data/wild_encounters.json`), to be tuned by hand.
- **Readability**: HP, Energy and trophies must be readable at a glance at 240x160;
  playtest on mGBA at 1x before adding more information.
