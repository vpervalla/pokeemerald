// Host tests for the card game's rules engine (src/card_game_rules.c, src/card_game_effects.c).
// Build and run with tools/card_game/run_tests.sh. Uses its own cards, so the tests don't
// change when the game's card data does.

#include <stdio.h>
#include <string.h>
#include "global.h"
#include "card_game.h"

// ---------------------------------------------------------------------------------------------
// Test cards

enum
{
    T_NONE,
    T_BASIC,        // Grass, 70 HP, weak to Fire
    T_STAGE1,       // evolves from T_BASIC
    T_STAGE2,       // evolves from T_STAGE1
    T_FIRE,         // Fire, 60 HP, weak to Water
    T_EX,           // Psychic ex, 120 HP
    T_POISONER,     // poisons / burns
    T_SLEEPER,      // puts to sleep / paralyzes
    T_CONFUSER,     // confuses / hurts itself
    T_WALL,         // 300 HP, retreat 4
    T_SPARK,        // discards Energy
    T_FIRE_EVO,     // evolves from T_FIRE
    T_SPARK_EVO,    // evolves from T_SPARK
    T_WALL_EVO,     // evolves from T_WALL
    T_POKEMON_END,
    T_POTION = 20,
    T_POKE_BALL,
    T_X_SPEED,
    T_RESEARCH,
    T_CAPE,
    T_CARD_END,
};

#define G CARD_TYPE_GRASS
#define R CARD_TYPE_FIRE
#define W CARD_TYPE_WATER
#define L CARD_TYPE_LIGHTNING
#define P CARD_TYPE_PSYCHIC
#define D CARD_TYPE_DARKNESS
#define C CARD_TYPE_COLORLESS

static const struct PokemonCardInfo sCards[T_POKEMON_END] =
{
    [T_BASIC] = {0, T_NONE, 70, G, R, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {G, C}, 40, CARD_EFFECT_NONE, 0}}},
    [T_STAGE1] = {0, T_BASIC, 90, G, R, CARD_STAGE_1, 2, FALSE, 1, {{NULL, {G, C, C}, 60, CARD_EFFECT_NONE, 0}}},
    [T_STAGE2] = {0, T_STAGE1, 150, G, R, CARD_STAGE_2, 3, FALSE, 2, {{NULL, {G, G, C, C}, 100, CARD_EFFECT_NONE, 0},
                                                                      {NULL, {G}, 10, CARD_EFFECT_HEAL_SELF, 30}}},
    [T_FIRE] = {0, T_NONE, 60, R, W, CARD_STAGE_BASIC, 1, FALSE, 2, {{NULL, {R}, 30, CARD_EFFECT_NONE, 0},
                                                                     {NULL, {R, R}, 50, CARD_EFFECT_FLIP_BONUS, 30}}},
    [T_EX] = {0, T_NONE, 120, P, D, CARD_STAGE_BASIC, 2, TRUE, 2, {{NULL, {P}, 200, CARD_EFFECT_NONE, 0},
                                                                   {NULL, {P}, 50, CARD_EFFECT_FLIP_OR_FAIL, 0}}},
    [T_POISONER] = {0, T_NONE, 70, D, CARD_TYPE_FIGHTING, CARD_STAGE_BASIC, 1, FALSE, 2,
                    {{NULL, {D}, 10, CARD_EFFECT_STATUS, CARD_STATUS_POISONED},
                     {NULL, {D}, 10, CARD_EFFECT_STATUS, CARD_STATUS_BURNED}}},
    [T_SLEEPER] = {0, T_NONE, 70, P, D, CARD_STAGE_BASIC, 1, FALSE, 2,
                   {{NULL, {P}, 0, CARD_EFFECT_STATUS, CARD_STATUS_ASLEEP},
                    {NULL, {P}, 0, CARD_EFFECT_FLIP_STATUS, CARD_STATUS_PARALYZED}}},
    [T_CONFUSER] = {0, T_NONE, 70, P, D, CARD_STAGE_BASIC, 1, FALSE, 2,
                    {{NULL, {P}, 10, CARD_EFFECT_STATUS, CARD_STATUS_CONFUSED},
                     {NULL, {P}, 70, CARD_EFFECT_SELF_DAMAGE, 70}}},
    [T_WALL] = {0, T_NONE, 300, C, CARD_TYPE_NONE, CARD_STAGE_BASIC, 4, FALSE, 1, {{NULL, {C}, 10, CARD_EFFECT_NONE, 0}}},
    [T_SPARK] = {0, T_NONE, 100, L, CARD_TYPE_FIGHTING, CARD_STAGE_BASIC, 1, FALSE, 2,
                 {{NULL, {L, L, L}, 100, CARD_EFFECT_DISCARD_ALL_ENERGY, 0},
                  {NULL, {L}, 30, CARD_EFFECT_DISCARD_ENERGY, 1}}},
    [T_FIRE_EVO] = {0, T_FIRE, 100, R, W, CARD_STAGE_1, 2, FALSE, 1, {{NULL, {R, C}, 60, CARD_EFFECT_NONE, 0}}},
    [T_SPARK_EVO] = {0, T_SPARK, 130, L, CARD_TYPE_FIGHTING, CARD_STAGE_1, 1, TRUE, 1, {{NULL, {L, C}, 70, CARD_EFFECT_FLIP_STATUS, CARD_STATUS_PARALYZED}}},
    [T_WALL_EVO] = {0, T_WALL, 320, C, CARD_TYPE_NONE, CARD_STAGE_1, 4, FALSE, 1, {{NULL, {C, C}, 30, CARD_EFFECT_NONE, 0}}},
};

