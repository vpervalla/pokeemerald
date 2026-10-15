#include "global.h"
#include "card_game.h"
#include "event_data.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "task.h"
#include "constants/characters.h"
#include "constants/rgb.h"

// The player's cards and decks (docs/card_game_plan.md, section 3), saved in
// gSaveBlock1Ptr->cardGame.
//
// A Pokémon card counts as owned twice (the most a deck can hold) when its species has been
// caught; otherwise the player owns the copies received from packs, gifts and opponents.
// ex and Trainer cards are only received.

STATIC_ASSERT(CARD_FIRST_EX - 1 <= sizeof(((struct CardGameSave *)0)->pokemonCopies) * 4, CardGameSavePokemon);
STATIC_ASSERT(CARD_POKEMON_END - CARD_FIRST_EX <= sizeof(((struct CardGameSave *)0)->exCopies), CardGameSaveEx);
STATIC_ASSERT(CARD_COUNT - CARD_POKEMON_END <= sizeof(((struct CardGameSave *)0)->trainerCopies), CardGameSaveTrainers);
STATIC_ASSERT(CARD_SAVED_DECKS == ARRAY_COUNT(((struct CardGameSave *)0)->decks), CardGameSaveDecks);

static struct CardGameSave *GetSave(void)
{
    return &gSaveBlock1Ptr->cardGame;
}

static u8 GetPokemonCopiesReceived(u16 cardId)
{
    return (GetSave()->pokemonCopies[(cardId - 1) / 4] >> (((cardId - 1) % 4) * 2)) & 3;
}

static void SetPokemonCopiesReceived(u16 cardId, u8 copies)
{
    u8 *byte = &GetSave()->pokemonCopies[(cardId - 1) / 4];
    u8 shift = ((cardId - 1) % 4) * 2;

    *byte = (*byte & ~(3 << shift)) | (copies << shift);
}

static bool8 IsCaught(u16 cardId)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(cardId);

    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(info->species), FLAG_GET_CAUGHT);
}

u8 CardGame_GetOwnedCopies(u16 cardId)
{
    if (cardId == CARD_NONE || cardId >= CARD_COUNT)
        return 0;
    if (cardId >= CARD_POKEMON_END)
        return GetSave()->trainerCopies[cardId - CARD_POKEMON_END];
    if (cardId >= CARD_FIRST_EX)
        return GetSave()->exCopies[cardId - CARD_FIRST_EX];
    if (IsCaught(cardId))
        return CARD_MAX_COPIES;
    return GetPokemonCopiesReceived(cardId);
}

// One more copy, up to CARD_MAX_COPIES; FALSE if the player already has them all
bool8 CardGame_GiveCard(u16 cardId)
{
    u8 copies = CardGame_GetOwnedCopies(cardId);

    if (cardId == CARD_NONE || cardId >= CARD_COUNT || copies >= CARD_MAX_COPIES)
        return FALSE;
    if (cardId >= CARD_POKEMON_END)
        GetSave()->trainerCopies[cardId - CARD_POKEMON_END] = copies + 1;
    else if (cardId >= CARD_FIRST_EX)
        GetSave()->exCopies[cardId - CARD_FIRST_EX] = copies + 1;
    else
        SetPokemonCopiesReceived(cardId, copies + 1);
    return TRUE;
}

u8 CardGame_CountCopies(const u16 *deck, u16 cardId)
{
    u8 i, count = 0;

    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (deck[i] == cardId)
            count++;
    }
    return count;
}

u16 *CardGame_GetSavedDeck(u8 deckIndex)
{
    return GetSave()->decks[deckIndex < CARD_SAVED_DECKS ? deckIndex : 0];
}

void CardGame_GetDeckZone(u8 deckIndex, u8 *zoneTypes)
{
    u16 zone = GetSave()->deckZones[deckIndex < CARD_SAVED_DECKS ? deckIndex : 0];
    u8 i;

    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
        zoneTypes[i] = (zone >> (i * 4)) & 0xF;
}

void CardGame_SetDeckZone(u8 deckIndex, const u8 *zoneTypes)
{
    u16 zone = 0;
    u8 i;

    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
        zone |= (zoneTypes[i] & 0xF) << (i * 4);
    GetSave()->deckZones[deckIndex < CARD_SAVED_DECKS ? deckIndex : 0] = zone;
}

// CARD_DECK_OK if the saved deck follows the rules and only uses owned cards
u8 CardGame_ValidateSavedDeck(u8 deckIndex)
{
    const u16 *deck = CardGame_GetSavedDeck(deckIndex);
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];
    u8 i, result;

    CardGame_GetDeckZone(deckIndex, zoneTypes);
    result = Duel_ValidateDeck(deck, zoneTypes);
    if (result != CARD_DECK_OK)
        return result;
    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (CardGame_CountCopies(deck, deck[i]) > CardGame_GetOwnedCopies(deck[i]))
            return CARD_DECK_NOT_OWNED;
    }
    return CARD_DECK_OK;
}

// The deck the player duels with; FALSE if it can't be played
bool8 CardGame_GetPlayerDeck(struct CardDuelSetup *setup)
{
    u8 deckIndex = GetSave()->activeDeck;

    setup->deck = CardGame_GetSavedDeck(deckIndex);
    CardGame_GetDeckZone(deckIndex, setup->zoneTypes);
    return CardGame_ValidateSavedDeck(deckIndex) == CARD_DECK_OK;
}

void CardGame_RecordResult(u8 result)
{
    u16 *count;

    if (result == CARD_GAME_RESULT_WON)
        count = &GetSave()->wins;
    else if (result == CARD_GAME_RESULT_DRAW)
        count = &GetSave()->draws;
    else
        count = &GetSave()->losses;
    if (*count < 0xFFFF)
        (*count)++;
}

