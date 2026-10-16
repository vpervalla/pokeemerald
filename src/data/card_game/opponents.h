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

static const u8 sName_Nigel[] = _("NIGEL");
static const u8 sChallenge_Nigel[] = _(
    "Silph's fame brought me psychic\n"
    "cards. Care to test my mind?");
static const u8 sWon_Nigel[] = _(
    "Your mind is sharper than mine.");
static const u8 sLost_Nigel[] = _(
    "My psychics saw it all coming.");

static const u8 sName_Koichi[] = _("KOICHI");
static const u8 sChallenge_Koichi[] = _(
    "Hwaa! Fight me with cards too,\n"
    "challenger!");
static const u8 sWon_Koichi[] = _(
    "Hwaa! Even with cards, you win!");
static const u8 sLost_Koichi[] = _(
    "Your deck lacks fighting spirit!");

static const u8 sName_Copycat[] = _("COPYCAT");
static const u8 sChallenge_Copycat[] = _(
    "Cards? I'll copy your deck and\n"
    "play it! Mimic, mimic!");
static const u8 sWon_Copycat[] = _(
    "My copy lost to the original!");
static const u8 sLost_Copycat[] = _(
    "Mimic, mimic! I play you better!");

static const u8 sName_Gramps[] = _("GRAMPS");
static const u8 sChallenge_Gramps[] = _(
    "BILL taught me cards. He loves\n"
    "EEVEE, you know. Shall we?");
static const u8 sWon_Gramps[] = _(
    "BILL would be proud of you.");
static const u8 sLost_Gramps[] = _(
    "Ho ho! Grandpa still has it!");

static const u8 sName_Paige[] = _("PAIGE");
static const u8 sChallenge_Paige[] = _(
    "I play the POKéMON of the SAFARI\n"
    "ZONE. Are you game?");
static const u8 sWon_Paige[] = _(
    "You'd be a great SAFARI hunter!");
static const u8 sLost_Paige[] = _(
    "The wild side wins again!");

static const u8 sName_Billy[] = _("BILLY");
static const u8 sChallenge_Billy[] = _(
    "Bikes and cards, that's my life.\n"
    "Race you to 3 trophies!");
static const u8 sWon_Billy[] = _(
    "Whoa, faster than my bike!");
static const u8 sLost_Billy[] = _(
    "Eat my dust, kid!");

static const u8 sName_Gideon[] = _("GIDEON");
static const u8 sChallenge_Gideon[] = _(
    "We revive fossils here. Want to\n"
    "see them in card form?");
static const u8 sWon_Gideon[] = _(
    "Remarkable! A new specimen of\n"
    "skill!");
static const u8 sLost_Gideon[] = _(
    "Science wins, as always.");

static const u8 sName_Ned[] = _("NED");
static const u8 sChallenge_Ned[] = _(
    "I came from the SEAFOAM ISLANDS.\n"
    "Cold cards! Want a duel?");
static const u8 sWon_Ned[] = _(
    "Brr… You froze me out!");
static const u8 sLost_Ned[] = _(
    "Chilly, huh? Ice cards hit hard!");

static const u8 sName_Takeo[] = _("TAKEO");
static const u8 sChallenge_Takeo[] = _(
    "AGATHA beat me with ghosts. So I\n"
    "play them now. Duel me!");
static const u8 sWon_Takeo[] = _(
    "Even AGATHA would admire that.");
static const u8 sLost_Takeo[] = _(
    "The ghosts are on my side now.");

static const u8 sName_Koga[] = _("KOGA");
static const u8 sChallenge_Koga[] = _(
    "Fwahaha! A ninja's cards are\n"
    "full of tricks. Duel me!");
static const u8 sWon_Koga[] = _(
    "Hmm… Your skill is no illusion.");
static const u8 sLost_Koga[] = _(
    "Fwahaha! Poison wins slowly!");

