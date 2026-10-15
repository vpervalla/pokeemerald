#include "global.h"
#include "card_game.h"

// The card game's rules, after Pokémon TCG Pocket (see docs/card_game_plan.md, section 1).
// Nothing here touches the hardware: the screen drives a duel with Duel_DoAction and shows
// the events it records, and tools/card_game/test_rules.c runs it on the host.

static void StartTurn(struct CardDuel *duel);
static void EndTurn(struct CardDuel *duel);
static void DoAttack(struct CardDuel *duel, u8 attackId, u8 target);
static void ResolveKnockOuts(struct CardDuel *duel);
static bool8 CheckGameOver(struct CardDuel *duel);
static void SetGameOver(struct CardDuel *duel, u8 winner);

// ---------------------------------------------------------------------------------------------
// Small helpers

static void ZeroBytes(void *ptr, u32 size)
{
    u8 *bytes = ptr;
    u32 i;

    for (i = 0; i < size; i++)
        bytes[i] = 0;
}

static bool8 IsBasicPokemon(u16 cardId)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(cardId);

    return info != NULL && info->stage == CARD_STAGE_BASIC;
}

static bool8 IsBenchSlot(u8 slot)
{
    return slot >= CARD_SLOT_BENCH_1 && slot < CARD_SLOT_COUNT;
}

static bool8 IsSlotFilled(const struct CardDuelSide *side, u8 slot)
{
    return slot < CARD_SLOT_COUNT && side->slots[slot].cardId != CARD_NONE;
}

bool8 Duel_HasBenchPokemon(const struct CardDuelSide *side)
{
    u8 slot;

    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (side->slots[slot].cardId != CARD_NONE)
            return TRUE;
    }
    return FALSE;
}

static bool8 HasAnyPokemon(const struct CardDuelSide *side)
{
    return side->slots[CARD_SLOT_ACTIVE].cardId != CARD_NONE || Duel_HasBenchPokemon(side);
}

static u8 GetFirstEmptyBenchSlot(const struct CardDuelSide *side)
{
    u8 slot;

    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (side->slots[slot].cardId == CARD_NONE)
            return slot;
    }
    return CARD_SLOT_NONE;
}

static u8 GetRetreatCost(const struct CardDuelSide *side)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(side->slots[CARD_SLOT_ACTIVE].cardId);

    if (info == NULL || info->retreatCost <= side->retreatDiscount)
        return 0;
    if (CardEffects_HasFreeRetreat(&side->slots[CARD_SLOT_ACTIVE]))
        return 0;
    return info->retreatCost - side->retreatDiscount;
}

static u16 RemoveFromHand(struct CardDuelSide *side, u8 handIndex)
{
    u16 cardId = side->hand[handIndex];
    u8 i;

    for (i = handIndex; i + 1 < side->handCount; i++)
        side->hand[i] = side->hand[i + 1];
    side->handCount--;
    side->hand[side->handCount] = CARD_NONE;
    return cardId;
}

static void AddToDiscard(struct CardDuelSide *side, u16 cardId)
{
    if (cardId != CARD_NONE && side->discardCount < CARD_DECK_SIZE)
        side->discard[side->discardCount++] = cardId;
}

static void Shuffle(struct CardDuel *duel, u16 *cards, u8 count)
{
    u8 i, j;
    u16 temp;

    for (i = count; i > 1; i--)
    {
        j = Duel_Random(duel) % i;
        temp = cards[i - 1];
        cards[i - 1] = cards[j];
        cards[j] = temp;
    }
}

static u8 RollEnergy(struct CardDuel *duel, const struct CardDuelSide *side)
{
    u8 count = 0;

    while (count < CARD_MAX_ZONE_TYPES && side->zoneTypes[count] != CARD_TYPE_NONE)
        count++;
    if (count == 0)
        return CARD_TYPE_NONE;
    return side->zoneTypes[Duel_Random(duel) % count];
}

static bool8 IsZoneType(u8 type)
{
    return type >= CARD_TYPE_GRASS && type <= CARD_TYPE_METAL;
}

u8 Duel_GetCardType(u16 cardId)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(cardId);

    return info != NULL ? info->type : CARD_TYPE_NONE;
}

// ---------------------------------------------------------------------------------------------
// Internals shared with card_game_effects.c

u16 Duel_Random(struct CardDuel *duel)
{
    duel->rngState = 1103515245 * duel->rngState + 24691;
    return duel->rngState >> 16;
}

bool8 Duel_FlipCoin(struct CardDuel *duel, u8 side)
{
    bool8 heads;

    if (duel->forcedFlipCount != 0)
    {
        heads = duel->forcedFlips & 1;
        duel->forcedFlips >>= 1;
        duel->forcedFlipCount--;
    }
    else
    {
        heads = Duel_Random(duel) >> 15;
    }
    Duel_PushEvent(duel, CARD_EVENT_COIN, side, CARD_SLOT_NONE, heads);
    return heads;
}

void Duel_PushEvent(struct CardDuel *duel, u8 type, u8 side, u8 slot, u16 value)
{
    struct CardDuelEvent *event;

    if (duel->eventCount >= CARD_MAX_EVENTS)
        return;
    event = &duel->events[duel->eventCount++];
    event->type = type;
    event->side = side;
    event->slot = slot;
    event->value = value;
    event->cardId = (side < CARD_SIDE_COUNT && slot < CARD_SLOT_COUNT) ? duel->sides[side].slots[slot].cardId : CARD_NONE;
}

