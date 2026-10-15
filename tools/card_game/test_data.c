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

// Counts the characters of each line of a text (UTF-8 here: the host build keeps the strings as
// they are written), and returns the longest; -1 if it has more than 2 lines, the message box's
static int LongestLine(const char *text)
{
    int longest = 0, length = 0, lines = 1;

    for (; *text != '\0'; text++)
    {
        if (*text == '\n')
        {
            length = 0;
            if (++lines > 2)
                return -1;
            continue;
        }
        if ((*text & 0xC0) != 0x80)
            length++;
        if (length > longest)
            longest = length;
    }
    return longest;
}

// The card players (src/data/card_game/opponents.h): a deck, a reward that is a card, lines that
// fit in the message box, and a different reward for each
static void CheckOpponents(void)
{
    int o, p;

    for (o = 0; o < CARD_OPPONENTS_COUNT; o++)
    {
        const struct CardOpponent *opponent = &gCardOpponents[o];

        CHECK(opponent->name != NULL && opponent->challengeText != NULL
           && opponent->playerWonText != NULL && opponent->playerLostText != NULL, "opponent %d: a text is missing", o);
        if (opponent->name == NULL || opponent->challengeText == NULL || opponent->playerWonText == NULL || opponent->playerLostText == NULL)
            continue;
        CHECK(opponent->deck != CARD_DECK_STARTER && opponent->deck < CARD_DECKS_COUNT, "opponent %d: deck", o);
        CHECK(opponent->rewardCard == CARD_NONE || CardGame_GetCardKind(opponent->rewardCard) != CARD_KIND_NONE, "opponent %d: reward", o);
        // No paragraphs (\p is unknown to the host's compiler): 2 lines at most
        CHECK(LongestLine((const char *)opponent->name) >= 0 && LongestLine((const char *)opponent->name) <= 10, "opponent %d: name", o);
        CHECK(LongestLine((const char *)opponent->challengeText) >= 0 && LongestLine((const char *)opponent->challengeText) <= 32, "opponent %d: challenge too long", o);
        CHECK(LongestLine((const char *)opponent->playerWonText) >= 0 && LongestLine((const char *)opponent->playerWonText) <= 32, "opponent %d: won line too long", o);
        CHECK(LongestLine((const char *)opponent->playerLostText) >= 0 && LongestLine((const char *)opponent->playerLostText) <= 32, "opponent %d: lost line too long", o);
        for (p = 0; p < o; p++)
            CHECK(opponent->rewardCard == CARD_NONE || opponent->rewardCard != gCardOpponents[p].rewardCard, "opponent %d: same reward as %d", o, p);
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
            for (game = 0; game < 40; game++)
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
                        CardAi_ChooseAction(&duel, side, CARD_AI_HARD, actions, &chosen);
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
    printf("  AI against AI, the games each deck won (out of 40 against each deck, on both sides):\n");
    for (a = 0; a < CARD_DECKS_COUNT; a++)
    {
        count = 0;
        for (b = 0; b < CARD_DECKS_COUNT; b++)
            count += wins[a][b] + 40 - wins[b][a];
        printf("    %-12s %3d%%\n", (const char *)gCardDecks[a].name, count * 100 / (80 * CARD_DECKS_COUNT));
    }
}

// A duel in the middle of the player's turn 5, with empty boards and hands, the player's Energy
// Zone making zoneType
static void MakeScenario(struct CardDuel *duel, u8 zoneType)
{
    struct CardDuelSetup setup;
    u8 side;

    setup.deck = gCardDecks[CARD_DECK_STARTER].cards;
    memset(setup.zoneTypes, 0, sizeof(setup.zoneTypes));
    setup.zoneTypes[0] = CARD_TYPE_GRASS;
    Duel_Init(duel, &setup, &setup, 1);
    duel->phase = CARD_PHASE_MAIN;
    duel->turn = 5;
    duel->currentSide = CARD_SIDE_PLAYER;
    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        memset(duel->sides[side].slots, 0, sizeof(duel->sides[side].slots));
        memset(duel->sides[side].hand, 0, sizeof(duel->sides[side].hand));
        duel->sides[side].handCount = 0;
        duel->sides[side].setupDone = TRUE;
        duel->sides[side].zoneEnergy = CARD_TYPE_NONE;
        duel->sides[side].slots[CARD_SLOT_ACTIVE].cardId = CARD_ONIX; // something to aim at
    }
    duel->sides[CARD_SIDE_PLAYER].zoneTypes[0] = zoneType;
}

static void Choose(struct CardDuel *duel, u8 level, struct CardAction *chosen)
{
    struct CardAction actions[CARD_MAX_ACTIONS];

    CardAi_ChooseAction(duel, CARD_SIDE_PLAYER, level, actions, chosen);
}