static const u8 sName_Sabrina[] = _("SABRINA");
static const u8 sChallenge_Sabrina[] = _(
    "I foresaw this. A card duel…\n"
    "Shall we begin?");
static const u8 sWon_Sabrina[] = _(
    "I didn't foresee this outcome…");
static const u8 sLost_Sabrina[] = _(
    "As I foresaw. You lose.");

static const u8 sName_Blaine[] = _("BLAINE");
static const u8 sChallenge_Blaine[] = _(
    "Hah! Cards burn hot too!\n"
    "Want to get scorched?");
static const u8 sWon_Blaine[] = _(
    "I've burned out!");
static const u8 sLost_Blaine[] = _(
    "Hah! You need a burn heal!");

static const u8 sName_Giovanni[] = _("GIOVANNI");
static const u8 sChallenge_Giovanni[] = _(
    "Cards… A game of strategy.\n"
    "Show me what you're made of.");
static const u8 sWon_Giovanni[] = _(
    "Hmph. You are truly strong.");
static const u8 sLost_Giovanni[] = _(
    "Power always wins, child.");

static const u8 sName_Rocko[] = _("ROCKO");
static const u8 sChallenge_Rocko[] = _(
    "I climb MT. EMBER with my fire\n"
    "cards! Want a duel?");
static const u8 sWon_Rocko[] = _(
    "You cooled my MT. EMBER!");
static const u8 sLost_Rocko[] = _(
    "My deck erupts like a volcano!");

static const u8 sName_Hanako[] = _("HANAKO");
static const u8 sChallenge_Hanako[] = _(
    "The spa keeps me young! Play a\n"
    "gentle card duel with me?");
static const u8 sWon_Hanako[] = _(
    "My, you are good, dearie.");
static const u8 sLost_Hanako[] = _(
    "Healing always wins, dearie.");

static const u8 sName_Gordon[] = _("GORDON");
static const u8 sChallenge_Gordon[] = _(
    "Until the games are running,\n"
    "how about a card duel?");
static const u8 sWon_Gordon[] = _(
    "You'd clean out any Game Corner!");
static const u8 sLost_Gordon[] = _(
    "Jackpot! I win!");

static const u8 sName_Lostelle[] = _("LOSTELLE");
static const u8 sChallenge_Lostelle[] = _(
    "Thanks for saving me! Do you\n"
    "play cards? Play with me!");
static const u8 sWon_Lostelle[] = _(
    "You're so cool!");
static const u8 sLost_Lostelle[] = _(
    "Yay! I won! I won!");

static const u8 sName_Marlon[] = _("MARLON");
static const u8 sChallenge_Marlon[] = _(
    "Deep sea cards for a sailor!\n"
    "Care for a round, matey?");
static const u8 sWon_Marlon[] = _(
    "You sank my BLASTOISE!");
static const u8 sLost_Marlon[] = _(
    "Ha! Back to the shallows, matey!");

static const u8 sName_Lorelei[] = _("LORELEI");
static const u8 sChallenge_Lorelei[] = _(
    "I play cards too, when I'm home.\n"
    "Will you face my ice deck?");
static const u8 sWon_Lorelei[] = _(
    "You've melted my heart again.");
static const u8 sLost_Lorelei[] = _(
    "Your deck froze, didn't it?");

static const u8 sName_Waldo[] = _("WALDO");
static const u8 sChallenge_Waldo[] = _(
    "I collect cards of baby POKéMON.\n"
    "Aren't they precious? A duel?");
static const u8 sWon_Waldo[] = _(
    "Babies grow up so fast!");
static const u8 sLost_Waldo[] = _(
    "Never underestimate the little\n"
    "ones!");

static const u8 sName_Dahlia[] = _("DAHLIA");
static const u8 sChallenge_Dahlia[] = _(
    "My deck blooms like the MEADOW.\n"
    "Shall we duel?");
static const u8 sWon_Dahlia[] = _(
    "You've plucked every last petal.");
static const u8 sLost_Dahlia[] = _(
    "In full bloom! I win!");

