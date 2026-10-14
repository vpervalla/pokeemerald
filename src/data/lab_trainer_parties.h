// The trainers who took the Kanto starters from OAK's lab. LEAF (or KRIS) took BULBASAUR and meets the
// player four times; RED (or ETHAN) took CHARMANDER and waits deep in CERULEAN CAVE, where
// MEWTWO used to be, after the Hall of Fame.
// The ace, with its MEGA STONE when it has one, is last.

static const struct TrainerMonItemCustomMoves sParty_LabLeafRoute24[] = {
    {
        .iv = 50,
        .lvl = 13,
        .species = SPECIES_NIDORAN_F,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_DOUBLE_KICK, MOVE_POISON_STING, MOVE_SCRATCH, MOVE_TAIL_WHIP},
    },
    {
        .iv = 50,
        .lvl = 13,
        .species = SPECIES_CLEFAIRY,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_POUND, MOVE_SING, MOVE_ENCORE, MOVE_GROWL},
    },
    {
        .iv = 50,
        .lvl = 15,
        .species = SPECIES_BULBASAUR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_VINE_WHIP, MOVE_LEECH_SEED, MOVE_TACKLE, MOVE_GROWL},
    },
};

static const struct TrainerMonItemCustomMoves sParty_LabLeafCeladon[] = {
    {
        .iv = 100,
        .lvl = 30,
        .species = SPECIES_NIDORINA,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_DOUBLE_KICK, MOVE_BITE, MOVE_POISON_STING, MOVE_FURY_SWIPES},
    },
    {
        .iv = 100,
        .lvl = 30,
        .species = SPECIES_CLEFAIRY,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_METRONOME, MOVE_SING, MOVE_MINIMIZE, MOVE_DOUBLE_SLAP},
    },
    {
        .iv = 100,
        .lvl = 31,
        .species = SPECIES_PONYTA,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EMBER, MOVE_STOMP, MOVE_FIRE_SPIN, MOVE_AGILITY},
    },
    {
        .iv = 100,
        .lvl = 30,
        .species = SPECIES_PSYDUCK,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_CONFUSION, MOVE_WATER_GUN, MOVE_DISABLE, MOVE_FURY_SWIPES},
    },
    {
        .iv = 100,
        .lvl = 33,
        .species = SPECIES_IVYSAUR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_RAZOR_LEAF, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_POISON_POWDER},
    },
};

static const struct TrainerMonItemCustomMoves sParty_LabLeafRoute23[] = {
    {
        .iv = 200,
        .lvl = 50,
        .species = SPECIES_NIDOQUEEN,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_BODY_SLAM, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
    },
    {
        .iv = 200,
        .lvl = 49,
        .species = SPECIES_CLEFABLE,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_MOONBLAST, MOVE_THUNDERBOLT, MOVE_SOFT_BOILED, MOVE_CALM_MIND},
    },
    {
        .iv = 200,
        .lvl = 50,
        .species = SPECIES_RAPIDASH,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_FLAMETHROWER, MOVE_STOMP, MOVE_AGILITY, MOVE_BOUNCE},
    },
    {
        .iv = 200,
        .lvl = 50,
        .species = SPECIES_GOLDUCK,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_CALM_MIND},
    },
    {
        .iv = 200,
        .lvl = 51,
        .species = SPECIES_DRAGONAIR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_DRAGON_BREATH, MOVE_THUNDER_WAVE, MOVE_ICE_BEAM, MOVE_BODY_SLAM},
    },
    {
        .iv = 200,
        .lvl = 53,
        .species = SPECIES_VENUSAUR,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER, MOVE_SYNTHESIS},
    },
};

static const struct TrainerMonItemCustomMoves sParty_LabLeafCeruleanCave[] = {
    {
        .iv = 255,
        .lvl = 74,
        .species = SPECIES_NIDOQUEEN,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_EARTHQUAKE, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
    },
    {
        .iv = 255,
        .lvl = 74,
        .species = SPECIES_CLEFABLE,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_MOONBLAST, MOVE_THUNDERBOLT, MOVE_CALM_MIND, MOVE_SOFT_BOILED},
    },
    {
        .iv = 255,
        .lvl = 75,
        .species = SPECIES_RAPIDASH,
        .heldItem = ITEM_SHELL_BELL,
        .moves = {MOVE_FIRE_BLAST, MOVE_BOUNCE, MOVE_AGILITY, MOVE_HYPNOSIS},
    },
    {
        .iv = 255,
        .lvl = 75,
        .species = SPECIES_STARMIE,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_RECOVER},
    },
    {
        .iv = 255,
        .lvl = 76,
        .species = SPECIES_DRAGONITE,
        .heldItem = ITEM_LUM_BERRY,
        .moves = {MOVE_DRAGON_DANCE, MOVE_OUTRAGE, MOVE_EARTHQUAKE, MOVE_EXTREME_SPEED},
    },
    {
        .iv = 255,
        .lvl = 78,
        .species = SPECIES_VENUSAUR,
        .heldItem = ITEM_VENUSAURITE,
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLUDGE_BOMB, MOVE_EARTHQUAKE, MOVE_SYNTHESIS},
    },
};

static const struct TrainerMonItemCustomMoves sParty_LabRedCeruleanCave[] = {
    {
        .iv = 255,
        .lvl = 77,
        .species = SPECIES_SNORLAX,
        .heldItem = ITEM_CHESTO_BERRY,
        .moves = {MOVE_BODY_SLAM, MOVE_CURSE, MOVE_REST, MOVE_EARTHQUAKE},
    },
    {
        .iv = 255,
        .lvl = 77,
        .species = SPECIES_LAPRAS,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_CONFUSE_RAY},
    },
    {
        .iv = 255,
        .lvl = 76,
        .species = SPECIES_ESPEON,
        .heldItem = ITEM_LEFTOVERS,
        .moves = {MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_MORNING_SUN, MOVE_REFLECT},
    },
    {
        .iv = 255,
        .lvl = 76,
        .species = SPECIES_AERODACTYL,
        .heldItem = ITEM_KINGS_ROCK,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_CRUNCH},
    },
    {
        .iv = 255,
        .lvl = 76,
        .species = SPECIES_GENGAR,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_SHADOW_BALL, MOVE_THUNDERBOLT, MOVE_HYPNOSIS, MOVE_DREAM_EATER},
    },
    {
        .iv = 255,
        .lvl = 80,
        .species = SPECIES_CHARIZARD,
        .heldItem = ITEM_CHARIZARDITE_X,
        .moves = {MOVE_DRAGON_DANCE, MOVE_DRAGON_CLAW, MOVE_FIRE_BLAST, MOVE_EARTHQUAKE},
    },
};
