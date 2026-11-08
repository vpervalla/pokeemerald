// Kanto (FRLG): TRAINER TOWER on SEVEN ISLAND, ported from pret/pokefirered (src/trainer_tower.c).
//
// The player picks a challenge in the lobby (CHALLENGE_TYPE_SINGLE, _DOUBLE, _KNOCKOUT or _MIXED) and
// climbs eight floors against the clock, each floor laid out for its battle type; the owner on the
// roof gives a prize once and records the best time of each challenge. The floors are FRLG's
// built-in ones (src/data/kanto_trainer_tower_sets.h); the e-Reader's aren't supported.
//
// Changes from FRLG:
// - Saved with the Kanto port's other data in PokemonStorage (struct KantoTrainerTowerSave), with a
//   single timer for the challenge under way.
// - The battles run as BATTLE_TYPE_TRAINER_HILL (Emerald's 32 battle types are all taken): while
//   sInBattle is set, Trainer Hill's battle functions (src/trainer_hill.c) call the ones at the end
//   of this file for the opponent's class, name, picture and lines.
// - The trainers' classes, pictures and overworld sprites are the Kanto port's (sTowerClassInfo);
//   their encounter music goes by gender (FRLG's lookup compared mismatched ids).
// - The floor is the map (MAP_TRAINER_TOWER_1F-8F) rather than the layout id.
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "easy_chat.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "item.h"
#include "kanto_trainer_tower.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/event_objects.h"
#include "constants/easy_chat.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/layouts.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/trainers.h"

#include "data/kanto_trainer_tower_sets.h"

#define TOWER (gPokemonStoragePtr->trainerTower)

struct TowerClassInfo
{
    u8 trainerClass;
    u8 trainerPic;
    u16 objGfx;  // the trainer, or the first of a pair
    u16 objGfx2; // the second of a pair (double battle classes)
    bool8 female;
};