static const u8 sName_Eldon[] = _("ELDON");
static const u8 sChallenge_Eldon[] = _(
    "The ruins hold old secrets. So\n"
    "does my deck. Try it?");
static const u8 sWon_Eldon[] = _(
    "You've solved my deck's riddle.");
static const u8 sLost_Eldon[] = _(
    "The ruins guard their secrets.");

static const u8 sName_Rodette[] = _("RODETTE");
static const u8 sChallenge_Rodette[] = _(
    "I sense the sky's power in my\n"
    "cards. Will you face it?");
static const u8 sWon_Rodette[] = _(
    "The sky has fallen…");
static const u8 sLost_Rodette[] = _(
    "The sky dragon has spoken.");

static const u8 sName_Blue[] = _("{RIVAL}");
static const u8 sChallenge_Blue[] = _(
    "Cards, huh? Fine! I'll beat you\n"
    "at this too!");
static const u8 sWon_Blue[] = _(
    "What?! Not again!");
static const u8 sLost_Blue[] = _(
    "Ha! Smell ya later, loser!");

static const u8 sName_Leon[] = _("LEON");
static const u8 sChallenge_Leon[] = _(
    "A card duel? Let's have a\n"
    "champion time!");
static const u8 sWon_Leon[] = _(
    "That was a champion time!");
static const u8 sLost_Leon[] = _(
    "My CHARIZARD card never loses!");

static const u8 sName_Lance[] = _("LANCE");
static const u8 sChallenge_Lance[] = _(
    "Dragon cards are as fierce as\n"
    "the real ones. Duel me!");
static const u8 sWon_Lance[] = _(
    "My dragons have fallen.");
static const u8 sLost_Lance[] = _(
    "The dragon's might prevails.");

static const u8 sName_Steven[] = _("STEVEN");
static const u8 sChallenge_Steven[] = _(
    "I collect rare stones, and rare\n"
    "cards too. A duel?");
static const u8 sWon_Steven[] = _(
    "Brilliant! A gem of a duel!");
static const u8 sLost_Steven[] = _(
    "Steel stands firm.");

static const u8 sName_Cynthia[] = _("CYNTHIA");
static const u8 sChallenge_Cynthia[] = _(
    "I love card games, you know.\n"
    "Will you play with me?");
static const u8 sWon_Cynthia[] = _(
    "Wonderful! A fair loss for me.");
static const u8 sLost_Cynthia[] = _(
    "That was so much fun!");

static const u8 sName_Alder[] = _("ALDER");
static const u8 sChallenge_Alder[] = _(
    "Ha ha! A card duel? I'm in!\n"
    "Show me your spirit!");
static const u8 sWon_Alder[] = _(
    "Your spirit burns bright!");
static const u8 sLost_Alder[] = _(
    "Keep playing, keep growing!");

static const u8 sName_Diantha[] = _("DIANTHA");
static const u8 sChallenge_Diantha[] = _(
    "Cards are like a stage. Shall\n"
    "we perform together?");
static const u8 sWon_Diantha[] = _(
    "Bravo! A splendid performance!");
static const u8 sLost_Diantha[] = _(
    "And that's a wrap!");

static const u8 sName_MrM[] = _("MR. M.");
static const u8 sChallenge_MrM[] = _(
    "…You have come this far.\n"
    "Now face my true power.");
static const u8 sWon_MrM[] = _(
    "…Your bond is strong. I yield.");
static const u8 sLost_MrM[] = _(
    "…As expected. You are not ready.");