static const struct TrainerCardInfo sTrainers[T_CARD_END - T_POTION] =
{
    [T_POTION - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_HEAL, 20},
    [T_POKE_BALL - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_POKE_BALL, 0},
    [T_X_SPEED - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_RETREAT_DISCOUNT, 1},
    [T_RESEARCH - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW, 2},
    [T_CAPE - T_POTION] = {NULL, CARD_KIND_TOOL, CARD_TRAINER_TOOL_HP, 20},
};

u8 CardGame_GetCardKind(u16 cardId)
{
    if (cardId > T_NONE && cardId < T_POKEMON_END)
        return CARD_KIND_POKEMON;
    if (cardId >= T_POTION && cardId < T_CARD_END)
        return sTrainers[cardId - T_POTION].kind;
    return CARD_KIND_NONE;
}

const struct PokemonCardInfo *CardGame_GetPokemonInfo(u16 cardId)
{
    return (cardId > T_NONE && cardId < T_POKEMON_END) ? &sCards[cardId] : NULL;
}

const struct TrainerCardInfo *CardGame_GetTrainerInfo(u16 cardId)
{
    return (cardId >= T_POTION && cardId < T_CARD_END) ? &sTrainers[cardId - T_POTION] : NULL;
}

// ---------------------------------------------------------------------------------------------
// Harness

static int sChecks, sFailures;
static const char *sTest;

#define CHECK(cond) do { sChecks++; if (!(cond)) { sFailures++; printf("FAIL %s:%d [%s] %s\n", __FILE__, __LINE__, sTest, #cond); } } while (0)

static const u16 sDeckA[CARD_DECK_SIZE] = {
    T_BASIC, T_BASIC, T_STAGE1, T_STAGE1, T_STAGE2, T_FIRE, T_FIRE, T_EX, T_POISONER, T_SLEEPER,
    T_CONFUSER, T_WALL, T_SPARK, T_POTION, T_POTION, T_POKE_BALL, T_X_SPEED, T_RESEARCH, T_RESEARCH, T_CAPE,
};
static const u16 sDeckB[CARD_DECK_SIZE] = {
    T_FIRE, T_FIRE, T_FIRE_EVO, T_FIRE_EVO, T_SPARK, T_SPARK, T_SPARK_EVO, T_SPARK_EVO, T_WALL, T_WALL_EVO,
    T_SLEEPER, T_EX, T_POTION, T_POTION, T_POKE_BALL, T_POKE_BALL, T_RESEARCH, T_RESEARCH, T_X_SPEED, T_CAPE,
};
static const u16 sDeckNoBasic[CARD_DECK_SIZE] = {
    T_STAGE1, T_STAGE1, T_STAGE2, T_STAGE2, T_FIRE_EVO, T_FIRE_EVO, T_SPARK_EVO, T_SPARK_EVO, T_WALL_EVO, T_WALL_EVO,
    T_POTION, T_POTION, T_POKE_BALL, T_POKE_BALL, T_RESEARCH, T_RESEARCH, T_X_SPEED, T_X_SPEED, T_CAPE, T_CAPE,
};

static struct CardDuelSetup MakeSetup(const u16 *deck, u8 type1, u8 type2)
{
    struct CardDuelSetup setup;

    memset(&setup, 0, sizeof(setup));
    setup.deck = deck;
    setup.zoneTypes[0] = type1;
    setup.zoneTypes[1] = type2;
    return setup;
}

static bool8 Act(struct CardDuel *duel, u8 side, u8 type, u8 handIndex, u8 slot, u8 arg)
{
    struct CardAction action = {type, handIndex, slot, arg};

    return Duel_DoAction(duel, side, &action);
}

static bool8 Legal(struct CardDuel *duel, u8 side, u8 type, u8 handIndex, u8 slot, u8 arg)
{
    struct CardAction action = {type, handIndex, slot, arg};

    return Duel_IsLegalAction(duel, side, &action);
}

static void SetHand(struct CardDuel *duel, u8 side, u16 card)
{
    memset(duel->sides[side].hand, 0, sizeof(duel->sides[side].hand));
    duel->sides[side].hand[0] = card;
    duel->sides[side].handCount = card != T_NONE ? 1 : 0;
}