void Duel_ClearEvents(struct CardDuel *duel)
{
    duel->eventCount = 0;
}

bool8 Duel_DrawCard(struct CardDuel *duel, u8 sideId)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    u16 cardId;

    if (side->deckCount == 0)
    {
        Duel_PushEvent(duel, CARD_EVENT_DECK_EMPTY, sideId, CARD_SLOT_NONE, 0);
        return FALSE;
    }
    if (side->handCount >= CARD_HAND_MAX)
    {
        Duel_PushEvent(duel, CARD_EVENT_HAND_FULL, sideId, CARD_SLOT_NONE, 0);
        return FALSE;
    }
    cardId = side->deck[--side->deckCount];
    side->deck[side->deckCount] = CARD_NONE;
    side->hand[side->handCount++] = cardId;
    Duel_PushEvent(duel, CARD_EVENT_DRAW, sideId, CARD_SLOT_NONE, cardId);
    return TRUE;
}

void Duel_DamageSlot(struct CardDuel *duel, u8 side, u8 slot, u16 damage)
{
    struct CardInPlay *mon = &duel->sides[side].slots[slot];

    if (mon->cardId == CARD_NONE || damage == 0)
        return;
    mon->damage += damage;
    if (mon->damage > 999)
        mon->damage = 999;
    Duel_PushEvent(duel, CARD_EVENT_DAMAGE, side, slot, damage);
}

void Duel_HealSlot(struct CardDuel *duel, u8 side, u8 slot, u16 amount)
{
    struct CardInPlay *mon = &duel->sides[side].slots[slot];

    if (mon->cardId == CARD_NONE)
        return;
    if (amount > mon->damage)
        amount = mon->damage;
    if (amount == 0)
        return;
    mon->damage -= amount;
    Duel_PushEvent(duel, CARD_EVENT_HEAL, side, slot, amount);
}

void Duel_GiveStatus(struct CardDuel *duel, u8 side, u8 slot, u8 status)
{
    struct CardInPlay *mon = &duel->sides[side].slots[slot];

    if (mon->cardId == CARD_NONE || mon->damage >= Duel_GetMaxHp(mon))
        return;
    if (status & CARD_STATUS_ROTATION)
    {
        mon->status &= ~CARD_STATUS_ROTATION;
        mon->statusTurn = duel->turn;
    }
    mon->status |= status;
    Duel_PushEvent(duel, CARD_EVENT_STATUS, side, slot, status);
}

static void CureStatus(struct CardDuel *duel, u8 side, u8 slot, u8 status)
{
    struct CardInPlay *mon = &duel->sides[side].slots[slot];

    status &= mon->status;
    if (status == 0)
        return;
    mon->status &= ~status;
    Duel_PushEvent(duel, CARD_EVENT_STATUS_CURED, side, slot, status);
}

// Discards count Energy: preferredType first, then the other types, keeping the Pokémon's own
// type for last.
void Duel_DiscardEnergy(struct CardDuel *duel, u8 side, u8 slot, u8 count, u8 preferredType)
{
    struct CardInPlay *mon = &duel->sides[side].slots[slot];
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);
    u8 ownType = info != NULL ? info->type : CARD_TYPE_NONE;
    u8 discarded = 0;
    u8 type;

    while (discarded < count && preferredType != CARD_TYPE_NONE && mon->energy[preferredType] != 0)
    {
        mon->energy[preferredType]--;
        discarded++;
    }
    for (type = CARD_TYPE_COUNT - 1; type > CARD_TYPE_NONE && discarded < count; type--)
    {
        if (type == ownType)
            continue;
        while (discarded < count && mon->energy[type] != 0)
        {
            mon->energy[type]--;
            discarded++;
        }
    }
    while (discarded < count && ownType != CARD_TYPE_NONE && mon->energy[ownType] != 0)
    {
        mon->energy[ownType]--;
        discarded++;
    }
    if (discarded != 0)
        Duel_PushEvent(duel, CARD_EVENT_DISCARD_ENERGY, side, slot, discarded);
}

// ---------------------------------------------------------------------------------------------
// Queries

u16 Duel_GetMaxHp(const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);

    if (info == NULL)
        return 0;
    return info->hp + CardEffects_GetToolHpBonus(mon->tool);
}

u8 Duel_CountEnergy(const struct CardInPlay *mon)
{
    u8 type, total = 0;

    for (type = 0; type < CARD_TYPE_COUNT; type++)
        total += mon->energy[type];
    return total;
}

bool8 Duel_CanPayAttack(const struct CardInPlay *mon, const struct CardAttack *attack)
{
    u8 left[CARD_TYPE_COUNT];
    u8 i, type, colorless = 0, remaining = 0;

    for (type = 0; type < CARD_TYPE_COUNT; type++)
        left[type] = mon->energy[type];
    for (i = 0; i < CARD_ATTACK_MAX_COST; i++)
    {
        type = attack->cost[i];
        if (type == CARD_TYPE_NONE)
            break;
        if (type == CARD_TYPE_COLORLESS)
            colorless++;
        else if (left[type] == 0)
            return FALSE;
        else
            left[type]--;
    }
    for (type = 0; type < CARD_TYPE_COUNT; type++)
        remaining += left[type];
    return remaining >= colorless;
}

