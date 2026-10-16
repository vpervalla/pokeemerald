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
    T_MULTI,        // flip several coins / damage the Bench
    T_QUAKE,        // damages its own Bench / discards the defender's Energy
    T_BEAM,         // can't attack next turn / draws
    T_PUMP,         // bonus with extra Energy / switches the defender
    T_REVENGE,      // bonus if the defender is damaged / snipes
    T_HEALER,       // abilities...
    T_ATTACHER,
    T_GASSER,
    T_HYPNO,
    T_DRIVER,
    T_SNIPER,
    T_SHELL,
    T_SPIKY,
    T_LIGHT,
    T_MEGA,         // a Mega ex
    T_POKEMON_END,
    T_POTION = 40,
    T_POKE_BALL,
    T_X_SPEED,
    T_RESEARCH,
    T_CAPE,
    T_HAND_SCOPE,
    T_RED_CARD,
    T_RARE_CANDY,
    T_HELMET,
    T_BROCK,
    T_MISTY,
    T_SURGE,
    T_ERIKA,
    T_SABRINA,
    T_BLAINE,
    T_GIOVANNI,
    T_KOGA,
    T_BILL,
    T_BLUE,
    T_AGATHA,
    T_DAISY,
    T_CELIO,
    T_FULL_HEAL,
    T_LEFTOVERS,
    T_CHOICE_BAND,
    T_CARD_END,
};

