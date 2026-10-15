#include "global.h"
#include "card_game.h"

// Attack, ability and Trainer effects of the card game, called by card_game_rules.c.

static bool8 IsFilled(const struct CardDuelSide *side, u8 slot)
{
    return slot < CARD_SLOT_COUNT && side->slots[slot].cardId != CARD_NONE;
}

static bool8 IsOfType(const struct CardDuelSide *side, u8 slot, u8 type)
{
    return IsFilled(side, slot) && (type == CARD_TYPE_NONE || Duel_GetCardType(side->slots[slot].cardId) == type);
}

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

static bool8 HasDamagedPokemon(const struct CardDuelSide *side)
{
    u8 slot;

    for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
    {
        if (IsFilled(side, slot) && side->slots[slot].damage != 0)
            return TRUE;
    }
    return FALSE;
}

static void ShuffleDeck(struct CardDuel *duel, struct CardDuelSide *side)
{
    u8 i, j;
    u16 temp;

    for (i = side->deckCount; i > 1; i--)
    {
        j = Duel_Random(duel) % i;
        temp = side->deck[i - 1];
        side->deck[i - 1] = side->deck[j];
        side->deck[j] = temp;
    }
}

static void AttachEnergy(struct CardDuel *duel, u8 sideId, u8 slot, u8 type)
{
    duel->sides[sideId].slots[slot].energy[type]++;
    Duel_PushEvent(duel, CARD_EVENT_ATTACH_ENERGY, sideId, slot, type);
}

// ---------------------------------------------------------------------------------------------
// Trainers

// Rare Candy: the Stage 2 in the hand at arg must evolve from a Stage 1 of the Basic in slot
static bool8 CanPlayRareCandy(const struct CardDuel *duel, const struct CardDuelSide *side, u8 slot, u8 arg)
{
    const struct PokemonCardInfo *basic, *stage2, *stage1;

    if (!IsFilled(side, slot) || arg >= side->handCount)
        return FALSE;
    basic = CardGame_GetPokemonInfo(side->slots[slot].cardId);
    stage2 = CardGame_GetPokemonInfo(side->hand[arg]);
    if (basic->stage != CARD_STAGE_BASIC || stage2 == NULL || stage2->stage != CARD_STAGE_2)
        return FALSE;
    stage1 = CardGame_GetPokemonInfo(stage2->evolvesFrom);
    // Same timing as evolving
    return stage1 != NULL && stage1->evolvesFrom == side->slots[slot].cardId
        && duel->turn > 2 && side->slots[slot].turnPlayed != duel->turn;
}

static bool8 CanGatherEnergy(const struct CardDuelSide *side, u8 type)
{
    u8 slot;

    if (!IsOfType(side, CARD_SLOT_ACTIVE, type))
        return FALSE;
    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (IsFilled(side, slot) && side->slots[slot].energy[type] != 0)
            return TRUE;
    }
    return FALSE;
}

// slot is the target the player chose, or CARD_SLOT_NONE for cards without one, and arg is 0
// except for Rare Candy, so each card has a single way to be played.
bool8 CardEffects_CanPlayTrainer(const struct CardDuel *duel, u8 sideId, const struct TrainerCardInfo *info, u8 slot, u8 arg)
{
    const struct CardDuelSide *side = &duel->sides[sideId];
    const struct CardDuelSide *opponent = &duel->sides[sideId ^ 1];

    if (info->effect == CARD_TRAINER_RARE_CANDY)
        return CanPlayRareCandy(duel, side, slot, arg);
    if (arg != 0)
        return FALSE;

    switch (info->effect)
    {
    case CARD_TRAINER_DRAW:
        return slot == CARD_SLOT_NONE && side->deckCount != 0;
    case CARD_TRAINER_HEAL:
        return IsFilled(side, slot) && side->slots[slot].damage != 0;
    case CARD_TRAINER_POKE_BALL:
        return slot == CARD_SLOT_NONE && DeckHasBasic(side);
    case CARD_TRAINER_RETREAT_DISCOUNT:
        return slot == CARD_SLOT_NONE && IsFilled(side, CARD_SLOT_ACTIVE);
    case CARD_TRAINER_TOOL_HP:
    case CARD_TRAINER_TOOL_COUNTER:
        return slot < CARD_SLOT_COUNT;
    case CARD_TRAINER_HAND_SCOPE:
        return slot == CARD_SLOT_NONE && opponent->handCount != 0;
    case CARD_TRAINER_RED_CARD:
        return slot == CARD_SLOT_NONE && opponent->handCount + opponent->deckCount != 0;
    case CARD_TRAINER_ATTACH_ENERGY:
    case CARD_TRAINER_FLIP_ATTACH_ENERGY:
        return IsOfType(side, slot, info->effectType);
    case CARD_TRAINER_GATHER_ENERGY:
        return slot == CARD_SLOT_NONE && CanGatherEnergy(side, info->effectType);
    case CARD_TRAINER_HEAL_TYPE:
        return IsOfType(side, slot, info->effectType) && side->slots[slot].damage != 0;
    case CARD_TRAINER_SWITCH_OPPONENT:
        return slot == CARD_SLOT_NONE && Duel_HasBenchPokemon(opponent);
    case CARD_TRAINER_ATTACK_BONUS:
        return slot == CARD_SLOT_NONE;
    }
    return FALSE;
}