bool8 Duel_CanAct(const struct CardDuel *duel, u8 side)
{
    if (side >= CARD_SIDE_COUNT)
        return FALSE;
    switch (duel->phase)
    {
    case CARD_PHASE_SETUP:
        return !duel->sides[side].setupDone;
    case CARD_PHASE_MAIN:
        return duel->currentSide == side;
    case CARD_PHASE_PROMOTE:
        return (duel->promoteMask & (1 << side)) != 0;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------
// Setup

u8 Duel_ValidateDeck(const u16 *deck, const u8 *zoneTypes)
{
    u8 i, j, copies;
    bool8 hasBasic = FALSE, hasZoneType = FALSE;

    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (deck[i] == CARD_NONE)
            return CARD_DECK_WRONG_SIZE;
        if (CardGame_GetCardKind(deck[i]) == CARD_KIND_NONE)
            return CARD_DECK_UNKNOWN_CARD;
        if (IsBasicPokemon(deck[i]))
            hasBasic = TRUE;
        copies = 0;
        for (j = 0; j < CARD_DECK_SIZE; j++)
        {
            if (deck[j] == deck[i])
                copies++;
        }
        if (copies > CARD_MAX_COPIES)
            return CARD_DECK_TOO_MANY_COPIES;
    }
    if (!hasBasic)
        return CARD_DECK_NO_BASIC;
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == CARD_TYPE_NONE)
            continue;
        if (!IsZoneType(zoneTypes[i]))
            return CARD_DECK_NO_ENERGY_TYPE;
        hasZoneType = TRUE;
    }
    if (!hasZoneType)
        return CARD_DECK_NO_ENERGY_TYPE;
    return CARD_DECK_OK;
}

static bool8 HasBasicOnTop(const struct CardDuelSide *side)
{
    u8 i;

    for (i = 0; i < CARD_OPENING_HAND; i++)
    {
        if (IsBasicPokemon(side->deck[side->deckCount - 1 - i]))
            return TRUE;
    }
    return FALSE;
}

// Shuffles until the opening hand has a Basic Pokémon (Pocket has no mulligans).
static void ShuffleOpeningDeck(struct CardDuel *duel, struct CardDuelSide *side)
{
    u8 tries, i;
    u16 temp;

    for (tries = 0; tries < 100; tries++)
    {
        Shuffle(duel, side->deck, side->deckCount);
        if (HasBasicOnTop(side))
            return;
    }
    // Practically unreachable: swap a Basic onto the top
    for (i = 0; i < side->deckCount; i++)
    {
        if (IsBasicPokemon(side->deck[i]))
        {
            temp = side->deck[side->deckCount - 1];
            side->deck[side->deckCount - 1] = side->deck[i];
            side->deck[i] = temp;
            return;
        }
    }
}

bool8 Duel_Init(struct CardDuel *duel, const struct CardDuelSetup *player, const struct CardDuelSetup *opponent, u32 seed)
{
    const struct CardDuelSetup *setups[CARD_SIDE_COUNT];
    struct CardDuelSide *side;
    u8 sideId, i, zoneCount;

    ZeroBytes(duel, sizeof(*duel));
    setups[CARD_SIDE_PLAYER] = player;
    setups[CARD_SIDE_OPPONENT] = opponent;
    for (sideId = 0; sideId < CARD_SIDE_COUNT; sideId++)
    {
        if (Duel_ValidateDeck(setups[sideId]->deck, setups[sideId]->zoneTypes) != CARD_DECK_OK)
            return FALSE;
    }

    duel->rngState = seed;
    duel->phase = CARD_PHASE_SETUP;
    duel->winner = CARD_SIDE_NONE;
    for (sideId = 0; sideId < CARD_SIDE_COUNT; sideId++)
    {
        side = &duel->sides[sideId];
        for (i = 0; i < CARD_DECK_SIZE; i++)
            side->deck[i] = setups[sideId]->deck[i];
        side->deckCount = CARD_DECK_SIZE;
        // Packed at the start, so RollEnergy can count them
        zoneCount = 0;
        for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
        {
            if (IsZoneType(setups[sideId]->zoneTypes[i]))
                side->zoneTypes[zoneCount++] = setups[sideId]->zoneTypes[i];
        }
        side->zoneEnergy = CARD_TYPE_NONE;
        side->nextEnergy = RollEnergy(duel, side);
        ShuffleOpeningDeck(duel, side);
        for (i = 0; i < CARD_OPENING_HAND; i++)
            Duel_DrawCard(duel, sideId);
    }
    duel->firstSide = Duel_FlipCoin(duel, CARD_SIDE_PLAYER) ? CARD_SIDE_PLAYER : CARD_SIDE_OPPONENT;
    duel->currentSide = duel->firstSide;
    Duel_PushEvent(duel, CARD_EVENT_FIRST_SIDE, duel->firstSide, CARD_SLOT_NONE, 0);
    return TRUE;
}

// ---------------------------------------------------------------------------------------------
// Legality