static const struct TowerClassInfo sTowerClassInfo[NUM_TT_CLASSES] = {
    [TT_CLASS_AROMA_LADY]    = {TRAINER_CLASS_AROMA_LADY,    TRAINER_PIC_KANTO_AROMA_LADY,       OBJ_EVENT_GFX_KANTO_WOMAN_2,        0, TRUE},
    [TT_CLASS_BEAUTY]        = {TRAINER_CLASS_BEAUTY,        TRAINER_PIC_KANTO_BEAUTY,           OBJ_EVENT_GFX_KANTO_BEAUTY,         0, TRUE},
    [TT_CLASS_BIKER]         = {TRAINER_CLASS_BIKER,         TRAINER_PIC_KANTO_BIKER,            OBJ_EVENT_GFX_KANTO_BIKER,          0, FALSE},
    [TT_CLASS_BLACK_BELT]    = {TRAINER_CLASS_BLACK_BELT,    TRAINER_PIC_KANTO_BLACK_BELT,       OBJ_EVENT_GFX_KANTO_BLACK_BELT,     0, FALSE},
    [TT_CLASS_BUG_CATCHER]   = {TRAINER_CLASS_BUG_CATCHER,   TRAINER_PIC_KANTO_BUG_CATCHER,      OBJ_EVENT_GFX_KANTO_BUG_CATCHER,    0, FALSE},
    [TT_CLASS_BURGLAR]       = {TRAINER_CLASS_BURGLAR,       TRAINER_PIC_KANTO_BURGLAR,          OBJ_EVENT_GFX_KANTO_POKE_MANIAC,    0, FALSE},
    [TT_CLASS_CAMPER]        = {TRAINER_CLASS_CAMPER,        TRAINER_PIC_KANTO_CAMPER,           OBJ_EVENT_GFX_KANTO_CAMPER,         0, FALSE},
    [TT_CLASS_COOLTRAINER_F] = {TRAINER_CLASS_COOLTRAINER,   TRAINER_PIC_KANTO_COOLTRAINER_F,    OBJ_EVENT_GFX_KANTO_COOLTRAINER_F,  0, TRUE},
    [TT_CLASS_COOLTRAINER_M] = {TRAINER_CLASS_COOLTRAINER,   TRAINER_PIC_KANTO_COOLTRAINER_M,    OBJ_EVENT_GFX_KANTO_COOLTRAINER_M,  0, FALSE},
    [TT_CLASS_COOL_COUPLE]   = {TRAINER_CLASS_COOL_COUPLE,   TRAINER_PIC_KANTO_COOL_COUPLE,      OBJ_EVENT_GFX_KANTO_COOLTRAINER_M,  OBJ_EVENT_GFX_KANTO_COOLTRAINER_F, FALSE},
    [TT_CLASS_CRUSH_GIRL]    = {TRAINER_CLASS_CRUSH_GIRL,    TRAINER_PIC_KANTO_CRUSH_GIRL,       OBJ_EVENT_GFX_KANTO_CRUSH_GIRL,     0, TRUE},
    [TT_CLASS_CRUSH_KIN]     = {TRAINER_CLASS_CRUSH_KIN,     TRAINER_PIC_KANTO_CRUSH_KIN,        OBJ_EVENT_GFX_KANTO_CRUSH_GIRL,     OBJ_EVENT_GFX_KANTO_BLACK_BELT, TRUE},
    [TT_CLASS_CUE_BALL]      = {TRAINER_CLASS_CUE_BALL,      TRAINER_PIC_KANTO_CUE_BALL,         OBJ_EVENT_GFX_KANTO_BIKER,          0, FALSE},
    [TT_CLASS_ENGINEER]      = {TRAINER_CLASS_ENGINEER,      TRAINER_PIC_KANTO_ENGINEER,         OBJ_EVENT_GFX_KANTO_BALDING_MAN,    0, FALSE},
    [TT_CLASS_FISHERMAN]     = {TRAINER_CLASS_FISHERMAN,     TRAINER_PIC_KANTO_FISHERMAN,        OBJ_EVENT_GFX_KANTO_FISHER,         0, FALSE},
    [TT_CLASS_GENTLEMAN]     = {TRAINER_CLASS_GENTLEMAN,     TRAINER_PIC_KANTO_GENTLEMAN,        OBJ_EVENT_GFX_KANTO_GENTLEMAN,      0, FALSE},
    [TT_CLASS_HIKER]         = {TRAINER_CLASS_HIKER,         TRAINER_PIC_KANTO_HIKER,            OBJ_EVENT_GFX_KANTO_HIKER,          0, FALSE},
    [TT_CLASS_JUGGLER]       = {TRAINER_CLASS_JUGGLER,       TRAINER_PIC_KANTO_JUGGLER,          OBJ_EVENT_GFX_KANTO_ROCKER,         0, FALSE},
    [TT_CLASS_LADY]          = {TRAINER_CLASS_LADY,          TRAINER_PIC_KANTO_LADY,             OBJ_EVENT_GFX_KANTO_WOMAN_2,        0, TRUE},
    [TT_CLASS_LASS]          = {TRAINER_CLASS_LASS,          TRAINER_PIC_KANTO_LASS,             OBJ_EVENT_GFX_KANTO_LASS,           0, TRUE},
    [TT_CLASS_PICNICKER]     = {TRAINER_CLASS_PICNICKER,     TRAINER_PIC_KANTO_PICNICKER,        OBJ_EVENT_GFX_KANTO_PICNICKER,      0, TRUE},
    [TT_CLASS_PKMN_BREEDER]  = {TRAINER_CLASS_PKMN_BREEDER,  TRAINER_PIC_KANTO_POKEMON_BREEDER,  OBJ_EVENT_GFX_KANTO_WOMAN_2,        0, TRUE},
    [TT_CLASS_PKMN_RANGER_F] = {TRAINER_CLASS_PKMN_RANGER,   TRAINER_PIC_KANTO_POKEMON_RANGER_F, OBJ_EVENT_GFX_KANTO_PICNICKER,      0, TRUE},
    [TT_CLASS_PKMN_RANGER_M] = {TRAINER_CLASS_PKMN_RANGER,   TRAINER_PIC_KANTO_POKEMON_RANGER_M, OBJ_EVENT_GFX_KANTO_CAMPER,         0, FALSE},
    [TT_CLASS_PSYCHIC_F]     = {TRAINER_CLASS_PSYCHIC,       TRAINER_PIC_KANTO_PSYCHIC_F,        OBJ_EVENT_GFX_KANTO_WOMAN_1,        0, TRUE},
    [TT_CLASS_PSYCHIC_M]     = {TRAINER_CLASS_PSYCHIC,       TRAINER_PIC_KANTO_PSYCHIC_M,        OBJ_EVENT_GFX_KANTO_BOY,            0, FALSE},
    [TT_CLASS_ROCKER]        = {TRAINER_CLASS_ROCKER,        TRAINER_PIC_KANTO_ROCKER,           OBJ_EVENT_GFX_KANTO_ROCKER,         0, FALSE},
    [TT_CLASS_SAILOR]        = {TRAINER_CLASS_SAILOR,        TRAINER_PIC_KANTO_SAILOR,           OBJ_EVENT_GFX_KANTO_SAILOR,         0, FALSE},
    [TT_CLASS_SCIENTIST]     = {TRAINER_CLASS_SCIENTIST,     TRAINER_PIC_KANTO_SCIENTIST,        OBJ_EVENT_GFX_KANTO_SCIENTIST,      0, FALSE},
    [TT_CLASS_SIS_AND_BRO]   = {TRAINER_CLASS_SIS_AND_BRO,   TRAINER_PIC_KANTO_SIS_AND_BRO,      OBJ_EVENT_GFX_KANTO_SWIMMER_F_LAND, OBJ_EVENT_GFX_KANTO_LITTLE_BOY, TRUE},
    [TT_CLASS_SUPER_NERD]    = {TRAINER_CLASS_SUPER_NERD,    TRAINER_PIC_KANTO_SUPER_NERD,       OBJ_EVENT_GFX_KANTO_POKE_MANIAC,    0, FALSE},
    [TT_CLASS_SWIMMER_F]     = {TRAINER_CLASS_SWIMMER_F,     TRAINER_PIC_KANTO_SWIMMER_F,        OBJ_EVENT_GFX_KANTO_SWIMMER_F_LAND, 0, TRUE},
    [TT_CLASS_SWIMMER_M]     = {TRAINER_CLASS_SWIMMER_M,     TRAINER_PIC_KANTO_SWIMMER_M,        OBJ_EVENT_GFX_KANTO_SWIMMER_M_LAND, 0, FALSE},
    [TT_CLASS_TUBER]         = {TRAINER_CLASS_TUBER,         TRAINER_PIC_KANTO_TUBER_F,          OBJ_EVENT_GFX_KANTO_TUBER_F,        0, TRUE},
    [TT_CLASS_TWINS]         = {TRAINER_CLASS_TWINS,         TRAINER_PIC_KANTO_TWINS,            OBJ_EVENT_GFX_KANTO_LITTLE_GIRL,    OBJ_EVENT_GFX_KANTO_LITTLE_GIRL, TRUE},
    [TT_CLASS_YOUNGSTER]     = {TRAINER_CLASS_YOUNGSTER,     TRAINER_PIC_KANTO_YOUNGSTER,        OBJ_EVENT_GFX_KANTO_YOUNGSTER,      0, FALSE},
    [TT_CLASS_YOUNG_COUPLE]  = {TRAINER_CLASS_YOUNG_COUPLE,  TRAINER_PIC_KANTO_YOUNG_COUPLE,     OBJ_EVENT_GFX_KANTO_BEAUTY,         OBJ_EVENT_GFX_KANTO_MAN, TRUE},
};

