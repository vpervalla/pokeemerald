// Host checks of the card game's real data (src/card_game_data.c): every generated and
// hand-written card is well formed, every deck is legal and can attack with its Energy Zone,
// and the decks play full random games against each other. Built by tools/card_game/run_tests.sh.

#include <stdio.h>
#include <string.h>
#include "global.h"
#include "card_game.h"
#include "constants/moves.h"

// Names only; the host build has no move data
const u8 gMoveNames[MOVES_COUNT][MOVE_NAME_LENGTH + 1];

static int sChecks, sFailures;

#define CHECK(cond, ...) do { sChecks++; if (!(cond)) { sFailures++; printf("FAIL %s:%d ", __FILE__, __LINE__); printf(__VA_ARGS__); printf(" (%s)\n", #cond); } } while (0)

static bool8 IsStatus(u8 status)
{
    return status == CARD_STATUS_ASLEEP || status == CARD_STATUS_PARALYZED || status == CARD_STATUS_CONFUSED
        || status == CARD_STATUS_POISONED || status == CARD_STATUS_BURNED;
}

static void CheckPokemonCards(void)
{
    u16 id;
    int i, j, abilities = 0, ex = 0;

    for (id = 1; id < CARD_POKEMON_END; id++)
    {
        const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(id);
        const struct PokemonCardInfo *from;

        CHECK(info != NULL && CardGame_GetCardKind(id) == CARD_KIND_POKEMON, "card %d", id);
        if (info == NULL)
            continue;
        CHECK(info->species != 0, "card %d", id);
        CHECK(info->hp >= 30 && info->hp <= 250 && info->hp % 10 == 0, "card %d hp %d", id, info->hp);
        CHECK(info->type >= CARD_TYPE_GRASS && info->type <= CARD_TYPE_COLORLESS, "card %d", id);
        CHECK(info->weakness <= CARD_TYPE_COLORLESS, "card %d", id);
        CHECK(info->retreatCost <= 4, "card %d", id);
        CHECK(info->numAttacks >= 1 && info->numAttacks <= CARD_MAX_ATTACKS, "card %d", id);
        CHECK(info->ability < CARD_ABILITY_COUNT, "card %d", id);
        CHECK(!info->isMega || info->isEx, "card %d", id);
        if (info->stage == CARD_STAGE_BASIC)
        {
            CHECK(info->evolvesFrom == CARD_NONE, "card %d", id);
        }
        else
        {
            from = CardGame_GetPokemonInfo(info->evolvesFrom);
            CHECK(from != NULL && from->stage == info->stage - 1, "card %d evolves from %d", id, info->evolvesFrom);
        }
        abilities += info->ability != CARD_ABILITY_NONE;
        ex += info->isEx;
        for (i = 0; i < info->numAttacks; i++)
        {
            const struct CardAttack *attack = &info->attacks[i];

            CHECK(attack->name != NULL, "card %d attack %d", id, i);
            CHECK(attack->cost[0] != CARD_TYPE_NONE, "card %d attack %d has no cost", id, i);
            for (j = 0; j < CARD_ATTACK_MAX_COST; j++)
                CHECK(attack->cost[j] <= CARD_TYPE_COLORLESS && attack->cost[j] != CARD_TYPE_DRAGON, "card %d attack %d", id, i);
            CHECK(attack->damage % 10 == 0, "card %d attack %d", id, i);
            CHECK(attack->effect < CARD_EFFECT_COUNT, "card %d attack %d", id, i);
            if (attack->effect == CARD_EFFECT_STATUS || attack->effect == CARD_EFFECT_FLIP_STATUS)
                CHECK(IsStatus(attack->effectArg), "card %d attack %d", id, i);
            if (attack->effect == CARD_EFFECT_FLIP_MULTI)
                CHECK(attack->effectArg >= 2, "card %d attack %d", id, i);
        }
    }
    printf("  %d Pokémon cards, %d ex, %d with an ability\n", CARD_POKEMON_END - 1, ex, abilities);
}

static void CheckTrainerCards(void)
{
    u16 id;

    for (id = CARD_POKEMON_END; id < CARD_COUNT; id++)
    {
        const struct TrainerCardInfo *info = CardGame_GetTrainerInfo(id);

        CHECK(info != NULL && info->name != NULL, "trainer %d", id);
        CHECK(info->kind >= CARD_KIND_ITEM && info->kind <= CARD_KIND_TOOL, "trainer %d", id);
        CHECK(info->effect != 0, "trainer %d", id);
        CHECK((info->kind == CARD_KIND_TOOL) == (info->effect == CARD_TRAINER_TOOL_HP || info->effect == CARD_TRAINER_TOOL_COUNTER), "trainer %d", id);
    }
    CHECK(CardGame_GetCardKind(CARD_COUNT) == CARD_KIND_NONE, "past the end");
    CHECK(CardGame_GetCardKind(CARD_NONE) == CARD_KIND_NONE, "none");
}

