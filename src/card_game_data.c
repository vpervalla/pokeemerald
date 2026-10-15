#include "global.h"
#include "card_game.h"
#include "constants/species.h"

// The card game's cards. Phase 2 of docs/card_game_plan.md replaces these hand-written first
// cards (after their Pokémon TCG Pocket versions) with the generated card pool.

static const u8 sText_VineWhip[] = _("VINE WHIP");
static const u8 sText_RazorLeaf[] = _("RAZOR LEAF");
static const u8 sText_MegaDrain[] = _("MEGA DRAIN");
static const u8 sText_Ember[] = _("EMBER");
static const u8 sText_FireClaws[] = _("FIRE CLAWS");
static const u8 sText_FireSpin[] = _("FIRE SPIN");
static const u8 sText_WaterGun[] = _("WATER GUN");
static const u8 sText_WaveSplash[] = _("WAVE SPLASH");
static const u8 sText_HydroPump[] = _("HYDRO PUMP");
static const u8 sText_Gnaw[] = _("GNAW");
static const u8 sText_Thunderbolt[] = _("THUNDERBOLT");

static const u8 sText_Potion[] = _("POTION");
static const u8 sText_PokeBall[] = _("POKé BALL");
static const u8 sText_XSpeed[] = _("X SPEED");
static const u8 sText_ProfOaksResearch[] = _("PROF. OAK'S RESEARCH");
static const u8 sText_GiantCape[] = _("GIANT CAPE");

static const struct PokemonCardInfo sPokemonCards[CARD_POKEMON_END] =
{
    [CARD_BULBASAUR] =
    {
        .species = SPECIES_BULBASAUR,
        .evolvesFrom = CARD_NONE,
        .hp = 70,
        .type = CARD_TYPE_GRASS,
        .weakness = CARD_TYPE_FIRE,
        .stage = CARD_STAGE_BASIC,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_VineWhip, {CARD_TYPE_GRASS, CARD_TYPE_COLORLESS}, 40, CARD_EFFECT_NONE, 0}},
    },
    [CARD_IVYSAUR] =
    {
        .species = SPECIES_IVYSAUR,
        .evolvesFrom = CARD_BULBASAUR,
        .hp = 90,
        .type = CARD_TYPE_GRASS,
        .weakness = CARD_TYPE_FIRE,
        .stage = CARD_STAGE_1,
        .retreatCost = 2,
        .numAttacks = 1,
        .attacks = {{sText_RazorLeaf, {CARD_TYPE_GRASS, CARD_TYPE_COLORLESS, CARD_TYPE_COLORLESS}, 60, CARD_EFFECT_NONE, 0}},
    },
    [CARD_VENUSAUR] =
    {
        .species = SPECIES_VENUSAUR,
        .evolvesFrom = CARD_IVYSAUR,
        .hp = 160,
        .type = CARD_TYPE_GRASS,
        .weakness = CARD_TYPE_FIRE,
        .stage = CARD_STAGE_2,
        .retreatCost = 3,
        .numAttacks = 1,
        .attacks = {{sText_MegaDrain, {CARD_TYPE_GRASS, CARD_TYPE_GRASS, CARD_TYPE_COLORLESS, CARD_TYPE_COLORLESS}, 80, CARD_EFFECT_HEAL_SELF, 30}},
    },
    [CARD_CHARMANDER] =
    {
        .species = SPECIES_CHARMANDER,
        .evolvesFrom = CARD_NONE,
        .hp = 60,
        .type = CARD_TYPE_FIRE,
        .weakness = CARD_TYPE_WATER,
        .stage = CARD_STAGE_BASIC,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_Ember, {CARD_TYPE_FIRE}, 30, CARD_EFFECT_DISCARD_ENERGY, 1}},
    },
    [CARD_CHARMELEON] =
    {
        .species = SPECIES_CHARMELEON,
        .evolvesFrom = CARD_CHARMANDER,
        .hp = 90,
        .type = CARD_TYPE_FIRE,
        .weakness = CARD_TYPE_WATER,
        .stage = CARD_STAGE_1,
        .retreatCost = 2,
        .numAttacks = 1,
        .attacks = {{sText_FireClaws, {CARD_TYPE_FIRE, CARD_TYPE_COLORLESS, CARD_TYPE_COLORLESS}, 60, CARD_EFFECT_NONE, 0}},
    },
    [CARD_CHARIZARD] =
    {
        .species = SPECIES_CHARIZARD,
        .evolvesFrom = CARD_CHARMELEON,
        .hp = 150,
        .type = CARD_TYPE_FIRE,
        .weakness = CARD_TYPE_WATER,
        .stage = CARD_STAGE_2,
        .retreatCost = 2,
        .numAttacks = 1,
        .attacks = {{sText_FireSpin, {CARD_TYPE_FIRE, CARD_TYPE_FIRE, CARD_TYPE_COLORLESS, CARD_TYPE_COLORLESS}, 150, CARD_EFFECT_DISCARD_ENERGY, 2}},
    },
    [CARD_SQUIRTLE] =
    {
        .species = SPECIES_SQUIRTLE,
        .evolvesFrom = CARD_NONE,
        .hp = 60,
        .type = CARD_TYPE_WATER,
        .weakness = CARD_TYPE_LIGHTNING,
        .stage = CARD_STAGE_BASIC,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_WaterGun, {CARD_TYPE_WATER}, 20, CARD_EFFECT_NONE, 0}},
    },
    [CARD_WARTORTLE] =
    {
        .species = SPECIES_WARTORTLE,
        .evolvesFrom = CARD_SQUIRTLE,
        .hp = 80,
        .type = CARD_TYPE_WATER,
        .weakness = CARD_TYPE_LIGHTNING,
        .stage = CARD_STAGE_1,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_WaveSplash, {CARD_TYPE_WATER, CARD_TYPE_COLORLESS}, 40, CARD_EFFECT_NONE, 0}},
    },
    [CARD_BLASTOISE] =
    {
        .species = SPECIES_BLASTOISE,
        .evolvesFrom = CARD_WARTORTLE,
        .hp = 150,
        .type = CARD_TYPE_WATER,
        .weakness = CARD_TYPE_LIGHTNING,
        .stage = CARD_STAGE_2,
        .retreatCost = 3,
        .numAttacks = 1,
        .attacks = {{sText_HydroPump, {CARD_TYPE_WATER, CARD_TYPE_WATER, CARD_TYPE_COLORLESS}, 80, CARD_EFFECT_NONE, 0}},
    },
    [CARD_PIKACHU] =
    {
        .species = SPECIES_PIKACHU,
        .evolvesFrom = CARD_NONE,
        .hp = 60,
        .type = CARD_TYPE_LIGHTNING,
        .weakness = CARD_TYPE_FIGHTING,
        .stage = CARD_STAGE_BASIC,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_Gnaw, {CARD_TYPE_LIGHTNING}, 20, CARD_EFFECT_NONE, 0}},
    },
    [CARD_RAICHU] =
    {
        .species = SPECIES_RAICHU,
        .evolvesFrom = CARD_PIKACHU,
        .hp = 100,
        .type = CARD_TYPE_LIGHTNING,
        .weakness = CARD_TYPE_FIGHTING,
        .stage = CARD_STAGE_1,
        .retreatCost = 1,
        .numAttacks = 1,
        .attacks = {{sText_Thunderbolt, {CARD_TYPE_LIGHTNING, CARD_TYPE_LIGHTNING, CARD_TYPE_LIGHTNING}, 140, CARD_EFFECT_DISCARD_ALL_ENERGY, 0}},
    },
};

