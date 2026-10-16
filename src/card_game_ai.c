#include "global.h"
#include "card_game.h"

// The card game's opponent (docs/card_game_plan.md, section 4). It is asked for one action at a
// time and decides from the board alone, in the order a player would:
//   1. get cards: Prof. Oak's Research-like draws, Poké Ball;
//   2. build the board: Basics to the Bench, Rare Candy, evolutions, abilities;
//   3. retreat a stuck Active;
//   4. attach the Energy where it brings an attack closest;
//   5. Supporters and Items when they help (Energy, healing, a knockout);
//   6. attack with the best attack, or end the turn.
// Coin flips count at their average: it never looks at the duel's random numbers.
//
// Levels (CARD_AI_*): EASY plays a random action half of the time. NORMAL follows the rules
// above, without retreating or the Supporters that set up a knockout (Giovanni, Blaine,
// Sabrina). HARD does those too, plays the weaknesses, and looks at the opponent's next attack,
// to not feed Energy to a Pokémon about to be knocked out unless it lets it attack first.
// (Retreating a Pokémon about to be knocked out, or holding back all Energy from it, lost more
// games than it saved: tools/card_game/test_data.c plays the levels against each other.)

#define SCORE_NONE  (-30000)

static const struct PokemonCardInfo *GetInfo(const struct CardInPlay *mon)
{
    return CardGame_GetPokemonInfo(mon->cardId);
}

static u16 GetHpLeft(const struct CardInPlay *mon)
{
    u16 maxHp = Duel_GetMaxHp(mon);

    return mon->damage >= maxHp ? 0 : maxHp - mon->damage;
}

static u8 CountBench(const struct CardDuelSide *side)
{
    u8 slot, count = 0;

    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        if (side->slots[slot].cardId != CARD_NONE)
            count++;
    }
    return count;
}

static u8 GetTrophyValue(const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = GetInfo(mon);

    if (info == NULL || !info->isEx)
        return 1;
    return info->isMega ? CARD_MEGA_EX_TROPHIES : CARD_EX_TROPHIES;
}

static bool8 ZoneMakes(const struct CardDuelSide *side, u8 type)
{
    u8 i;

    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (side->zoneTypes[i] == type)
            return TRUE;
    }
    return FALSE;
}

// How many more Energy the attack needs, or 0xFF if the Energy Zone can never pay it
static u8 GetMissingEnergy(const struct CardDuelSide *side, const struct CardInPlay *mon, const struct CardAttack *attack)
{
    u8 left[CARD_TYPE_COUNT];
    u8 i, type, missing = 0, colorless = 0, spare = 0;

    for (type = 0; type < CARD_TYPE_COUNT; type++)
        left[type] = mon->energy[type];
    for (i = 0; i < CARD_ATTACK_MAX_COST && attack->cost[i] != CARD_TYPE_NONE; i++)
    {
        type = attack->cost[i];
        if (type == CARD_TYPE_COLORLESS)
            colorless++;
        else if (left[type] != 0)
            left[type]--;
        else if (ZoneMakes(side, type))
            missing++;
        else
            return 0xFF;
    }
    for (type = 0; type < CARD_TYPE_COUNT; type++)
        spare += left[type];
    if (colorless > spare)
        missing += colorless - spare;
    return missing;
}