// The attack can be paid with Energy from these zone types only
static bool8 CanPayWithZone(const struct CardAttack *attack, const u8 *zoneTypes)
{
    int i, j;
    bool8 found;

    for (i = 0; i < CARD_ATTACK_MAX_COST && attack->cost[i] != CARD_TYPE_NONE; i++)
    {
        if (attack->cost[i] == CARD_TYPE_COLORLESS)
            continue;
        found = FALSE;
        for (j = 0; j < CARD_MAX_ZONE_TYPES; j++)
            found |= zoneTypes[j] == attack->cost[i];
        if (!found)
            return FALSE;
    }
    return TRUE;
}

static void CheckDecks(void)
{
    int d, i, j;
    bool8 usable;

    for (d = 0; d < CARD_DECKS_COUNT; d++)
    {
        const struct CardDeckData *deck = &gCardDecks[d];

        CHECK(Duel_ValidateDeck(deck->cards, deck->zoneTypes) == CARD_DECK_OK, "deck %d", d);
        for (i = 0; i < CARD_DECK_SIZE; i++)
        {
            const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(deck->cards[i]);

            if (info == NULL)
                continue;
            usable = FALSE;
            for (j = 0; j < info->numAttacks; j++)
                usable |= CanPayWithZone(&info->attacks[j], deck->zoneTypes);
            CHECK(usable, "deck %d: card %d can't attack with the deck's Energy", d, deck->cards[i]);
            if (info->stage != CARD_STAGE_BASIC)
            {
                usable = FALSE;
                for (j = 0; j < CARD_DECK_SIZE; j++)
                    usable |= deck->cards[j] == info->evolvesFrom;
                CHECK(usable, "deck %d: card %d has nothing to evolve from", d, deck->cards[i]);
            }
        }
    }
}

// The game's opponent (src/card_game_ai.c) against itself with every pair of decks: it only
// picks legal actions, games end, and the deck pairings get a rough balance report.
static void PlayRandomGames(void)
{
    struct CardDuel duel;
    struct CardDuelSetup setups[CARD_DECKS_COUNT];
    struct CardAction actions[CARD_MAX_ACTIONS], chosen;
    int a, b, game, steps, side, count, wins[CARD_DECKS_COUNT][CARD_DECKS_COUNT];

    memset(wins, 0, sizeof(wins));
    for (a = 0; a < CARD_DECKS_COUNT; a++)
    {
        setups[a].deck = gCardDecks[a].cards;
        memcpy(setups[a].zoneTypes, gCardDecks[a].zoneTypes, CARD_MAX_ZONE_TYPES);
    }
    for (a = 0; a < CARD_DECKS_COUNT; a++)
    {
        for (b = 0; b < CARD_DECKS_COUNT; b++)
        {
            for (game = 0; game < 300; game++)
            {
                CHECK(Duel_Init(&duel, &setups[a], &setups[b], game * 31 + a * 7 + b), "init");
                for (steps = 0; steps < 3000 && duel.phase != CARD_PHASE_OVER; steps++)
                {
                    Duel_ClearEvents(&duel);
                    for (side = 0; side < CARD_SIDE_COUNT; side++)
                    {
                        if (!Duel_CanAct(&duel, side))
                            continue;
                        count = Duel_GetLegalActions(&duel, side, actions);
                        CHECK(count > 0 && count < CARD_MAX_ACTIONS, "actions");
                        // The game's opponent plays both sides
                        CardAi_ChooseAction(&duel, side, actions, &chosen);
                        CHECK(Duel_DoAction(&duel, side, &chosen), "deck %d vs %d: the AI chose an illegal action", a, b);
                        break;
                    }
                }
                CHECK(duel.phase == CARD_PHASE_OVER, "deck %d vs %d: game didn't end", a, b);
                if (duel.winner == CARD_SIDE_PLAYER)
                    wins[a][b]++;
            }
        }
    }
    printf("  AI against AI, wins of the row's deck out of 300:\n");
    for (a = 0; a < CARD_DECKS_COUNT; a++)
    {
        printf("   ");
        for (b = 0; b < CARD_DECKS_COUNT; b++)
            printf(" %4d", wins[a][b]);
        printf("\n");
    }
}

int main(void)
{
    CheckPokemonCards();
    CheckTrainerCards();
    CheckDecks();
    PlayRandomGames();
    printf("%d checks, %d failures\n", sChecks, sFailures);
    return sFailures != 0;
}
