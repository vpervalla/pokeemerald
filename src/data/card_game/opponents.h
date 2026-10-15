// The card players in the world (docs/card_game_plan.md, section 5). Each map script runs
// `cardduel CARD_OPPONENT_*` after the NPC's usual line; the lines below do the rest. Each
// fits the message box: 2 lines, no \p (tools/card_game/test_data.c checks it).

static const u8 sName_Gus[] = _("GUS");
static const u8 sChallenge_Gus[] = _(
    "You have a CARD CASE too?\n"
    "How about a friendly duel?");
static const u8 sWon_Gus[] = _(
    "You play like PROF. OAK's own\n"
    "grandson!");
static const u8 sLost_Gus[] = _(
    "Ho ho! Small towns have\n"
    "big card players!");

static const u8 sName_Tommy[] = _("TOMMY");
static const u8 sChallenge_Tommy[] = _(
    "My NIDORAN cards are the best!\n"
    "Want to see? Let's duel!");
static const u8 sWon_Tommy[] = _(
    "Aww… My NIDORAN lost…");
static const u8 sLost_Tommy[] = _(
    "Told you!\n"
    "NIDORAN are the best!");

static const u8 sName_Rick[] = _("RICK");
static const u8 sChallenge_Rick[] = _(
    "Bugs make the best cards!\n"
    "Want me to prove it?");
static const u8 sWon_Rick[] = _(
    "My bugs got squashed…");
static const u8 sLost_Rick[] = _(
    "See? Bugs win!");

static const u8 sName_Dane[] = _("DANE");
static const u8 sChallenge_Dane[] = _(
    "I collect fossil cards now!\n"
    "Want to duel my ancient deck?");
static const u8 sWon_Dane[] = _(
    "Wow, you beat the ancients!");
static const u8 sLost_Dane[] = _(
    "Ancient power is the real\n"
    "power!");

static const u8 sName_Wes[] = _("WES");
static const u8 sChallenge_Wes[] = _(
    "I can read your mind…\n"
    "You want a card duel, right?");
static const u8 sWon_Wes[] = _(
    "I didn't see that coming…");
static const u8 sLost_Wes[] = _(
    "I knew every card you held.");

static const u8 sName_Ansel[] = _("ANSEL");
static const u8 sChallenge_Ansel[] = _(
    "HIKERS play cards here, too.\n"
    "Care for a duel, young one?");
static const u8 sWon_Ansel[] = _(
    "Solid as a rock, your play!");
static const u8 sLost_Ansel[] = _(
    "Rocks don't crumble so easily.");

static const u8 sName_Coral[] = _("CORAL");
static const u8 sChallenge_Coral[] = _(
    "MISTY's my idol, so I play\n"
    "WATER cards! Shall we duel?");
static const u8 sWon_Coral[] = _(
    "I got washed away!");
static const u8 sLost_Coral[] = _(
    "Splash! I won!");

static const u8 sName_Zack[] = _("ZACK");
static const u8 sChallenge_Zack[] = _(
    "My deck's charged up!\n"
    "Want to get zapped?");
static const u8 sWon_Zack[] = _(
    "I'm all out of juice…");
static const u8 sLost_Zack[] = _(
    "Zap! Too shocking for you?");

static const u8 sName_Duncan[] = _("DUNCAN");
static const u8 sChallenge_Duncan[] = _(
    "Sailors play cards on long\n"
    "trips. Want a round, kid?");
static const u8 sWon_Duncan[] = _(
    "Shiver me timbers!\n"
    "You beat me fair and square.");
static const u8 sLost_Duncan[] = _(
    "Ha! You've got a lot of sea\n"
    "to cross yet!");

static const u8 sName_Mort[] = _("MORT");
static const u8 sChallenge_Mort[] = _(
    "My deck has my favorite\n"
    "POKéMON. Want to see them?");
static const u8 sWon_Mort[] = _(
    "You're a fan of cards, I see!");
static const u8 sLost_Mort[] = _(
    "Aren't my favorites the cutest?");

static const u8 sName_Ivan[] = _("IVAN");
static const u8 sChallenge_Ivan[] = _(
    "My cards come from the TOWER…\n"
    "Dare to duel them?");
static const u8 sWon_Ivan[] = _(
    "The ghosts have left me…");
static const u8 sLost_Ivan[] = _(
    "Boo! Scared you, didn't I?");

static const u8 sName_Lou[] = _("LOU");
static const u8 sChallenge_Lou[] = _(
    "My POLIWRATH and I play cards\n"
    "too. Will you duel us?");
static const u8 sWon_Lou[] = _(
    "Even my POLIWRATH is impressed!");
static const u8 sLost_Lou[] = _(
    "POLIWRATH and I make a great\n"
    "team!");

static const u8 sName_Flora[] = _("FLORA");
static const u8 sChallenge_Flora[] = _(
    "Cards are nicer than slots.\n"
    "Will you play with me?");
static const u8 sWon_Flora[] = _(
    "Your deck has bloomed!");
static const u8 sLost_Flora[] = _(
    "Patience makes a garden grow.");

static const u8 sName_Bert[] = _("BERT");
static const u8 sChallenge_Bert[] = _(
    "Psst… Who runs this place?\n"
    "Beat my deck and I'll tell you.");
static const u8 sWon_Bert[] = _(
    "It's TEAM ROCKET, kid…\n"
    "You didn't hear it from me.");
static const u8 sLost_Bert[] = _(
    "Some cards are better left\n"
    "unturned, kid.");

