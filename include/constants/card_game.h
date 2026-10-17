#ifndef GUARD_CONSTANTS_CARD_GAME_H
#define GUARD_CONSTANTS_CARD_GAME_H

// The card game (see docs/card_game_plan.md), with the rules of Pokémon TCG Pocket.

#define CARD_DECK_SIZE              20
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
#define CARD_TRAINER_STATUS_OPPONENT    16  // the opponent's Active gets status effectArg (CARD_STATUS_*)
#define CARD_TRAINER_SEARCH_POKEMON     17  // put a random Pokémon (any stage) from your deck into your hand
#define CARD_TRAINER_SHIELD             18  // during the opponent's next turn, your Pokémon take effectArg less damage from attacks
#define CARD_TRAINER_HEAL_ALL           19  // heal effectArg from each of your Pokémon
#define CARD_TRAINER_DRAW_UNTIL         20  // draw until you have effectArg cards in your hand
#define CARD_TRAINER_CURE               21  // your Active recovers from all Special Conditions
#define CARD_TRAINER_TOOL_HEAL          22  // Tool: at the end of your turn, as your Active, heal effectArg
#define CARD_TRAINER_TOOL_DAMAGE        23  // Tool: its attacks do effectArg more damage to the opponent's Active

// Deck validation (Duel_ValidateDeck)
#define CARD_DECK_OK                0
#define CARD_DECK_WRONG_SIZE        1
#define CARD_DECK_TOO_MANY_COPIES   2
#define CARD_DECK_NO_BASIC          3
#define CARD_DECK_UNKNOWN_CARD      4
#define CARD_DECK_NO_ENERGY_TYPE    5
#define CARD_DECK_NOT_OWNED         6   // more copies of a card than the player has

// CardGameSave.flags
#define CARD_SAVE_FLAG_STARTER      (1 << 0)    // got the starter deck
#define CARD_SAVE_FLAG_ALL_LEADERS  (1 << 1)    // got CARD_LEADERS_BONUS

// CardGameSave.version: 1 when the Trainer cards moved to CARD_TRAINER_START (they followed the
// Pokémon cards, from 256)
#define CARD_SAVE_VERSION           1
#define CARD_OLD_TRAINER_START      256

#define CARD_SAVED_DECKS            3
#define CARD_PACK_PRICE             500 // at the Celadon Dept. Store
#define CARD_PACK_SIZE              5

// Card ids: the Pokémon cards are generated (tools/card_game/gen_cards.py), the Trainers start
// at CARD_TRAINER_START, so new Pokémon cards don't move them (the saved decks hold card ids)
#define CARD_NONE                   0
#include "constants/card_game_cards.h"
#define CARD_TRAINER_START          300
#define CARD_POTION                 (CARD_TRAINER_START + 0)
#define CARD_POKE_BALL              (CARD_TRAINER_START + 1)
#define CARD_X_SPEED                (CARD_TRAINER_START + 2)
#define CARD_HAND_SCOPE             (CARD_TRAINER_START + 3)
#define CARD_RED_CARD               (CARD_TRAINER_START + 4)
#define CARD_RARE_CANDY             (CARD_TRAINER_START + 5)
#define CARD_GIANT_CAPE             (CARD_TRAINER_START + 6)
#define CARD_ROCKY_HELMET           (CARD_TRAINER_START + 7)
#define CARD_PROF_OAKS_RESEARCH     (CARD_TRAINER_START + 8)
#define CARD_BROCK                  (CARD_TRAINER_START + 9)
#define CARD_MISTY                  (CARD_TRAINER_START + 10)
#define CARD_LT_SURGE               (CARD_TRAINER_START + 11)
#define CARD_ERIKA                  (CARD_TRAINER_START + 12)
#define CARD_SABRINA                (CARD_TRAINER_START + 13)
#define CARD_BLAINE                 (CARD_TRAINER_START + 14)
#define CARD_GIOVANNI               (CARD_TRAINER_START + 15)
#define CARD_KOGA                   (CARD_TRAINER_START + 16)
#define CARD_BILL                   (CARD_TRAINER_START + 17)
#define CARD_BLUE                   (CARD_TRAINER_START + 18)
#define CARD_LORELEI                (CARD_TRAINER_START + 19)
#define CARD_BRUNO                  (CARD_TRAINER_START + 20)
#define CARD_AGATHA                 (CARD_TRAINER_START + 21)
#define CARD_LANCE                  (CARD_TRAINER_START + 22)
#define CARD_DAISY                  (CARD_TRAINER_START + 23)
#define CARD_CELIO                  (CARD_TRAINER_START + 24)
#define CARD_FULL_HEAL              (CARD_TRAINER_START + 25)
#define CARD_LEFTOVERS              (CARD_TRAINER_START + 26)
#define CARD_CHOICE_BAND            (CARD_TRAINER_START + 27)
#define CARD_COUNT                  (CARD_TRAINER_START + 28)