#define G CARD_TYPE_GRASS
#define R CARD_TYPE_FIRE
#define W CARD_TYPE_WATER
#define L CARD_TYPE_LIGHTNING
#define P CARD_TYPE_PSYCHIC
#define D CARD_TYPE_DARKNESS
#define C CARD_TYPE_COLORLESS
#define F CARD_TYPE_FIGHTING

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
    [T_MULTI] = {0, T_NONE, 100, C, F, CARD_STAGE_BASIC, 1, FALSE, 2, {{NULL, {C}, 30, CARD_EFFECT_FLIP_MULTI, 3},
                                                                       {NULL, {C}, 10, CARD_EFFECT_SPREAD_BENCH, 10}}},
    [T_QUAKE] = {0, T_NONE, 100, F, G, CARD_STAGE_BASIC, 1, FALSE, 2, {{NULL, {F}, 50, CARD_EFFECT_SELF_BENCH_DAMAGE, 10},
                                                                       {NULL, {F}, 30, CARD_EFFECT_FLIP_DISCARD_ENERGY, 0}}},
    [T_BEAM] = {0, T_NONE, 120, C, F, CARD_STAGE_BASIC, 2, FALSE, 2, {{NULL, {C}, 100, CARD_EFFECT_CANT_ATTACK_NEXT, 0},
                                                                      {NULL, {C}, 20, CARD_EFFECT_DRAW, 2}}},
    [T_PUMP] = {0, T_NONE, 120, W, L, CARD_STAGE_BASIC, 2, FALSE, 2, {{NULL, {W}, 50, CARD_EFFECT_BONUS_EXTRA_ENERGY, 60},
                                                                      {NULL, {W}, 30, CARD_EFFECT_SWITCH_DEFENDER, 0}}},
    [T_REVENGE] = {0, T_NONE, 100, D, F, CARD_STAGE_BASIC, 1, FALSE, 2, {{NULL, {D}, 30, CARD_EFFECT_BONUS_IF_DAMAGED, 30},
                                                                         {NULL, {D}, 20, CARD_EFFECT_SNIPE, 0}}},
    [T_HEALER] = {0, T_NONE, 90, G, R, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {G}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_HEAL_ALL, 20},
    [T_ATTACHER] = {0, T_NONE, 90, P, D, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {P}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_ATTACH_OWN_TYPE, 0},
    [T_GASSER] = {0, T_NONE, 90, D, F, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {D}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_POISON_DEFENDER, 0},
    [T_HYPNO] = {0, T_NONE, 90, P, D, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {P}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_FLIP_SLEEP, 0},
    [T_DRIVER] = {0, T_NONE, 90, C, L, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {C}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_DRIVE_OFF, 0},
    [T_SNIPER] = {0, T_NONE, 90, W, L, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {W}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_SNIPE, 20},
    [T_SHELL] = {0, T_NONE, 100, W, L, CARD_STAGE_BASIC, 2, FALSE, 1, {{NULL, {W}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_REDUCE_DAMAGE, 20},
    [T_SPIKY] = {0, T_NONE, 100, D, F, CARD_STAGE_BASIC, 1, FALSE, 1, {{NULL, {D}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_COUNTER, 20},
    [T_LIGHT] = {0, T_NONE, 100, C, F, CARD_STAGE_BASIC, 3, FALSE, 1, {{NULL, {C}, 10, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_FREE_RETREAT, 0},
    [T_MEGA] = {0, T_NONE, 150, P, D, CARD_STAGE_BASIC, 2, TRUE, 1, {{NULL, {P}, 50, CARD_EFFECT_NONE, 0}}, CARD_ABILITY_NONE, 0, TRUE},
};

static const struct TrainerCardInfo sTrainers[T_CARD_END - T_POTION] =
{
    [T_POTION - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_HEAL, 20},
    [T_POKE_BALL - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_POKE_BALL, 0},
    [T_X_SPEED - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_RETREAT_DISCOUNT, 1},
    [T_RESEARCH - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW, 2},
    [T_CAPE - T_POTION] = {NULL, CARD_KIND_TOOL, CARD_TRAINER_TOOL_HP, 20},
    [T_HAND_SCOPE - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_HAND_SCOPE, 0},
    [T_RED_CARD - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_RED_CARD, 3},
    [T_RARE_CANDY - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_RARE_CANDY, 0},
    [T_HELMET - T_POTION] = {NULL, CARD_KIND_TOOL, CARD_TRAINER_TOOL_COUNTER, 20},
    [T_BROCK - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACH_ENERGY, 0, F},
    [T_MISTY - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_FLIP_ATTACH_ENERGY, 0, W},
    [T_SURGE - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_GATHER_ENERGY, 0, L},
    [T_ERIKA - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_HEAL_TYPE, 50, G},
    [T_SABRINA - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_SWITCH_OPPONENT, 0},
    [T_BLAINE - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS, 30, R},
    [T_GIOVANNI - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS, 10, CARD_TYPE_NONE},
    [T_KOGA - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_STATUS_OPPONENT, CARD_STATUS_POISONED},
    [T_BILL - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_SEARCH_POKEMON, 0},
    [T_BLUE - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_SHIELD, 10},
    [T_AGATHA - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_STATUS_OPPONENT, CARD_STATUS_CONFUSED},
    [T_DAISY - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_HEAL_ALL, 20},
    [T_CELIO - T_POTION] = {NULL, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW_UNTIL, 5},
    [T_FULL_HEAL - T_POTION] = {NULL, CARD_KIND_ITEM, CARD_TRAINER_CURE, 0},
    [T_LEFTOVERS - T_POTION] = {NULL, CARD_KIND_TOOL, CARD_TRAINER_TOOL_HEAL, 10},
    [T_CHOICE_BAND - T_POTION] = {NULL, CARD_KIND_TOOL, CARD_TRAINER_TOOL_DAMAGE, 10},
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
static const u16 sDeckC[CARD_DECK_SIZE] = {
    T_MULTI, T_MULTI, T_QUAKE, T_QUAKE, T_PUMP, T_PUMP, T_REVENGE, T_BEAM, T_HEALER, T_GASSER,
    T_DRIVER, T_SNIPER, T_SPIKY, T_LIGHT, T_GIOVANNI, T_POTION, T_POTION, T_POKE_BALL, T_POKE_BALL, T_RESEARCH,
};
static const u16 sDeckD[CARD_DECK_SIZE] = {
    T_BASIC, T_BASIC, T_STAGE1, T_STAGE2, T_STAGE2, T_RARE_CANDY, T_RARE_CANDY, T_SHELL, T_ATTACHER, T_HYPNO,
    T_MEGA, T_HAND_SCOPE, T_RED_CARD, T_HELMET, T_BROCK, T_MISTY, T_SURGE, T_ERIKA, T_SABRINA, T_BLAINE,
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

static int CountPokemonInHand(const struct CardDuel *duel, u8 side)
{
    int i, count = 0;

    for (i = 0; i < duel->sides[side].handCount; i++)
        count += CardGame_GetPokemonInfo(duel->sides[side].hand[i]) != NULL;
    return count;
}

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

static void Test_AttackEffects(void)
{
    struct CardDuel duel;
    struct CardDuelSide *me, *them;

    sTest = "attack effects";
    // Flip 3 coins, 30 per heads; then 10 to each opponent's Benched Pokémon
    StartDuel(&duel, T_MULTI, T_WALL, CARD_SIDE_OPPONENT);
    me = &duel.sides[0];
    them = &duel.sides[1];
    them->slots[1].cardId = T_BASIC;
    them->slots[3].cardId = T_FIRE;
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, C, 1);
    ForceFlips(&duel, 5, 3); // heads, tails, heads
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 60);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(them->slots[0].damage == 70 && them->slots[1].damage == 10 && them->slots[3].damage == 10);

    // Damage to its own Bench; flip to discard the defender's Energy
    StartDuel(&duel, T_QUAKE, T_WALL, CARD_SIDE_OPPONENT);
    me = &duel.sides[0];
    them = &duel.sides[1];
    me->slots[2].cardId = T_BASIC;
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, F, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 50 && me->slots[2].damage == 10 && me->slots[0].damage == 0);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_OPPONENT, 0, R, 2);
    ForceFlips(&duel, 1, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(Duel_CountEnergy(&them->slots[0]) == 1);

    // Can't attack during the next turn; drawing
    StartDuel(&duel, T_BEAM, T_WALL, CARD_SIDE_OPPONENT);
    me = &duel.sides[0];
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, C, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    EndTurn(&duel);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(me->handCount == 5); // 3 turns' draws + 2

    // Bonus with 2 extra Energy of its type; switching the defender out
    StartDuel(&duel, T_PUMP, T_WALL, CARD_SIDE_OPPONENT);
    them = &duel.sides[1];
    them->slots[2].cardId = T_FIRE;
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, W, 1);
    Energize(&duel, CARD_SIDE_PLAYER, 0, P, 2);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 50); // the extra Energy isn't Water
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, W, 2);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 160);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 1));
    CHECK(them->slots[0].cardId == T_FIRE && them->slots[2].cardId == T_WALL && them->slots[2].damage == 190);
    CHECK(CountEvents(&duel, CARD_EVENT_SWITCH) == 1);

    // Bonus against a damaged defender; sniping a Benched Pokémon (no weakness there)
    StartDuel(&duel, T_REVENGE, T_WALL, CARD_SIDE_OPPONENT);
    them = &duel.sides[1];
    them->slots[1].cardId = T_POISONER; // weak to Fighting, not Darkness
    them->slots[2].cardId = T_EX;       // weak to Darkness
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, D, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 30);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 90);
    EndTurn(&duel);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 3, 1)); // empty slot
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 1, 0)); // not a sniping attack
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 2, 1));
    CHECK(them->slots[2].damage == 20 && them->slots[0].damage == 90);
}