static bool8 IsLegalSetupAction(const struct CardDuel *duel, u8 sideId, const struct CardAction *action)
{
    const struct CardDuelSide *side = &duel->sides[sideId];

    switch (action->type)
    {
    case CARD_ACTION_PLAY_BASIC:
        if (action->handIndex >= side->handCount || !IsBasicPokemon(side->hand[action->handIndex]))
            return FALSE;
        if (action->slot >= CARD_SLOT_COUNT || IsSlotFilled(side, action->slot))
            return FALSE;
        // The Active first, then the Bench
        return action->slot == CARD_SLOT_ACTIVE || IsSlotFilled(side, CARD_SLOT_ACTIVE);
    case CARD_ACTION_END_SETUP:
        return IsSlotFilled(side, CARD_SLOT_ACTIVE);
    }
    return FALSE;
}

static bool8 IsLegalTurnAction(const struct CardDuel *duel, u8 sideId, const struct CardAction *action)
{
    const struct CardDuelSide *side = &duel->sides[sideId];
    const struct CardInPlay *active = &side->slots[CARD_SLOT_ACTIVE];
    const struct PokemonCardInfo *pokemon;
    const struct TrainerCardInfo *trainer;
    u16 cardId = CARD_NONE;

    if (action->handIndex < side->handCount)
        cardId = side->hand[action->handIndex];

    switch (action->type)
    {
    case CARD_ACTION_PLAY_BASIC:
        return IsBasicPokemon(cardId) && IsBenchSlot(action->slot) && !IsSlotFilled(side, action->slot);
    case CARD_ACTION_ATTACH_ENERGY:
        return side->zoneEnergy != CARD_TYPE_NONE && !side->energyAttached && IsSlotFilled(side, action->slot);
    case CARD_ACTION_EVOLVE:
        pokemon = CardGame_GetPokemonInfo(cardId);
        if (pokemon == NULL || pokemon->stage == CARD_STAGE_BASIC || !IsSlotFilled(side, action->slot))
            return FALSE;
        // Not on either player's first turn, and not a Pokémon put in play or evolved this turn
        return duel->turn > 2
            && pokemon->evolvesFrom == side->slots[action->slot].cardId
            && side->slots[action->slot].turnPlayed != duel->turn;
    case CARD_ACTION_PLAY_TRAINER:
        trainer = CardGame_GetTrainerInfo(cardId);
        if (trainer == NULL)
            return FALSE;
        if (trainer->kind == CARD_KIND_SUPPORTER && side->supporterPlayed)
            return FALSE;
        if (trainer->kind == CARD_KIND_TOOL
         && (!IsSlotFilled(side, action->slot) || side->slots[action->slot].tool != CARD_NONE))
            return FALSE;
        if (trainer->effect == CARD_TRAINER_RARE_CANDY && action->arg == action->handIndex)
            return FALSE;
        return CardEffects_CanPlayTrainer(duel, sideId, trainer, action->slot, action->arg);
    case CARD_ACTION_RETREAT:
        return !side->retreated
            && IsSlotFilled(side, CARD_SLOT_ACTIVE)
            && !(active->status & (CARD_STATUS_ASLEEP | CARD_STATUS_PARALYZED))
            && IsBenchSlot(action->slot) && IsSlotFilled(side, action->slot)
            && Duel_CountEnergy(active) >= GetRetreatCost(side);
    case CARD_ACTION_ATTACK:
        // The first player can't attack on the first turn
        if (duel->turn <= 1 || !IsSlotFilled(side, CARD_SLOT_ACTIVE))
            return FALSE;
        if ((active->status & (CARD_STATUS_ASLEEP | CARD_STATUS_PARALYZED)) || active->cantAttackTurn == duel->turn)
            return FALSE;
        pokemon = CardGame_GetPokemonInfo(active->cardId);
        if (action->arg >= pokemon->numAttacks || !Duel_CanPayAttack(active, &pokemon->attacks[action->arg]))
            return FALSE;
        // A sniping attack names its target; the others hit the Active
        if (pokemon->attacks[action->arg].effect == CARD_EFFECT_SNIPE)
            return IsSlotFilled(&duel->sides[sideId ^ 1], action->slot);
        return action->slot == CARD_SLOT_ACTIVE;
    case CARD_ACTION_USE_ABILITY:
        return IsSlotFilled(side, action->slot) && CardEffects_CanUseAbility(duel, sideId, action->slot, action->arg);
    case CARD_ACTION_END_TURN:
        return TRUE;
    }
    return FALSE;
}

bool8 Duel_IsLegalAction(const struct CardDuel *duel, u8 side, const struct CardAction *action)
{
    if (!Duel_CanAct(duel, side))
        return FALSE;
    switch (duel->phase)
    {
    case CARD_PHASE_SETUP:
        return IsLegalSetupAction(duel, side, action);
    case CARD_PHASE_MAIN:
        return IsLegalTurnAction(duel, side, action);
    case CARD_PHASE_PROMOTE:
        return action->type == CARD_ACTION_PROMOTE
            && IsBenchSlot(action->slot) && IsSlotFilled(&duel->sides[side], action->slot);
    }
    return FALSE;
}