static u8 AddToHand(struct CardDuel *duel, u8 side, u16 card)
{
    struct CardDuelSide *s = &duel->sides[side];

    s->hand[s->handCount] = card;
    return s->handCount++;
}

static u8 FindInHand(const struct CardDuel *duel, u8 side, u16 card)
{
    u8 i;

    for (i = 0; i < duel->sides[side].handCount; i++)
    {
        if (duel->sides[side].hand[i] == card)
            return i;
    }
    return 0xFF;
}

static void Energize(struct CardDuel *duel, u8 side, u8 slot, u8 type, u8 count)
{
    duel->sides[side].slots[slot].energy[type] += count;
}

static int CountEvents(const struct CardDuel *duel, u8 type)
{
    int i, count = 0;

    for (i = 0; i < duel->eventCount; i++)
    {
        if (duel->events[i].type == type)
            count++;
    }
    return count;
}

static void ForceFlips(struct CardDuel *duel, u32 bits, u8 count)
{
    duel->forcedFlips = bits;
    duel->forcedFlipCount = count;
}

// A duel in turn 1 with the given Active Pokémon and empty hands (after the turn's draw)
static void StartDuel(struct CardDuel *duel, u16 playerActive, u16 opponentActive, u8 firstSide)
{
    struct CardDuelSetup a = MakeSetup(sDeckA, G, P);
    struct CardDuelSetup b = MakeSetup(sDeckB, R, CARD_TYPE_NONE);

    CHECK(Duel_Init(duel, &a, &b, 1234));
    duel->firstSide = duel->currentSide = firstSide;
    SetHand(duel, CARD_SIDE_PLAYER, playerActive);
    SetHand(duel, CARD_SIDE_OPPONENT, opponentActive);
    CHECK(Act(duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(duel, CARD_SIDE_OPPONENT, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(duel, CARD_SIDE_PLAYER, CARD_ACTION_END_SETUP, 0, 0, 0));
    CHECK(Act(duel, CARD_SIDE_OPPONENT, CARD_ACTION_END_SETUP, 0, 0, 0));
    CHECK(duel->phase == CARD_PHASE_MAIN && duel->turn == 1);
    SetHand(duel, CARD_SIDE_PLAYER, T_NONE);
    SetHand(duel, CARD_SIDE_OPPONENT, T_NONE);
    Duel_ClearEvents(duel);
}

static void EndTurn(struct CardDuel *duel)
{
    CHECK(Act(duel, duel->currentSide, CARD_ACTION_END_TURN, 0, 0, 0));
}

// ---------------------------------------------------------------------------------------------
// Tests

static void Test_ValidateDeck(void)
{
    u16 deck[CARD_DECK_SIZE];
    u8 zone[CARD_MAX_ZONE_TYPES] = {G, CARD_TYPE_NONE, CARD_TYPE_NONE};
    u8 dragon[CARD_MAX_ZONE_TYPES] = {CARD_TYPE_DRAGON, CARD_TYPE_NONE, CARD_TYPE_NONE};
    u8 none[CARD_MAX_ZONE_TYPES] = {CARD_TYPE_NONE, CARD_TYPE_NONE, CARD_TYPE_NONE};
    int i;

    sTest = "validate deck";
    CHECK(Duel_ValidateDeck(sDeckA, zone) == CARD_DECK_OK);
    CHECK(Duel_ValidateDeck(sDeckA, none) == CARD_DECK_NO_ENERGY_TYPE);
    CHECK(Duel_ValidateDeck(sDeckA, dragon) == CARD_DECK_NO_ENERGY_TYPE);
    memcpy(deck, sDeckA, sizeof(deck));
    deck[2] = T_BASIC;
    CHECK(Duel_ValidateDeck(deck, zone) == CARD_DECK_TOO_MANY_COPIES);
    memcpy(deck, sDeckA, sizeof(deck));
    deck[19] = T_NONE;
    CHECK(Duel_ValidateDeck(deck, zone) == CARD_DECK_WRONG_SIZE);
    deck[19] = 99;
    CHECK(Duel_ValidateDeck(deck, zone) == CARD_DECK_UNKNOWN_CARD);
    CHECK(Duel_ValidateDeck(sDeckNoBasic, zone) == CARD_DECK_NO_BASIC);
    for (i = 0; i < CARD_DECK_SIZE; i++)
        deck[i] = (i % 2) ? T_STAGE1 : T_BASIC;
    CHECK(Duel_ValidateDeck(deck, zone) == CARD_DECK_TOO_MANY_COPIES);
}

static void Test_Init(void)
{
    struct CardDuel duel;
    struct CardDuelSetup a = MakeSetup(sDeckA, G, P);
    struct CardDuelSetup b = MakeSetup(sDeckB, R, CARD_TYPE_NONE);
    struct CardDuelSetup bad = MakeSetup(sDeckA, CARD_TYPE_NONE, CARD_TYPE_NONE);
    u32 seed;
    int side, i, basics, firsts[2] = {0, 0};

    sTest = "init";
    CHECK(!Duel_Init(&duel, &bad, &b, 1));
    for (seed = 0; seed < 300; seed++)
    {
        CHECK(Duel_Init(&duel, &a, &b, seed));
        CHECK(duel.phase == CARD_PHASE_SETUP && duel.winner == CARD_SIDE_NONE);
        firsts[duel.firstSide]++;
        for (side = 0; side < CARD_SIDE_COUNT; side++)
        {
            CHECK(duel.sides[side].handCount == CARD_OPENING_HAND);
            CHECK(duel.sides[side].deckCount == CARD_DECK_SIZE - CARD_OPENING_HAND);
            basics = 0;
            for (i = 0; i < CARD_OPENING_HAND; i++)
            {
                if (CardGame_GetPokemonInfo(duel.sides[side].hand[i]) != NULL
                 && CardGame_GetPokemonInfo(duel.sides[side].hand[i])->stage == CARD_STAGE_BASIC)
                    basics++;
            }
            CHECK(basics > 0);
        }
        CHECK(duel.sides[0].nextEnergy == G || duel.sides[0].nextEnergy == P);
        CHECK(duel.sides[1].nextEnergy == R);
    }
    CHECK(firsts[0] > 100 && firsts[1] > 100);
}

static void Test_Setup(void)
{
    struct CardDuel duel;
    struct CardDuelSetup a = MakeSetup(sDeckA, G, P);
    struct CardDuelSetup b = MakeSetup(sDeckB, R, CARD_TYPE_NONE);

    sTest = "setup";
    CHECK(Duel_Init(&duel, &a, &b, 7));
    duel.firstSide = duel.currentSide = CARD_SIDE_PLAYER;
    SetHand(&duel, CARD_SIDE_PLAYER, T_BASIC);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FIRE);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_STAGE1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_POTION);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_END_SETUP, 0, 0, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_BENCH_1, 0)); // Active first
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 2, CARD_SLOT_ACTIVE, 0));  // not a Basic
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 3, CARD_SLOT_ACTIVE, 0));  // a Trainer
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_END_TURN, 0, 0, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_BENCH_1, 0));
    CHECK(duel.sides[0].slots[CARD_SLOT_BENCH_1].cardId == T_FIRE);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_END_SETUP, 0, 0, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_BENCH_1, 0)); // done
    CHECK(duel.phase == CARD_PHASE_SETUP);
    SetHand(&duel, CARD_SIDE_OPPONENT, T_WALL);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_END_SETUP, 0, 0, 0));
    CHECK(duel.phase == CARD_PHASE_MAIN && duel.turn == 1 && duel.currentSide == CARD_SIDE_PLAYER);
    CHECK(duel.sides[0].handCount == 3);  // 2 left + the turn's draw
    CHECK(duel.sides[0].zoneEnergy == CARD_TYPE_NONE);
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_END_TURN, 0, 0, 0));
}

