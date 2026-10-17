#ifndef GUARD_CARD_GAME_H
#define GUARD_CARD_GAME_H

#include "constants/card_game.h"

// The card game's rules engine (src/card_game_rules.c, src/card_game_effects.c).
// It only uses the types from global.h, so tools/card_game/test_rules.c can build it on the
// host. The screen (phase 3) drives it with actions and animates the events it records.

// ---------------------------------------------------------------------------------------------
// Card data. Looked up through the CardGame_Get* functions, which src/card_game_data.c defines
// for the game and the test harness defines for its own cards.

struct CardAttack
{
    const u8 *name;
    u8 cost[CARD_ATTACK_MAX_COST];  // CARD_TYPE_*, CARD_TYPE_NONE padded; COLORLESS = any Energy
    u8 damage;
    u8 effect;                      // CARD_EFFECT_*
    u8 effectArg;
};

struct PokemonCardInfo
{
    u16 species;
    u16 evolvesFrom;    // CARD_* or CARD_NONE
    u16 hp;
    u8 type;            // CARD_TYPE_*
    u8 weakness;        // CARD_TYPE_* or CARD_TYPE_NONE
    u8 stage;           // CARD_STAGE_*
    u8 retreatCost;
    bool8 isEx;         // knocked out, it gives CARD_EX_TROPHIES (CARD_MEGA_EX_TROPHIES if isMega)
    u8 numAttacks;
    struct CardAttack attacks[CARD_MAX_ATTACKS];
    u8 ability;         // CARD_ABILITY_*
    u8 abilityArg;
    bool8 isMega;
};

struct TrainerCardInfo
{
    const u8 *name;
    u8 kind;            // CARD_KIND_ITEM, _SUPPORTER or _TOOL
    u8 effect;          // CARD_TRAINER_*
    u8 effectArg;
    u8 effectType;      // CARD_TYPE_* the effect is limited to, CARD_TYPE_NONE for any
    u16 iconItem;       // ITEM_* whose icon shows the card, ITEM_NONE for none
};

struct CardDeckData
{
    const u8 *name;                         // a name for tests and the debug duel
    const u16 *cards;                       // CARD_DECK_SIZE cards
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];      // CARD_TYPE_NONE padded
    u8 aiLevel;                             // CARD_AI_*, when the NPC plays it
};

extern const struct CardDeckData gCardDecks[CARD_DECKS_COUNT];

// A card player in the world (docs/card_game_plan.md, section 5)
struct CardOpponent
{
    const u8 *name;
    const u8 *challengeText;    // asks for a duel (yes/no)
    const u8 *playerWonText;
    const u8 *playerLostText;
    u16 rewardCard;             // given on the first win, CARD_NONE for none
    u8 deck;                    // CARD_DECK_*
    u8 mapsec;                  // MAPSEC_*, where they are, for the Card Player list
};

extern const struct CardOpponent gCardOpponents[CARD_OPPONENTS_COUNT];

u8 CardGame_GetCardKind(u16 cardId);
const struct PokemonCardInfo *CardGame_GetPokemonInfo(u16 cardId);  // NULL if not a Pokémon
const struct TrainerCardInfo *CardGame_GetTrainerInfo(u16 cardId);  // NULL if not a Trainer

// ---------------------------------------------------------------------------------------------
// State of a duel

struct CardInPlay
{
    u16 cardId;                 // CARD_NONE: empty slot
    u16 evolvedFrom[2];         // the cards under it, discarded with it
    u16 tool;
    u16 damage;
    u8 energy[CARD_TYPE_COUNT]; // attached Energy, by type
    u8 status;                  // CARD_STATUS_*
    u8 statusTurn;              // turn the rotation status was given (Paralysis wears off after it)
    u8 turnPlayed;              // turn it was put in play or last evolved
    u8 abilityUsedTurn;         // turn its ability was last used
    u8 cantAttackTurn;          // turn it can't attack in (CARD_EFFECT_CANT_ATTACK_NEXT)
};

