// Kanto (FRLG): TRAINER TOWER, from pret/pokefirered (include/constants/trainer_tower.h). FRLG's
// CHALLENGE_STATUS_* are TT_STATUS_* here (Emerald's Battle Frontier has its own).
#ifndef GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H
#define GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H

#define CHALLENGE_TYPE_SINGLE     0
#define CHALLENGE_TYPE_DOUBLE     1
#define CHALLENGE_TYPE_KNOCKOUT   2
#define CHALLENGE_TYPE_MIXED      3
// Mixed challenges use one of the above 3 for each floor

#define TT_STATUS_LOST     0
#define TT_STATUS_UNK      1  // Never set, possibly e-Card related
#define TT_STATUS_NORMAL   2  // Not started or ongoing

#define TTPRIZE_HP_UP               0
#define TTPRIZE_PROTEIN             1
#define TTPRIZE_IRON                2
#define TTPRIZE_CARBOS              3
#define TTPRIZE_CALCIUM             4
#define TTPRIZE_ZINC                5
#define TTPRIZE_BRIGHT_POWDER       6
#define TTPRIZE_WHITE_HERB          7
#define TTPRIZE_MENTAL_HERB         8
#define TTPRIZE_CHOICE_BAND         9
#define TTPRIZE_KINGS_ROCK         10
#define TTPRIZE_SCOPE_LENS         11
#define TTPRIZE_METAL_COAT         12
#define TTPRIZE_DRAGON_SCALE       13
#define TTPRIZE_UP_GRADE           14

#define MAX_TRAINER_TOWER_FLOORS 8

#define MAX_TRAINERS_PER_FLOOR  3

#define TRAINER_TOWER_FUNC_INIT_FLOOR               0
#define TRAINER_TOWER_FUNC_GET_SPEECH               1
#define TRAINER_TOWER_FUNC_DO_BATTLE                2
#define TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE       3
#define TRAINER_TOWER_FUNC_CLEARED_FLOOR            4
#define TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED        5
#define TRAINER_TOWER_FUNC_START_CHALLENGE          6
#define TRAINER_TOWER_FUNC_GET_OWNER_STATE          7
#define TRAINER_TOWER_FUNC_GIVE_PRIZE               8
#define TRAINER_TOWER_FUNC_CHECK_FINAL_TIME         9
#define TRAINER_TOWER_FUNC_RESUME_TIMER             10
#define TRAINER_TOWER_FUNC_SET_LOST                 11
#define TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS     12
#define TRAINER_TOWER_FUNC_GET_TIME                 13
#define TRAINER_TOWER_FUNC_SHOW_RESULTS             14
#define TRAINER_TOWER_FUNC_CLOSE_RESULTS            15
#define TRAINER_TOWER_FUNC_CHECK_DOUBLES            16
#define TRAINER_TOWER_FUNC_GET_NUM_FLOORS           17
#define TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER   18
#define TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC          19
#define TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE       20

#define TRAINER_TOWER_TEXT_INTRO        2
#define TRAINER_TOWER_TEXT_PLAYER_LOST  3
#define TRAINER_TOWER_TEXT_PLAYER_WON   4
#define TRAINER_TOWER_TEXT_AFTER        5

#define TRAINER_TOWER_MAX_TIME 215999 // 60 * 60 * 60 - 1

#define NUM_TOWER_CHALLENGE_TYPES 4

// The trainer classes of the built-in floors (FRLG's FACILITY_CLASS_*), see sTowerClassInfo in
// src/kanto_trainer_tower.c
#define TT_CLASS_AROMA_LADY     0
#define TT_CLASS_BEAUTY         1
#define TT_CLASS_BIKER          2
#define TT_CLASS_BLACK_BELT     3
#define TT_CLASS_BUG_CATCHER    4
#define TT_CLASS_BURGLAR        5
#define TT_CLASS_CAMPER         6
#define TT_CLASS_COOLTRAINER_F  7
#define TT_CLASS_COOLTRAINER_M  8
#define TT_CLASS_COOL_COUPLE    9
#define TT_CLASS_CRUSH_GIRL     10
#define TT_CLASS_CRUSH_KIN      11
#define TT_CLASS_CUE_BALL       12
#define TT_CLASS_ENGINEER       13
#define TT_CLASS_FISHERMAN      14
#define TT_CLASS_GENTLEMAN      15
#define TT_CLASS_HIKER          16
#define TT_CLASS_JUGGLER        17
#define TT_CLASS_LADY           18
#define TT_CLASS_LASS           19
#define TT_CLASS_PICNICKER      20
#define TT_CLASS_PKMN_BREEDER   21
#define TT_CLASS_PKMN_RANGER_F  22
#define TT_CLASS_PKMN_RANGER_M  23
#define TT_CLASS_PSYCHIC_F      24
#define TT_CLASS_PSYCHIC_M      25
#define TT_CLASS_ROCKER         26
#define TT_CLASS_SAILOR         27
#define TT_CLASS_SCIENTIST      28
#define TT_CLASS_SIS_AND_BRO    29
#define TT_CLASS_SUPER_NERD     30
#define TT_CLASS_SWIMMER_F      31
#define TT_CLASS_SWIMMER_M      32
#define TT_CLASS_TUBER          33
#define TT_CLASS_TWINS          34
#define TT_CLASS_YOUNGSTER      35
#define TT_CLASS_YOUNG_COUPLE   36
#define NUM_TT_CLASSES        37

#endif //GUARD_CONSTANTS_KANTO_TRAINER_TOWER_H