// Damage the attack would do to the target: its average, or at least (no lucky flips)
static u16 GetAttackDamage(const struct CardDuel *duel, u8 sideId, const struct CardInPlay *attacker,
                           const struct CardAttack *attack, const struct CardInPlay *target, bool8 isActiveTarget, bool8 guaranteed)
{
    const struct CardDuelSide *side = &duel->sides[sideId];
    const struct PokemonCardInfo *info = GetInfo(attacker);
    const struct PokemonCardInfo *targetInfo = GetInfo(target);
    u16 damage = attack->damage;
    u8 i, typed = 0, total = 0;

    if (info == NULL || targetInfo == NULL)
        return 0;
    switch (attack->effect)
    {
    case CARD_EFFECT_FLIP_BONUS:
        if (!guaranteed)
            damage += attack->effectArg / 2;
        break;
    case CARD_EFFECT_FLIP_MULTI:
        damage = guaranteed ? 0 : damage * attack->effectArg / 2;
        break;
    case CARD_EFFECT_FLIP_OR_FAIL:
        damage = guaranteed ? 0 : damage / 2;
        break;
    case CARD_EFFECT_BONUS_EXTRA_ENERGY:
        for (i = 0; i < CARD_ATTACK_MAX_COST && attack->cost[i] != CARD_TYPE_NONE; i++)
        {
            total++;
            if (attack->cost[i] == info->type)
                typed++;
        }
        if (attacker->energy[info->type] >= typed + 2 && Duel_CountEnergy(attacker) >= total + 2)
            damage += attack->effectArg;
        break;
    case CARD_EFFECT_BONUS_IF_DAMAGED:
        if (target->damage != 0)
            damage += attack->effectArg;
        break;
    }
    if (damage == 0)
        return 0;
    if (side->attackBonus != 0 && (side->attackBonusType == CARD_TYPE_NONE || side->attackBonusType == info->type))
        damage += side->attackBonus;
    if (isActiveTarget && targetInfo->weakness != CARD_TYPE_NONE && targetInfo->weakness == info->type)
        damage += CARD_WEAKNESS_BONUS;
    if (targetInfo->ability == CARD_ABILITY_REDUCE_DAMAGE)
        damage = damage > targetInfo->abilityArg ? damage - targetInfo->abilityArg : 0;
    if (attacker->status & CARD_STATUS_CONFUSED)
        damage = guaranteed ? 0 : damage / 2;
    return damage;
}

// The most damage the Pokémon could do now (payable attacks) to the opponent's Active
static u16 GetBestDamageNow(const struct CardDuel *duel, u8 sideId, const struct CardInPlay *mon, bool8 guaranteed)
{
    const struct PokemonCardInfo *info = GetInfo(mon);
    const struct CardInPlay *target = &duel->sides[sideId ^ 1].slots[CARD_SLOT_ACTIVE];
    u16 damage, best = 0;
    u8 i;

    if (info == NULL)
        return 0;
    for (i = 0; i < info->numAttacks; i++)
    {
        if (!Duel_CanPayAttack(mon, &info->attacks[i]))
            continue;
        damage = GetAttackDamage(duel, sideId, mon, &info->attacks[i], target, TRUE, guaranteed);
        if (damage > best)
            best = damage;
    }
    return best;
}

// The most damage the opponent's Active could do next turn, with one more Energy
static u16 GetThreat(const struct CardDuel *duel, u8 sideId)
{
    const struct CardDuelSide *opponent = &duel->sides[sideId ^ 1];
    const struct CardInPlay *attacker = &opponent->slots[CARD_SLOT_ACTIVE];
    const struct PokemonCardInfo *info = GetInfo(attacker);
    u16 damage, best = 0;
    u8 i;

    if (info == NULL || (attacker->status & (CARD_STATUS_ASLEEP | CARD_STATUS_PARALYZED)))
        return 0;
    for (i = 0; i < info->numAttacks; i++)
    {
        if (GetMissingEnergy(opponent, attacker, &info->attacks[i]) > 1)
            continue;
        damage = GetAttackDamage(duel, sideId ^ 1, attacker, &info->attacks[i], &duel->sides[sideId].slots[CARD_SLOT_ACTIVE], TRUE, FALSE);
        if (damage > best)
            best = damage;
    }
    return best;
}

static bool8 IsDoomed(const struct CardDuel *duel, u8 sideId, u8 level)
{
    return level >= CARD_AI_HARD && GetThreat(duel, sideId) >= GetHpLeft(&duel->sides[sideId].slots[CARD_SLOT_ACTIVE]);
}