static void Test_Abilities(void)
{
    struct CardDuel duel;
    struct CardDuelSide *me, *them;

    sTest = "abilities";
    // Heal 20 from each of yours, once a turn, not when nothing is damaged
    StartDuel(&duel, T_HEALER, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    me->slots[0].damage = 30;
    me->slots[1].cardId = T_BASIC;
    me->slots[1].damage = 10;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 1)); // no target
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 1, 0)); // T_BASIC has none
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    CHECK(me->slots[0].damage == 10 && me->slots[1].damage == 0);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));

    // Attach its type's Energy to the Active of that type, from the Bench
    StartDuel(&duel, T_SLEEPER, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[1].cardId = T_ATTACHER;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 1, 0));
    CHECK(me->slots[0].energy[P] == 1);
    me->slots[0].cardId = T_FIRE;
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 1, 0)); // Active isn't Psychic

    // Poison the opponent's Active, only from the Active Spot
    StartDuel(&duel, T_GASSER, T_WALL, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    CHECK(them->slots[0].status == CARD_STATUS_POISONED);
    duel.sides[0].slots[1] = duel.sides[0].slots[0];
    duel.sides[0].slots[1].abilityUsedTurn = 0;
    them->slots[0].status = 0;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 1, 0));

    // Flip: Asleep
    StartDuel(&duel, T_HYPNO, T_WALL, CARD_SIDE_PLAYER);
    ForceFlips(&duel, 1, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    CHECK(duel.sides[1].slots[0].status == CARD_STATUS_ASLEEP);

    // Drive off the opponent's Active
    StartDuel(&duel, T_DRIVER, T_WALL, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0)); // no Bench
    them->slots[3].cardId = T_FIRE;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    CHECK(them->slots[0].cardId == T_FIRE && them->slots[3].cardId == T_WALL);

    // Snipe 20: a knocked out Active is replaced right away, and the turn goes on
    StartDuel(&duel, T_SNIPER, T_BASIC, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    them->slots[2].cardId = T_FIRE;
    them->slots[0].damage = 60;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0));
    CHECK(duel.sides[0].trophies == 1);
    CHECK(duel.phase == CARD_PHASE_PROMOTE && duel.promoteMidTurn);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_END_TURN, 0, 0, 0));
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PROMOTE, 0, 2, 0));
    CHECK(duel.phase == CARD_PHASE_MAIN && duel.currentSide == CARD_SIDE_PLAYER && duel.turn == 1);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_USE_ABILITY, 0, 0, 0)); // used this turn

    // Takes 20 less; hits back for 20; retreats for free
    StartDuel(&duel, T_SHELL, T_FIRE, CARD_SIDE_OPPONENT);
    EndTurn(&duel);
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_OPPONENT, 0, R, 1);
    duel.currentSide = CARD_SIDE_OPPONENT;
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].slots[0].damage == 10);
    duel.sides[0].slots[0].cardId = T_SPIKY;
    duel.sides[0].slots[0].damage = 0;
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].slots[0].damage == 30 && duel.sides[1].slots[0].damage == 20);
    duel.sides[0].slots[0].cardId = T_LIGHT;
    duel.sides[0].slots[1].cardId = T_BASIC;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_RETREAT, 0, 1, 0));
}

