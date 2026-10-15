#include "global.h"
#include "card_game.h"

// Attack and Trainer effects of the card game, called by card_game_rules.c.

static bool8 DeckHasBasic(const struct CardDuelSide *side)
{
    u8 i;
    const struct PokemonCardInfo *info;

    for (i = 0; i < side->deckCount; i++)
    {
        info = CardGame_GetPokemonInfo(side->deck[i]);
        if (info != NULL && info->stage == CARD_STAGE_BASIC)
            return TRUE;
    }
    return FALSE;
}

// slot is the target the player chose, or CARD_SLOT_NONE for cards without one, so each card
// has a single way to be played.
bool8 CardEffects_CanPlayTrainer(const struct CardDuel *duel, u8 sideId, const struct TrainerCardInfo *info, u8 slot)
{
    const struct CardDuelSide *side = &duel->sides[sideId];

    switch (info->effect)
    {
    case CARD_TRAINER_DRAW:
        return slot == CARD_SLOT_NONE && side->deckCount != 0;
    case CARD_TRAINER_HEAL:
        return slot < CARD_SLOT_COUNT && side->slots[slot].cardId != CARD_NONE && side->slots[slot].damage != 0;
    case CARD_TRAINER_POKE_BALL:
        return slot == CARD_SLOT_NONE && DeckHasBasic(side);
    case CARD_TRAINER_RETREAT_DISCOUNT:
        return slot == CARD_SLOT_NONE && side->slots[CARD_SLOT_ACTIVE].cardId != CARD_NONE;
    case CARD_TRAINER_TOOL_HP:
        return slot < CARD_SLOT_COUNT;
    }
    return FALSE;
}

static void TakeRandomBasicFromDeck(struct CardDuel *duel, u8 sideId)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    const struct PokemonCardInfo *info;
    u8 i, count = 0, pick;
    u16 cardId, temp;

    for (i = 0; i < side->deckCount; i++)
    {
        info = CardGame_GetPokemonInfo(side->deck[i]);
        if (info != NULL && info->stage == CARD_STAGE_BASIC)
            count++;
    }
    if (count == 0 || side->handCount >= CARD_HAND_MAX)
        return;
    pick = Duel_Random(duel) % count;
    for (i = 0; i < side->deckCount; i++)
    {
        info = CardGame_GetPokemonInfo(side->deck[i]);
        if (info == NULL || info->stage != CARD_STAGE_BASIC)
            continue;
        if (pick-- != 0)
            continue;
        cardId = side->deck[i];
        side->deckCount--;
        for (; i < side->deckCount; i++)
            side->deck[i] = side->deck[i + 1];
        side->deck[side->deckCount] = CARD_NONE;
        side->hand[side->handCount++] = cardId;
        Duel_PushEvent(duel, CARD_EVENT_DRAW, sideId, CARD_SLOT_NONE, cardId);
        break;
    }
    // Then the deck is shuffled
    for (i = side->deckCount; i > 1; i--)
    {
        pick = Duel_Random(duel) % i;
        temp = side->deck[i - 1];
        side->deck[i - 1] = side->deck[pick];
        side->deck[pick] = temp;
    }
}

void CardEffects_PlayTrainer(struct CardDuel *duel, u8 sideId, const struct TrainerCardInfo *info, u8 slot)
{
    u8 i;

    switch (info->effect)
    {
    case CARD_TRAINER_DRAW:
        for (i = 0; i < info->effectArg; i++)
            Duel_DrawCard(duel, sideId);
        break;
    case CARD_TRAINER_HEAL:
        Duel_HealSlot(duel, sideId, slot, info->effectArg);
        break;
    case CARD_TRAINER_POKE_BALL:
        TakeRandomBasicFromDeck(duel, sideId);
        break;
    case CARD_TRAINER_RETREAT_DISCOUNT:
        duel->sides[sideId].retreatDiscount += info->effectArg;
        break;
    }
}

u16 CardEffects_GetToolHpBonus(u16 tool)
{
    const struct TrainerCardInfo *info = CardGame_GetTrainerInfo(tool);

    if (info != NULL && info->effect == CARD_TRAINER_TOOL_HP)
        return info->effectArg;
    return 0;
}

// Before the damage. Returns FALSE if the attack does nothing.
bool8 CardEffects_BeforeAttack(struct CardDuel *duel, u8 sideId, const struct CardAttack *attack, u16 *damage)
{
    switch (attack->effect)
    {
    case CARD_EFFECT_FLIP_BONUS:
        if (Duel_FlipCoin(duel, sideId))
            *damage += attack->effectArg;
        break;
    case CARD_EFFECT_FLIP_OR_FAIL:
        if (!Duel_FlipCoin(duel, sideId))
            return FALSE;
        break;
    }
    return TRUE;
}

// After the damage, before knockouts are resolved.
void CardEffects_AfterAttack(struct CardDuel *duel, u8 sideId, const struct CardAttack *attack)
{
    u8 opponentId = sideId ^ 1;
    const struct PokemonCardInfo *info;

    switch (attack->effect)
    {
    case CARD_EFFECT_STATUS:
        Duel_GiveStatus(duel, opponentId, CARD_SLOT_ACTIVE, attack->effectArg);
        break;
    case CARD_EFFECT_FLIP_STATUS:
        if (Duel_FlipCoin(duel, sideId))
            Duel_GiveStatus(duel, opponentId, CARD_SLOT_ACTIVE, attack->effectArg);
        break;
    case CARD_EFFECT_HEAL_SELF:
        Duel_HealSlot(duel, sideId, CARD_SLOT_ACTIVE, attack->effectArg);
        break;
    case CARD_EFFECT_SELF_DAMAGE:
        Duel_DamageSlot(duel, sideId, CARD_SLOT_ACTIVE, attack->effectArg);
        break;
    case CARD_EFFECT_DISCARD_ENERGY:
        info = CardGame_GetPokemonInfo(duel->sides[sideId].slots[CARD_SLOT_ACTIVE].cardId);
        Duel_DiscardEnergy(duel, sideId, CARD_SLOT_ACTIVE, attack->effectArg, info->type);
        break;
    case CARD_EFFECT_DISCARD_ALL_ENERGY:
        Duel_DiscardEnergy(duel, sideId, CARD_SLOT_ACTIVE,
                           Duel_CountEnergy(&duel->sides[sideId].slots[CARD_SLOT_ACTIVE]), CARD_TYPE_NONE);
        break;
    }
}