static void Test_FirstTurns(void)
{
    struct CardDuel duel;
    u8 expected;

    sTest = "first turns";
    StartDuel(&duel, T_FIRE, T_BASIC, CARD_SIDE_OPPONENT);
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, G, 2);
    // First player, first turn: no Energy, no attack
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACH_ENERGY, 0, CARD_SLOT_ACTIVE, 0));
    expected = duel.sides[CARD_SIDE_PLAYER].nextEnergy;
    EndTurn(&duel);
    CHECK(duel.turn == 2 && duel.currentSide == CARD_SIDE_PLAYER);
    CHECK(duel.sides[CARD_SIDE_PLAYER].handCount == 1);
    CHECK(duel.sides[CARD_SIDE_PLAYER].zoneEnergy == expected);
    CHECK(CountEvents(&duel, CARD_EVENT_ENERGY_ZONE) == 1);
    // Second player can attack on their first turn, and attaches once
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACH_ENERGY, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACH_ENERGY, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Duel_CountEnergy(&duel.sides[0].slots[0]) == 1);
    duel.sides[0].slots[0].energy[expected] = 0;
    duel.sides[0].slots[0].energy[R] = 1;
    CHECK(Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1)); // needs R R
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 2)); // no third attack
    // An Energy not attached is lost at the end of the turn
    EndTurn(&duel);
    CHECK(duel.sides[CARD_SIDE_OPPONENT].zoneEnergy == R);
    EndTurn(&duel);
    CHECK(duel.sides[CARD_SIDE_OPPONENT].zoneEnergy == CARD_TYPE_NONE);
}

static void Test_Bench(void)
{
    struct CardDuel duel;

    sTest = "bench";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FIRE);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FIRE);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_SPARK);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_EX);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 2, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 2, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 3, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 3, 0)); // Bench full
    CHECK(duel.sides[0].handCount == 1);
}

