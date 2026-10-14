// The KANTO GYM LEADERS' rematches after MR. M.'s invitation (see gym_leader_rematch_parties.h).

    [TRAINER_REMATCH_BROCK] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_MALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_BROCK,
        .trainerName = _("BROCK"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchBrock),
    },

    [TRAINER_REMATCH_MISTY] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_FEMALE | F_TRAINER_FEMALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_MISTY,
        .trainerName = _("MISTY"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchMisty),
    },

    [TRAINER_REMATCH_LT_SURGE] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_MALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_LT_SURGE,
        .trainerName = _("LT. SURGE"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchLtSurge),
    },

    [TRAINER_REMATCH_ERIKA] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_FEMALE | F_TRAINER_FEMALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_ERIKA,
        .trainerName = _("ERIKA"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchErika),
    },

    [TRAINER_REMATCH_KOGA] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_MALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_KOGA,
        .trainerName = _("KOGA"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchKoga),
    },

    [TRAINER_REMATCH_SABRINA] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_FEMALE | F_TRAINER_FEMALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_SABRINA,
        .trainerName = _("SABRINA"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchSabrina),
    },

    [TRAINER_REMATCH_BLAINE] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_MALE,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_BLAINE,
        .trainerName = _("BLAINE"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchBlaine),
    },

    [TRAINER_REMATCH_GIOVANNI] = {
        .trainerClass = TRAINER_CLASS_LEADER,
        .encounterMusic_gender = TRAINER_ENCOUNTER_MUSIC_AQUA,
        .trainerPic = TRAINER_PIC_KANTO_LEADER_GIOVANNI,
        .trainerName = _("GIOVANNI"),
        .items = {ITEM_FULL_RESTORE, ITEM_FULL_RESTORE},
        .doubleBattle = FALSE,
        .aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY,
        .party = ITEM_CUSTOM_MOVES(sParty_RematchGiovanni),
    },