// HARD: how well the Pokémon matches up with the opponent's Active (weaknesses both ways)
static s16 GetMatchup(const struct CardDuel *duel, u8 sideId, u8 level, const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = GetInfo(mon);
    const struct PokemonCardInfo *opponent = GetInfo(&duel->sides[sideId ^ 1].slots[CARD_SLOT_ACTIVE]);
    s16 matchup = 0;

    if (level < CARD_AI_HARD || info == NULL || opponent == NULL)
        return 0;
    if (opponent->weakness == info->type)
        matchup += 30;
    if (info->weakness == opponent->type)
        matchup -= 30;
    return matchup;
}

// How much a Pokémon is worth keeping: HP, stage, ex
static s16 GetValue(const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = GetInfo(mon);

    if (info == NULL)
        return 0;
    return GetHpLeft(mon) / 2 + info->stage * 20 + (info->isEx ? 40 : 0) + Duel_CountEnergy(mon) * 10;
}

// The Energy the Pokémon's best attack still needs (0xFF: never)
static u8 GetMissingForBestAttack(const struct CardDuelSide *side, const struct CardInPlay *mon, const struct CardAttack **bestAttack)
{
    const struct PokemonCardInfo *info = GetInfo(mon);
    u8 i, missing, bestMissing = 0xFF;
    u16 bestDamage = 0;

    *bestAttack = NULL;
    if (info == NULL)
        return 0xFF;
    for (i = 0; i < info->numAttacks; i++)
    {
        missing = GetMissingEnergy(side, mon, &info->attacks[i]);
        if (missing == 0xFF)
            continue;
        if (*bestAttack == NULL || info->attacks[i].damage > bestDamage)
        {
            *bestAttack = &info->attacks[i];
            bestDamage = info->attacks[i].damage;
            bestMissing = missing;
        }
    }
    return bestMissing;
}

// ---------------------------------------------------------------------------------------------
// Choosing among the legal actions

struct AiContext
{
    struct CardDuel *duel;
    u8 side;
    u8 level;
    const struct CardAction *actions;
    u8 count;
};

static const struct CardAction *FindAction(const struct AiContext *ctx, u8 type)
{
    u8 i;

    for (i = 0; i < ctx->count; i++)
    {
        if (ctx->actions[i].type == type)
            return &ctx->actions[i];
    }
    return NULL;
}

static u16 GetHandCard(const struct AiContext *ctx, const struct CardAction *action)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];

    return action->handIndex < side->handCount ? side->hand[action->handIndex] : CARD_NONE;
}

static const struct TrainerCardInfo *GetActionTrainer(const struct AiContext *ctx, const struct CardAction *action)
{
    if (action->type != CARD_ACTION_PLAY_TRAINER)
        return NULL;
    return CardGame_GetTrainerInfo(GetHandCard(ctx, action));
}

// The legal Trainer action with this effect that scores best (SCORE_NONE: don't)
static const struct CardAction *FindTrainer(const struct AiContext *ctx, u8 effect, s16 (*score)(const struct AiContext *, const struct CardAction *))
{
    const struct CardAction *best = NULL;
    const struct TrainerCardInfo *trainer;
    s16 value, bestValue = SCORE_NONE;
    u8 i;

    for (i = 0; i < ctx->count; i++)
    {
        trainer = GetActionTrainer(ctx, &ctx->actions[i]);
        if (trainer == NULL || trainer->effect != effect)
            continue;
        value = score != NULL ? score(ctx, &ctx->actions[i]) : 0;
        if (value > bestValue)
        {
            bestValue = value;
            best = &ctx->actions[i];
        }
    }
    return best;
}

static s16 ScoreHealTarget(const struct AiContext *ctx, const struct CardAction *action)
{
    const struct CardInPlay *mon = &ctx->duel->sides[ctx->side].slots[action->slot];

    if (mon->damage < 20)
        return SCORE_NONE;
    return mon->damage + (action->slot == CARD_SLOT_ACTIVE ? 20 : 0) + GetValue(mon) / 4;
}

static s16 ScoreToolTarget(const struct AiContext *ctx, const struct CardAction *action)
{
    return GetValue(&ctx->duel->sides[ctx->side].slots[action->slot]) + (action->slot == CARD_SLOT_ACTIVE ? 30 : 0);
}

