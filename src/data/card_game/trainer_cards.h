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
static const u8 sCardText_Koga[] = _("KOGA");
static const u8 sCardText_Bill[] = _("BILL");
static const u8 sCardText_Blue[] = _("BLUE");
static const u8 sCardText_Lorelei[] = _("LORELEI");
static const u8 sCardText_Bruno[] = _("BRUNO");
static const u8 sCardText_Agatha[] = _("AGATHA");
static const u8 sCardText_Lance[] = _("LANCE");
static const u8 sCardText_Daisy[] = _("DAISY");
static const u8 sCardText_Celio[] = _("CELIO");
static const u8 sCardText_FullHeal[] = _("FULL HEAL");
static const u8 sCardText_Leftovers[] = _("LEFTOVERS");
static const u8 sCardText_ChoiceBand[] = _("CHOICE BAND");

#define TRAINER(id) [id - CARD_TRAINER_START]

static const struct TrainerCardInfo sTrainerCards[CARD_COUNT - CARD_TRAINER_START] =
{
    TRAINER(CARD_POTION)             = {sCardText_Potion,           CARD_KIND_ITEM,      CARD_TRAINER_HEAL,              20, CARD_TYPE_NONE, ITEM_POTION},
    TRAINER(CARD_POKE_BALL)          = {sCardText_PokeBall,         CARD_KIND_ITEM,      CARD_TRAINER_POKE_BALL,          0, CARD_TYPE_NONE, ITEM_POKE_BALL},
    TRAINER(CARD_X_SPEED)            = {sCardText_XSpeed,           CARD_KIND_ITEM,      CARD_TRAINER_RETREAT_DISCOUNT,   1, CARD_TYPE_NONE, ITEM_X_SPEED},
    TRAINER(CARD_HAND_SCOPE)         = {sCardText_HandScope,        CARD_KIND_ITEM,      CARD_TRAINER_HAND_SCOPE,         0, CARD_TYPE_NONE, ITEM_SCOPE_LENS},
    TRAINER(CARD_RED_CARD)           = {sCardText_RedCard,          CARD_KIND_ITEM,      CARD_TRAINER_RED_CARD,           3, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_RARE_CANDY)         = {sCardText_RareCandy,        CARD_KIND_ITEM,      CARD_TRAINER_RARE_CANDY,         0, CARD_TYPE_NONE, ITEM_RARE_CANDY},
    TRAINER(CARD_GIANT_CAPE)         = {sCardText_GiantCape,        CARD_KIND_TOOL,      CARD_TRAINER_TOOL_HP,           20, CARD_TYPE_NONE, ITEM_SILK_SCARF},
    TRAINER(CARD_ROCKY_HELMET)       = {sCardText_RockyHelmet,      CARD_KIND_TOOL,      CARD_TRAINER_TOOL_COUNTER,      20, CARD_TYPE_NONE, ITEM_HARD_STONE},
    TRAINER(CARD_PROF_OAKS_RESEARCH) = {sCardText_ProfOaksResearch, CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW,               2, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_BROCK)              = {sCardText_Brock,            CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACH_ENERGY,      0, CARD_TYPE_FIGHTING, ITEM_NONE},
    TRAINER(CARD_MISTY)              = {sCardText_Misty,            CARD_KIND_SUPPORTER, CARD_TRAINER_FLIP_ATTACH_ENERGY, 0, CARD_TYPE_WATER, ITEM_NONE},
    TRAINER(CARD_LT_SURGE)           = {sCardText_LtSurge,          CARD_KIND_SUPPORTER, CARD_TRAINER_GATHER_ENERGY,      0, CARD_TYPE_LIGHTNING, ITEM_NONE},
    TRAINER(CARD_ERIKA)              = {sCardText_Erika,            CARD_KIND_SUPPORTER, CARD_TRAINER_HEAL_TYPE,         50, CARD_TYPE_GRASS, ITEM_NONE},
    TRAINER(CARD_SABRINA)            = {sCardText_Sabrina,          CARD_KIND_SUPPORTER, CARD_TRAINER_SWITCH_OPPONENT,    0, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_BLAINE)             = {sCardText_Blaine,           CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      30, CARD_TYPE_FIRE, ITEM_NONE},
    TRAINER(CARD_GIOVANNI)           = {sCardText_Giovanni,         CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      10, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_KOGA)               = {sCardText_Koga,             CARD_KIND_SUPPORTER, CARD_TRAINER_STATUS_OPPONENT,    CARD_STATUS_POISONED, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_BILL)               = {sCardText_Bill,             CARD_KIND_SUPPORTER, CARD_TRAINER_SEARCH_POKEMON,     0, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_BLUE)               = {sCardText_Blue,             CARD_KIND_SUPPORTER, CARD_TRAINER_SHIELD,            10, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_LORELEI)            = {sCardText_Lorelei,          CARD_KIND_SUPPORTER, CARD_TRAINER_STATUS_OPPONENT,    CARD_STATUS_ASLEEP, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_BRUNO)              = {sCardText_Bruno,            CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      20, CARD_TYPE_FIGHTING, ITEM_NONE},
    TRAINER(CARD_AGATHA)             = {sCardText_Agatha,           CARD_KIND_SUPPORTER, CARD_TRAINER_STATUS_OPPONENT,    CARD_STATUS_CONFUSED, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_LANCE)              = {sCardText_Lance,            CARD_KIND_SUPPORTER, CARD_TRAINER_ATTACK_BONUS,      30, CARD_TYPE_DRAGON, ITEM_NONE},
    TRAINER(CARD_DAISY)              = {sCardText_Daisy,            CARD_KIND_SUPPORTER, CARD_TRAINER_HEAL_ALL,          20, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_CELIO)              = {sCardText_Celio,            CARD_KIND_SUPPORTER, CARD_TRAINER_DRAW_UNTIL,         5, CARD_TYPE_NONE, ITEM_NONE},
    TRAINER(CARD_FULL_HEAL)          = {sCardText_FullHeal,         CARD_KIND_ITEM,      CARD_TRAINER_CURE,               0, CARD_TYPE_NONE, ITEM_FULL_HEAL},
    TRAINER(CARD_LEFTOVERS)          = {sCardText_Leftovers,        CARD_KIND_TOOL,      CARD_TRAINER_TOOL_HEAL,         10, CARD_TYPE_NONE, ITEM_LEFTOVERS},
    TRAINER(CARD_CHOICE_BAND)        = {sCardText_ChoiceBand,       CARD_KIND_TOOL,      CARD_TRAINER_TOOL_DAMAGE,       10, CARD_TYPE_NONE, ITEM_CHOICE_BAND},
};

#undef TRAINER