static void TryAddAction(const struct CardDuel *duel, u8 side, struct CardAction *actions, u8 *count, u8 type, u8 handIndex, u8 slot, u8 arg)
{
    struct CardAction action;

    if (*count >= CARD_MAX_ACTIONS)
        return;
    action.type = type;
    action.handIndex = handIndex;
    action.slot = slot;
    action.arg = arg;
    if (Duel_IsLegalAction(duel, side, &action))
        actions[(*count)++] = action;
}

// Lists every legal action of side. A Basic Pokémon goes to the first empty Bench slot, so each
// choice is listed once. actions must hold CARD_MAX_ACTIONS.
u8 Duel_GetLegalActions(const struct CardDuel *duel, u8 sideId, struct CardAction *actions)
{
    const struct CardDuelSide *side;
    u8 count = 0, i, slot;

    if (!Duel_CanAct(duel, sideId))
        return 0;
    side = &duel->sides[sideId];
    switch (duel->phase)
    {
    case CARD_PHASE_SETUP:
        for (i = 0; i < side->handCount; i++)
        {
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PLAY_BASIC, i, CARD_SLOT_ACTIVE, 0);
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PLAY_BASIC, i, GetFirstEmptyBenchSlot(side), 0);
        }
        TryAddAction(duel, sideId, actions, &count, CARD_ACTION_END_SETUP, 0, 0, 0);
        break;
    case CARD_PHASE_PROMOTE:
        for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PROMOTE, 0, slot, 0);
        break;
    case CARD_PHASE_MAIN:
        for (i = 0; i < CARD_MAX_ATTACKS; i++)
        {
            for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
                TryAddAction(duel, sideId, actions, &count, CARD_ACTION_ATTACK, 0, slot, i);
        }
        for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
        {
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_ATTACH_ENERGY, 0, slot, 0);
            for (i = 0; i < CARD_SLOT_COUNT; i++)
                TryAddAction(duel, sideId, actions, &count, CARD_ACTION_USE_ABILITY, 0, slot, i);
        }
        for (i = 0; i < side->handCount; i++)
        {
            const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(side->hand[i]);
            u8 arg, argCount = (trainer != NULL && trainer->effect == CARD_TRAINER_RARE_CANDY) ? side->handCount : 1;

            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PLAY_BASIC, i, GetFirstEmptyBenchSlot(side), 0);
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PLAY_TRAINER, i, CARD_SLOT_NONE, 0);
            for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
            {
                TryAddAction(duel, sideId, actions, &count, CARD_ACTION_EVOLVE, i, slot, 0);
                for (arg = 0; arg < argCount; arg++)
                    TryAddAction(duel, sideId, actions, &count, CARD_ACTION_PLAY_TRAINER, i, slot, arg);
            }
        }
        for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
            TryAddAction(duel, sideId, actions, &count, CARD_ACTION_RETREAT, 0, slot, 0);
        TryAddAction(duel, sideId, actions, &count, CARD_ACTION_END_TURN, 0, 0, 0);
        break;
    }
    return count;
}

// ---------------------------------------------------------------------------------------------
// Doing an action

static void PutPokemonInPlay(struct CardDuel *duel, u8 sideId, u8 slot, u16 cardId)
{
    struct CardInPlay *mon = &duel->sides[sideId].slots[slot];

    ZeroBytes(mon, sizeof(*mon));
    mon->cardId = cardId;
    mon->turnPlayed = duel->turn;
    Duel_PushEvent(duel, CARD_EVENT_PLAY_POKEMON, sideId, slot, cardId);
}

void Duel_EvolveSlot(struct CardDuel *duel, u8 sideId, u8 slot, u16 cardId)
{
    struct CardInPlay *mon = &duel->sides[sideId].slots[slot];

    if (mon->evolvedFrom[0] == CARD_NONE)
        mon->evolvedFrom[0] = mon->cardId;
    else
        mon->evolvedFrom[1] = mon->cardId;
    mon->cardId = cardId;
    mon->turnPlayed = duel->turn;
    mon->cantAttackTurn = 0;
    Duel_PushEvent(duel, CARD_EVENT_EVOLVE, sideId, slot, cardId);
    // Evolving cures every Special Condition
    CureStatus(duel, sideId, slot, mon->status);
}

static void SwapWithActive(struct CardDuelSide *side, u8 slot)
{
    struct CardInPlay temp = side->slots[CARD_SLOT_ACTIVE];

    side->slots[CARD_SLOT_ACTIVE] = side->slots[slot];
    side->slots[slot] = temp;
}

// The Active goes to the Bench, which cures its Special Conditions and ends the effects on it
static void MoveActiveToBench(struct CardDuel *duel, u8 sideId, u8 slot)
{
    struct CardDuelSide *side = &duel->sides[sideId];

    CureStatus(duel, sideId, CARD_SLOT_ACTIVE, side->slots[CARD_SLOT_ACTIVE].status);
    side->slots[CARD_SLOT_ACTIVE].cantAttackTurn = 0;
    SwapWithActive(side, slot);
}

// The opponent's effects switch side's Active with the Benched Pokémon in slot
void Duel_SwitchActive(struct CardDuel *duel, u8 sideId, u8 slot)
{
    if (!IsBenchSlot(slot) || !IsSlotFilled(&duel->sides[sideId], slot))
        return;
    MoveActiveToBench(duel, sideId, slot);
    Duel_PushEvent(duel, CARD_EVENT_SWITCH, sideId, slot, duel->sides[sideId].slots[CARD_SLOT_ACTIVE].cardId);
}