static s16 ScoreEnergyTarget(const struct AiContext *ctx, const struct CardAction *action)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardInPlay *mon = &side->slots[action->slot];
    const struct CardAttack *attack;
    u8 missing = GetMissingForBestAttack(side, mon, &attack);

    if (missing == 0 || missing == 0xFF)
        return SCORE_NONE;
    return 100 - missing * 10 + (action->slot == CARD_SLOT_ACTIVE ? 20 : 0) + GetValue(mon) / 8
         + GetMatchup(ctx->duel, ctx->side, ctx->level, mon) / 2;
}

static s16 ScoreRareCandy(const struct AiContext *ctx, const struct CardAction *action)
{
    return GetValue(&ctx->duel->sides[ctx->side].slots[action->slot]);
}

// 1. Cards: draw while the hand is small, Poké Ball while the Bench has room
static const struct CardAction *ChooseDraw(const struct AiContext *ctx)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardAction *action;

    if (side->handCount <= 6 && (action = FindTrainer(ctx, CARD_TRAINER_DRAW, NULL)) != NULL)
        return action;
    if (CountBench(side) < CARD_BENCH_MAX && (action = FindTrainer(ctx, CARD_TRAINER_POKE_BALL, NULL)) != NULL)
        return action;
    return NULL;
}

// The best damage this Pokémon would do now as a given card (to compare before/after evolving)
static u16 GetBestDamageAs(const struct CardDuel *duel, u8 sideId, const struct CardInPlay *mon, u16 cardId)
{
    struct CardInPlay copy = *mon;

    copy.cardId = cardId;
    copy.status = 0;
    return GetBestDamageNow(duel, sideId, &copy, FALSE);
}

// 2. Board: Rare Candy, evolutions (the Active only if it doesn't lose this turn's attack),
// Basics to the Bench
static const struct CardAction *ChooseBoard(const struct AiContext *ctx)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardAction *action;
    u8 i;

    if ((action = FindTrainer(ctx, CARD_TRAINER_RARE_CANDY, ScoreRareCandy)) != NULL)
        return action;
    for (i = 0; i < ctx->count; i++)
    {
        action = &ctx->actions[i];
        if (action->type != CARD_ACTION_EVOLVE)
            continue;
        if (action->slot != CARD_SLOT_ACTIVE
         || GetBestDamageAs(ctx->duel, ctx->side, &side->slots[CARD_SLOT_ACTIVE], GetHandCard(ctx, action))
            >= GetBestDamageNow(ctx->duel, ctx->side, &side->slots[CARD_SLOT_ACTIVE], FALSE))
            return action;
    }
    return FindAction(ctx, CARD_ACTION_PLAY_BASIC);
}

// 3. Abilities that are free value
static const struct CardAction *ChooseAbility(const struct AiContext *ctx)
{
    const struct CardDuel *duel = ctx->duel;
    const struct CardDuelSide *side = &duel->sides[ctx->side];
    const struct CardDuelSide *opponent = &duel->sides[ctx->side ^ 1];
    const struct CardAction *best = NULL;
    const struct PokemonCardInfo *info;
    s16 value, bestValue = SCORE_NONE;
    u8 i, slot;
    u16 totalDamage = 0;

    for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
        totalDamage += side->slots[slot].damage;
    for (i = 0; i < ctx->count; i++)
    {
        const struct CardAction *action = &ctx->actions[i];

        if (action->type != CARD_ACTION_USE_ABILITY)
            continue;
        info = GetInfo(&side->slots[action->slot]);
        value = SCORE_NONE;
        switch (info->ability)
        {
        case CARD_ABILITY_HEAL_ALL:
            if (totalDamage >= 20)
                value = totalDamage;
            break;
        case CARD_ABILITY_ATTACH_OWN_TYPE:
        case CARD_ABILITY_POISON_DEFENDER:
            value = 50;
            break;
        case CARD_ABILITY_FLIP_SLEEP:
            if (!(opponent->slots[CARD_SLOT_ACTIVE].status & CARD_STATUS_ASLEEP))
                value = 40;
            break;
        case CARD_ABILITY_DRIVE_OFF:
            // Bring up something weaker than their Active
            for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
            {
                if (opponent->slots[slot].cardId != CARD_NONE
                 && GetValue(&opponent->slots[slot]) < GetValue(&opponent->slots[CARD_SLOT_ACTIVE]))
                    value = 30;
            }
            break;
        case CARD_ABILITY_SNIPE:
            // A knockout first, then the most trophies, then the most hurt
            value = 10 + opponent->slots[action->arg].damage / 10;
            if (GetHpLeft(&opponent->slots[action->arg]) <= info->abilityArg)
                value += 100 * GetTrophyValue(&opponent->slots[action->arg]);
            break;
        }
        if (value > bestValue)
        {
            bestValue = value;
            best = action;
        }
    }
    return best;
}