static const u16 sPrizeList[] = {
    [TTPRIZE_HP_UP]         = ITEM_HP_UP,
    [TTPRIZE_PROTEIN]       = ITEM_PROTEIN,
    [TTPRIZE_IRON]          = ITEM_IRON,
    [TTPRIZE_CARBOS]        = ITEM_CARBOS,
    [TTPRIZE_CALCIUM]       = ITEM_CALCIUM,
    [TTPRIZE_ZINC]          = ITEM_ZINC,
    [TTPRIZE_BRIGHT_POWDER] = ITEM_BRIGHT_POWDER,
    [TTPRIZE_WHITE_HERB]    = ITEM_WHITE_HERB,
    [TTPRIZE_MENTAL_HERB]   = ITEM_MENTAL_HERB,
    [TTPRIZE_CHOICE_BAND]   = ITEM_CHOICE_BAND,
    [TTPRIZE_KINGS_ROCK]    = ITEM_KINGS_ROCK,
    [TTPRIZE_SCOPE_LENS]    = ITEM_SCOPE_LENS,
    [TTPRIZE_METAL_COAT]    = ITEM_METAL_COAT,
    [TTPRIZE_DRAGON_SCALE]  = ITEM_DRAGON_SCALE,
    [TTPRIZE_UP_GRADE]      = ITEM_UP_GRADE,
};