struct CardDuelSide
{
    u16 deck[CARD_DECK_SIZE];   // the top card is deck[deckCount - 1]
    u16 hand[CARD_HAND_MAX];
    u16 discard[CARD_DECK_SIZE];
    u8 deckCount;
    u8 handCount;
    u8 discardCount;
    u8 trophies;
    struct CardInPlay slots[CARD_SLOT_COUNT];   // CARD_SLOT_ACTIVE, then the Bench
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];          // CARD_TYPE_NONE padded
    u8 zoneEnergy;              // Energy to attach this turn, CARD_TYPE_NONE if none (or used)
    u8 nextEnergy;              // shown in advance
    u8 retreatDiscount;         // this turn
    u8 attackBonus;             // this turn, for attacks of attackBonusType (CARD_TYPE_NONE: all)
    u8 attackBonusType;
    u8 shield;                  // Blue: less damage from attacks during shieldTurn
    u8 shieldTurn;
    bool8 setupDone;
    bool8 energyAttached;       // this turn
    bool8 supporterPlayed;      // this turn
    bool8 retreated;            // this turn
};

#define CARD_PHASE_SETUP    0   // both sides place their Active and Bench, then END_SETUP
#define CARD_PHASE_MAIN     1   // currentSide plays its turn
#define CARD_PHASE_PROMOTE  2   // the sides in promoteMask must move a Bench Pokémon up
#define CARD_PHASE_OVER     3

struct CardDuelEvent
{
    u8 type;        // CARD_EVENT_*
    u8 side;
    u8 slot;
    u16 value;
    u16 cardId;     // the card in side's slot when it happened (CARD_NONE without a slot)
};

#define CARD_EVENT_FIRST_SIDE       0   // side goes first (after the coin flip)
#define CARD_EVENT_TURN_START       1   // value: turn number
#define CARD_EVENT_DRAW             2   // value: card
#define CARD_EVENT_ENERGY_ZONE      3   // value: Energy type made this turn
#define CARD_EVENT_COIN             4   // value: TRUE for heads
#define CARD_EVENT_PLAY_POKEMON     5   // value: card, slot: where
#define CARD_EVENT_ATTACH_ENERGY    6   // value: type
#define CARD_EVENT_EVOLVE           7   // value: new card
#define CARD_EVENT_PLAY_TRAINER     8   // value: card, slot: target or CARD_SLOT_NONE
#define CARD_EVENT_RETREAT          9   // slot: the Bench Pokémon that came up
#define CARD_EVENT_ATTACK           10  // value: attack index
#define CARD_EVENT_ATTACK_FAILED    11  // confused or a failed flip
#define CARD_EVENT_DAMAGE           12  // value: damage
#define CARD_EVENT_HEAL             13  // value: HP healed
#define CARD_EVENT_STATUS           14  // value: CARD_STATUS_* given
#define CARD_EVENT_STATUS_CURED     15  // value: CARD_STATUS_* removed
#define CARD_EVENT_DISCARD_ENERGY   16  // value: count
#define CARD_EVENT_KNOCK_OUT        17  // value: card
#define CARD_EVENT_TROPHY           18  // side: who earned them, value: trophies earned
#define CARD_EVENT_PROMOTE          19  // slot: the Bench Pokémon that came up
#define CARD_EVENT_DECK_EMPTY       20  // side could not draw
#define CARD_EVENT_HAND_FULL        21  // side could not draw
#define CARD_EVENT_GAME_OVER        22  // side: winner, or CARD_RESULT_DRAW
#define CARD_EVENT_ABILITY          23  // value: CARD_ABILITY_*
#define CARD_EVENT_REVEAL_HAND      24  // side: whose hand is shown
#define CARD_EVENT_SWITCH           25  // side's Active was switched with slot by the opponent
#define CARD_EVENT_COUNT            26

#define CARD_MAX_EVENTS 48

struct CardDuel
{
    struct CardDuelSide sides[CARD_SIDE_COUNT];
    u8 phase;               // CARD_PHASE_*
    u8 turn;                // 1 = the first player's first turn
    u8 currentSide;
    u8 firstSide;
    u8 winner;              // CARD_SIDE_*, CARD_RESULT_DRAW, or CARD_SIDE_NONE while playing
    u8 promoteMask;         // (1 << side) for each side that must promote
    bool8 promoteMidTurn;   // after the promotion, the current turn goes on
    u8 eventCount;
    u8 forcedFlipCount;     // debug/tests: the next flips come from forcedFlips, bit 0 first
    u32 forcedFlips;
    u32 rngState;
    struct CardDuelEvent events[CARD_MAX_EVENTS];
};

