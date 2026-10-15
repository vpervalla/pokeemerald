#ifndef GUARD_CONSTANTS_CARD_GAME_H
#define GUARD_CONSTANTS_CARD_GAME_H

// The card game (see docs/card_game_plan.md), with the rules of Pokémon TCG Pocket.

#define CARD_DECK_SIZE          20
#define CARD_MAX_COPIES         2   // of the same card in a deck
#define CARD_OPENING_HAND       5
#define CARD_HAND_MAX           10
#define CARD_BENCH_MAX          3
#define CARD_WIN_TROPHIES       3
#define CARD_TURN_LIMIT         30  // turns played by both players together; then the duel is a draw
#define CARD_MAX_ZONE_TYPES     3   // Energy types a deck's Energy Zone can make
#define CARD_ATTACK_MAX_COST    5
#define CARD_MAX_ATTACKS        2
#define CARD_WEAKNESS_BONUS     20
#define CARD_POISON_DAMAGE      10
#define CARD_BURN_DAMAGE        20

// Sides of a duel
#define CARD_SIDE_PLAYER        0
#define CARD_SIDE_OPPONENT      1
#define CARD_SIDE_COUNT         2
#define CARD_SIDE_NONE          0xFF
#define CARD_RESULT_DRAW        0xFE  // CardDuel.winner when both sides win or the turn limit is reached

// Slots of a side's Pokémon in play
#define CARD_SLOT_ACTIVE        0
#define CARD_SLOT_BENCH_1       1
#define CARD_SLOT_COUNT         (1 + CARD_BENCH_MAX)
#define CARD_SLOT_NONE          0xFF

// Card (and Energy) types
#define CARD_TYPE_NONE          0
#define CARD_TYPE_GRASS         1
#define CARD_TYPE_FIRE          2
#define CARD_TYPE_WATER         3
#define CARD_TYPE_LIGHTNING     4
#define CARD_TYPE_PSYCHIC       5
#define CARD_TYPE_FIGHTING      6
#define CARD_TYPE_DARKNESS      7
#define CARD_TYPE_METAL         8
#define CARD_TYPE_DRAGON        9   // no Energy of its own
#define CARD_TYPE_COLORLESS     10  // in an attack's cost: any Energy
#define CARD_TYPE_COUNT         11

// Kinds of card
#define CARD_KIND_NONE          0
#define CARD_KIND_POKEMON       1
#define CARD_KIND_ITEM          2
#define CARD_KIND_SUPPORTER     3
#define CARD_KIND_TOOL          4

#define CARD_STAGE_BASIC        0
#define CARD_STAGE_1            1
#define CARD_STAGE_2            2

// Special Conditions (CardInPlay.status). Asleep, Paralyzed and Confused replace each other;
// Poisoned and Burned stack with them.
#define CARD_STATUS_ASLEEP      (1 << 0)
#define CARD_STATUS_PARALYZED   (1 << 1)
#define CARD_STATUS_CONFUSED    (1 << 2)
#define CARD_STATUS_POISONED    (1 << 3)
#define CARD_STATUS_BURNED      (1 << 4)
#define CARD_STATUS_ROTATION    (CARD_STATUS_ASLEEP | CARD_STATUS_PARALYZED | CARD_STATUS_CONFUSED)

// Attack effects (CardAttack.effect), see src/card_game_effects.c
#define CARD_EFFECT_NONE                0
#define CARD_EFFECT_STATUS              1   // the defender gets status effectArg
#define CARD_EFFECT_FLIP_STATUS         2   // flip: heads, the defender gets status effectArg
#define CARD_EFFECT_HEAL_SELF           3   // heal effectArg from the attacker
#define CARD_EFFECT_SELF_DAMAGE         4   // effectArg damage to the attacker
#define CARD_EFFECT_FLIP_BONUS          5   // flip: heads, effectArg more damage
#define CARD_EFFECT_FLIP_OR_FAIL        6   // flip: tails, the attack does nothing
#define CARD_EFFECT_DISCARD_ENERGY      7   // discard effectArg Energy of the attacker's type from it
#define CARD_EFFECT_DISCARD_ALL_ENERGY  8   // discard all Energy from the attacker

// Trainer effects (TrainerCardInfo.effect)
#define CARD_TRAINER_DRAW               1   // draw effectArg cards
#define CARD_TRAINER_HEAL               2   // heal effectArg from one of your Pokémon
#define CARD_TRAINER_POKE_BALL          3   // put a random Basic Pokémon from your deck into your hand
#define CARD_TRAINER_RETREAT_DISCOUNT   4   // the Active's retreat cost is effectArg less this turn
#define CARD_TRAINER_TOOL_HP            5   // Tool: the Pokémon gets effectArg more HP

// Deck validation (Duel_ValidateDeck)
#define CARD_DECK_OK                0
#define CARD_DECK_WRONG_SIZE        1
#define CARD_DECK_TOO_MANY_COPIES   2
#define CARD_DECK_NO_BASIC          3
#define CARD_DECK_UNKNOWN_CARD      4
#define CARD_DECK_NO_ENERGY_TYPE    5

// Card ids. Phase 2 generates the full list; these are the first cards, used to try the engine.
#define CARD_NONE                   0
#define CARD_BULBASAUR              1
#define CARD_IVYSAUR                2
#define CARD_VENUSAUR               3
#define CARD_CHARMANDER             4
#define CARD_CHARMELEON             5
#define CARD_CHARIZARD              6
#define CARD_SQUIRTLE               7
#define CARD_WARTORTLE              8
#define CARD_BLASTOISE              9
#define CARD_PIKACHU                10
#define CARD_RAICHU                 11
#define CARD_POKEMON_END            12
#define CARD_POTION                 (CARD_POKEMON_END + 0)
#define CARD_POKE_BALL              (CARD_POKEMON_END + 1)
#define CARD_X_SPEED                (CARD_POKEMON_END + 2)
#define CARD_PROF_OAKS_RESEARCH     (CARD_POKEMON_END + 3)
#define CARD_GIANT_CAPE             (CARD_POKEMON_END + 4)
#define CARD_COUNT                  (CARD_POKEMON_END + 5)

#endif // GUARD_CONSTANTS_CARD_GAME_H