void Duel_SwitchToRandomBench(struct CardDuel *duel, u8 sideId)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    u8 slot, count = 0, pick;

    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (IsSlotFilled(side, slot))
            count++;
    }
    if (count == 0)
        return;
    pick = Duel_Random(duel) % count;
    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (IsSlotFilled(side, slot) && pick-- == 0)
        {
            Duel_SwitchActive(duel, sideId, slot);
            return;
        }
    }
}

static void Retreat(struct CardDuel *duel, u8 sideId, u8 slot)
{
    struct CardDuelSide *side = &duel->sides[sideId];

    Duel_DiscardEnergy(duel, sideId, CARD_SLOT_ACTIVE, GetRetreatCost(side), CARD_TYPE_NONE);
    MoveActiveToBench(duel, sideId, slot);
    side->retreated = TRUE;
    Duel_PushEvent(duel, CARD_EVENT_RETREAT, sideId, slot, side->slots[CARD_SLOT_ACTIVE].cardId);
}

static void PlayTrainer(struct CardDuel *duel, u8 sideId, u8 handIndex, u8 slot, u8 arg)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    const struct TrainerCardInfo *info = CardGame_GetTrainerInfo(side->hand[handIndex]);
    u16 cardId, extraCard = CARD_NONE;

    // Rare Candy also takes the Stage 2 from the hand: remove the later card first
    if (info->effect == CARD_TRAINER_RARE_CANDY)
    {
        if (arg > handIndex)
        {
            extraCard = RemoveFromHand(side, arg);
            cardId = RemoveFromHand(side, handIndex);
        }
        else
        {
            cardId = RemoveFromHand(side, handIndex);
            extraCard = RemoveFromHand(side, arg);
        }
    }
    else
    {
        cardId = RemoveFromHand(side, handIndex);
    }

    Duel_PushEvent(duel, CARD_EVENT_PLAY_TRAINER, sideId, slot, cardId);
    if (info->kind == CARD_KIND_SUPPORTER)
        side->supporterPlayed = TRUE;
    if (info->kind == CARD_KIND_TOOL)
    {
        side->slots[slot].tool = cardId;
    }
    else
    {
        AddToDiscard(side, cardId);
        CardEffects_PlayTrainer(duel, sideId, info, slot, extraCard);
    }
    ResolveKnockOuts(duel);
    CheckGameOver(duel);
}

// A Pokémon knocked out during its opponent's turn (by an ability or a Trainer) is replaced
// right away, then the turn goes on.
static void PromoteMidTurn(struct CardDuel *duel)
{
    u8 sideId;

    if (duel->phase != CARD_PHASE_MAIN)
        return;
    duel->promoteMask = 0;
    for (sideId = 0; sideId < CARD_SIDE_COUNT; sideId++)
    {
        if (duel->sides[sideId].slots[CARD_SLOT_ACTIVE].cardId == CARD_NONE)
            duel->promoteMask |= 1 << sideId;
    }
    if (duel->promoteMask != 0)
    {
        duel->phase = CARD_PHASE_PROMOTE;
        duel->promoteMidTurn = TRUE;
    }
}

static void UseAbility(struct CardDuel *duel, u8 sideId, u8 slot, u8 target)
{
    struct CardInPlay *mon = &duel->sides[sideId].slots[slot];
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);

    mon->abilityUsedTurn = duel->turn;
    Duel_PushEvent(duel, CARD_EVENT_ABILITY, sideId, slot, info->ability);
    CardEffects_UseAbility(duel, sideId, slot, target);
    ResolveKnockOuts(duel);
    if (!CheckGameOver(duel))
        PromoteMidTurn(duel);
}

bool8 Duel_DoAction(struct CardDuel *duel, u8 sideId, const struct CardAction *action)
{
    struct CardDuelSide *side;

    if (!Duel_IsLegalAction(duel, sideId, action))
        return FALSE;
    side = &duel->sides[sideId];

    switch (action->type)
    {
    case CARD_ACTION_PLAY_BASIC:
        PutPokemonInPlay(duel, sideId, action->slot, RemoveFromHand(side, action->handIndex));
        break;
    case CARD_ACTION_END_SETUP:
        side->setupDone = TRUE;
        if (duel->sides[CARD_SIDE_PLAYER].setupDone && duel->sides[CARD_SIDE_OPPONENT].setupDone)
            StartTurn(duel);
        break;
    case CARD_ACTION_ATTACH_ENERGY:
        side->slots[action->slot].energy[side->zoneEnergy]++;
        Duel_PushEvent(duel, CARD_EVENT_ATTACH_ENERGY, sideId, action->slot, side->zoneEnergy);
        side->zoneEnergy = CARD_TYPE_NONE;
        side->energyAttached = TRUE;
        break;
    case CARD_ACTION_EVOLVE:
        Duel_EvolveSlot(duel, sideId, action->slot, RemoveFromHand(side, action->handIndex));
        break;
    case CARD_ACTION_PLAY_TRAINER:
        PlayTrainer(duel, sideId, action->handIndex, action->slot, action->arg);
        PromoteMidTurn(duel);
        break;
    case CARD_ACTION_USE_ABILITY:
        UseAbility(duel, sideId, action->slot, action->arg);
        break;
    case CARD_ACTION_RETREAT:
        Retreat(duel, sideId, action->slot);
        break;
    case CARD_ACTION_ATTACK:
        DoAttack(duel, action->arg, action->slot);
        if (duel->phase == CARD_PHASE_MAIN)
            EndTurn(duel);
        break;
    case CARD_ACTION_END_TURN:
        EndTurn(duel);
        break;
    case CARD_ACTION_PROMOTE:
        SwapWithActive(side, action->slot);
        Duel_PushEvent(duel, CARD_EVENT_PROMOTE, sideId, action->slot, side->slots[CARD_SLOT_ACTIVE].cardId);
        duel->promoteMask &= ~(1 << sideId);
        if (duel->promoteMask == 0)
        {
            if (duel->promoteMidTurn)
            {
                duel->promoteMidTurn = FALSE;
                duel->phase = CARD_PHASE_MAIN;
            }
            else
            {
                StartTurn(duel);
            }
        }
        break;
    }
    return TRUE;
}