#define OPPONENT(id, n, deckId, reward, where) [CARD_OPPONENT_##id] = \
    {sName_##n, sChallenge_##n, sWon_##n, sLost_##n, reward, CARD_DECK_##deckId, MAPSEC_##where}

const struct CardOpponent gCardOpponents[CARD_OPPONENTS_COUNT] =
{
    OPPONENT(PALLET_FAT_MAN,          Gus,      NEIGHBOR,    CARD_X_SPEED,             PALLET_TOWN),
    OPPONENT(VIRIDIAN_BOY,            Tommy,    NIDO_KID,    CARD_NIDOKING,            VIRIDIAN_CITY),
    OPPONENT(PEWTER_BUG_CATCHER,      Rick,     BUG_CATCHER, CARD_SCYTHER,             PEWTER_CITY),
    OPPONENT(PEWTER_PC_YOUNGSTER,     Dane,     FOSSIL_FAN,  CARD_AERODACTYL,          PEWTER_CITY),
    OPPONENT(ROUTE2_ROCKER,           Wes,      PSYCHIC,     CARD_MR_MIME,             ROUTE_2),
    OPPONENT(ROUTE4_PC_GENTLEMAN,     Ansel,    HIKER,       CARD_GIANT_CAPE,          ROUTE_4),
    OPPONENT(CERULEAN_PC_LASS,        Coral,    SWIMMER,     CARD_LAPRAS,              CERULEAN_CITY),
    OPPONENT(CERULEAN_PC_ROCKER,      Zack,     ELECTRIC,    CARD_ELECTABUZZ,          CERULEAN_CITY),
    OPPONENT(VERMILION_SAILOR,        Duncan,   SAILOR,      CARD_RED_CARD,            VERMILION_CITY),
    OPPONENT(FAN_CLUB_FAT_MAN,        Mort,     FAN_CLUB,    CARD_CHANSEY,             VERMILION_CITY),
    OPPONENT(LAVENDER_BOY,            Ivan,     GHOST,       CARD_HAND_SCOPE,          LAVENDER_TOWN),
    OPPONENT(CELADON_FAT_MAN,         Lou,      POLIWAG,     CARD_POLITOED,            CELADON_CITY),
    OPPONENT(CELADON_WOMAN,           Flora,    GARDEN,      CARD_RARE_CANDY,          CELADON_CITY),
    OPPONENT(GAME_CORNER_BALDING_MAN, Bert,     ROCKET,      CARD_HOUNDOUR,            CELADON_CITY),
    OPPONENT(GAME_CORNER_WOMAN,       Vera,     TECH,        CARD_PORYGON,             CELADON_CITY),
    OPPONENT(GAME_CORNER_OLD_MAN,     Otto,     FIRE,        CARD_CHARIZARD_MEGA_Y_EX, CELADON_CITY),
    OPPONENT(BROCK,                   Brock,    BROCK,       CARD_BROCK,               PEWTER_CITY),
    OPPONENT(MISTY,                   Misty,    MISTY,       CARD_MISTY,               CERULEAN_CITY),
    OPPONENT(LT_SURGE,                LtSurge,  LT_SURGE,    CARD_LT_SURGE,            VERMILION_CITY),
    OPPONENT(ERIKA,                   Erika,    ERIKA,       CARD_ERIKA,               CELADON_CITY),
    OPPONENT(SAFFRON_PC_GENTLEMAN,    Nigel,    PSYCHIC_EX,  CARD_ALAKAZAM_MEGA_EX,    SAFFRON_CITY),
    OPPONENT(DOJO_MASTER,             Koichi,   DOJO,        CARD_HITMONTOP,           SAFFRON_CITY),
    OPPONENT(COPYCAT,                 Copycat,  COPYCAT,     CARD_DITTO,               SAFFRON_CITY),
    OPPONENT(FUCHSIA_OLD_MAN,         Gramps,   EEVEE,       CARD_UMBREON,             FUCHSIA_CITY),
    OPPONENT(FUCHSIA_PC_COOLTRAINER,  Paige,    SAFARI,      CARD_KANGASKHAN_MEGA_EX,  FUCHSIA_CITY),
    OPPONENT(ROUTE17_BIKER,           Billy,    BIKER,       CARD_ROCKY_HELMET,        ROUTE_17),
    OPPONENT(CINNABAR_LAB_SCIENTIST,  Gideon,   FOSSIL_EX,   CARD_AERODACTYL_MEGA_EX,  CINNABAR_ISLAND),
    OPPONENT(CINNABAR_PC_YOUNGSTER,   Ned,      SEAFOAM,     CARD_ARTICUNO,            CINNABAR_ISLAND),
    OPPONENT(INDIGO_VETERAN,          Takeo,    VETERAN,     CARD_GENGAR_MEGA_EX,      INDIGO_PLATEAU),
    OPPONENT(KOGA,                    Koga,     KOGA,        CARD_KOGA,                FUCHSIA_CITY),
    OPPONENT(SABRINA,                 Sabrina,  SABRINA,     CARD_SABRINA,             SAFFRON_CITY),
    OPPONENT(BLAINE,                  Blaine,   BLAINE,      CARD_BLAINE,              CINNABAR_ISLAND),
    OPPONENT(GIOVANNI,                Giovanni, GIOVANNI,    CARD_GIOVANNI,            VIRIDIAN_CITY),
    OPPONENT(ONE_ISLAND_HIKER,        Rocko,    EMBER,       CARD_MOLTRES,             ONE_ISLAND),
    OPPONENT(EMBER_SPA_OLD_WOMAN,     Hanako,   SPA,         CARD_BLISSEY,             KINDLE_ROAD),
    OPPONENT(GAME_CORNER_DADDY,       Gordon,   JOYFUL,      CARD_ZAPDOS,              TWO_ISLAND),
    OPPONENT(LOSTELLE,                Lostelle, CUTE,        CARD_TOGETIC,             TWO_ISLAND),
    OPPONENT(THREE_ISLAND_SAILOR,     Marlon,   DEEP_SEA,    CARD_BLASTOISE_MEGA_EX,   THREE_ISLAND),
    OPPONENT(LORELEI,                 Lorelei,  LORELEI,     CARD_SLOWBRO_MEGA_EX,     FOUR_ISLAND),
    OPPONENT(FOUR_ISLAND_GENTLEMAN,   Waldo,    BABIES,      CARD_TOGEPI,              FOUR_ISLAND),
    OPPONENT(FIVE_ISLAND_COOLTRAINER, Dahlia,   BLOOM,       CARD_VENUSAUR_MEGA_EX,    FIVE_ISLAND),
    OPPONENT(SIX_ISLAND_OLD_MAN,      Eldon,    RUINS,       CARD_TYRANITAR_MEGA_EX,   SIX_ISLAND),
    OPPONENT(TRAINER_TOWER_RODETTE,   Rodette,  SKY,         CARD_RAYQUAZA_MEGA_EX,    TRAINER_TOWER),
    OPPONENT(BLUE,                    Blue,     BLUE,        CARD_PIDGEOT_MEGA_EX,     MEGA_ISLAND),
    OPPONENT(LEON,                    Leon,     LEON,        CARD_CHARIZARD_MEGA_X_EX, MEGA_ISLAND),
    OPPONENT(LANCE,                   Lance,    LANCE,       CARD_DRAGONITE_MEGA_EX,   MEGA_ISLAND),
    OPPONENT(STEVEN,                  Steven,   STEVEN,      CARD_STEELIX_MEGA_EX,     MEGA_ISLAND),
    OPPONENT(CYNTHIA,                 Cynthia,  CYNTHIA,     CARD_LATIAS_MEGA_EX,      MEGA_ISLAND),
    OPPONENT(ALDER,                   Alder,    ALDER,       CARD_HERACROSS_MEGA_EX,   MEGA_ISLAND),
    OPPONENT(DIANTHA,                 Diantha,  DIANTHA,     CARD_LATIOS_MEGA_EX,      MEGA_ISLAND),
    OPPONENT(MR_M,                    MrM,      MR_M,        CARD_MEWTWO_MEGA_X_EX,    MEGA_ISLAND),
};

#undef OPPONENT