static void TakeRandomBasicFromDeck(struct CardDuel *duel, u8 sideId)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    const struct PokemonCardInfo *info;
    u8 i, count = 0, pick;
    u16 cardId;

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
    ShuffleDeck(duel, side);
}

static void RedCard(struct CardDuel *duel, u8 sideId, u8 count)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    u8 i;

    while (side->handCount != 0 && side->deckCount < CARD_DECK_SIZE)
    {
        side->deck[side->deckCount++] = side->hand[--side->handCount];
        side->hand[side->handCount] = CARD_NONE;
    }
    ShuffleDeck(duel, side);
    for (i = 0; i < count; i++)
        Duel_DrawCard(duel, sideId);
}

static void GatherEnergy(struct CardDuel *duel, u8 sideId, u8 type)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    u8 slot;

    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        while (side->slots[slot].energy[type] != 0)
        {
            side->slots[slot].energy[type]--;
            AttachEnergy(duel, sideId, CARD_SLOT_ACTIVE, type);
        }
    }
}

// extraCard is the Stage 2 taken from the hand by Rare Candy
void CardEffects_PlayTrainer(struct CardDuel *duel, u8 sideId, const struct TrainerCardInfo *info, u8 slot, u16 extraCard)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    u8 i;

    switch (info->effect)
    {
    case CARD_TRAINER_DRAW:
        for (i = 0; i < info->effectArg; i++)
            Duel_DrawCard(duel, sideId);
        break;
    case CARD_TRAINER_HEAL:
    case CARD_TRAINER_HEAL_TYPE:
        Duel_HealSlot(duel, sideId, slot, info->effectArg);
        break;
    case CARD_TRAINER_POKE_BALL:
        TakeRandomBasicFromDeck(duel, sideId);
        break;
    case CARD_TRAINER_RETREAT_DISCOUNT:
        side->retreatDiscount += info->effectArg;
        break;
    case CARD_TRAINER_HAND_SCOPE:
        Duel_PushEvent(duel, CARD_EVENT_REVEAL_HAND, sideId ^ 1, CARD_SLOT_NONE, 0);
        break;
    case CARD_TRAINER_RED_CARD:
        RedCard(duel, sideId ^ 1, info->effectArg);
        break;
    case CARD_TRAINER_RARE_CANDY:
        Duel_EvolveSlot(duel, sideId, slot, extraCard);
        break;
    case CARD_TRAINER_ATTACH_ENERGY:
        AttachEnergy(duel, sideId, slot, info->effectType);
        break;
    case CARD_TRAINER_FLIP_ATTACH_ENERGY:
        for (i = 0; i < 20 && Duel_FlipCoin(duel, sideId); i++)
            AttachEnergy(duel, sideId, slot, info->effectType);
        break;
    case CARD_TRAINER_GATHER_ENERGY:
        GatherEnergy(duel, sideId, info->effectType);
        break;
    case CARD_TRAINER_SWITCH_OPPONENT:
        Duel_SwitchToRandomBench(duel, sideId ^ 1);
        break;
    case CARD_TRAINER_ATTACK_BONUS:
        side->attackBonus += info->effectArg;
        side->attackBonusType = info->effectType;
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

// ---------------------------------------------------------------------------------------------
// Abilities

static const struct PokemonCardInfo *GetInfo(const struct CardDuel *duel, u8 sideId, u8 slot)
{
    return CardGame_GetPokemonInfo(duel->sides[sideId].slots[slot].cardId);
}

bool8 CardEffects_CanUseAbility(const struct CardDuel *duel, u8 sideId, u8 slot, u8 target)
{
    const struct CardDuelSide *side = &duel->sides[sideId];
    const struct CardDuelSide *opponent = &duel->sides[sideId ^ 1];
    const struct PokemonCardInfo *info = GetInfo(duel, sideId, slot);

    if (info == NULL || info->ability == CARD_ABILITY_NONE || info->ability >= CARD_ABILITY_FIRST_PASSIVE)
        return FALSE;
    if (side->slots[slot].abilityUsedTurn == duel->turn)
        return FALSE;
    if (info->ability == CARD_ABILITY_SNIPE)
        return IsFilled(opponent, target);
    if (target != 0)
        return FALSE;

    switch (info->ability)
    {
    case CARD_ABILITY_HEAL_ALL:
        return HasDamagedPokemon(side);
    case CARD_ABILITY_ATTACH_OWN_TYPE:
        return IsOfType(side, CARD_SLOT_ACTIVE, info->type);
    case CARD_ABILITY_POISON_DEFENDER:
        return slot == CARD_SLOT_ACTIVE && IsFilled(opponent, CARD_SLOT_ACTIVE)
            && !(opponent->slots[CARD_SLOT_ACTIVE].status & CARD_STATUS_POISONED);
    case CARD_ABILITY_FLIP_SLEEP:
        return IsFilled(opponent, CARD_SLOT_ACTIVE);
    case CARD_ABILITY_DRIVE_OFF:
        return slot == CARD_SLOT_ACTIVE && Duel_HasBenchPokemon(opponent);
    }
    return FALSE;
}

void CardEffects_UseAbility(struct CardDuel *duel, u8 sideId, u8 slot, u8 target)
{
    const struct PokemonCardInfo *info = GetInfo(duel, sideId, slot);
    u8 opponentId = sideId ^ 1;
    u8 i;

    switch (info->ability)
    {
    case CARD_ABILITY_HEAL_ALL:
        for (i = 0; i < CARD_SLOT_COUNT; i++)
            Duel_HealSlot(duel, sideId, i, info->abilityArg);
        break;
    case CARD_ABILITY_ATTACH_OWN_TYPE:
        AttachEnergy(duel, sideId, CARD_SLOT_ACTIVE, info->type);
        break;
    case CARD_ABILITY_POISON_DEFENDER:
        Duel_GiveStatus(duel, opponentId, CARD_SLOT_ACTIVE, CARD_STATUS_POISONED);
        break;
    case CARD_ABILITY_FLIP_SLEEP:
        if (Duel_FlipCoin(duel, sideId))
            Duel_GiveStatus(duel, opponentId, CARD_SLOT_ACTIVE, CARD_STATUS_ASLEEP);
        break;
    case CARD_ABILITY_DRIVE_OFF:
        Duel_SwitchToRandomBench(duel, opponentId);
        break;
    case CARD_ABILITY_SNIPE:
        Duel_DamageSlot(duel, opponentId, target, info->abilityArg);
        break;
    }
}

u16 CardEffects_ModifyDamageTaken(const struct CardDuel *duel, u8 sideId, u8 slot, u16 damage)
{
    const struct PokemonCardInfo *info = GetInfo(duel, sideId, slot);

    if (info != NULL && info->ability == CARD_ABILITY_REDUCE_DAMAGE)
        damage = damage > info->abilityArg ? damage - info->abilityArg : 0;
    return damage;
}

// side's Active was damaged by an attack: Rough Skin-like abilities and Rocky Helmet
void CardEffects_AfterDamagedByAttack(struct CardDuel *duel, u8 sideId, u8 slot)
{
    const struct CardInPlay *mon = &duel->sides[sideId].slots[slot];
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);
    const struct TrainerCardInfo *tool = CardGame_GetTrainerInfo(mon->tool);
    u16 damage = 0;

    if (info != NULL && info->ability == CARD_ABILITY_COUNTER)
        damage += info->abilityArg;
    if (tool != NULL && tool->effect == CARD_TRAINER_TOOL_COUNTER)
        damage += tool->effectArg;
    Duel_DamageSlot(duel, sideId ^ 1, CARD_SLOT_ACTIVE, damage);
}