static void Test_Evolution(void)
{
    struct CardDuel duel;
    struct CardInPlay *active;
    u8 hand;

    sTest = "evolution";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    active = &duel.sides[0].slots[CARD_SLOT_ACTIVE];
    hand = AddToHand(&duel, CARD_SIDE_PLAYER, T_STAGE1);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, hand, CARD_SLOT_ACTIVE, 0)); // turn 1
    EndTurn(&duel);
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_EVOLVE, 0, CARD_SLOT_ACTIVE, 0));
    EndTurn(&duel); // turn 3
    // A Pokémon put in play this turn can't evolve
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BASIC);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, duel.sides[0].handCount - 1, 1, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, 0, 1, 0));
    // Stage 2 can't go on a Basic
    hand = AddToHand(&duel, CARD_SIDE_PLAYER, T_STAGE2);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, hand, CARD_SLOT_ACTIVE, 0));
    // Evolving keeps damage and Energy and cures conditions
    active->damage = 30;
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, G, 2);
    active->status = CARD_STATUS_POISONED | CARD_STATUS_ASLEEP;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, FindInHand(&duel, CARD_SIDE_PLAYER, T_STAGE1), CARD_SLOT_ACTIVE, 0));
    CHECK(active->cardId == T_STAGE1 && active->evolvedFrom[0] == T_BASIC);
    CHECK(active->damage == 30 && active->energy[G] == 2 && active->status == 0);
    // Not twice in a turn
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, 0, CARD_SLOT_ACTIVE, 0));
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_EVOLVE, FindInHand(&duel, CARD_SIDE_PLAYER, T_STAGE2), CARD_SLOT_ACTIVE, 0));
    CHECK(active->cardId == T_STAGE2 && active->evolvedFrom[1] == T_STAGE1);
    CHECK(Duel_GetMaxHp(active) == 150);
}

static void Test_AttackAndWeakness(void)
{
    struct CardDuel duel;

    sTest = "attack";
    StartDuel(&duel, T_BASIC, T_FIRE, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    // Colorless takes any Energy, but the Grass Energy is needed
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 2);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, G, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[1].slots[0].damage == 40);  // T_FIRE isn't weak to Grass
    CHECK(duel.currentSide == CARD_SIDE_OPPONENT && duel.turn == 3); // attacking ends the turn
    // Fire hits the Grass Pokémon's weakness: 30 + 20
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].slots[0].damage == 50);
    // Flip for bonus damage: heads
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    ForceFlips(&duel, 1, 1);
    duel.sides[0].slots[0].damage = 0;
    duel.sides[0].slots[0].cardId = T_WALL;
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[0].slots[0].damage == 80);
}

static void Test_KnockOutsAndTrophies(void)
{
    struct CardDuel duel;

    sTest = "knockouts";
    StartDuel(&duel, T_EX, T_BASIC, CARD_SIDE_OPPONENT);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_FIRE);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PLAY_BASIC, 0, 2, 0));
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].trophies == 1);
    CHECK(CountEvents(&duel, CARD_EVENT_KNOCK_OUT) == 1);
    CHECK(duel.sides[1].discardCount == 1 && duel.sides[1].discard[0] == T_BASIC);
    CHECK(duel.phase == CARD_PHASE_PROMOTE && duel.promoteMask == (1 << CARD_SIDE_OPPONENT));
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PROMOTE, 0, 1, 0)); // empty slot
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PROMOTE, 0, 2, 0));
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PROMOTE, 0, 2, 0));
    CHECK(duel.sides[1].slots[0].cardId == T_FIRE && duel.sides[1].slots[2].cardId == T_NONE);
    CHECK(duel.phase == CARD_PHASE_MAIN && duel.currentSide == CARD_SIDE_OPPONENT && duel.turn == 3);
    // An ex gives 2 trophies
    duel.sides[0].slots[0].damage = 110;
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[1].trophies == 2);
    // ... and the player had no Bench: the opponent wins
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_SIDE_OPPONENT);
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_END_TURN, 0, 0, 0));

    // Three trophies win
    StartDuel(&duel, T_EX, T_BASIC, CARD_SIDE_OPPONENT);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_FIRE);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    EndTurn(&duel);
    duel.sides[0].trophies = 2;
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_SIDE_PLAYER);
    CHECK(CountEvents(&duel, CARD_EVENT_GAME_OVER) == 1);

    // Both win at once: a draw
    StartDuel(&duel, T_CONFUSER, T_BASIC, CARD_SIDE_OPPONENT);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FIRE);
    CHECK(!Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 1, 0)); // not the player's turn
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    duel.sides[0].trophies = 2;
    duel.sides[1].trophies = 2;
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1)); // 70 to both
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_RESULT_DRAW);
}