// 4. Retreat (HARD): when the Active can't attack for a while and a Benched Pokémon can now
static const struct CardAction *ChooseRetreat(const struct AiContext *ctx)
{
    const struct CardDuel *duel = ctx->duel;
    const struct CardDuelSide *side = &duel->sides[ctx->side];
    const struct CardInPlay *active = &side->slots[CARD_SLOT_ACTIVE];
    const struct CardAttack *attack;
    const struct CardAction *best = NULL, *action;
    s16 value, bestValue = SCORE_NONE;
    u8 missing, i;
    bool8 stuck;

    if (ctx->level < CARD_AI_HARD || side->retreated)
        return NULL;
    // Keep the Active when it can knock out the opponent's now
    if (GetBestDamageNow(duel, ctx->side, active, TRUE) >= GetHpLeft(&duel->sides[ctx->side ^ 1].slots[CARD_SLOT_ACTIVE]))
        return NULL;
    missing = GetMissingForBestAttack(side, active, &attack);
    stuck = (missing >= 2 || (active->status & CARD_STATUS_CONFUSED)) && GetBestDamageNow(duel, ctx->side, active, FALSE) == 0;
    if (!stuck)
        return NULL;

    for (i = 0; i < ctx->count; i++)
    {
        const struct CardInPlay *mon;

        action = &ctx->actions[i];
        if (action->type != CARD_ACTION_RETREAT)
            continue;
        mon = &side->slots[action->slot];
        value = GetBestDamageNow(duel, ctx->side, mon, FALSE) + GetHpLeft(mon) / 4 + GetMatchup(duel, ctx->side, ctx->level, mon);
        if (GetBestDamageNow(duel, ctx->side, mon, FALSE) == 0)
            continue;
        if (value > bestValue)
        {
            bestValue = value;
            best = action;
        }
    }
    // X Speed first when it makes the retreat cheaper
    if (best != NULL && (action = FindTrainer(ctx, CARD_TRAINER_RETREAT_DISCOUNT, NULL)) != NULL
     && GetInfo(active)->retreatCost != 0 && !CardEffects_HasFreeRetreat(active))
        return action;
    return best;
}

// Whether one more Energy lets the Active attack this turn
static bool8 CanAttackWithOneMore(const struct AiContext *ctx)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardInPlay *active = &side->slots[CARD_SLOT_ACTIVE];
    const struct PokemonCardInfo *info = GetInfo(active);
    u8 i;

    if (info == NULL)
        return FALSE;
    for (i = 0; i < info->numAttacks; i++)
    {
        if (info->attacks[i].damage != 0 && GetMissingEnergy(side, active, &info->attacks[i]) <= 1)
            return TRUE;
    }
    return FALSE;
}

// 5. Energy: where it brings an attack closest (HARD: not to an Active about to be knocked out,
// unless it can attack first)
static const struct CardAction *ChooseEnergy(const struct AiContext *ctx)
{
    const struct CardAction *best = NULL;
    s16 value, bestValue = SCORE_NONE;
    bool8 doomed = IsDoomed(ctx->duel, ctx->side, ctx->level) && !CanAttackWithOneMore(ctx);
    u8 i;

    for (i = 0; i < ctx->count; i++)
    {
        const struct CardAction *action = &ctx->actions[i];

        if (action->type != CARD_ACTION_ATTACH_ENERGY)
            continue;
        value = ScoreEnergyTarget(ctx, action);
        if (value == SCORE_NONE)
            value = 0; // nobody needs it: anywhere is better than losing it
        if (doomed && action->slot == CARD_SLOT_ACTIVE)
            value -= 60;
        if (value > bestValue)
        {
            bestValue = value;
            best = action;
        }
    }
    return best;
}