bool8 CardEffects_HasFreeRetreat(const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);

    return info != NULL && info->ability == CARD_ABILITY_FREE_RETREAT;
}

// ---------------------------------------------------------------------------------------------
// Attacks

// Energy of the attacker's type beyond what the attack's cost asks for
static bool8 HasExtraEnergy(const struct CardInPlay *mon, const struct CardAttack *attack, u8 type, u8 extra)
{
    u8 i, typed = 0, total = 0;

    for (i = 0; i < CARD_ATTACK_MAX_COST && attack->cost[i] != CARD_TYPE_NONE; i++)
    {
        total++;
        if (attack->cost[i] == type)
            typed++;
    }
    return mon->energy[type] >= typed + extra && Duel_CountEnergy(mon) >= total + extra;
}

// Before the damage. Returns FALSE if the attack does nothing.
bool8 CardEffects_BeforeAttack(struct CardDuel *duel, u8 sideId, const struct CardAttack *attack, u16 *damage)
{
    const struct CardInPlay *attacker = &duel->sides[sideId].slots[CARD_SLOT_ACTIVE];
    const struct CardInPlay *defender = &duel->sides[sideId ^ 1].slots[CARD_SLOT_ACTIVE];
    u8 i, heads;

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
    case CARD_EFFECT_FLIP_MULTI:
        heads = 0;
        for (i = 0; i < attack->effectArg; i++)
            heads += Duel_FlipCoin(duel, sideId);
        *damage = attack->damage * heads;
        break;
    case CARD_EFFECT_BONUS_EXTRA_ENERGY:
        if (HasExtraEnergy(attacker, attack, Duel_GetCardType(attacker->cardId), 2))
            *damage += attack->effectArg;
        break;
    case CARD_EFFECT_BONUS_IF_DAMAGED:
        if (defender->damage != 0)
            *damage += attack->effectArg;
        break;
    }
    return TRUE;
}