// ---------------------------------------------------------------------------------------------
// Actions

#define CARD_ACTION_PLAY_BASIC      0   // handIndex to slot (setup: Active or Bench; turn: Bench)
#define CARD_ACTION_END_SETUP       1
#define CARD_ACTION_ATTACH_ENERGY   2   // the zone's Energy to slot
#define CARD_ACTION_EVOLVE          3   // handIndex onto slot
#define CARD_ACTION_PLAY_TRAINER    4   // handIndex, slot: target (or CARD_SLOT_NONE), arg: the Stage 2 for Rare Candy
#define CARD_ACTION_RETREAT         5   // slot: the Bench Pokémon that comes up
#define CARD_ACTION_ATTACK          6   // arg: attack index, slot: the target of a CARD_EFFECT_SNIPE attack; ends the turn
#define CARD_ACTION_END_TURN        7
#define CARD_ACTION_PROMOTE         8   // slot: the Bench Pokémon that comes up
#define CARD_ACTION_USE_ABILITY     9   // slot: the Pokémon using it, arg: the opponent's target slot (CARD_ABILITY_SNIPE)

struct CardAction
{
    u8 type;
    u8 handIndex;
    u8 slot;
    u8 arg;
};

#define CARD_MAX_ACTIONS 96

struct CardDuelSetup
{
    const u16 *deck;                        // CARD_DECK_SIZE cards
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];      // CARD_TYPE_NONE padded
};

u8 Duel_ValidateDeck(const u16 *deck, const u8 *zoneTypes);
bool8 Duel_Init(struct CardDuel *duel, const struct CardDuelSetup *player, const struct CardDuelSetup *opponent, u32 seed);
bool8 Duel_IsLegalAction(const struct CardDuel *duel, u8 side, const struct CardAction *action);
u8 Duel_GetLegalActions(const struct CardDuel *duel, u8 side, struct CardAction *actions);
bool8 Duel_DoAction(struct CardDuel *duel, u8 side, const struct CardAction *action);
void Duel_Forfeit(struct CardDuel *duel, u8 side);
bool8 Duel_CanAct(const struct CardDuel *duel, u8 side);
bool8 Duel_CanPayAttack(const struct CardInPlay *mon, const struct CardAttack *attack);
u16 Duel_GetMaxHp(const struct CardInPlay *mon);
u8 Duel_CountEnergy(const struct CardInPlay *mon);
void Duel_ClearEvents(struct CardDuel *duel);

// ---------------------------------------------------------------------------------------------
// Engine internals shared by card_game_rules.c and card_game_effects.c

bool8 Duel_FlipCoin(struct CardDuel *duel, u8 side);
u16 Duel_Random(struct CardDuel *duel);
void Duel_PushEvent(struct CardDuel *duel, u8 type, u8 side, u8 slot, u16 value);
bool8 Duel_DrawCard(struct CardDuel *duel, u8 side);
void Duel_DamageSlot(struct CardDuel *duel, u8 side, u8 slot, u16 damage);
void Duel_HealSlot(struct CardDuel *duel, u8 side, u8 slot, u16 amount);
void Duel_GiveStatus(struct CardDuel *duel, u8 side, u8 slot, u8 status);
void Duel_CureStatus(struct CardDuel *duel, u8 side, u8 slot, u8 status);
void Duel_DiscardEnergy(struct CardDuel *duel, u8 side, u8 slot, u8 count, u8 preferredType);
void Duel_EvolveSlot(struct CardDuel *duel, u8 side, u8 slot, u16 cardId);
void Duel_SwitchActive(struct CardDuel *duel, u8 side, u8 slot);
void Duel_SwitchToRandomBench(struct CardDuel *duel, u8 side);
bool8 Duel_HasBenchPokemon(const struct CardDuelSide *side);
u8 Duel_GetCardType(u16 cardId);