static void CheckAiDecisions(void)
{
    struct CardDuel duel;
    struct CardDuelSide *me = &duel.sides[CARD_SIDE_PLAYER];
    struct CardDuelSide *them = &duel.sides[CARD_SIDE_OPPONENT];
    struct CardAction chosen;
    u8 level;

    for (level = CARD_AI_NORMAL; level <= CARD_AI_HARD; level++)
    {
        // A sure knockout: attack (Geodude's 40 + weakness against Pikachu's 30 HP left)
        MakeScenario(&duel, CARD_TYPE_FIGHTING);
        me->slots[0].cardId = CARD_GEODUDE;
        me->slots[0].energy[CARD_TYPE_FIGHTING] = 2;
        them->slots[0].cardId = CARD_PIKACHU;
        them->slots[0].damage = 30;
        Choose(&duel, level, &chosen);
        CHECK(chosen.type == CARD_ACTION_ATTACK, "level %d: knockout", level);

        // Evolve the Bench
        MakeScenario(&duel, CARD_TYPE_GRASS);
        me->slots[0].cardId = CARD_WEEDLE;
        me->slots[1].cardId = CARD_CATERPIE;
        me->slots[1].turnPlayed = 1;
        me->hand[me->handCount++] = CARD_METAPOD;
        Choose(&duel, level, &chosen);
        CHECK(chosen.type == CARD_ACTION_EVOLVE && chosen.slot == 1, "level %d: evolve", level);

        // The Energy where it completes an attack
        MakeScenario(&duel, CARD_TYPE_GRASS);
        me->slots[0].cardId = CARD_BULBASAUR;
        me->slots[0].energy[CARD_TYPE_GRASS] = 1;
        me->slots[1].cardId = CARD_RATTATA;
        me->zoneEnergy = CARD_TYPE_GRASS;
        Choose(&duel, level, &chosen);
        CHECK(chosen.type == CARD_ACTION_ATTACH_ENERGY && chosen.slot == 0, "level %d: Energy", level);

        // Giovanni for a knockout: HARD only
        MakeScenario(&duel, CARD_TYPE_FIGHTING);
        me->slots[0].cardId = CARD_GEODUDE;
        me->slots[0].energy[CARD_TYPE_FIGHTING] = 2;
        them->slots[0].damage = 65; // Onix: 45 HP left, Geodude does 40
        me->hand[me->handCount++] = CARD_GIOVANNI;
        Choose(&duel, level, &chosen);
        if (level == CARD_AI_HARD)
            CHECK(chosen.type == CARD_ACTION_PLAY_TRAINER, "HARD: Giovanni");
        else
            CHECK(chosen.type == CARD_ACTION_ATTACK, "NORMAL: attack without Giovanni");

        // A stuck Active (no Lightning Energy in a Fighting deck), a Benched one ready: HARD retreats
        MakeScenario(&duel, CARD_TYPE_FIGHTING);
        me->slots[0].cardId = CARD_PIKACHU;
        me->slots[0].energy[CARD_TYPE_FIGHTING] = 1;
        me->slots[1].cardId = CARD_GEODUDE;
        me->slots[1].energy[CARD_TYPE_FIGHTING] = 2;
        Choose(&duel, level, &chosen);
        if (level == CARD_AI_HARD)
            CHECK(chosen.type == CARD_ACTION_RETREAT && chosen.slot == 1, "HARD: retreat");
        else
            CHECK(chosen.type == CARD_ACTION_END_TURN, "NORMAL: no retreat");

        // After a knockout, send out the Pokémon that can attack
        MakeScenario(&duel, CARD_TYPE_FIGHTING);
        me->slots[0].cardId = CARD_NONE;
        me->slots[1].cardId = CARD_MAGIKARP;
        me->slots[2].cardId = CARD_GEODUDE;
        me->slots[2].energy[CARD_TYPE_FIGHTING] = 1;
        duel.phase = CARD_PHASE_PROMOTE;
        duel.promoteMask = 1 << CARD_SIDE_PLAYER;
        Choose(&duel, level, &chosen);
        CHECK(chosen.type == CARD_ACTION_PROMOTE && chosen.slot == 2, "level %d: promotion", level);

        // Setup: the Active that can attack with the deck's Energy
        MakeScenario(&duel, CARD_TYPE_FIGHTING);
        me->slots[0].cardId = CARD_NONE;
        me->setupDone = FALSE;
        duel.phase = CARD_PHASE_SETUP;
        me->hand[me->handCount++] = CARD_PIKACHU;
        me->hand[me->handCount++] = CARD_GEODUDE;
        Choose(&duel, level, &chosen);
        CHECK(chosen.type == CARD_ACTION_PLAY_BASIC && chosen.slot == CARD_SLOT_ACTIVE && me->hand[chosen.handIndex] == CARD_GEODUDE,
              "level %d: setup", level);
    }
}