static void Test_Retreat(void)
{
    struct CardDuel duel;
    struct CardDuelSide *side;

    sTest = "retreat";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    side = &duel.sides[0];
    side->slots[0].cardId = T_STAGE1; // retreat cost 2
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FIRE);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, G, 1);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0)); // costs 2
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 2, 0)); // empty slot
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 2);
    side->slots[0].status = CARD_STATUS_ASLEEP;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
    side->slots[0].status = CARD_STATUS_PARALYZED;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
    side->slots[0].status = CARD_STATUS_CONFUSED | CARD_STATUS_POISONED;
    side->slots[0].damage = 20;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
    CHECK(side->slots[0].cardId == T_FIRE && side->slots[1].cardId == T_STAGE1);
    // Paid with the other types first, kept its own Grass Energy, conditions cured, damage kept
    CHECK(side->slots[1].energy[G] == 1 && side->slots[1].energy[P] == 0);
    CHECK(side->slots[1].status == 0 && side->slots[1].damage == 20);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0)); // once a turn
}

static void Test_RetreatDiscount(void)
{
    struct CardDuel duel;
    struct CardDuelSide *side;

    sTest = "x speed";
    StartDuel(&duel, T_FIRE, T_WALL, CARD_SIDE_PLAYER);
    side = &duel.sides[0];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BASIC);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
    AddToHand(&duel, CARD_SIDE_PLAYER, T_X_SPEED);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 0, 0)); // no target
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(side->discardCount == 1 && side->discard[0] == T_X_SPEED);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(side->retreatDiscount == 0);
}

static void Test_Poison(void)
{
    struct CardDuel duel;

    sTest = "poison";
    StartDuel(&duel, T_POISONER, T_WALL, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, D, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    // 10 from the attack, 10 from the checkup after the player's turn
    CHECK(duel.sides[1].slots[0].status == CARD_STATUS_POISONED);
    CHECK(duel.sides[1].slots[0].damage == 20);
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].damage == 30);
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].damage == 40);
}

static void Test_Burn(void)
{
    struct CardDuel duel;

    sTest = "burn";
    StartDuel(&duel, T_POISONER, T_WALL, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, D, 1);
    ForceFlips(&duel, 0, 1); // tails: still burned
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[1].slots[0].damage == 30 && (duel.sides[1].slots[0].status & CARD_STATUS_BURNED));
    ForceFlips(&duel, 1, 1); // heads: cured
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].damage == 50 && duel.sides[1].slots[0].status == 0);
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].damage == 50);
}

static void Test_Sleep(void)
{
    struct CardDuel duel;

    sTest = "sleep";
    StartDuel(&duel, T_SLEEPER, T_FIRE, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    ForceFlips(&duel, 0, 1); // tails at the checkup: stays asleep
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[1].slots[0].status == CARD_STATUS_ASLEEP);
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    ForceFlips(&duel, 1, 1); // heads: wakes up
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].status == 0);
    EndTurn(&duel);
    CHECK(Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
}

static void Test_Paralysis(void)
{
    struct CardDuel duel;

    sTest = "paralysis";
    StartDuel(&duel, T_SLEEPER, T_FIRE, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    ForceFlips(&duel, 1, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[1].slots[0].status == CARD_STATUS_PARALYZED);
    // Still paralyzed during its owner's next turn, cured at its end
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    CHECK(!Legal(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    EndTurn(&duel);
    CHECK(duel.sides[1].slots[0].status == 0);
    // Tails: no paralysis
    ForceFlips(&duel, 0, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[1].slots[0].status == 0);
}

static void Test_Confusion(void)
{
    struct CardDuel duel;

    sTest = "confusion";
    StartDuel(&duel, T_CONFUSER, T_FIRE, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[1].slots[0].status == CARD_STATUS_CONFUSED && duel.sides[1].slots[0].damage == 10);
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, R, 1);
    ForceFlips(&duel, 0, 1); // tails: the attack fails
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].slots[0].damage == 0 && CountEvents(&duel, CARD_EVENT_ATTACK_FAILED) == 1);
    CHECK(duel.currentSide == CARD_SIDE_PLAYER); // the turn still ends
    EndTurn(&duel);
    ForceFlips(&duel, 1, 1); // heads: it hits
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].slots[0].damage == 30);
    // A Special Condition replaces another one, but not poison
    duel.sides[1].slots[0].status = CARD_STATUS_POISONED | CARD_STATUS_CONFUSED;
    Duel_GiveStatus(&duel, CARD_SIDE_OPPONENT, 0, CARD_STATUS_ASLEEP);
    CHECK(duel.sides[1].slots[0].status == (CARD_STATUS_POISONED | CARD_STATUS_ASLEEP));
}

static void Test_FlipOrFail(void)
{
    struct CardDuel duel;

    sTest = "flip or fail";
    StartDuel(&duel, T_EX, T_WALL, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, P, 1);
    ForceFlips(&duel, 0, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[1].slots[0].damage == 0);
    EndTurn(&duel);
    ForceFlips(&duel, 1, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[1].slots[0].damage == 50);
}

