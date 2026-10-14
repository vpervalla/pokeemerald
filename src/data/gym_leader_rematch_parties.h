// The KANTO GYM LEADERS' rematches after MR. M.'s invitation (see data/scripts/gym_rematches.inc).
// Each leader's ace, sent out last, holds the MEGA STONE they give the player when beaten.

static const struct TrainerMonItemCustomMoves sParty_RematchBrock[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_GOLEM,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_EXPLOSION, MOVE_DOUBLE_EDGE},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_OMASTAR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_ANCIENT_POWER, MOVE_SPIKES},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_KABUTOPS,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_ROCK_SLIDE, MOVE_SURF, MOVE_SWORDS_DANCE, MOVE_BRICK_BREAK},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_RHYDON,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_MEGAHORN, MOVE_IRON_TAIL},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_STEELIX,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_IRON_TAIL, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_CRUNCH},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_AERODACTYL,
        .heldItem = ITEM_AERODACTYLITE,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_DRAGON_CLAW},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchMisty[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_LAPRAS,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_CONFUSE_RAY},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_GOLDUCK,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_CALM_MIND},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_VAPOREON,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_ACID_ARMOR, MOVE_REST},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_BLASTOISE,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_EARTHQUAKE, MOVE_RAIN_DANCE},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_STARMIE,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_GYARADOS,
        .heldItem = ITEM_GYARADOSITE,
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_DOUBLE_EDGE, MOVE_HYDRO_PUMP},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchLtSurge[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_ELECTRODE,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_THUNDERBOLT, MOVE_EXPLOSION, MOVE_LIGHT_SCREEN, MOVE_RAIN_DANCE},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_MAGNETON,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_TRI_ATTACK, MOVE_METAL_SOUND},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_ELECTABUZZ,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_THUNDERBOLT, MOVE_CROSS_CHOP, MOVE_ICE_PUNCH, MOVE_FIRE_PUNCH},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_JOLTEON,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_THUNDERBOLT, MOVE_PIN_MISSILE, MOVE_SHADOW_BALL, MOVE_DOUBLE_KICK},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_RAICHU,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_THUNDERBOLT, MOVE_BRICK_BREAK, MOVE_QUICK_ATTACK, MOVE_THUNDER_WAVE},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_AMPHAROS,
        .heldItem = ITEM_AMPHAROSITE,
        .moves = {MOVE_THUNDERBOLT, MOVE_FIRE_PUNCH, MOVE_IRON_TAIL, MOVE_THUNDER_WAVE},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchErika[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_TANGELA,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_ANCIENT_POWER, MOVE_BODY_SLAM},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_EXEGGUTOR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_PSYCHIC, MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER, MOVE_EGG_BOMB},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_VICTREEBEL,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_LEAF_BLADE, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER, MOVE_SUNNY_DAY},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_BELLOSSOM,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SOLAR_BEAM, MOVE_PETAL_DANCE, MOVE_STUN_SPORE, MOVE_SUNNY_DAY},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_VILEPLUME,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_SOLAR_BEAM, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER, MOVE_SUNNY_DAY},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_BEEDRILL,
        .heldItem = ITEM_BEEDRILLITE,
        .moves = {MOVE_TWINEEDLE, MOVE_SLUDGE_BOMB, MOVE_AERIAL_ACE, MOVE_SWORDS_DANCE},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchKoga[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_MUK,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FIRE_PUNCH, MOVE_MINIMIZE, MOVE_TOXIC},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_VENOMOTH,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_PSYCHIC, MOVE_SLUDGE_BOMB, MOVE_SLEEP_POWDER, MOVE_SILVER_WIND},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_NIDOKING,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_CROBAT,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_AERIAL_ACE, MOVE_CONFUSE_RAY, MOVE_TOXIC},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_WEEZING,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FLAMETHROWER, MOVE_WILL_O_WISP, MOVE_EXPLOSION},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_SCIZOR,
        .heldItem = ITEM_SCIZORITE,
        .moves = {MOVE_SILVER_WIND, MOVE_METAL_CLAW, MOVE_AERIAL_ACE, MOVE_SWORDS_DANCE},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchSabrina[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_MR_MIME,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_REFLECT, MOVE_LIGHT_SCREEN},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_HYPNO,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_PSYCHIC, MOVE_HYPNOSIS, MOVE_DREAM_EATER, MOVE_THUNDER_PUNCH},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_JYNX,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_LOVELY_KISS, MOVE_CALM_MIND},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_SLOWBRO,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_SLACK_OFF},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_ESPEON,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_CALM_MIND, MOVE_MORNING_SUN},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_ALAKAZAM,
        .heldItem = ITEM_ALAKAZITE,
        .moves = {MOVE_PSYCHIC, MOVE_SHADOW_BALL, MOVE_FIRE_PUNCH, MOVE_CALM_MIND},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchBlaine[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_NINETALES,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_FLAMETHROWER, MOVE_WILL_O_WISP, MOVE_CONFUSE_RAY, MOVE_SUNNY_DAY},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_RAPIDASH,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_FIRE_BLAST, MOVE_AGILITY, MOVE_STOMP, MOVE_BODY_SLAM},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_MAGMAR,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_FIRE_BLAST, MOVE_THUNDER_PUNCH, MOVE_CROSS_CHOP, MOVE_PSYCHIC},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_CHARIZARD,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_FLAMETHROWER, MOVE_SOLAR_BEAM, MOVE_AERIAL_ACE, MOVE_SUNNY_DAY},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_ARCANINE,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_FLAMETHROWER, MOVE_EXTREME_SPEED, MOVE_CRUNCH, MOVE_IRON_TAIL},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_HOUNDOOM,
        .heldItem = ITEM_HOUNDOOMINITE,
        .moves = {MOVE_FLAMETHROWER, MOVE_CRUNCH, MOVE_WILL_O_WISP, MOVE_SOLAR_BEAM},
    },
};

static const struct TrainerMonItemCustomMoves sParty_RematchGiovanni[] = {
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_DUGTRIO,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SLASH, MOVE_AERIAL_ACE},
    },
    {
        .iv = 250,
        .lvl = 66,
        .species = SPECIES_MAROWAK,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_BONEMERANG, MOVE_ROCK_SLIDE, MOVE_SWORDS_DANCE, MOVE_DOUBLE_EDGE},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_NIDOQUEEN,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
    },
    {
        .iv = 250,
        .lvl = 67,
        .species = SPECIES_NIDOKING,
        .heldItem = ITEM_NONE,
        .moves = {MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_SLUDGE_BOMB, MOVE_THUNDERBOLT},
    },
    {
        .iv = 250,
        .lvl = 68,
        .species = SPECIES_RHYDON,
        .heldItem = ITEM_SITRUS_BERRY,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_MEGAHORN, MOVE_SWORDS_DANCE},
    },
    {
        .iv = 250,
        .lvl = 70,
        .species = SPECIES_KANGASKHAN,
        .heldItem = ITEM_KANGASKHANITE,
        .moves = {MOVE_FAKE_OUT, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_CRUNCH},
    },
};
