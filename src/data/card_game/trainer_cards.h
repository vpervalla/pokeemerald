// The card game's Trainer cards (ids in include/constants/card_game.h), after Pokémon TCG
// Pocket's. Supporters limited to some Pokémon in Pocket work here for a type.

static const u8 sCardText_Potion[] = _("POTION");
static const u8 sCardText_PokeBall[] = _("POKé BALL");
static const u8 sCardText_XSpeed[] = _("X SPEED");
static const u8 sCardText_HandScope[] = _("HAND SCOPE");
static const u8 sCardText_RedCard[] = _("RED CARD");
static const u8 sCardText_RareCandy[] = _("RARE CANDY");
static const u8 sCardText_GiantCape[] = _("GIANT CAPE");
static const u8 sCardText_RockyHelmet[] = _("ROCKY HELMET");
static const u8 sCardText_ProfOaksResearch[] = _("PROF. OAK'S RESEARCH");
static const u8 sCardText_Brock[] = _("BROCK");
static const u8 sCardText_Misty[] = _("MISTY");
static const u8 sCardText_LtSurge[] = _("LT. SURGE");
static const u8 sCardText_Erika[] = _("ERIKA");
static const u8 sCardText_Sabrina[] = _("SABRINA");
static const u8 sCardText_Blaine[] = _("BLAINE");
static const u8 sCardText_Giovanni[] = _("GIOVANNI");

#define TRAINER(id) [id - CARD_POKEMON_END]

static const struct TrainerCardInfo sTrainerCards[CARD_COUNT - CARD_POKEMON_END] =
{
    TRAINER(CARD_POTION)             = {sCardText_Potion,           CARD_KIND_ITEM,      CARD_TRAINER_HEAL,              20, CARD_TYPE_NONE},
    TRAINER(CARD_POKE_BALL)          = {sCardText_PokeBall,         CARD_KIND_ITEM,      CARD_TRAINER_POKE_BALL,          0, CARD_TYPE_NONE},
    TRAINER(CARD_X_SPEED)            = {sCardText_XSpeed,           CARD_KIND_ITEM,      CARD_TRAINER_RETREAT_DISCOUNT,   1, CARD_TYPE_NONE},
    TRAINER(CARD_HAND_SCOPE)         = {sCardText_HandScope,        CARD_KIND_ITEM,      CARD_TRAINER_HAND_SCOPE,         0, CARD_TYPE_NONE},
    TRAINER(CARD_RED_CARD)           = {sCardText_RedCard,          CARD_KIND_ITEM,      CARD_TRAINER_RED_CARD,           3, CARD_TYPE_NONE},
    TRAINER(CARD_RARE_CANDY)         = {sCardText_RareCandy,        CARD_KIND_ITEM,      CARD_TRAINER_RARE_CANDY,         0, CARD_TYPE_NONE},
    TRAINER(CARD_GIANT_CAPE)         = {sCardText_GiantCape,        CARD_KIND_TOOL,      CARD_TRAINER_TOOL_HP,           20, CARD_TYPE_NONE},
    TRAINER(CARD_ROCKY_HELMET)       = {sCardText_RockyHelmet,      CARD_KIND_TOOL,      CARD_TRAINER_TOOL_COUNTER,      20, CARD_TYPE_NONE},
    TRAINER(CARD_PROF_OAKS_RESEARCH) = {sCardText_ProfOaksResearch, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW,               2, CARD_TYPE_NONE},
    TRAINER(CARD_BROCK)              = {sCardText_Brock,            CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACH_ENERGY,      0, CARD_TYPE_FIGHTING},
    TRAINER(CARD_MISTY)              = {sCardText_Misty,            CARD_KIND_SUPPORTER, CARD_TRAINER_FLIP_ATTACH_ENERGY, 0, CARD_TYPE_WATER},
    TRAINER(CARD_LT_SURGE)           = {sCardText_LtSurge,          CARD_KIND_SUPPORTER, CARD_TRAINER_GATHER_ENERGY,      0, CARD_TYPE_LIGHTNING},
    TRAINER(CARD_ERIKA)              = {sCardText_Erika,            CARD_KIND_SUPPORTER, CARD_TRAINER_HEAL_TYPE,         50, CARD_TYPE_GRASS},
    TRAINER(CARD_SABRINA)            = {sCardText_Sabrina,          CARD_KIND_SUPPORTER, CARD_TRAINER_SWITCH_OPPONENT,    0, CARD_TYPE_NONE},
    TRAINER(CARD_BLAINE)             = {sCardText_Blaine,           CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      30, CARD_TYPE_FIRE},
    TRAINER(CARD_GIOVANNI)           = {sCardText_Giovanni,         CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      10, CARD_TYPE_NONE},
};

#undef TRAINER