// Damage of the Active's best attack this turn, at least or on average
static u16 GetActiveDamage(const struct AiContext *ctx, bool8 guaranteed)
{
    return GetBestDamageNow(ctx->duel, ctx->side, &ctx->duel->sides[ctx->side].slots[CARD_SLOT_ACTIVE], guaranteed);
}

// 6. Supporters and Items that help now
static const struct CardAction *ChooseTrainer(const struct AiContext *ctx)
{
    const struct CardDuel *duel = ctx->duel;
    const struct CardDuelSide *side = &duel->sides[ctx->side];
    const struct CardDuelSide *opponent = &duel->sides[ctx->side ^ 1];
    const struct CardInPlay *active = &side->slots[CARD_SLOT_ACTIVE];
    const struct CardInPlay *defender = &opponent->slots[CARD_SLOT_ACTIVE];
    const struct CardAction *action;
    u16 hpLeft = GetHpLeft(defender);
    u8 i;

    // A bonus that turns the attack into a knockout (HARD)
    for (i = 0; i < ctx->count && ctx->level >= CARD_AI_HARD; i++)
    {
        const struct TrainerCardInfo *trainer = GetActionTrainer(ctx, &ctx->actions[i]);
        u16 damage = GetActiveDamage(ctx, TRUE);

        if (trainer == NULL || trainer->effect != CARD_TRAINER_ATTACK_BONUS || damage == 0)
            continue;
        if (trainer->effectType != CARD_TYPE_NONE && trainer->effectType != GetInfo(active)->type)
            continue;
        if (damage < hpLeft && damage + trainer->effectArg >= hpLeft)
            return &ctx->actions[i];
    }
    // Switching their Active when a Benched one could be knocked out instead
    if (ctx->level >= CARD_AI_HARD && (action = FindTrainer(ctx, CARD_TRAINER_SWITCH_OPPONENT, NULL)) != NULL
     && GetActiveDamage(ctx, TRUE) < hpLeft)
    {
        for (i = CARD_SLOT_BENCH_1; i < CARD_SLOT_COUNT; i++)
        {
            if (opponent->slots[i].cardId != CARD_NONE && GetHpLeft(&opponent->slots[i]) <= GetActiveDamage(ctx, TRUE))
                return action;
        }
    }
    // Energy Supporters, while someone still needs Energy
    if ((action = FindTrainer(ctx, CARD_TRAINER_FLIP_ATTACH_ENERGY, ScoreEnergyTarget)) != NULL && ScoreEnergyTarget(ctx, action) != SCORE_NONE)
        return action;
    if ((action = FindTrainer(ctx, CARD_TRAINER_ATTACH_ENERGY, ScoreEnergyTarget)) != NULL && ScoreEnergyTarget(ctx, action) != SCORE_NONE)
        return action;
    if ((action = FindTrainer(ctx, CARD_TRAINER_GATHER_ENERGY, NULL)) != NULL && GetActiveDamage(ctx, FALSE) == 0)
        return action;
    // Healing
    if ((action = FindTrainer(ctx, CARD_TRAINER_HEAL_TYPE, ScoreHealTarget)) != NULL && ScoreHealTarget(ctx, action) != SCORE_NONE)
        return action;
    if ((action = FindTrainer(ctx, CARD_TRAINER_HEAL, ScoreHealTarget)) != NULL && ScoreHealTarget(ctx, action) != SCORE_NONE)
        return action;
    // Tools on the best Pokémon
    if ((action = FindTrainer(ctx, CARD_TRAINER_TOOL_HP, ScoreToolTarget)) != NULL)
        return action;
    if ((action = FindTrainer(ctx, CARD_TRAINER_TOOL_COUNTER, ScoreToolTarget)) != NULL)
        return action;
    // Poison an Active that will stay a while (not about to be knocked out)
    if ((action = FindTrainer(ctx, CARD_TRAINER_POISON_OPPONENT, NULL)) != NULL && GetActiveDamage(ctx, TRUE) < hpLeft)
        return action;
    // Red Card against a big hand
    if (opponent->handCount >= 5 && (action = FindTrainer(ctx, CARD_TRAINER_RED_CARD, NULL)) != NULL)
        return action;
    return NULL;
}