void Duel_Forfeit(struct CardDuel *duel, u8 side)
{
    if (duel->phase != CARD_PHASE_OVER)
        SetGameOver(duel, side == CARD_SIDE_PLAYER ? CARD_SIDE_OPPONENT : CARD_SIDE_PLAYER);
}

// ---------------------------------------------------------------------------------------------
// Attacks, knockouts and the flow of turns

static void DoAttack(struct CardDuel *duel, u8 attackId, u8 target)
{
    u8 sideId = duel->currentSide;
    u8 opponentId = sideId ^ 1;
    struct CardDuelSide *side = &duel->sides[sideId];
    struct CardInPlay *attacker = &side->slots[CARD_SLOT_ACTIVE];
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(attacker->cardId);
    const struct CardAttack *attack = &info->attacks[attackId];
    u8 targetSlot = attack->effect == CARD_EFFECT_SNIPE ? target : CARD_SLOT_ACTIVE;
    const struct PokemonCardInfo *defenderInfo = CardGame_GetPokemonInfo(duel->sides[opponentId].slots[targetSlot].cardId);
    u16 damage;

    Duel_PushEvent(duel, CARD_EVENT_ATTACK, sideId, CARD_SLOT_ACTIVE, attackId);
    if ((attacker->status & CARD_STATUS_CONFUSED) && !Duel_FlipCoin(duel, sideId))
    {
        Duel_PushEvent(duel, CARD_EVENT_ATTACK_FAILED, sideId, CARD_SLOT_ACTIVE, attackId);
        return;
    }
    damage = attack->damage;
    if (!CardEffects_BeforeAttack(duel, sideId, attack, &damage))
    {
        Duel_PushEvent(duel, CARD_EVENT_ATTACK_FAILED, sideId, CARD_SLOT_ACTIVE, attackId);
        return;
    }
    if (damage != 0 && defenderInfo != NULL)
    {
        if (side->attackBonus != 0 && (side->attackBonusType == CARD_TYPE_NONE || side->attackBonusType == info->type))
            damage += side->attackBonus;
        // Weakness only counts against the Active Pokémon
        if (targetSlot == CARD_SLOT_ACTIVE && defenderInfo->weakness != CARD_TYPE_NONE && defenderInfo->weakness == info->type)
            damage += CARD_WEAKNESS_BONUS;
        damage = CardEffects_ModifyDamageTaken(duel, opponentId, targetSlot, damage);
        if (damage != 0)
        {
            Duel_DamageSlot(duel, opponentId, targetSlot, damage);
            if (targetSlot == CARD_SLOT_ACTIVE)
                CardEffects_AfterDamagedByAttack(duel, opponentId, CARD_SLOT_ACTIVE);
        }
    }
    CardEffects_AfterAttack(duel, sideId, attack, targetSlot);
    ResolveKnockOuts(duel);
    CheckGameOver(duel);
}

static void KnockOut(struct CardDuel *duel, u8 sideId, u8 slot)
{
    struct CardDuelSide *side = &duel->sides[sideId];
    struct CardDuelSide *other = &duel->sides[sideId ^ 1];
    struct CardInPlay *mon = &side->slots[slot];
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);
    u8 trophies = 1;

    if (info != NULL && info->isEx)
        trophies = info->isMega ? CARD_MEGA_EX_TROPHIES : CARD_EX_TROPHIES;

    Duel_PushEvent(duel, CARD_EVENT_KNOCK_OUT, sideId, slot, mon->cardId);
    other->trophies += trophies;
    Duel_PushEvent(duel, CARD_EVENT_TROPHY, sideId ^ 1, CARD_SLOT_NONE, trophies);
    AddToDiscard(side, mon->cardId);
    AddToDiscard(side, mon->evolvedFrom[0]);
    AddToDiscard(side, mon->evolvedFrom[1]);
    AddToDiscard(side, mon->tool);
    ZeroBytes(mon, sizeof(*mon));
}

static void ResolveKnockOuts(struct CardDuel *duel)
{
    u8 i, sideId, slot;

    // The side whose turn it is goes second, so the defender's knockouts come first
    for (i = 0; i < CARD_SIDE_COUNT; i++)
    {
        sideId = duel->currentSide ^ 1 ^ i;
        for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
        {
            struct CardInPlay *mon = &duel->sides[sideId].slots[slot];

            if (mon->cardId != CARD_NONE && mon->damage >= Duel_GetMaxHp(mon))
                KnockOut(duel, sideId, slot);
        }
    }
}