// Which of a trainer's six POKéMON battle, by the number of floors cleared
static const u8 sSingleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 1}, {5, 2}, {0, 3}, {1, 4}
};

static const u8 sDoubleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 1}, {1, 3}, {2, 0}, {3, 4}, {4, 2}, {5, 2}, {0, 3}, {1, 5}
};

static const u8 sKnockoutChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][3] = {
    {0, 2, 4}, {1, 3, 5}, {2, 3, 1}, {3, 4, 0}, {4, 1, 2}, {5, 0, 3}, {0, 5, 2}, {1, 4, 5}
};

static const u16 sFloorLayouts[MAX_TRAINER_TOWER_FLOORS][NUM_TOWER_CHALLENGE_TYPES - 1] = {
    {LAYOUT_TRAINER_TOWER_1F, LAYOUT_TRAINER_TOWER_1F_DOUBLES, LAYOUT_TRAINER_TOWER_1F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_2F, LAYOUT_TRAINER_TOWER_2F_DOUBLES, LAYOUT_TRAINER_TOWER_2F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_3F, LAYOUT_TRAINER_TOWER_3F_DOUBLES, LAYOUT_TRAINER_TOWER_3F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_4F, LAYOUT_TRAINER_TOWER_4F_DOUBLES, LAYOUT_TRAINER_TOWER_4F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_5F, LAYOUT_TRAINER_TOWER_5F_DOUBLES, LAYOUT_TRAINER_TOWER_5F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_6F, LAYOUT_TRAINER_TOWER_6F_DOUBLES, LAYOUT_TRAINER_TOWER_6F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_7F, LAYOUT_TRAINER_TOWER_7F_DOUBLES, LAYOUT_TRAINER_TOWER_7F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_8F, LAYOUT_TRAINER_TOWER_8F_DOUBLES, LAYOUT_TRAINER_TOWER_8F_KNOCKOUT},
};

static const struct WindowTemplate sTimeBoardWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 3,
    .tilemapTop = 1,
    .width = 27,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x001,
};

// The opponent of the battle under way (VAR_TEMP_1: the trainer on the floor)
static EWRAM_DATA bool8 sInBattle = FALSE;
static EWRAM_DATA u8 sBattleTrainerId = 0;

static const struct TrainerTowerFloor *GetFloorData(u8 floorIdx)
{
    u8 challengeId = TOWER.challengeId < NUM_TOWER_CHALLENGE_TYPES ? TOWER.challengeId : 0;
    return sTrainerTowerFloors[challengeId][floorIdx < MAX_TRAINER_TOWER_FLOORS ? floorIdx : 0];
}