static void Test_MoreTrainers(void)
{
    struct CardDuel duel;
    struct CardDuelSide *me, *them;
    u8 candy;

    sTest = "more trainers";
    // Hand Scope, Red Card
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    them = &duel.sides[1];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_HAND_SCOPE);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0)); // empty hand
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_POTION);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_POTION);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_POTION);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_POTION);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(CountEvents(&duel, CARD_EVENT_REVEAL_HAND) == 1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_RED_CARD);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(them->handCount == 3);

    // Rare Candy: Basic to Stage 2, not on the first turns
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    candy = AddToHand(&duel, CARD_SIDE_PLAYER, T_RARE_CANDY);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_STAGE1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_STAGE2);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, candy, 0, 2));
    EndTurn(&duel);
    EndTurn(&duel);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, candy, 0, 1)); // a Stage 1
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, candy, 0, candy));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, candy, 0, 2));
    CHECK(me->slots[0].cardId == T_STAGE2 && me->handCount == 2 && me->hand[0] == T_STAGE1);
    CHECK(me->discard[me->discardCount - 1] == T_RARE_CANDY);

    // Rocky Helmet
    StartDuel(&duel, T_BASIC, T_FIRE, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_HELMET);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 0, 0));
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_OPPONENT, 0, R, 1);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(me->slots[0].damage == 50 && duel.sides[1].slots[0].damage == 20);

    // Brock: a Fighting Energy to a Fighting Pokémon; Misty: flips until tails
    StartDuel(&duel, T_QUAKE, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[1].cardId = T_PUMP;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BROCK);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 1, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 0, 0));
    CHECK(me->slots[0].energy[F] == 1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_MISTY);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 1, 0)); // one Supporter a turn
    EndTurn(&duel);
    EndTurn(&duel);
    ForceFlips(&duel, 3, 3); // heads, heads, tails
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_MISTY), 1, 0));
    CHECK(me->slots[1].energy[W] == 2);

    // Lt. Surge: the Bench's Lightning Energy to the Lightning Active
    StartDuel(&duel, T_SPARK, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[1].cardId = T_BASIC;
    me->slots[1].energy[L] = 2;
    me->slots[1].energy[G] = 1;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_SURGE);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(me->slots[0].energy[L] == 2 && me->slots[1].energy[L] == 0 && me->slots[1].energy[G] == 1);

    // Erika: heal 50 from a Grass Pokémon
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[0].damage = 60;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_ERIKA);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, 0, 0));
    CHECK(me->slots[0].damage == 10);

    // Sabrina: the opponent's Active is switched out
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_SABRINA);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    them->slots[2].cardId = T_FIRE;
    them->slots[0].status = CARD_STATUS_POISONED;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(them->slots[0].cardId == T_FIRE && them->slots[2].status == 0);

    // Koga: the opponent's Active is Poisoned, once
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_KOGA);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_KOGA);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_KOGA), CARD_SLOT_NONE, 0));
    CHECK(them->slots[0].status & CARD_STATUS_POISONED);
    duel.sides[0].supporterPlayed = FALSE;
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_KOGA), CARD_SLOT_NONE, 0));

    // Blaine: Fire attacks +30 this turn; Giovanni: all attacks +10
    StartDuel(&duel, T_FIRE, T_WALL, CARD_SIDE_OPPONENT);
    them = &duel.sides[1];
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, R, 1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BLAINE);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_BLAINE), CARD_SLOT_NONE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 60);
    EndTurn(&duel);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0)); // the bonus is gone
    CHECK(them->slots[0].damage == 90);
    EndTurn(&duel);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_GIOVANNI);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_GIOVANNI), CARD_SLOT_NONE, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(them->slots[0].damage == 130);
}