// The opponent's skill (src/card_game_ai.c)
#define CARD_AI_EASY                0   // a random action half of the time
#define CARD_AI_CASUAL              1   // a random action an eighth of the time
#define CARD_AI_NORMAL              2
#define CARD_AI_HARD                3

// VAR_RESULT after `special PlayCardGame`
#define CARD_GAME_RESULT_LOST       0
#define CARD_GAME_RESULT_WON        1
#define CARD_GAME_RESULT_DRAW       2
#define CARD_GAME_RESULT_FORFEITED  3

// Decks (gCardDecks)
#define CARD_DECK_STARTER           0   // the player's first deck, with the Card Case
#define CARD_DECK_BUG_CATCHER       1
#define CARD_DECK_HIKER             2
#define CARD_DECK_SWIMMER           3
#define CARD_DECK_NEIGHBOR          4
#define CARD_DECK_NIDO_KID          5
#define CARD_DECK_FOSSIL_FAN        6
#define CARD_DECK_PSYCHIC           7
#define CARD_DECK_ELECTRIC          8
#define CARD_DECK_SAILOR            9
#define CARD_DECK_FAN_CLUB          10
#define CARD_DECK_GHOST             11
#define CARD_DECK_POLIWAG           12
#define CARD_DECK_GARDEN            13
#define CARD_DECK_ROCKET            14
#define CARD_DECK_TECH              15
#define CARD_DECK_FIRE              16
#define CARD_DECK_BROCK             17
#define CARD_DECK_MISTY             18
#define CARD_DECK_LT_SURGE          19
#define CARD_DECK_ERIKA             20
#define CARD_DECK_PSYCHIC_EX        21
#define CARD_DECK_DOJO              22
#define CARD_DECK_COPYCAT           23
#define CARD_DECK_EEVEE             24
#define CARD_DECK_SAFARI            25
#define CARD_DECK_BIKER             26
#define CARD_DECK_FOSSIL_EX         27
#define CARD_DECK_SEAFOAM           28
#define CARD_DECK_VETERAN           29
#define CARD_DECK_KOGA              30
#define CARD_DECK_SABRINA           31
#define CARD_DECK_BLAINE            32
#define CARD_DECK_GIOVANNI          33
#define CARD_DECK_EMBER             34
#define CARD_DECK_SPA               35
#define CARD_DECK_JOYFUL            36
#define CARD_DECK_CUTE              37
#define CARD_DECK_DEEP_SEA          38
#define CARD_DECK_LORELEI           39
#define CARD_DECK_BABIES            40
#define CARD_DECK_BLOOM             41
#define CARD_DECK_RUINS             42
#define CARD_DECK_SKY               43
#define CARD_DECK_BLUE              44
#define CARD_DECK_LEON              45
#define CARD_DECK_LANCE             46
#define CARD_DECK_STEVEN            47
#define CARD_DECK_CYNTHIA           48
#define CARD_DECK_ALDER             49
#define CARD_DECK_DIANTHA           50
#define CARD_DECK_MR_M              51
#define CARD_DECKS_COUNT            52