static const u8 sName_Vera[] = _("VERA");
static const u8 sChallenge_Vera[] = _(
    "The slots are no fun today.\n"
    "Want to play cards instead?");
static const u8 sWon_Vera[] = _(
    "You hit the jackpot!");
static const u8 sLost_Vera[] = _(
    "Luck had nothing to do with it!");

static const u8 sName_Otto[] = _("OTTO");
static const u8 sChallenge_Otto[] = _(
    "I played cards with BLAINE once.\n"
    "Want to try my fire deck?");
static const u8 sWon_Otto[] = _(
    "You put out my fire!");
static const u8 sLost_Otto[] = _(
    "Ha! The old flame still burns!");

static const u8 sName_Brock[] = _("BROCK");
static const u8 sChallenge_Brock[] = _(
    "My card deck is as hard as\n"
    "my rocks. Will you take it on?");
static const u8 sWon_Brock[] = _(
    "You're as good with cards as\n"
    "with POKéMON!");
static const u8 sLost_Brock[] = _(
    "A rock-hard defense always wins.");

static const u8 sName_Misty[] = _("MISTY");
static const u8 sChallenge_Misty[] = _(
    "A card duel? I'm the best at\n"
    "those too! Ready to lose?");
static const u8 sWon_Misty[] = _(
    "Again?! I can't believe it!");
static const u8 sLost_Misty[] = _(
    "Ha! My WATER cards are\n"
    "unbeatable!");

static const u8 sName_LtSurge[] = _("LT. SURGE");
static const u8 sChallenge_LtSurge[] = _(
    "Hey, kid! Cards kept us sharp\n"
    "in the war! Want a duel?");
static const u8 sWon_LtSurge[] = _(
    "Whoa! You're the real deal!");
static const u8 sLost_LtSurge[] = _(
    "Ha! You got zapped!");

static const u8 sName_Erika[] = _("ERIKA");
static const u8 sChallenge_Erika[] = _(
    "Oh, you play cards? How lovely.\n"
    "Would you like a duel?");
static const u8 sWon_Erika[] = _(
    "What a graceful duel.");
static const u8 sLost_Erika[] = _(
    "Flowers bloom for those who\n"
    "wait.");

#define OPPONENT(id, n, deckId, reward, where) [CARD_OPPONENT_##id] = \
    {sName_##n, sChallenge_##n, sWon_##n, sLost_##n, reward, CARD_DECK_##deckId, MAPSEC_##where}

const struct CardOpponent gCardOpponents[CARD_OPPONENTS_COUNT] =
{
    OPPONENT(PALLET_FAT_MAN,          Gus,     NEIGHBOR,    CARD_X_SPEED,                PALLET_TOWN),
    OPPONENT(VIRIDIAN_BOY,            Tommy,   NIDO_KID,    CARD_NIDOKING,               VIRIDIAN_CITY),
    OPPONENT(PEWTER_BUG_CATCHER,      Rick,    BUG_CATCHER, CARD_SCYTHER,                PEWTER_CITY),
    OPPONENT(PEWTER_PC_YOUNGSTER,     Dane,    FOSSIL_FAN,  CARD_AERODACTYL,             PEWTER_CITY),
    OPPONENT(ROUTE2_ROCKER,           Wes,     PSYCHIC,     CARD_MR_MIME,                ROUTE_2),
    OPPONENT(ROUTE4_PC_GENTLEMAN,     Ansel,   HIKER,       CARD_GIANT_CAPE,             ROUTE_4),
    OPPONENT(CERULEAN_PC_LASS,        Coral,   SWIMMER,     CARD_LAPRAS,                 CERULEAN_CITY),
    OPPONENT(CERULEAN_PC_ROCKER,      Zack,    ELECTRIC,    CARD_ELECTABUZZ,             CERULEAN_CITY),
    OPPONENT(VERMILION_SAILOR,        Duncan,  SAILOR,      CARD_RED_CARD,               VERMILION_CITY),
    OPPONENT(FAN_CLUB_FAT_MAN,        Mort,    FAN_CLUB,    CARD_CHANSEY,                VERMILION_CITY),
    OPPONENT(LAVENDER_BOY,            Ivan,    GHOST,       CARD_HAND_SCOPE,             LAVENDER_TOWN),
    OPPONENT(CELADON_FAT_MAN,         Lou,     POLIWAG,     CARD_POLITOED,               CELADON_CITY),
    OPPONENT(CELADON_WOMAN,           Flora,   GARDEN,      CARD_RARE_CANDY,             CELADON_CITY),
    OPPONENT(GAME_CORNER_BALDING_MAN, Bert,    ROCKET,      CARD_GIOVANNI,               CELADON_CITY),
    OPPONENT(GAME_CORNER_WOMAN,       Vera,    TECH,        CARD_PORYGON,                CELADON_CITY),
    OPPONENT(GAME_CORNER_OLD_MAN,     Otto,    FIRE,        CARD_CHARIZARD_MEGA_Y_EX,    CELADON_CITY),
    OPPONENT(BROCK,                   Brock,   BROCK,       CARD_BROCK,                  PEWTER_CITY),
    OPPONENT(MISTY,                   Misty,   MISTY,       CARD_MISTY,                  CERULEAN_CITY),
    OPPONENT(LT_SURGE,                LtSurge, LT_SURGE,    CARD_LT_SURGE,               VERMILION_CITY),
    OPPONENT(ERIKA,                   Erika,   ERIKA,       CARD_ERIKA,                  CELADON_CITY),
};

#undef OPPONENT