static const struct TrainerCardInfo sTrainerCards[CARD_COUNT - CARD_POKEMON_END] =
{
    [CARD_POTION - CARD_POKEMON_END] = {sText_Potion, CARD_KIND_ITEM, CARD_TRAINER_HEAL, 20},
    [CARD_POKE_BALL - CARD_POKEMON_END] = {sText_PokeBall, CARD_KIND_ITEM, CARD_TRAINER_POKE_BALL, 0},
    [CARD_X_SPEED - CARD_POKEMON_END] = {sText_XSpeed, CARD_KIND_ITEM, CARD_TRAINER_RETREAT_DISCOUNT, 1},
    [CARD_PROF_OAKS_RESEARCH - CARD_POKEMON_END] = {sText_ProfOaksResearch, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW, 2},
    [CARD_GIANT_CAPE - CARD_POKEMON_END] = {sText_GiantCape, CARD_KIND_TOOL, CARD_TRAINER_TOOL_HP, 20},
};

u8 CardGame_GetCardKind(u16 cardId)
{
    if (cardId > CARD_NONE && cardId < CARD_POKEMON_END)
        return CARD_KIND_POKEMON;
    if (cardId >= CARD_POKEMON_END && cardId < CARD_COUNT)
        return sTrainerCards[cardId - CARD_POKEMON_END].kind;
    return CARD_KIND_NONE;
}

const struct PokemonCardInfo *CardGame_GetPokemonInfo(u16 cardId)
{
    if (cardId > CARD_NONE && cardId < CARD_POKEMON_END)
        return &sPokemonCards[cardId];
    return NULL;
}

const struct TrainerCardInfo *CardGame_GetTrainerInfo(u16 cardId)
{
    if (cardId >= CARD_POKEMON_END && cardId < CARD_COUNT)
        return &sTrainerCards[cardId - CARD_POKEMON_END];
    return NULL;
}