// A card of a pack: mostly Trainers, often a Pokémon, rarely an ex
u16 CardGame_RollPackCard(void)
{
    u16 roll = Random() % 100;

    if (roll < 5)
        return CARD_FIRST_EX + Random() % (CARD_POKEMON_END - CARD_FIRST_EX);
    if (roll < 45)
        return 1 + Random() % (CARD_FIRST_EX - 1);
    return CARD_POKEMON_END + Random() % (CARD_COUNT - CARD_POKEMON_END);
}

// ---------------------------------------------------------------------------------------------
// Specials

// The starter deck's cards, and the deck itself as the first saved deck
void CardGame_GiveStarterDeck(void)
{
    const struct CardDeckData *starter = &gCardDecks[CARD_DECK_STARTER];
    u16 *deck = CardGame_GetSavedDeck(0);
    u8 i;

    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        while (CardGame_GetOwnedCopies(starter->cards[i]) < CardGame_CountCopies(starter->cards, starter->cards[i]))
            CardGame_GiveCard(starter->cards[i]);
        deck[i] = starter->cards[i];
    }
    CardGame_SetDeckZone(0, starter->zoneTypes);
    GetSave()->activeDeck = 0;
    GetSave()->flags |= CARD_SAVE_FLAG_STARTER;
}

// VAR_RESULT: TRUE if the player's deck can be played
void CardGame_CheckActiveDeck(void)
{
    gSpecialVar_Result = CardGame_ValidateSavedDeck(GetSave()->activeDeck) == CARD_DECK_OK;
}

// The card players (gCardOpponents): met, beaten, and what they say

bool8 CardGame_HasMetOpponent(u8 opponentId)
{
    return (GetSave()->opponentsMet[opponentId / 8] >> (opponentId % 8)) & 1;
}

bool8 CardGame_HasBeatenOpponent(u8 opponentId)
{
    return (GetSave()->opponentsBeaten[opponentId / 8] >> (opponentId % 8)) & 1;
}

static u8 GetScriptOpponent(void)
{
    return gSpecialVar_0x8004 < CARD_OPPONENTS_COUNT ? gSpecialVar_0x8004 : 0;
}

// VAR_0x8004: the opponent, now on the Card Player list
void CardGame_MeetOpponent(void)
{
    u8 opponentId = GetScriptOpponent();

    GetSave()->opponentsMet[opponentId / 8] |= 1 << (opponentId % 8);
}

// VAR_0x8004: the opponent, VAR_0x8005: CARD_OPPONENT_TEXT_*. The line goes to gStringVar4.
void CardGame_BufferOpponentText(void)
{
    const struct CardOpponent *opponent = &gCardOpponents[GetScriptOpponent()];
    const u8 *text;

    if (gSpecialVar_0x8005 == CARD_OPPONENT_TEXT_PLAYER_WON)
        text = opponent->playerWonText;
    else if (gSpecialVar_0x8005 == CARD_OPPONENT_TEXT_PLAYER_LOST)
        text = opponent->playerLostText;
    else
        text = opponent->challengeText;
    StringExpandPlaceholders(gStringVar4, text);
}

// After the player beat VAR_0x8004. The first time: beaten, and the reward card received.
// VAR_RESULT: TRUE if a card was received (its name in gStringVar1).
void CardGame_RewardOpponentWin(void)
{
    u8 opponentId = GetScriptOpponent();
    u16 reward = gCardOpponents[opponentId].rewardCard;

    gSpecialVar_Result = FALSE;
    if (CardGame_HasBeatenOpponent(opponentId))
        return;
    GetSave()->opponentsBeaten[opponentId / 8] |= 1 << (opponentId % 8);
    if (reward == CARD_NONE)
        return;
    CardGame_GiveCard(reward);
    gStringVar1[0] = EOS;
    CardGame_AppendCardName(gStringVar1, reward);
    gSpecialVar_Result = TRUE;
}

static EWRAM_DATA u16 sPackCards[CARD_PACK_SIZE] = {0};
static EWRAM_DATA u8 sPackCount = 0;

#define tState  data[0]
#define tIsPack data[1]

// From a script: fade out of the field, then open the screen, which returns to the script
static void Task_OpenFromField(u8 taskId)
{
    if (gTasks[taskId].tState == 0)
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].tState++;
    }
    else if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        if (gTasks[taskId].tIsPack)
            OpenCardPackScreen(sPackCards, sPackCount, CB2_ReturnToFieldContinueScriptPlayMapMusic);
        else
            OpenCardDeckBuilder(CB2_ReturnToFieldContinueScriptPlayMapMusic);
        DestroyTask(taskId);
    }
}

// Opens a pack: CARD_PACK_SIZE cards, received and shown one by one
void OpenCardPack(void)
{
    u8 i;

    for (i = 0; i < CARD_PACK_SIZE; i++)
    {
        sPackCards[i] = CardGame_RollPackCard();
        CardGame_GiveCard(sPackCards[i]);
    }
    sPackCount = CARD_PACK_SIZE;
    gTasks[CreateTask(Task_OpenFromField, 0)].tIsPack = TRUE;
}

// Shows the reward card of VAR_0x8004 (already received, with CardGame_RewardOpponentWin)
void OpenCardRewardScreen(void)
{
    sPackCards[0] = gCardOpponents[GetScriptOpponent()].rewardCard;
    sPackCount = 1;
    gTasks[CreateTask(Task_OpenFromField, 0)].tIsPack = TRUE;
}

void OpenCardDeckBuilderFromScript(void)
{
    gTasks[CreateTask(Task_OpenFromField, 0)].tIsPack = FALSE;
}

#undef tState
#undef tIsPack