// Card players in the world (gCardOpponents), for `cardduel` and `special PlayCardGame`.
// Their order is the Card Player list's; saved as bits, so never reorder them.
#define CARD_OPPONENT_PALLET_FAT_MAN            0
#define CARD_OPPONENT_VIRIDIAN_BOY              1
#define CARD_OPPONENT_PEWTER_BUG_CATCHER        2
#define CARD_OPPONENT_PEWTER_PC_YOUNGSTER       3
#define CARD_OPPONENT_ROUTE2_ROCKER             4
#define CARD_OPPONENT_ROUTE4_PC_GENTLEMAN       5
#define CARD_OPPONENT_CERULEAN_PC_LASS          6
#define CARD_OPPONENT_CERULEAN_PC_ROCKER        7
#define CARD_OPPONENT_VERMILION_SAILOR          8
#define CARD_OPPONENT_FAN_CLUB_FAT_MAN          9
#define CARD_OPPONENT_LAVENDER_BOY              10
#define CARD_OPPONENT_CELADON_FAT_MAN           11
#define CARD_OPPONENT_CELADON_WOMAN             12
#define CARD_OPPONENT_GAME_CORNER_BALDING_MAN   13
#define CARD_OPPONENT_GAME_CORNER_WOMAN         14
#define CARD_OPPONENT_GAME_CORNER_OLD_MAN       15
#define CARD_OPPONENT_BROCK                     16
#define CARD_OPPONENT_MISTY                     17
#define CARD_OPPONENT_LT_SURGE                  18
#define CARD_OPPONENT_ERIKA                     19
#define CARD_OPPONENT_SAFFRON_PC_GENTLEMAN      20
#define CARD_OPPONENT_DOJO_MASTER               21
#define CARD_OPPONENT_COPYCAT                   22
#define CARD_OPPONENT_FUCHSIA_OLD_MAN           23
#define CARD_OPPONENT_FUCHSIA_PC_COOLTRAINER    24
#define CARD_OPPONENT_ROUTE17_BIKER             25
#define CARD_OPPONENT_CINNABAR_LAB_SCIENTIST    26
#define CARD_OPPONENT_CINNABAR_PC_YOUNGSTER     27
#define CARD_OPPONENT_INDIGO_VETERAN            28
#define CARD_OPPONENT_KOGA                      29
#define CARD_OPPONENT_SABRINA                   30
#define CARD_OPPONENT_BLAINE                    31
#define CARD_OPPONENT_GIOVANNI                  32
#define CARD_OPPONENT_ONE_ISLAND_HIKER          33
#define CARD_OPPONENT_EMBER_SPA_OLD_WOMAN       34
#define CARD_OPPONENT_GAME_CORNER_DADDY         35
#define CARD_OPPONENT_LOSTELLE                  36
#define CARD_OPPONENT_THREE_ISLAND_SAILOR       37
#define CARD_OPPONENT_LORELEI                   38
#define CARD_OPPONENT_FOUR_ISLAND_GENTLEMAN     39
#define CARD_OPPONENT_FIVE_ISLAND_COOLTRAINER   40
#define CARD_OPPONENT_SIX_ISLAND_OLD_MAN        41
#define CARD_OPPONENT_TRAINER_TOWER_RODETTE     42
#define CARD_OPPONENT_BLUE                      43
#define CARD_OPPONENT_LEON                      44
#define CARD_OPPONENT_LANCE                     45
#define CARD_OPPONENT_STEVEN                    46
#define CARD_OPPONENT_CYNTHIA                   47
#define CARD_OPPONENT_ALDER                     48
#define CARD_OPPONENT_DIANTHA                   49
#define CARD_OPPONENT_MR_M                      50
#define CARD_OPPONENT_TEST_AREA_TESTER          51  // the Test Area's debug card player
#define CARD_OPPONENTS_COUNT                    52
#define CARD_OPPONENTS_MAX                      80  // the bits of CardGameSave.opponentsBeaten

// Given with the reward of the last of the 8 gym leaders beaten at cards
#define CARD_LEADERS_BONUS                      CARD_GYARADOS_MEGA_EX

// VAR_0x8005 for `special CardGame_BufferOpponentText`
#define CARD_OPPONENT_TEXT_CHALLENGE    0   // a yes/no question
#define CARD_OPPONENT_TEXT_PLAYER_WON   1
#define CARD_OPPONENT_TEXT_PLAYER_LOST  2

#endif // GUARD_CONSTANTS_CARD_GAME_H