// src/card_game_effects.c
bool8 CardEffects_CanPlayTrainer(const struct CardDuel *duel, u8 side, const struct TrainerCardInfo *info, u8 slot, u8 arg);
void CardEffects_PlayTrainer(struct CardDuel *duel, u8 side, const struct TrainerCardInfo *info, u8 slot, u16 extraCard);
u16 CardEffects_GetToolHpBonus(u16 tool);
u16 CardEffects_GetToolDamageBonus(u16 tool);
void CardEffects_EndOfTurnTool(struct CardDuel *duel, u8 sideId);
bool8 CardEffects_BeforeAttack(struct CardDuel *duel, u8 side, const struct CardAttack *attack, u16 *damage);
void CardEffects_AfterAttack(struct CardDuel *duel, u8 side, const struct CardAttack *attack, u8 target);
u16 CardEffects_ModifyDamageTaken(const struct CardDuel *duel, u8 side, u8 slot, u16 damage);
void CardEffects_AfterDamagedByAttack(struct CardDuel *duel, u8 side, u8 slot);
bool8 CardEffects_CanUseAbility(const struct CardDuel *duel, u8 side, u8 slot, u8 target);
void CardEffects_UseAbility(struct CardDuel *duel, u8 side, u8 slot, u8 target);
bool8 CardEffects_HasFreeRetreat(const struct CardInPlay *mon);

// src/card_game_ai.c
void CardAi_ChooseAction(struct CardDuel *duel, u8 side, u8 level, struct CardAction *actions, struct CardAction *chosen);

// src/card_game_collection.c: the player's cards and decks (saved in gSaveBlock1Ptr->cardGame)
u8 CardGame_GetOwnedCopies(u16 cardId);
bool8 CardGame_GiveCard(u16 cardId);
u8 CardGame_CountCopies(const u16 *deck, u16 cardId);
u16 *CardGame_GetSavedDeck(u8 deckIndex);
void CardGame_GetDeckZone(u8 deckIndex, u8 *zoneTypes);
void CardGame_SetDeckZone(u8 deckIndex, const u8 *zoneTypes);
u8 CardGame_ValidateSavedDeck(u8 deckIndex);
bool8 CardGame_GetPlayerDeck(struct CardDuelSetup *setup);
void CardGame_RecordResult(u8 result);
u16 CardGame_RollPackCard(void);
void CardGame_GiveStarterDeck(void);
void CardGame_CheckActiveDeck(void);
bool8 CardGame_HasMetOpponent(u8 opponentId);
bool8 CardGame_HasBeatenOpponent(u8 opponentId);
void CardGame_MeetOpponent(void);
void CardGame_BufferOpponentText(void);
void CardGame_RewardOpponentWin(void);
u8 *CardGame_GetOpponentName(u8 *dst, u8 opponentId);
void CardGame_BufferOpponentName(void);
void CardGame_StartCardTournament(void);
void OpenCardRewardScreen(void);
void OpenCardPack(void);
void OpenCardDeckBuilderFromScript(void);

// src/card_game_deck.c: the deck builder and the pack opening screen
void OpenCardDeckBuilder(void (*exitCallback)(void));
void OpenCardPackScreen(const u16 *cards, u8 count, void (*exitCallback)(void));

// src/card_game.c
void PlayCardGame(void);
u8 *CardGame_AppendCardName(u8 *dst, u16 cardId);
u8 *CardGame_AppendAttackText(u8 *dst, const struct CardAttack *attack);
u8 *CardGame_AppendAbilityText(u8 *dst, const struct PokemonCardInfo *info);
u8 *CardGame_AppendExRule(u8 *dst, const struct PokemonCardInfo *info);
u8 *CardGame_AppendTrainerKind(u8 *dst, const struct TrainerCardInfo *info);
u8 *CardGame_AppendTrainerText(u8 *dst, const struct TrainerCardInfo *info);
u16 CardGame_CreatePokemonPic(u16 species, s16 x, s16 y);
void CardGame_DestroyPokemonPic(u16 spriteId);
extern const u8 *const gCardTypeNames[CARD_TYPE_COUNT];

#endif // GUARD_CARD_GAME_H