static void SetGameOver(struct CardDuel *duel, u8 winner)
{
    duel->phase = CARD_PHASE_OVER;
    duel->winner = winner;
    duel->promoteMask = 0;
    Duel_PushEvent(duel, CARD_EVENT_GAME_OVER, winner, CARD_SLOT_NONE, 0);
}

// A side wins with 3 trophies or when the other has no Pokémon left; both at once is a draw.
static bool8 CheckGameOver(struct CardDuel *duel)
{
    struct CardDuelSide *player = &duel->sides[CARD_SIDE_PLAYER];
    struct CardDuelSide *opponent = &duel->sides[CARD_SIDE_OPPONENT];
    bool8 playerWins, opponentWins;

    if (duel->phase == CARD_PHASE_OVER)
        return TRUE;
    playerWins = player->trophies >= CARD_WIN_TROPHIES || !HasAnyPokemon(opponent);
    opponentWins = opponent->trophies >= CARD_WIN_TROPHIES || !HasAnyPokemon(player);
    if (playerWins && opponentWins)
        SetGameOver(duel, CARD_RESULT_DRAW);
    else if (playerWins)
        SetGameOver(duel, CARD_SIDE_PLAYER);
    else if (opponentWins)
        SetGameOver(duel, CARD_SIDE_OPPONENT);
    else
        return FALSE;
    return TRUE;
}

// Pokémon Checkup, between turns, on both Active Pokémon
static void Checkup(struct CardDuel *duel)
{
    u8 i, sideId;
    struct CardInPlay *mon;

    for (i = 0; i < CARD_SIDE_COUNT; i++)
    {
        sideId = duel->currentSide ^ i;
        mon = &duel->sides[sideId].slots[CARD_SLOT_ACTIVE];
        if (mon->cardId == CARD_NONE)
            continue;
        if (mon->status & CARD_STATUS_POISONED)
            Duel_DamageSlot(duel, sideId, CARD_SLOT_ACTIVE, CARD_POISON_DAMAGE);
        if (mon->status & CARD_STATUS_BURNED)
        {
            Duel_DamageSlot(duel, sideId, CARD_SLOT_ACTIVE, CARD_BURN_DAMAGE);
            if (Duel_FlipCoin(duel, sideId))
                CureStatus(duel, sideId, CARD_SLOT_ACTIVE, CARD_STATUS_BURNED);
        }
        if ((mon->status & CARD_STATUS_ASLEEP) && Duel_FlipCoin(duel, sideId))
            CureStatus(duel, sideId, CARD_SLOT_ACTIVE, CARD_STATUS_ASLEEP);
        // Paralysis wears off at the end of its owner's next turn
        if ((mon->status & CARD_STATUS_PARALYZED) && sideId == duel->currentSide && mon->statusTurn != duel->turn)
            CureStatus(duel, sideId, CARD_SLOT_ACTIVE, CARD_STATUS_PARALYZED);
    }
}

static void EndTurn(struct CardDuel *duel)
{
    u8 sideId;

    // An Energy that wasn't attached is lost
    duel->sides[duel->currentSide].zoneEnergy = CARD_TYPE_NONE;
    if (CheckGameOver(duel))
        return;
    Checkup(duel);
    ResolveKnockOuts(duel);
    if (CheckGameOver(duel))
        return;
    if (duel->turn >= CARD_TURN_LIMIT)
    {
        SetGameOver(duel, CARD_RESULT_DRAW);
        return;
    }

    duel->currentSide ^= 1;
    duel->promoteMask = 0;
    for (sideId = 0; sideId < CARD_SIDE_COUNT; sideId++)
    {
        if (duel->sides[sideId].slots[CARD_SLOT_ACTIVE].cardId == CARD_NONE)
            duel->promoteMask |= 1 << sideId;
    }
    if (duel->promoteMask != 0)
        duel->phase = CARD_PHASE_PROMOTE;
    else
        StartTurn(duel);
}

static void StartTurn(struct CardDuel *duel)
{
    struct CardDuelSide *side = &duel->sides[duel->currentSide];

    duel->phase = CARD_PHASE_MAIN;
    duel->turn++;
    side->energyAttached = FALSE;
    side->supporterPlayed = FALSE;
    side->retreated = FALSE;
    side->retreatDiscount = 0;
    side->attackBonus = 0;
    side->attackBonusType = CARD_TYPE_NONE;
    Duel_PushEvent(duel, CARD_EVENT_TURN_START, duel->currentSide, CARD_SLOT_NONE, duel->turn);
    Duel_DrawCard(duel, duel->currentSide);
    // The first player gets no Energy on the first turn
    if (duel->turn == 1)
    {
        side->zoneEnergy = CARD_TYPE_NONE;
    }
    else
    {
        side->zoneEnergy = side->nextEnergy;
        side->nextEnergy = RollEnergy(duel, side);
        Duel_PushEvent(duel, CARD_EVENT_ENERGY_ZONE, duel->currentSide, CARD_SLOT_NONE, side->zoneEnergy);
    }
}
