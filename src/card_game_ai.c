#include "global.h"
#include "card_game.h"

// The card game's opponent. For now (phase 3 of docs/card_game_plan.md) it plays a random legal
// action, leaning towards progress so duels end; phase 5 replaces it with a real player.

static u8 CountActions(const struct CardAction *actions, u8 count, u8 type)
{
    u8 i, found = 0;

    for (i = 0; i < count; i++)
    {
        if (actions[i].type == type)
            found++;
    }
    return found;
}

static const struct CardAction *PickOfType(struct CardDuel *duel, const struct CardAction *actions, u8 count, u8 type)
{
    u8 i, pick = Duel_Random(duel) % CountActions(actions, count, type);

    for (i = 0; i < count; i++)
    {
        if (actions[i].type == type && pick-- == 0)
            return &actions[i];
    }
    return &actions[0];
}

// The strongest attack it can use
static const struct CardAction *PickAttack(const struct CardDuel *duel, u8 side, const struct CardAction *actions, u8 count)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(duel->sides[side].slots[CARD_SLOT_ACTIVE].cardId);
    const struct CardAction *best = NULL;
    u8 i;

    for (i = 0; i < count; i++)
    {
        if (actions[i].type != CARD_ACTION_ATTACK)
            continue;
        if (best == NULL || info->attacks[actions[i].arg].damage > info->attacks[best->arg].damage)
            best = &actions[i];
    }
    return best;
}

// actions must hold CARD_MAX_ACTIONS; side must be able to act.
void CardAi_ChooseAction(struct CardDuel *duel, u8 side, struct CardAction *actions, struct CardAction *chosen)
{
    u8 count = Duel_GetLegalActions(duel, side, actions);
    u8 i, others = 0;

    for (i = 0; i < count; i++)
    {
        if (actions[i].type != CARD_ACTION_ATTACK && actions[i].type != CARD_ACTION_END_TURN
         && actions[i].type != CARD_ACTION_END_SETUP)
            others++;
    }

    if (duel->phase == CARD_PHASE_SETUP)
    {
        // Any Basic as the Active, then each other one to the Bench half the time
        if (duel->sides[side].slots[CARD_SLOT_ACTIVE].cardId == CARD_NONE || (others != 0 && (Duel_Random(duel) & 1)))
            *chosen = *PickOfType(duel, actions, count, CARD_ACTION_PLAY_BASIC);
        else
            *chosen = *PickOfType(duel, actions, count, CARD_ACTION_END_SETUP);
        return;
    }
    if (duel->phase == CARD_PHASE_PROMOTE)
    {
        *chosen = *PickOfType(duel, actions, count, CARD_ACTION_PROMOTE);
        return;
    }
    // Something else 7 times in 8, then the best attack, or the end of the turn
    if (others != 0 && (Duel_Random(duel) % 8) != 0)
    {
        u8 pick = Duel_Random(duel) % others;

        for (i = 0; i < count; i++)
        {
            if (actions[i].type != CARD_ACTION_ATTACK && actions[i].type != CARD_ACTION_END_TURN && pick-- == 0)
            {
                *chosen = actions[i];
                return;
            }
        }
    }
    if (CountActions(actions, count, CARD_ACTION_ATTACK) != 0)
        *chosen = *PickAttack(duel, side, actions, count);
    else
        *chosen = *PickOfType(duel, actions, count, CARD_ACTION_END_TURN);
}