// 0-7 on MAP_TRAINER_TOWER_1F-8F, MAX_TRAINER_TOWER_FLOORS elsewhere
static u8 GetCurrentFloorIdx(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;

    if (map >= MAP_TRAINER_TOWER_1F && map <= MAP_TRAINER_TOWER_8F)
        return map - MAP_TRAINER_TOWER_1F;
    return MAX_TRAINER_TOWER_FLOORS;
}

#define CURR_FLOOR (*GetFloorData(GetCurrentFloorIdx()))

static const struct TowerClassInfo *GetClassInfo(u8 facilityClass)
{
    return &sTowerClassInfo[facilityClass < NUM_TT_CLASSES ? facilityClass : TT_CLASS_YOUNGSTER];
}

// FRLG's TT_ConvertEasyChatMessageToString: 3x2 words, or 2x3 when that's too wide
static void ConvertSpeech(const u16 *ecWords, u8 *dest)
{
    s32 i;

    ConvertEasyChatWordsToString(dest, ecWords, 3, 2);
    if ((unsigned)GetStringWidth(FONT_NORMAL, dest, -1) > 196)
    {
        ConvertEasyChatWordsToString(dest, ecWords, 2, 3);
        i = 0;
        while (dest[i++] != CHAR_NEWLINE)
            ;
        while (dest[i] != CHAR_NEWLINE)
            i++;
        dest[i] = CHAR_PROMPT_SCROLL;
    }
}

static u32 GetRecord(u8 challengeId)
{
    u32 time = TOWER.bestTimes[challengeId];
    return time != 0 ? time : TRAINER_TOWER_MAX_TIME;
}

static void PrintTowerTime(u32 frames)
{
    s32 minutes, seconds, centiseconds;

    minutes = frames / (60 * 60);
    frames %= (60 * 60);
    seconds = frames / 60;
    frames %= 60;
    centiseconds = frames * 168 / 100;
    ConvertIntToDecimalStringN(gStringVar1, minutes, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, seconds, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, centiseconds, STR_CONV_MODE_LEADING_ZEROS, 2);
}

// ---------------------------------------------------------------------------------------------
// Script functions (VAR_0x8004: TRAINER_TOWER_FUNC_*, see the ttower_* macros)

static void InitTrainerTowerFloor(void)
{
    u8 floorIdx = GetCurrentFloorIdx();
    const struct TrainerTowerFloor *floor;
    const struct TowerClassInfo *info;
    s32 j;

    if (floorIdx >= MAX_TRAINER_TOWER_FLOORS)
    {
        gSpecialVar_Result = NUM_TOWER_CHALLENGE_TYPES - 1; // no trainers
        return;
    }
    floor = GetFloorData(floorIdx);
    gSpecialVar_Result = floor->challengeType;
    SetCurrentMapLayout(sFloorLayouts[floorIdx][floor->challengeType]);
    switch (floor->challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
        VarSet(VAR_OBJ_GFX_ID_1, GetClassInfo(floor->trainers[0].facilityClass)->objGfx);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        info = GetClassInfo(floor->trainers[0].facilityClass);
        VarSet(VAR_OBJ_GFX_ID_0, info->objGfx);
        VarSet(VAR_OBJ_GFX_ID_3, info->objGfx2 != 0 ? info->objGfx2 : info->objGfx);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        for (j = 0; j < MAX_TRAINERS_PER_FLOOR; j++)
        {
            static const u16 sVars[MAX_TRAINERS_PER_FLOOR] = {VAR_OBJ_GFX_ID_2, VAR_OBJ_GFX_ID_0, VAR_OBJ_GFX_ID_1};
            VarSet(sVars[j], GetClassInfo(floor->trainers[j].facilityClass)->objGfx);
        }
        break;
    }
}