// 7. The best attack: a sure knockout, then the most damage and the best effects
static s16 ScoreAttack(const struct AiContext *ctx, const struct CardAction *action)
{
    const struct CardDuel *duel = ctx->duel;
    const struct CardDuelSide *side = &duel->sides[ctx->side];
    const struct CardDuelSide *opponent = &duel->sides[ctx->side ^ 1];
    const struct CardInPlay *attacker = &side->slots[CARD_SLOT_ACTIVE];
    const struct CardAttack *attack = &GetInfo(attacker)->attacks[action->arg];
    u8 targetSlot = attack->effect == CARD_EFFECT_SNIPE ? action->slot : CARD_SLOT_ACTIVE;
    const struct CardInPlay *target = &opponent->slots[targetSlot];
    u16 hpLeft = GetHpLeft(target);
    u16 average = GetAttackDamage(duel, ctx->side, attacker, attack, target, targetSlot == CARD_SLOT_ACTIVE, FALSE);
    u16 sure = GetAttackDamage(duel, ctx->side, attacker, attack, target, targetSlot == CARD_SLOT_ACTIVE, TRUE);
    s16 score = average < hpLeft ? average : hpLeft;

    if (sure >= hpLeft)
        score += 200 + 100 * GetTrophyValue(target);
    else if (average >= hpLeft)
        score += 60 * GetTrophyValue(target);

    switch (attack->effect)
    {
    case CARD_EFFECT_STATUS:
        score += 15;
        break;
    case CARD_EFFECT_FLIP_STATUS:
    case CARD_EFFECT_FLIP_DISCARD_ENERGY:
    case CARD_EFFECT_SWITCH_DEFENDER:
        score += 8;
        break;
    case CARD_EFFECT_HEAL_SELF:
        score += (attacker->damage < attack->effectArg ? attacker->damage : attack->effectArg) / 2;
        break;
    case CARD_EFFECT_SELF_DAMAGE:
        if (attack->effectArg >= GetHpLeft(attacker))
            score -= 100 * GetTrophyValue(attacker) + 50;
        else
            score -= attack->effectArg / 2;
        break;
    case CARD_EFFECT_DISCARD_ENERGY:
        score -= sure >= hpLeft ? 0 : attack->effectArg * 8;
        break;
    case CARD_EFFECT_DISCARD_ALL_ENERGY:
        score -= sure >= hpLeft ? 0 : Duel_CountEnergy(attacker) * 8;
        break;
    case CARD_EFFECT_CANT_ATTACK_NEXT:
        score -= sure >= hpLeft ? 0 : 15;
        break;
    case CARD_EFFECT_DRAW:
        score += 10 * attack->effectArg;
        break;
    case CARD_EFFECT_SPREAD_BENCH:
        score += CountBench(opponent) * attack->effectArg / 2;
        break;
    case CARD_EFFECT_SELF_BENCH_DAMAGE:
        score -= CountBench(side) * attack->effectArg / 2;
        break;
    }
    return score;
}

static const struct CardAction *ChooseAttack(const struct AiContext *ctx)
{
    const struct CardAction *best = NULL;
    s16 value, bestValue = 0;
    u8 i;

    for (i = 0; i < ctx->count; i++)
    {
        if (ctx->actions[i].type != CARD_ACTION_ATTACK)
            continue;
        value = ScoreAttack(ctx, &ctx->actions[i]);
        if (best == NULL || value > bestValue)
        {
            bestValue = value;
            best = &ctx->actions[i];
        }
    }
    // An attack that only hurts itself is worse than ending the turn
    return (best != NULL && bestValue > 0) ? best : NULL;
}