// Plays games between two AI levels with every deck (each level on both sides); returns the
// percentage of the games the first level won, and prints it with the draws
static int PlayLevels(u8 levelA, u8 levelB, const char *name)
{
    struct CardDuel duel;
    struct CardDuelSetup setups[CARD_DECKS_COUNT];
    struct CardAction actions[CARD_MAX_ACTIONS], chosen;
    int d, e, game, steps, side, winsA = 0, draws = 0, games = 0;
    u8 levels[CARD_SIDE_COUNT];

    for (d = 0; d < CARD_DECKS_COUNT; d++)
    {
        setups[d].deck = gCardDecks[d].cards;
        memcpy(setups[d].zoneTypes, gCardDecks[d].zoneTypes, CARD_MAX_ZONE_TYPES);
    }
    for (d = 0; d < CARD_DECKS_COUNT; d++)
    {
        for (e = 0; e < CARD_DECKS_COUNT; e++)
        {
            for (game = 0; game < 16; game++, games++)
            {
                // levelA plays the player's side in even games
                levels[CARD_SIDE_PLAYER] = (game & 1) ? levelB : levelA;
                levels[CARD_SIDE_OPPONENT] = (game & 1) ? levelA : levelB;
                Duel_Init(&duel, &setups[d], &setups[e], game * 977 + d * 13 + e);
                for (steps = 0; steps < 3000 && duel.phase != CARD_PHASE_OVER; steps++)
                {
                    Duel_ClearEvents(&duel);
                    for (side = 0; side < CARD_SIDE_COUNT; side++)
                    {
                        if (!Duel_CanAct(&duel, side))
                            continue;
                        CardAi_ChooseAction(&duel, side, levels[side], actions, &chosen);
                        CHECK(Duel_DoAction(&duel, side, &chosen), "%s: illegal action", name);
                        break;
                    }
                }
                CHECK(duel.phase == CARD_PHASE_OVER, "%s: game didn't end", name);
                if (duel.winner == CARD_RESULT_DRAW)
                    draws++;
                else if (duel.winner == ((game & 1) ? CARD_SIDE_OPPONENT : CARD_SIDE_PLAYER))
                    winsA++;
            }
        }
    }
    printf("  %s: %d%% won, %d%% draws\n", name, winsA * 100 / games, draws * 100 / games);
    return winsA * 100 / games;
}

// The starter deck (played by NORMAL, standing in for the player) against each card player at
// their level: the difficulty curve along the story. The EASY ones must be beatable.
static void PlayStarterAgainstOpponents(void)
{
    struct CardDuel duel;
    struct CardDuelSetup starter, setup;
    struct CardAction actions[CARD_MAX_ACTIONS], chosen;
    int o, game, steps, side, wins;
    u8 levels[CARD_SIDE_COUNT];

    starter.deck = gCardDecks[CARD_DECK_STARTER].cards;
    memcpy(starter.zoneTypes, gCardDecks[CARD_DECK_STARTER].zoneTypes, CARD_MAX_ZONE_TYPES);
    printf("  The starter deck (NORMAL) against each card player:\n");
    for (o = 0; o < CARD_OPPONENTS_COUNT; o++)
    {
        const struct CardDeckData *deck = &gCardDecks[gCardOpponents[o].deck];

        setup.deck = deck->cards;
        memcpy(setup.zoneTypes, deck->zoneTypes, CARD_MAX_ZONE_TYPES);
        levels[CARD_SIDE_PLAYER] = CARD_AI_NORMAL;
        levels[CARD_SIDE_OPPONENT] = deck->aiLevel;
        for (game = 0, wins = 0; game < 100; game++)
        {
            Duel_Init(&duel, &starter, &setup, game * 7919 + o);
            for (steps = 0; steps < 3000 && duel.phase != CARD_PHASE_OVER; steps++)
            {
                Duel_ClearEvents(&duel);
                for (side = 0; side < CARD_SIDE_COUNT; side++)
                {
                    if (!Duel_CanAct(&duel, side))
                        continue;
                    CardAi_ChooseAction(&duel, side, levels[side], actions, &chosen);
                    CHECK(Duel_DoAction(&duel, side, &chosen), "starter against %d: illegal action", o);
                    break;
                }
            }
            if (duel.winner == CARD_SIDE_PLAYER)
                wins++;
        }
        printf("    %-10s %3d%%\n", (const char *)gCardOpponents[o].name, wins);
        if (deck->aiLevel == CARD_AI_EASY)
            CHECK(wins >= 60, "the starter deck should beat %s (EASY)", (const char *)gCardOpponents[o].name);
    }
}

int main(void)
{
    CheckPokemonCards();
    CheckTrainerCards();
    CheckDecks();
    CheckOpponents();
    PlayRandomGames();
    CheckAiDecisions();
    CHECK(PlayLevels(CARD_AI_HARD, CARD_AI_EASY, "HARD against EASY") >= 70, "HARD should beat EASY");
    CHECK(PlayLevels(CARD_AI_NORMAL, CARD_AI_EASY, "NORMAL against EASY") >= 65, "NORMAL should beat EASY");
    PlayLevels(CARD_AI_HARD, CARD_AI_NORMAL, "HARD against NORMAL");
    PlayStarterAgainstOpponents();
    printf("%d checks, %d failures\n", sChecks, sFailures);
    return sFailures != 0;
}