// VAR_0x8005: TRAINER_TOWER_TEXT_*, VAR_0x8006: the trainer
static void BufferTowerOpponentSpeech(void)
{
    const struct TrainerTowerTrainer *trainer = &CURR_FLOOR.trainers[gSpecialVar_0x8006 % MAX_TRAINERS_PER_FLOOR];

    switch (gSpecialVar_0x8005)
    {
    case TRAINER_TOWER_TEXT_INTRO:
        ConvertSpeech(trainer->speechBefore, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_LOST:
        ConvertSpeech(trainer->speechWin, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_WON:
        ConvertSpeech(trainer->speechLose, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_AFTER:
        ConvertSpeech(trainer->speechAfter, gStringVar4);
        break;
    }
}

static s32 GetPartyMaxLevel(void)
{
    s32 i, topLevel = 0;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) != SPECIES_EGG)
        {
            s32 level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
            if (level > topLevel)
                topLevel = level;
        }
    }
    return topLevel;
}

static void CreateTowerMon(struct Pokemon *dest, const struct BattleTowerPokemon *src, u8 level)
{
    struct BattleTowerPokemon mon = *src;

    mon.level = level;
    CreateBattleTowerMon(dest, &mon);
}

static void BuildEnemyParty(void)
{
    const struct TrainerTowerFloor *floor = &CURR_FLOOR;
    u8 trainerIdx = VarGet(VAR_TEMP_1) % MAX_TRAINERS_PER_FLOOR;
    u8 floorsCleared = TOWER.floorsCleared % MAX_TRAINER_TOWER_FLOORS;
    s32 level = GetPartyMaxLevel();
    s32 i;

    ZeroEnemyPartyMons();
    switch (floor->challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
    default:
        for (i = 0; i < 2; i++)
            CreateTowerMon(&gEnemyParty[i], &floor->trainers[trainerIdx].mons[sSingleBattleChallengeMonIdxs[floorsCleared][i]], level);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        CreateTowerMon(&gEnemyParty[0], &floor->trainers[0].mons[sDoubleBattleChallengeMonIdxs[floorsCleared][0]], level);
        CreateTowerMon(&gEnemyParty[1], &floor->trainers[1].mons[sDoubleBattleChallengeMonIdxs[floorsCleared][1]], level);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        CreateTowerMon(&gEnemyParty[0], &floor->trainers[trainerIdx].mons[sKnockoutChallengeMonIdxs[floorsCleared][trainerIdx]], level);
        break;
    }
}