static void Test_EnergyDiscard(void)
{
    struct CardDuel duel;
    struct CardInPlay *mon;

    sTest = "energy discard";
    StartDuel(&duel, T_SPARK, T_WALL, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    mon = &duel.sides[0].slots[0];
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, L, 2);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, G, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1)); // discard 1 of its own type
    CHECK(mon->energy[L] == 1 && mon->energy[G] == 1);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, L, 2);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0)); // discard all
    CHECK(Duel_CountEnergy(mon) == 0);
    CHECK(duel.sides[1].slots[0].damage == 130);
}

static void Test_HealAttack(void)
{
    struct CardDuel duel;

    sTest = "heal attack";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    duel.sides[0].slots[0].cardId = T_STAGE2;
    duel.sides[0].slots[0].damage = 50;
    Energize(&duel, CARD_SIDE_PLAYER, CARD_SLOT_ACTIVE, G, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(duel.sides[0].slots[0].damage == 20);
}

static void Test_Trainers(void)
{
    struct CardDuel duel;
    struct CardDuelSide *side;
    u8 i;

    sTest = "trainers";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    side = &duel.sides[0];
    // Supporter: once a turn
    AddToHand(&duel, CARD_SIDE_PLAYER, T_RESEARCH);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_RESEARCH);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(side->handCount == 3); // 1 left + 2 drawn
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    // Potion: needs damage, heals up to 20
    SetHand(&duel, CARD_SIDE_PLAYER, T_POTION);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_ACTIVE, 0));
    side->slots[0].damage = 10;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(side->slots[0].damage == 0);
    // Poké Ball: a Basic from the deck
    SetHand(&duel, CARD_SIDE_PLAYER, T_POKE_BALL);
    i = side->deckCount;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(side->handCount == 1 && side->deckCount == i - 1);
    CHECK(CardGame_GetPokemonInfo(side->hand[0])->stage == CARD_STAGE_BASIC);
    side->deckCount = 0;
    SetHand(&duel, CARD_SIDE_PLAYER, T_POKE_BALL);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    // Giant Cape: +20 HP, one Tool per Pokémon, discarded with it
    SetHand(&duel, CARD_SIDE_PLAYER, T_CAPE);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_CAPE);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 1, 0)); // empty slot
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_ACTIVE, 0));
    CHECK(side->slots[0].tool == T_CAPE && Duel_GetMaxHp(&side->slots[0]) == 90);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_ACTIVE, 0));
    side->slots[0].damage = 80; // would be knocked out without the Cape
    EndTurn(&duel);
    CHECK(side->slots[0].cardId == T_BASIC);
    Energize(&duel, CARD_SIDE_OPPONENT, CARD_SLOT_ACTIVE, C, 1);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_SIDE_OPPONENT);
    CHECK(side->discard[side->discardCount - 1] == T_CAPE);
}

static void Test_DrawLimits(void)
{
    struct CardDuel duel;
    struct CardDuelSide *side;
    u8 i;

    sTest = "draw limits";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_OPPONENT);
    side = &duel.sides[0];
    for (i = 0; i < CARD_HAND_MAX; i++)
        AddToHand(&duel, CARD_SIDE_PLAYER, T_POTION);
    i = side->deckCount;
    EndTurn(&duel);
    CHECK(side->handCount == CARD_HAND_MAX && side->deckCount == i);
    CHECK(CountEvents(&duel, CARD_EVENT_HAND_FULL) == 1);
    // An empty deck isn't a loss
    SetHand(&duel, CARD_SIDE_PLAYER, T_NONE);
    side->deckCount = 0;
    EndTurn(&duel);
    Duel_ClearEvents(&duel);
    EndTurn(&duel);
    CHECK(CountEvents(&duel, CARD_EVENT_DECK_EMPTY) == 1 && duel.phase == CARD_PHASE_MAIN);
}

static void Test_TurnLimit(void)
{
    struct CardDuel duel;
    int i;

    sTest = "turn limit";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    for (i = 1; i < CARD_TURN_LIMIT; i++)
        EndTurn(&duel);
    CHECK(duel.turn == CARD_TURN_LIMIT && duel.phase == CARD_PHASE_MAIN);
    EndTurn(&duel);
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_RESULT_DRAW);
}

static void Test_Forfeit(void)
{
    struct CardDuel duel;

    sTest = "forfeit";
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    Duel_Forfeit(&duel, CARD_SIDE_PLAYER);
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_SIDE_OPPONENT);
}

// Every card of a side is somewhere: deck, hand, discard pile or in play
static int CountCards(const struct CardDuelSide *side)
{
    int slot, count = side->deckCount + side->handCount + side->discardCount;

    for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
    {
        const struct CardInPlay *mon = &side->slots[slot];

        count += (mon->cardId != T_NONE) + (mon->evolvedFrom[0] != T_NONE)
               + (mon->evolvedFrom[1] != T_NONE) + (mon->tool != T_NONE);
    }
    return count;
}