// Setup: the sturdiest Basic that can attack soon as the Active, the others on the Bench
static const struct CardAction *ChooseSetup(const struct AiContext *ctx)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardAction *best = NULL;
    const struct PokemonCardInfo *info;
    struct CardInPlay noEnergy = {0};
    s16 value, bestValue = SCORE_NONE;
    u8 i, j;

    if (side->slots[CARD_SLOT_ACTIVE].cardId != CARD_NONE)
    {
        const struct CardAction *bench = FindAction(ctx, CARD_ACTION_PLAY_BASIC);

        return bench != NULL ? bench : FindAction(ctx, CARD_ACTION_END_SETUP);
    }
    for (i = 0; i < ctx->count; i++)
    {
        if (ctx->actions[i].type != CARD_ACTION_PLAY_BASIC || ctx->actions[i].slot != CARD_SLOT_ACTIVE)
            continue;
        info = CardGame_GetPokemonInfo(GetHandCard(ctx, &ctx->actions[i]));
        value = info->hp;
        for (j = 0; j < info->numAttacks; j++)
        {
            // Can attack with this deck's Energy at all
            if (GetMissingEnergy(side, &noEnergy, &info->attacks[j]) <= 2)
            {
                value += 50;
                break;
            }
        }
        if (value > bestValue)
        {
            bestValue = value;
            best = &ctx->actions[i];
        }
    }
    return best;
}

// After a knockout: the Benched Pokémon that hits hardest now, then the sturdiest
static const struct CardAction *ChoosePromotion(const struct AiContext *ctx)
{
    const struct CardDuelSide *side = &ctx->duel->sides[ctx->side];
    const struct CardAction *best = NULL;
    const struct CardAttack *attack;
    s16 value, bestValue = SCORE_NONE;
    u8 i;

    for (i = 0; i < ctx->count; i++)
    {
        const struct CardInPlay *mon = &side->slots[ctx->actions[i].slot];

        if (ctx->actions[i].type != CARD_ACTION_PROMOTE)
            continue;
        value = GetBestDamageNow(ctx->duel, ctx->side, mon, FALSE) * 2 + GetHpLeft(mon) / 2
              - GetMissingForBestAttack(side, mon, &attack) * 5 + GetMatchup(ctx->duel, ctx->side, ctx->level, mon);
        if (value > bestValue)
        {
            bestValue = value;
            best = &ctx->actions[i];
        }
    }
    return best;
}

static const struct CardAction *ChooseRandom(struct CardDuel *duel, const struct CardAction *actions, u8 count)
{
    return &actions[Duel_Random(duel) % count];
}

// actions must hold CARD_MAX_ACTIONS; side must be able to act.
void CardAi_ChooseAction(struct CardDuel *duel, u8 side, u8 level, struct CardAction *actions, struct CardAction *chosen)
{
    struct AiContext ctx;
    const struct CardAction *action = NULL;

    ctx.duel = duel;
    ctx.side = side;
    ctx.level = level;
    ctx.actions = actions;
    ctx.count = Duel_GetLegalActions(duel, side, actions);

    if (duel->phase == CARD_PHASE_SETUP)
        action = ChooseSetup(&ctx);
    else if (duel->phase == CARD_PHASE_PROMOTE)
        action = ChoosePromotion(&ctx);
    else if (level == CARD_AI_EASY && (Duel_Random(duel) & 1))
        action = ChooseRandom(duel, actions, ctx.count);
    else if ((action = ChooseDraw(&ctx)) == NULL
          && (action = ChooseBoard(&ctx)) == NULL
          && (action = ChooseAbility(&ctx)) == NULL
          && (action = ChooseRetreat(&ctx)) == NULL
          && (action = ChooseEnergy(&ctx)) == NULL
          && (action = ChooseTrainer(&ctx)) == NULL
          && (action = ChooseAttack(&ctx)) == NULL)
        action = FindAction(&ctx, CARD_ACTION_END_TURN);

    if (action == NULL)
        action = ChooseRandom(duel, actions, ctx.count);
    *chosen = *action;
}