static void CB2_EndTrainerTowerBattle(void)
{
    sInBattle = FALSE;
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_DoTrainerTowerBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndTrainerTowerBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

static void DoTrainerTowerBattle(void)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_HILL;
    if (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
    gTrainerBattleOpponent_A = 1;  // Trainer Hill's battle functions take ids from 1
    sBattleTrainerId = (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE) ? 0 : VarGet(VAR_TEMP_1) % MAX_TRAINERS_PER_FLOOR;
    sInBattle = TRUE;
    BuildEnemyParty();
    CreateTask(Task_DoTrainerTowerBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(B_TRANSITION_BIG_POKEBALL);
}

static void TrainerTowerGetChallengeType(void)
{
    if (!gSpecialVar_0x8005)
        gSpecialVar_Result = CURR_FLOOR.challengeType;
}

static void TrainerTowerAddFloorCleared(void)
{
    TOWER.floorsCleared++;
}

// So the player can go back through beaten floors (to the POKéMON CENTER, or to give up)
static void GetFloorAlreadyCleared(void)
{
    gSpecialVar_Result = (GetCurrentFloorIdx() != TOWER.floorsCleared);
}

static void StartTrainerTowerChallenge(void)
{
    TOWER.challengeId = gSpecialVar_0x8005 < NUM_TOWER_CHALLENGE_TYPES ? gSpecialVar_0x8005 : 0;
    TOWER.floorsCleared = 0;
    TOWER.timer = 0;
    TOWER.spokeToOwner = FALSE;
    TOWER.checkedFinalTime = FALSE;
    TOWER.hasLost = FALSE;
    SetTrainerHillVBlankCounter(&TOWER.timer);
}

static void GetOwnerState(void)
{
    ClearTrainerHillVBlankCounter();
    gSpecialVar_Result = 0;
    if (TOWER.spokeToOwner)
        gSpecialVar_Result++;
    if ((TOWER.receivedPrizes & (1 << TOWER.challengeId)) && TOWER.checkedFinalTime)
        gSpecialVar_Result++;
    TOWER.spokeToOwner = TRUE;
}

static void GiveChallengePrize(void)
{
    u16 itemId = sPrizeList[GetFloorData(0)->prize];

    if (TOWER.receivedPrizes & (1 << TOWER.challengeId))
    {
        gSpecialVar_Result = 2;
    }
    else if (AddBagItem(itemId, 1) == TRUE)
    {
        CopyItemName(itemId, gStringVar2);
        TOWER.receivedPrizes |= 1 << TOWER.challengeId;
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
}

static void CheckFinalTime(void)
{
    if (TOWER.checkedFinalTime)
    {
        gSpecialVar_Result = 2;
    }
    else if (GetRecord(TOWER.challengeId) > TOWER.timer)
    {
        TOWER.bestTimes[TOWER.challengeId] = TOWER.timer != 0 ? TOWER.timer : 1;
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
    TOWER.checkedFinalTime = TRUE;
}

static void TrainerTowerResumeTimer(void)
{
    if (!TOWER.spokeToOwner)
    {
        if (TOWER.timer >= TRAINER_TOWER_MAX_TIME)
            TOWER.timer = TRAINER_TOWER_MAX_TIME;
        else
            SetTrainerHillVBlankCounter(&TOWER.timer);
    }
}

static void TrainerTowerSetPlayerLost(void)
{
    TOWER.hasLost = TRUE;
}

static void GetTrainerTowerChallengeStatus(void)
{
    if (TOWER.hasLost)
    {
        TOWER.hasLost = FALSE;
        gSpecialVar_Result = TT_STATUS_LOST;
    }
    else
    {
        gSpecialVar_Result = TT_STATUS_NORMAL;
    }
}

static void GetCurrentTime(void)
{
    if (TOWER.timer >= TRAINER_TOWER_MAX_TIME)
    {
        ClearTrainerHillVBlankCounter();
        TOWER.timer = TRAINER_TOWER_MAX_TIME;
    }
    PrintTowerTime(TOWER.timer);
}

static void ShowResultsBoard(void)
{
    u8 windowId;
    s32 i;

    windowId = AddWindow(&sTimeBoardWindowTemplate);
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_KantoTowerTimeBoard, 74, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PrintTowerTime(GetRecord(i));
        StringExpandPlaceholders(gStringVar4, gText_KantoTowerXMinYZSec);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gKantoTowerChallengeTypeTexts[i], 24, 36 + 20 * i, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 96, 36 + 20 * i, TEXT_SKIP_DRAW, NULL);
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    VarSet(VAR_TEMP_1, windowId);
}

static void CloseResultsBoard(void)
{
    u8 windowId = VarGet(VAR_TEMP_1);

    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
}

static void TrainerTowerGetDoublesEligiblity(void)
{
    gSpecialVar_Result = GetMonsStateToDoubles();
}

// FALSE: all eight floors (the built-in sets always have them)
static void TrainerTowerGetNumFloors(void)
{
    gSpecialVar_Result = FALSE;
}

static void ShouldWarpToCounter(void)
{
    gSpecialVar_Result = FALSE;
}

static void PlayTrainerTowerEncounterMusic(void)
{
    const struct TowerClassInfo *info = GetClassInfo(CURR_FLOOR.trainers[VarGet(VAR_TEMP_1) % MAX_TRAINERS_PER_FLOOR].facilityClass);

    PlayNewMapMusic(info->female ? MUS_RG_ENCOUNTER_GIRL : MUS_RG_ENCOUNTER_BOY);
}

static void HasSpokenToOwner(void)
{
    gSpecialVar_Result = TOWER.spokeToOwner;
}

static void (*const sTrainerTowerFunctions[])(void) = {
    [TRAINER_TOWER_FUNC_INIT_FLOOR]             = InitTrainerTowerFloor,
    [TRAINER_TOWER_FUNC_GET_SPEECH]             = BufferTowerOpponentSpeech,
    [TRAINER_TOWER_FUNC_DO_BATTLE]              = DoTrainerTowerBattle,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE]     = TrainerTowerGetChallengeType,
    [TRAINER_TOWER_FUNC_CLEARED_FLOOR]          = TrainerTowerAddFloorCleared,
    [TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED]      = GetFloorAlreadyCleared,
    [TRAINER_TOWER_FUNC_START_CHALLENGE]        = StartTrainerTowerChallenge,
    [TRAINER_TOWER_FUNC_GET_OWNER_STATE]        = GetOwnerState,
    [TRAINER_TOWER_FUNC_GIVE_PRIZE]             = GiveChallengePrize,
    [TRAINER_TOWER_FUNC_CHECK_FINAL_TIME]       = CheckFinalTime,
    [TRAINER_TOWER_FUNC_RESUME_TIMER]           = TrainerTowerResumeTimer,
    [TRAINER_TOWER_FUNC_SET_LOST]               = TrainerTowerSetPlayerLost,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS]   = GetTrainerTowerChallengeStatus,
    [TRAINER_TOWER_FUNC_GET_TIME]               = GetCurrentTime,
    [TRAINER_TOWER_FUNC_SHOW_RESULTS]           = ShowResultsBoard,
    [TRAINER_TOWER_FUNC_CLOSE_RESULTS]          = CloseResultsBoard,
    [TRAINER_TOWER_FUNC_CHECK_DOUBLES]          = TrainerTowerGetDoublesEligiblity,
    [TRAINER_TOWER_FUNC_GET_NUM_FLOORS]         = TrainerTowerGetNumFloors,
    [TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER] = ShouldWarpToCounter,
    [TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC]        = PlayTrainerTowerEncounterMusic,
    [TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE]     = HasSpokenToOwner,
};

void CallTrainerTowerFunc(void)
{
    if (gSpecialVar_0x8004 < ARRAY_COUNT(sTrainerTowerFunctions))
        sTrainerTowerFunctions[gSpecialVar_0x8004]();
}

void ResetTrainerTowerResults(void)
{
    memset(&TOWER, 0, sizeof(TOWER));
}

// ---------------------------------------------------------------------------------------------
// The battle (called by Trainer Hill's battle functions, src/trainer_hill.c)

bool8 KantoTower_InBattle(void)
{
    return sInBattle;
}

static const struct TrainerTowerTrainer *GetBattleTrainer(void)
{
    return &CURR_FLOOR.trainers[sBattleTrainerId];
}

u8 KantoTower_GetOpponentClass(void)
{
    return GetClassInfo(GetBattleTrainer()->facilityClass)->trainerClass;
}

void KantoTower_GetOpponentName(u8 *dst)
{
    StringCopyN(dst, GetBattleTrainer()->name, TRAINER_NAME_LENGTH + 1);
    dst[TRAINER_NAME_LENGTH] = EOS;
}

u8 KantoTower_GetFrontPic(void)
{
    return GetClassInfo(GetBattleTrainer()->facilityClass)->trainerPic;
}

// The opponent's line when the battle ends
void KantoTower_GetBattleText(u8 *dst, bool8 playerWon)
{
    const struct TrainerTowerTrainer *trainer = GetBattleTrainer();
    ConvertSpeech(playerWon ? trainer->speechLose : trainer->speechWin, dst);
}

// The timer runs through the battle
void KantoTower_InitBattle(void)
{
    SetTrainerHillVBlankCounter(&TOWER.timer);
}

void KantoTower_EndBattle(void)
{
}