static void Test_MegaEx(void)
{
    struct CardDuel duel;

    sTest = "mega ex";
    StartDuel(&duel, T_EX, T_MEGA, CARD_SIDE_OPPONENT);
    AddToHand(&duel, CARD_SIDE_OPPONENT, T_FIRE);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_PLAY_BASIC, 0, 1, 0));
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, P, 1);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[0].trophies == CARD_MEGA_EX_TROPHIES);
    CHECK(duel.phase == CARD_PHASE_OVER && duel.winner == CARD_SIDE_PLAYER);
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
    struct CardDuelSetup setups[4];
    struct CardDuelSetup a, b;
    struct CardAction actions[CARD_MAX_ACTIONS];
    u32 seed, rng = 99;
    int steps, side, count, pick, i, results[3] = {0, 0, 0};
    static int eventCounts[CARD_EVENT_COUNT];
    bool8 ok;

    sTest = "random play";
    setups[0] = MakeSetup(sDeckA, G, P);
    setups[1] = MakeSetup(sDeckB, R, L);
    setups[2] = MakeSetup(sDeckC, F, W);
    setups[2].zoneTypes[2] = D;
    setups[3] = MakeSetup(sDeckD, G, P);
    setups[3].zoneTypes[2] = W;
    for (seed = 0; seed < 4000; seed++)
    {
        a = setups[seed % 4];
        b = setups[(seed / 4 + seed) % 4];
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
    for (i = 0; i < CARD_EVENT_COUNT; i++)
    {
        if (eventCounts[i] == 0)
            printf("  random play never had event %d\n", i);
        CHECK(eventCounts[i] > 0);
    }

    // The same seed and actions give the same duel
    CHECK(Duel_Init(&duel, &setups[0], &setups[1], 42));
    CHECK(Duel_Init(&replay, &setups[0], &setups[1], 42));
    CHECK(memcmp(&duel, &replay, sizeof(duel)) == 0);
}