static void DiscardRandomEnergy(struct CardDuel *duel, u8 sideId, u8 slot)
{
    struct CardInPlay *mon = &duel->sides[sideId].slots[slot];
    u8 type, pick, total = Duel_CountEnergy(mon);

    if (total == 0)
        return;
    pick = Duel_Random(duel) % total;
    for (type = 0; type < CARD_TYPE_COUNT; type++)
    {
        if (pick < mon->energy[type])
        {
            Duel_DiscardEnergy(duel, sideId, slot, 1, type);
            return;
        }
        pick -= mon->energy[type];
    }
}

// After the damage, before knockouts are resolved. target is the slot the attack hit.
void CardEffects_AfterAttack(struct CardDuel *duel, u8 sideId, const struct CardAttack *attack, u8 target)
{
    u8 opponentId = sideId ^ 1;
    struct CardInPlay *attacker = &duel->sides[sideId].slots[CARD_SLOT_ACTIVE];
    struct CardInPlay *defender = &duel->sides[opponentId].slots[CARD_SLOT_ACTIVE];
    u8 slot;

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
        Duel_DiscardEnergy(duel, sideId, CARD_SLOT_ACTIVE, attack->effectArg, Duel_GetCardType(attacker->cardId));
        break;
    case CARD_EFFECT_DISCARD_ALL_ENERGY:
        Duel_DiscardEnergy(duel, sideId, CARD_SLOT_ACTIVE, Duel_CountEnergy(attacker), CARD_TYPE_NONE);
        break;
    case CARD_EFFECT_SPREAD_BENCH:
        for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
            Duel_DamageSlot(duel, opponentId, slot, attack->effectArg);
        break;
    case CARD_EFFECT_SELF_BENCH_DAMAGE:
        for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
            Duel_DamageSlot(duel, sideId, slot, attack->effectArg);
        break;
    case CARD_EFFECT_FLIP_DISCARD_ENERGY:
        if (Duel_FlipCoin(duel, sideId))
            DiscardRandomEnergy(duel, opponentId, CARD_SLOT_ACTIVE);
        break;
    case CARD_EFFECT_CANT_ATTACK_NEXT:
        attacker->cantAttackTurn = duel->turn + 2;
        break;
    case CARD_EFFECT_DRAW:
        for (slot = 0; slot < attack->effectArg; slot++)
            Duel_DrawCard(duel, sideId);
        break;
    case CARD_EFFECT_SWITCH_DEFENDER:
        if (defender->cardId != CARD_NONE && defender->damage < Duel_GetMaxHp(defender))
            Duel_SwitchToRandomBench(duel, opponentId);
        break;
    }
}
