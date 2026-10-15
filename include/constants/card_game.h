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
#define CARD_EX_TROPHIES        2   // knocking out an ex
#define CARD_MEGA_EX_TROPHIES   3   // knocking out a Mega ex, as in Pocket's Mega Rising set

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
#define CARD_EFFECT_FLIP_MULTI          9   // flip effectArg coins: the attack's damage for each heads
#define CARD_EFFECT_SNIPE               10  // the damage goes to the opponent's Pokémon in the action's slot
#define CARD_EFFECT_SPREAD_BENCH        11  // also effectArg damage to each of the opponent's Benched Pokémon
#define CARD_EFFECT_SELF_BENCH_DAMAGE   12  // also effectArg damage to each of your Benched Pokémon
#define CARD_EFFECT_FLIP_DISCARD_ENERGY 13  // flip: heads, discard a random Energy from the defender
#define CARD_EFFECT_CANT_ATTACK_NEXT    14  // the attacker can't attack during your next turn
#define CARD_EFFECT_BONUS_EXTRA_ENERGY  15  // effectArg more damage with 2 extra Energy of its type beyond the cost
#define CARD_EFFECT_DRAW                16  // draw effectArg cards
#define CARD_EFFECT_SWITCH_DEFENDER     17  // after the damage, a random opponent's Benched Pokémon switches in
#define CARD_EFFECT_BONUS_IF_DAMAGED    18  // effectArg more damage if the defender already has damage
#define CARD_EFFECT_COUNT               19

// Abilities (PokemonCardInfo.ability, with PokemonCardInfo.abilityArg). The first ones are used
// once a turn with CARD_ACTION_USE_ABILITY; the passive ones work by themselves.
#define CARD_ABILITY_NONE               0
#define CARD_ABILITY_HEAL_ALL           1   // heal abilityArg from each of your Pokémon
#define CARD_ABILITY_ATTACH_OWN_TYPE    2   // attach an Energy of this Pokémon's type to your Active Pokémon of that type
#define CARD_ABILITY_POISON_DEFENDER    3   // if this is your Active Pokémon: the opponent's Active is Poisoned
#define CARD_ABILITY_FLIP_SLEEP         4   // flip: heads, the opponent's Active is Asleep
#define CARD_ABILITY_DRIVE_OFF          5   // if this is your Active Pokémon: a random opponent's Benched Pokémon switches in
#define CARD_ABILITY_SNIPE              6   // abilityArg damage to the opponent's Pokémon in the action's arg
#define CARD_ABILITY_FIRST_PASSIVE      7
#define CARD_ABILITY_REDUCE_DAMAGE      7   // takes abilityArg less damage from attacks
#define CARD_ABILITY_COUNTER            8   // as the Active, damaged by an attack: abilityArg damage to the attacker
#define CARD_ABILITY_FREE_RETREAT       9   // no retreat cost
#define CARD_ABILITY_COUNT              10

// Trainer effects (TrainerCardInfo.effect)
#define CARD_TRAINER_DRAW               1   // draw effectArg cards
#define CARD_TRAINER_HEAL               2   // heal effectArg from one of your Pokémon
#define CARD_TRAINER_POKE_BALL          3   // put a random Basic Pokémon from your deck into your hand
#define CARD_TRAINER_RETREAT_DISCOUNT   4   // the Active's retreat cost is effectArg less this turn
#define CARD_TRAINER_TOOL_HP            5   // Tool: the Pokémon gets effectArg more HP
#define CARD_TRAINER_HAND_SCOPE         6   // the opponent reveals their hand
#define CARD_TRAINER_RED_CARD           7   // the opponent shuffles their hand into their deck and draws effectArg
#define CARD_TRAINER_RARE_CANDY         8   // a Basic evolves straight into the Stage 2 in hand (the action's arg)
#define CARD_TRAINER_TOOL_COUNTER       9   // Tool: as the Active, damaged by an attack: effectArg damage to the attacker
#define CARD_TRAINER_ATTACH_ENERGY      10  // attach an Energy of effectType to one of your effectType Pokémon
#define CARD_TRAINER_FLIP_ATTACH_ENERGY 11  // one of your effectType Pokémon: flip until tails, an effectType Energy per heads
#define CARD_TRAINER_GATHER_ENERGY      12  // move every effectType Energy from your Bench to your effectType Active
#define CARD_TRAINER_HEAL_TYPE          13  // heal effectArg from one of your effectType Pokémon
#define CARD_TRAINER_SWITCH_OPPONENT    14  // a random opponent's Benched Pokémon switches in
#define CARD_TRAINER_ATTACK_BONUS       15  // this turn, attacks of effectType (NONE: all) do effectArg more damage

// Deck validation (Duel_ValidateDeck)
#define CARD_DECK_OK                0
#define CARD_DECK_WRONG_SIZE        1
#define CARD_DECK_TOO_MANY_COPIES   2
#define CARD_DECK_NO_BASIC          3
#define CARD_DECK_UNKNOWN_CARD      4
#define CARD_DECK_NO_ENERGY_TYPE    5

// Card ids: the Pokémon cards are generated (tools/card_game/gen_cards.py), the Trainers follow
#define CARD_NONE                   0
#include "constants/card_game_cards.h"
#define CARD_POTION                 (CARD_POKEMON_END + 0)
#define CARD_POKE_BALL              (CARD_POKEMON_END + 1)
#define CARD_X_SPEED                (CARD_POKEMON_END + 2)
#define CARD_HAND_SCOPE             (CARD_POKEMON_END + 3)
#define CARD_RED_CARD               (CARD_POKEMON_END + 4)
#define CARD_RARE_CANDY             (CARD_POKEMON_END + 5)
#define CARD_GIANT_CAPE             (CARD_POKEMON_END + 6)
#define CARD_ROCKY_HELMET           (CARD_POKEMON_END + 7)
#define CARD_PROF_OAKS_RESEARCH     (CARD_POKEMON_END + 8)
#define CARD_BROCK                  (CARD_POKEMON_END + 9)
#define CARD_MISTY                  (CARD_POKEMON_END + 10)
#define CARD_LT_SURGE               (CARD_POKEMON_END + 11)
#define CARD_ERIKA                  (CARD_POKEMON_END + 12)
#define CARD_SABRINA                (CARD_POKEMON_END + 13)
#define CARD_BLAINE                 (CARD_POKEMON_END + 14)
#define CARD_GIOVANNI               (CARD_POKEMON_END + 15)
#define CARD_COUNT                  (CARD_POKEMON_END + 16)

// Decks (gCardDecks)
#define CARD_DECK_STARTER           0   // the player's first deck, with the Card Case
#define CARD_DECK_BUG_CATCHER       1
#define CARD_DECK_HIKER             2
#define CARD_DECK_SWIMMER           3
#define CARD_DECKS_COUNT            4

#endif // GUARD_CONSTANTS_CARD_GAME_H