// Bill, Blue, Agatha, Daisy, Celio, Full Heal, Leftovers, Choice Band
static void Test_LateTrainers(void)
{
    struct CardDuel duel;
    struct CardDuelSide *me, *them;
    int pokemon;

    // Bill: a Pokémon from the deck
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    pokemon = CountPokemonInHand(&duel, CARD_SIDE_PLAYER);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BILL);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_BILL), CARD_SLOT_NONE, 0));
    CHECK(CountPokemonInHand(&duel, CARD_SIDE_PLAYER) == pokemon + 1);

    // Blue: 10 less damage during the opponent's next turn only
    StartDuel(&duel, T_WALL, T_FIRE, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_BLUE);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_BLUE), CARD_SLOT_NONE, 0));
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_OPPONENT, 0, R, 1);
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(me->slots[0].damage == 20);
    EndTurn(&duel); // the attack ended the opponent's turn
    CHECK(Act(&duel, CARD_SIDE_OPPONENT, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(me->slots[0].damage == 50);

    // Agatha: the opponent's Active is Confused
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    them = &duel.sides[1];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_AGATHA);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_AGATHA), CARD_SLOT_NONE, 0));
    CHECK(them->slots[0].status == CARD_STATUS_CONFUSED);

    // Daisy: 20 from each damaged Pokémon
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[0].damage = 60;
    me->slots[1].cardId = T_BASIC;
    me->slots[1].damage = 10;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_DAISY);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_DAISY), CARD_SLOT_NONE, 0));
    CHECK(me->slots[0].damage == 40 && me->slots[1].damage == 0);

    // Celio: draw until 5 cards; not with 5 or more besides itself
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->handCount = 0;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_CELIO);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, 0, CARD_SLOT_NONE, 0));
    CHECK(me->handCount == 5);
    duel.sides[0].supporterPlayed = FALSE;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_CELIO);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_CELIO), CARD_SLOT_NONE, 0));

    // Full Heal: every Special Condition of the Active, only when it has one
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    AddToHand(&duel, CARD_SIDE_PLAYER, T_FULL_HEAL);
    CHECK(!Legal(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_FULL_HEAL), CARD_SLOT_NONE, 0));
    me->slots[0].status = CARD_STATUS_POISONED | CARD_STATUS_CONFUSED;
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_FULL_HEAL), CARD_SLOT_NONE, 0));
    CHECK(me->slots[0].status == 0);

    // Leftovers: 10 at the end of its owner's turn, as the Active
    StartDuel(&duel, T_BASIC, T_WALL, CARD_SIDE_PLAYER);
    me = &duel.sides[0];
    me->slots[0].damage = 30;
    AddToHand(&duel, CARD_SIDE_PLAYER, T_LEFTOVERS);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_LEFTOVERS), 0, 0));
    CHECK(me->slots[0].damage == 30);
    EndTurn(&duel);
    CHECK(me->slots[0].damage == 20);
    EndTurn(&duel);
    CHECK(me->slots[0].damage == 20);

    // Choice Band: 10 more damage to the opponent's Active
    StartDuel(&duel, T_WALL, T_FIRE, CARD_SIDE_OPPONENT);
    me = &duel.sides[0];
    EndTurn(&duel);
    Energize(&duel, CARD_SIDE_PLAYER, 0, G, 1);
    AddToHand(&duel, CARD_SIDE_PLAYER, T_CHOICE_BAND);
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_PLAY_TRAINER, FindInHand(&duel, CARD_SIDE_PLAYER, T_CHOICE_BAND), 0, 0));
    CHECK(Act(&duel, CARD_SIDE_PLAYER, CARD_ACTION_ATTACK, 0, 0, 0));
    CHECK(duel.sides[1].slots[0].damage == 20);
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
    Test_AttackEffects();
    Test_Abilities();
    Test_MoreTrainers();
    Test_MegaEx();
    Test_LateTrainers();
    Test_RandomPlay();
    printf("%d checks, %d failures\n", sChecks, sFailures);
    return sFailures != 0;
}