// Attacks half the time it can, and ends the turn only when nothing else is left or 1 time in 8,
// so random games reach knockouts instead of the turn limit.
static int PickRandomAction(const struct CardAction *actions, int count, u32 *rng)
{
    int i, others = 0, attacks = 0;

    *rng = *rng * 1664525 + 1013904223;
    for (i = 0; i < count; i++)
    {
        if (actions[i].type == CARD_ACTION_ATTACK)
            attacks++;
    }
    if (attacks != 0 && ((*rng >> 4) & 1))
    {
        for (i = 0; i < count; i++)
        {
            if (actions[i].type == CARD_ACTION_ATTACK)
                return i;
        }
    }
    for (i = 0; i < count; i++)
    {
        if (actions[i].type != CARD_ACTION_END_TURN && actions[i].type != CARD_ACTION_ATTACK)
            others++;
    }
    // END_TURN is listed last
    if (others == 0 || ((*rng >> 5) & 7) == 0)
        return count - 1;
    *rng = *rng * 1664525 + 1013904223;
    others = (*rng >> 8) % others;
    for (i = 0; i < count; i++)
    {
        if (actions[i].type != CARD_ACTION_END_TURN && actions[i].type != CARD_ACTION_ATTACK && others-- == 0)
            return i;
    }
    return count - 1;
}

// Random legal play, many games: no illegal state, every game ends, cards are never lost
static void Test_RandomPlay(void)
{
    struct CardDuel duel, replay;
    struct CardDuelSetup a = MakeSetup(sDeckA, G, P);
    struct CardDuelSetup b = MakeSetup(sDeckB, R, L);
    struct CardAction actions[CARD_MAX_ACTIONS];
    u32 seed, rng = 99;
    int steps, side, count, pick, i, results[3] = {0, 0, 0};
    static int eventCounts[CARD_EVENT_GAME_OVER + 1];
    bool8 ok;

    sTest = "random play";
    for (seed = 0; seed < 3000; seed++)
    {
        CHECK(Duel_Init(&duel, &a, &b, seed));
        for (steps = 0; steps < 2000 && duel.phase != CARD_PHASE_OVER; steps++)
        {
            for (i = 0; i < duel.eventCount; i++)
                eventCounts[duel.events[i].type]++;
            Duel_ClearEvents(&duel);
            for (side = 0; side < CARD_SIDE_COUNT; side++)
            {
                if (!Duel_CanAct(&duel, side))
                    continue;
                count = Duel_GetLegalActions(&duel, side, actions);
                CHECK(count > 0 && count < CARD_MAX_ACTIONS);
                if (count == 0)
                    break;
                pick = PickRandomAction(actions, count, &rng);
                ok = Duel_DoAction(&duel, side, &actions[pick]);
                CHECK(ok);
                break;
            }
            CHECK(CountCards(&duel.sides[0]) == CARD_DECK_SIZE);
            CHECK(CountCards(&duel.sides[1]) == CARD_DECK_SIZE);
            CHECK(duel.sides[0].handCount <= CARD_HAND_MAX && duel.sides[1].handCount <= CARD_HAND_MAX);
            if (duel.phase == CARD_PHASE_MAIN)
                CHECK(duel.sides[0].slots[0].cardId != T_NONE && duel.sides[1].slots[0].cardId != T_NONE);
            if (sFailures > 20)
                return;
        }
        for (i = 0; i < duel.eventCount; i++)
            eventCounts[duel.events[i].type]++;
        CHECK(duel.phase == CARD_PHASE_OVER);
        CHECK(duel.turn <= CARD_TURN_LIMIT);
        results[duel.winner == CARD_RESULT_DRAW ? 2 : duel.winner]++;
    }
    printf("  random play: player %d, opponent %d, draws %d\n", results[0], results[1], results[2]);
    CHECK(results[0] > 0 && results[1] > 0);
    // Every kind of event happened in some game
    for (i = 0; i <= CARD_EVENT_GAME_OVER; i++)
    {
        if (eventCounts[i] == 0)
            printf("  random play never had event %d\n", i);
        CHECK(eventCounts[i] > 0);
    }

    // The same seed and actions give the same duel
    CHECK(Duel_Init(&duel, &a, &b, 42));
    CHECK(Duel_Init(&replay, &a, &b, 42));
    CHECK(memcmp(&duel, &replay, sizeof(duel)) == 0);
}

int main(void)
{
    Test_ValidateDeck();
    Test_Init();
    Test_Setup();
    Test_FirstTurns();
    Test_Bench();
    Test_Evolution();
    Test_AttackAndWeakness();
    Test_KnockOutsAndTrophies();
    Test_Retreat();
    Test_RetreatDiscount();
    Test_Poison();
    Test_Burn();
    Test_Sleep();
    Test_Paralysis();
    Test_Confusion();
    Test_FlipOrFail();
    Test_EnergyDiscard();
    Test_HealAttack();
    Test_Trainers();
    Test_DrawLimits();
    Test_TurnLimit();
    Test_Forfeit();
    Test_RandomPlay();
    printf("%d checks, %d failures\n", sChecks, sFailures);
    return sFailures != 0;
}
