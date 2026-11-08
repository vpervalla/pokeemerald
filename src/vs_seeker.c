// The VS SEEKER (FRLG's, from pret/pokefirered's src/vs_seeker.c).
//
// Once its battery is charged (VSSEEKER_RECHARGE_STEPS steps with it in the bag), using it beeps
// at the Kanto trainers on screen. Those not fought yet show "!", those who don't want a rematch
// an X, and the others (70% of them) raise a hand: talked to, they battle with their next party
// (sRematches). The battery then charges again; once it is full, the trainers who raised a hand
// but weren't battled stop waiting. Leaving the map stops them too.
//
// Only the Kanto trainers have rematch parties here: the Hoenn ones have Match Call's rematches.

#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "item_menu.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "script_movement.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "vs_seeker.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/opponents.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"

// Each trainer can have up to 6 parties, its first one included. Each rematch party is only
// offered once the story has got far enough (see TryGetRematchTrainerIdGivenGameState). A stage
// at which a trainer's party doesn't change is SKIP in sRematches.
#define MAX_REMATCH_PARTIES 6
#define SKIP 0xFFFF

#define VSSEEKER_RECHARGE_STEPS 100

#define NO_REMATCH_LOCALID LOCALID_PLAYER

#define SCRIPT_OP_TRAINERBATTLE 0x5C // SCR_OP_TRAINERBATTLE in data/script_cmd_table.inc

enum
{
    VSSEEKER_NOT_CHARGED,
    VSSEEKER_NO_ONE_IN_RANGE,
    VSSEEKER_CAN_USE,
};

enum
{
    VSSEEKER_SINGLE_RESP_RAND,
    VSSEEKER_SINGLE_RESP_NO,
    VSSEEKER_SINGLE_RESP_YES,
};

enum
{
    VSSEEKER_RESPONSE_NO_RESPONSE,
    VSSEEKER_RESPONSE_UNFOUGHT_TRAINERS,
    VSSEEKER_RESPONSE_FOUND_REMATCHES,
};

struct RematchData
{
    u16 trainerIdxs[MAX_REMATCH_PARTIES];
};

struct VsSeekerTrainerInfo
{
    const u8 *script;
    u16 trainerIdx;
    u8 localId;
    u8 objectEventId;
    s16 xCoord;
    s16 yCoord;
    u16 graphicsId;
};

struct VsSeekerStruct
{
    struct VsSeekerTrainerInfo trainerInfo[OBJECT_EVENT_TEMPLATES_COUNT + 1];
    u16 trainerIdxArray[OBJECT_EVENTS_COUNT];
    u8 runningBehaviourEtcArray[OBJECT_EVENTS_COUNT];
    u8 numRematchableTrainers;
    u8 trainerHasNotYetBeenFought:1;
    u8 trainerDoesNotWantRematch:1;
    u8 trainerWantsRematch:1;
    u8 responseCode:5;
};

static EWRAM_DATA struct VsSeekerStruct *sVsSeeker = NULL;

static void Task_VsSeeker_PlayerAnim(u8 taskId);
static void Task_VsSeeker_Beep(u8 taskId);
static void Task_VsSeeker_WaitResponses(u8 taskId);
static void Task_VsSeeker_CloseMessage(u8 taskId);
static void GatherNearbyTrainerInfo(void);
static u8 CanUseVsSeeker(void);
static u8 GetVsSeekerResponseInArea(void);
static u8 GetRematchTrainerIdGivenGameState(const u16 *trainerIdxs, u8 rematchIdx);
static int LookupVsSeekerOpponentInArray(u16 trainerId);
static int GetRematchIdx(u16 trainerFlagIdx);
static u16 GetTrainerFlagFromScript(const u8 *script);
static bool32 IsThisTrainerRematchable(u32 localId);
static void SetRematchWanted(u32 localId, bool32 wanted);
static void ClearAllTrainerRematchStates(void);
static bool8 IsTrainerVisibleOnScreen(struct VsSeekerTrainerInfo *trainerInfo);
static u8 GetNextAvailableRematchTrainer(u16 trainerFlagNo, u8 *idxPtr);
static u8 GetRematchableTrainerLocalId(void);
static void StartTrainerObjectMovementScript(struct VsSeekerTrainerInfo *trainerInfo, const u8 *script);
static u8 GetCurVsSeekerResponse(s32 vsSeekerIdx, u16 trainerIdx);
static void StartAllRespondantIdleMovements(void);
static bool8 ObjectEventIdIsSane(u8 objectEventId);
static u8 GetRandomFaceDirectionMovementType(void);
static void ResetMovementOfRematchableTrainers(void);

#include "data/vs_seeker_rematches.h"

static const u8 sText_BatteryNotChargedNeedXSteps[] = _(
    "The battery isn't charged enough.\p"
    "No. of steps required to fully\n"
    "charge the battery: {STR_VAR_1}{PAUSE_UNTIL_PRESS}");

static const u8 sText_NoTrainersWithinRange[] = _(
    "There are no TRAINERS within range\n"
    "who can battle…\p"
    "The VS SEEKER was turned off.{PAUSE_UNTIL_PRESS}");

static const u8 sText_TrainersNotReady[] = _(
    "The other TRAINERS don't appear\n"
    "to be ready for battle.\p"
    "Let's wait till later.{PAUSE_UNTIL_PRESS}");

static const u8 sMovementScript_Wait48[] = {
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_DELAY_16,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerUnfought[] = {
    MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerNoRematch[] = {
    MOVEMENT_ACTION_EMOTE_X,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sMovementScript_TrainerRematch[] = {
    MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN,
    MOVEMENT_ACTION_EMOTE_DOUBLE_EXCL_MARK,
    MOVEMENT_ACTION_STEP_END
};

static const u8 sFaceDirectionMovementTypeByFacingDirection[] = {
    MOVEMENT_TYPE_FACE_DOWN,
    MOVEMENT_TYPE_FACE_DOWN,
    MOVEMENT_TYPE_FACE_UP,
    MOVEMENT_TYPE_FACE_LEFT,
    MOVEMENT_TYPE_FACE_RIGHT
};

#define sVsSeekerSave (gPokemonStoragePtr->vsSeeker)

static bool32 IsRaiseHandMovementType(u8 movementType)
{
    return movementType == MOVEMENT_TYPE_RAISE_HAND_AND_STOP
        || movementType == MOVEMENT_TYPE_RAISE_HAND_AND_JUMP
        || movementType == MOVEMENT_TYPE_RAISE_HAND_AND_SWIM;
}

// The Hoenn trainers don't answer: Match Call has their rematches
static bool32 IsKantoTrainer(u16 trainerId)
{
    return trainerId >= KANTO_TRAINERS_START && trainerId < ALL_TRAINERS_COUNT;
}

static bool32 IsTrainerTemplate(const struct ObjectEventTemplate *template)
{
    return (template->trainerType == TRAINER_TYPE_NORMAL || template->trainerType == TRAINER_TYPE_BURIED)
        && template->script != NULL && template->script[0] == SCRIPT_OP_TRAINERBATTLE;
}

// EventScript_VsSeekerChargingDone: the trainers who raised a hand go back to facing a direction
void VsSeekerResetObjectMovementAfterChargeComplete(void)
{
    struct ObjectEventTemplate *templates = gSaveBlock1Ptr->objectEventTemplates;
    u8 i;
    u8 movementType;
    u8 objEventId;

    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        if (IsTrainerTemplate(&templates[i]) && IsRaiseHandMovementType(templates[i].movementType))
        {
            movementType = GetRandomFaceDirectionMovementType();
            if (!TryGetObjectEventIdByLocalIdAndMap(templates[i].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objEventId)
             && ObjectEventIdIsSane(objEventId) == TRUE)
            {
                gSprites[gObjectEvents[objEventId].spriteId].x2 = 0;
                gSprites[gObjectEvents[objEventId].spriteId].y2 = 0;
                SetTrainerMovementType(&gObjectEvents[objEventId], movementType);
            }
            templates[i].movementType = movementType;
        }
    }
}

// Each step: charges the battery. Returns TRUE when the trainers who answered stop waiting.
bool8 UpdateVsSeekerStepCounter(void)
{
    if (CheckBagHasItem(ITEM_VS_SEEKER, 1) == TRUE && sVsSeekerSave.bagSteps < VSSEEKER_RECHARGE_STEPS)
        sVsSeekerSave.bagSteps++;

    if (sVsSeekerSave.charging)
    {
        if (sVsSeekerSave.chargeSteps < VSSEEKER_RECHARGE_STEPS)
            sVsSeekerSave.chargeSteps++;
        if (sVsSeekerSave.chargeSteps >= VSSEEKER_RECHARGE_STEPS)
        {
            sVsSeekerSave.charging = FALSE;
            sVsSeekerSave.chargeSteps = 0;
            ClearAllTrainerRematchStates();
            return TRUE;
        }
    }
    return FALSE;
}

// A new map: the trainers of the last one stop waiting
void MapResetTrainerRematches(void)
{
    sVsSeekerSave.charging = FALSE;
    sVsSeekerSave.chargeSteps = 0;
    ClearAllTrainerRematchStates();
    ResetMovementOfRematchableTrainers();
}

static void ResetMovementOfRematchableTrainers(void)
{
    u8 i;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *objectEvent = &gObjectEvents[i];
        if (objectEvent->active && IsRaiseHandMovementType(objectEvent->movementType))
        {
            u8 movementType = GetRandomFaceDirectionMovementType();
            if (gSprites[objectEvent->spriteId].data[0] == i)
            {
                gSprites[objectEvent->spriteId].x2 = 0;
                gSprites[objectEvent->spriteId].y2 = 0;
                SetTrainerMovementType(objectEvent, movementType);
            }
        }
    }
}

// The bag's (or SELECT's) VS SEEKER
void Task_VsSeeker_0(u8 taskId)
{
    u8 i;
    u8 respval;

    for (i = 0; i < NUM_TASK_DATA; i++)
        gTasks[taskId].data[i] = 0;

    LockPlayerFieldControls();
    FreezeObjectEvents();
    PlayerFreeze();
    StopPlayerAvatar();

    sVsSeeker = AllocZeroed(sizeof(struct VsSeekerStruct));
    GatherNearbyTrainerInfo();
    respval = CanUseVsSeeker();
    if (respval == VSSEEKER_NOT_CHARGED)
    {
        FREE_AND_SET_NULL(sVsSeeker);
        StringExpandPlaceholders(gStringVar4, sText_BatteryNotChargedNeedXSteps);
        DisplayItemMessageOnField(taskId, gStringVar4, Task_VsSeeker_CloseMessage);
    }
    else if (respval == VSSEEKER_NO_ONE_IN_RANGE)
    {
        FREE_AND_SET_NULL(sVsSeeker);
        DisplayItemMessageOnField(taskId, sText_NoTrainersWithinRange, Task_VsSeeker_CloseMessage);
    }
    else
    {
        gPlayerAvatar.preventStep = TRUE;
        gTasks[taskId].func = Task_VsSeeker_PlayerAnim;
    }
}

#define tState    data[0]
#define tTimer    data[1]
#define tBeeps    data[2]
#define tSavedGfx data[3]

// FRLG's FLDEFF_USE_VS_SEEKER: the player holds up the VS SEEKER, which beeps three times
static void Task_VsSeeker_PlayerAnim(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    struct ObjectEvent *playerObj = &gObjectEvents[gPlayerAvatar.objectEventId];

    switch (tState)
    {
    case 0:
        if (!ObjectEventIsMovementOverridden(playerObj) || ObjectEventClearHeldMovementIfFinished(playerObj))
        {
            tSavedGfx = GetObjectEventGraphicsId(playerObj);
            // On the water the player just beeps it
            if (!(gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING))
            {
                ObjectEventSetGraphicsId(playerObj, OBJ_EVENT_GFX_PLAYER_VS_SEEKER);
                StartSpriteAnim(&gSprites[gPlayerAvatar.spriteId], ANIM_VS_SEEKER);
                ObjectEventSetHeldMovement(playerObj, MOVEMENT_ACTION_START_ANIM_IN_DIRECTION);
            }
            tTimer = 16;
            tState++;
        }
        break;
    case 1:
        if (tBeeps < 3 && --tTimer <= 0)
        {
            PlaySE(SE_CONTEST_MONS_TURN);
            tTimer = 11;
            tBeeps++;
        }
        // The pose lasts past the third beep; on the water (no pose) wait for the beeps
        if (tBeeps == 3 && (!ObjectEventIsMovementOverridden(playerObj) || ObjectEventClearHeldMovementIfFinished(playerObj)))
        {
            if (GetObjectEventGraphicsId(playerObj) != tSavedGfx)
                ObjectEventSetGraphicsId(playerObj, tSavedGfx);
            ObjectEventForceSetHeldMovement(playerObj, GetFaceDirectionMovementAction(playerObj->facingDirection));
            tState++;
        }
        break;
    case 2:
        if (ObjectEventClearHeldMovementIfFinished(playerObj))
        {
            gPlayerAvatar.preventStep = FALSE;
            gTasks[taskId].func = Task_VsSeeker_Beep;
        }
        break;
    }
}

static void Task_VsSeeker_Beep(u8 taskId)
{
    sVsSeekerSave.bagSteps = 0;
    sVsSeeker->responseCode = GetVsSeekerResponseInArea();
    ScriptMovement_StartObjectMovementScript(LOCALID_PLAYER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sMovementScript_Wait48);
    gTasks[taskId].func = Task_VsSeeker_WaitResponses;
}

#undef tState
#undef tTimer
#undef tBeeps
#undef tSavedGfx

static void GatherNearbyTrainerInfo(void)
{
    struct ObjectEventTemplate *templates = gSaveBlock1Ptr->objectEventTemplates;
    u8 objectEventId = 0;
    u8 vsSeekerObjectIdx = 0;
    s32 objectEventIdx;

    for (objectEventIdx = 0; objectEventIdx < gMapHeader.events->objectEventCount; objectEventIdx++)
    {
        if (IsTrainerTemplate(&templates[objectEventIdx])
         && !TryGetObjectEventIdByLocalIdAndMap(templates[objectEventIdx].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objectEventId))
        {
            struct VsSeekerTrainerInfo *info = &sVsSeeker->trainerInfo[vsSeekerObjectIdx];
            info->script = templates[objectEventIdx].script;
            info->trainerIdx = GetTrainerFlagFromScript(templates[objectEventIdx].script);
            info->localId = templates[objectEventIdx].localId;
            info->objectEventId = objectEventId;
            info->xCoord = gObjectEvents[objectEventId].currentCoords.x - MAP_OFFSET;
            info->yCoord = gObjectEvents[objectEventId].currentCoords.y - MAP_OFFSET;
            info->graphicsId = GetTemplateGraphicsId(&templates[objectEventIdx]);
            vsSeekerObjectIdx++;
        }
    }
    sVsSeeker->trainerInfo[vsSeekerObjectIdx].localId = NO_REMATCH_LOCALID;
}

static void Task_VsSeeker_WaitResponses(u8 taskId)
{
    if (ScriptMovement_IsObjectMovementFinished(LOCALID_PLAYER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup))
    {
        if (sVsSeeker->responseCode == VSSEEKER_RESPONSE_NO_RESPONSE)
        {
            DisplayItemMessageOnField(taskId, sText_TrainersNotReady, Task_VsSeeker_CloseMessage);
        }
        else
        {
            if (sVsSeeker->responseCode == VSSEEKER_RESPONSE_FOUND_REMATCHES)
                StartAllRespondantIdleMovements();
            ClearDialogWindowAndFrame(0, TRUE);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        FREE_AND_SET_NULL(sVsSeeker);
    }
}

static void Task_VsSeeker_CloseMessage(u8 taskId)
{
    ClearDialogWindowAndFrame(0, TRUE);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static u8 CanUseVsSeeker(void)
{
    if (sVsSeekerSave.bagSteps >= VSSEEKER_RECHARGE_STEPS)
    {
        if (GetRematchableTrainerLocalId() == NO_REMATCH_LOCALID)
            return VSSEEKER_NO_ONE_IN_RANGE;
        else
            return VSSEEKER_CAN_USE;
    }
    ConvertIntToDecimalStringN(gStringVar1, VSSEEKER_RECHARGE_STEPS - sVsSeekerSave.bagSteps, STR_CONV_MODE_LEFT_ALIGN, 3);
    return VSSEEKER_NOT_CHARGED;
}

static u8 GetVsSeekerResponseInArea(void)
{
    u16 trainerIdx = 0;
    u16 rval = 0;
    u8 rematchTrainerIdx;
    u8 unusedIdx = 0;
    u8 response = 0;
    s32 vsSeekerIdx = 0;

    while (sVsSeeker->trainerInfo[vsSeekerIdx].localId != NO_REMATCH_LOCALID)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[vsSeekerIdx]) == TRUE
         && IsKantoTrainer(sVsSeeker->trainerInfo[vsSeekerIdx].trainerIdx))
        {
            trainerIdx = sVsSeeker->trainerInfo[vsSeekerIdx].trainerIdx;
            if (!HasTrainerBeenFought(trainerIdx))
            {
                StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerUnfought);
                sVsSeeker->trainerHasNotYetBeenFought = TRUE;
                vsSeekerIdx++;
                continue;
            }
            rematchTrainerIdx = GetNextAvailableRematchTrainer(trainerIdx, &unusedIdx);
            if (rematchTrainerIdx == 0)
            {
                StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerNoRematch);
                sVsSeeker->trainerDoesNotWantRematch = TRUE;
            }
            else
            {
                rval = Random() % 100; // Even if it's overwritten below, it advances the RNG.
                response = GetCurVsSeekerResponse(vsSeekerIdx, trainerIdx);
                if (response == VSSEEKER_SINGLE_RESP_YES)
                    rval = 100; // Definitely yes
                else if (response == VSSEEKER_SINGLE_RESP_NO)
                    rval = 0; // Definitely no
                // Otherwise there's a 70% chance the trainer wants a rematch
                if (rval < 30)
                {
                    StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerNoRematch);
                    sVsSeeker->trainerDoesNotWantRematch = TRUE;
                }
                else if (sVsSeeker->numRematchableTrainers < OBJECT_EVENTS_COUNT)
                {
                    SetRematchWanted(sVsSeeker->trainerInfo[vsSeekerIdx].localId, TRUE);
                    ShiftStillObjectEventCoords(&gObjectEvents[sVsSeeker->trainerInfo[vsSeekerIdx].objectEventId]);
                    StartTrainerObjectMovementScript(&sVsSeeker->trainerInfo[vsSeekerIdx], sMovementScript_TrainerRematch);
                    sVsSeeker->trainerIdxArray[sVsSeeker->numRematchableTrainers] = trainerIdx;
                    sVsSeeker->runningBehaviourEtcArray[sVsSeeker->numRematchableTrainers] = GetRunningBehaviorFromGraphicsId(sVsSeeker->trainerInfo[vsSeekerIdx].graphicsId);
                    sVsSeeker->numRematchableTrainers++;
                    sVsSeeker->trainerWantsRematch = TRUE;
                }
            }
        }
        vsSeekerIdx++;
    }

    if (sVsSeeker->trainerWantsRematch)
    {
        PlaySE(SE_PIN);
        sVsSeekerSave.charging = TRUE;
        sVsSeekerSave.chargeSteps = 0;
        return VSSEEKER_RESPONSE_FOUND_REMATCHES;
    }
    if (sVsSeeker->trainerHasNotYetBeenFought)
        return VSSEEKER_RESPONSE_UNFOUGHT_TRAINERS;
    return VSSEEKER_RESPONSE_NO_RESPONSE;
}

// After a trainer battle won: the trainers of that rematch row stop raising a hand
void ClearRematchStateByTrainerId(void)
{
    u8 objEventId = 0;
    struct ObjectEventTemplate *objectEventTemplates = gSaveBlock1Ptr->objectEventTemplates;
    int vsSeekerDataIdx = LookupVsSeekerOpponentInArray(gTrainerBattleOpponent_A);
    int i;

    if (vsSeekerDataIdx == -1)
        return;

    for (i = 0; i < gMapHeader.events->objectEventCount; i++)
    {
        if (IsTrainerTemplate(&objectEventTemplates[i])
         && vsSeekerDataIdx == LookupVsSeekerOpponentInArray(GetTrainerFlagFromScript(objectEventTemplates[i].script))
         && !TryGetObjectEventIdByLocalIdAndMap(objectEventTemplates[i].localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objEventId))
        {
            struct ObjectEvent *objectEvent = &gObjectEvents[objEventId];
            u8 faceType = sFaceDirectionMovementTypeByFacingDirection[objectEvent->facingDirection];

            objectEventTemplates[i].movementType = faceType;
            SetRematchWanted(objectEventTemplates[i].localId, FALSE);
            if (gSelectedObjectEvent == objEventId)
                objectEvent->movementType = faceType;
            else
                objectEvent->movementType = MOVEMENT_TYPE_FACE_DOWN;
            gSprites[objectEvent->spriteId].x2 = 0;
            gSprites[objectEvent->spriteId].y2 = 0;
        }
    }
}

// Which rematch party the trainer can have at this point of the story
static void TryGetRematchTrainerIdGivenGameState(const u16 *trainerIdxs, u8 *rematchIdx_p)
{
    switch (*rematchIdx_p)
    {
    case 0:
        break;
    case 1:
        if (!FlagGet(FLAG_GOT_VS_SEEKER))
            *rematchIdx_p = GetRematchTrainerIdGivenGameState(trainerIdxs, *rematchIdx_p);
        break;
    case 2:
        if (!FlagGet(FLAG_VISITED_CELADON_CITY))
            *rematchIdx_p = GetRematchTrainerIdGivenGameState(trainerIdxs, *rematchIdx_p);
        break;
    case 3:
        if (!FlagGet(FLAG_VISITED_FUCHSIA_CITY))
            *rematchIdx_p = GetRematchTrainerIdGivenGameState(trainerIdxs, *rematchIdx_p);
        break;
    case 4:
        if (!FlagGet(FLAG_SYS_GAME_CLEAR))
            *rematchIdx_p = GetRematchTrainerIdGivenGameState(trainerIdxs, *rematchIdx_p);
        break;
    case 5:
        if (!FlagGet(FLAG_SYS_CAN_LINK_WITH_RS))
            *rematchIdx_p = GetRematchTrainerIdGivenGameState(trainerIdxs, *rematchIdx_p);
        break;
    }
}

static u8 GetRematchTrainerIdGivenGameState(const u16 *trainerIdxs, u8 rematchIdx)
{
    while (--rematchIdx != 0)
    {
        if (trainerIdxs[rematchIdx] != SKIP)
            return rematchIdx;
    }
    return 0;
}

bool32 IsVsSeekerTrainer(u16 trainerId)
{
    return LookupVsSeekerOpponentInArray(trainerId) != -1;
}

// specialvar ShouldTryRematchBattle, for the Kanto trainers
bool8 VsSeeker_ShouldTryRematchBattle(u16 trainerId)
{
    s32 rematchIdx = GetRematchIdx(trainerId);

    if (rematchIdx == -1)
        return FALSE;
    if (IsThisTrainerRematchable(gSpecialVar_LastTalked))
        return TRUE;
    // The rematch script then gives the after-battle text
    return HasTrainerBeenFought(sRematches[rematchIdx].trainerIdxs[0]);
}

// A rematch battle won
void VsSeeker_ClearRematchStateOfLastTalked(void)
{
    SetRematchWanted(gSpecialVar_LastTalked, FALSE);
}

static int LookupVsSeekerOpponentInArray(u16 trainerId)
{
    int i, j;

    for (i = 0; i < (int)ARRAY_COUNT(sRematches); i++)
    {
        for (j = 0; j < MAX_REMATCH_PARTIES; j++)
        {
            u16 testTrainerId = sRematches[i].trainerIdxs[j];
            if (testTrainerId == TRAINER_NONE)
                break;
            if (testTrainerId == SKIP)
                continue;
            if (testTrainerId == trainerId)
                return i;
        }
    }

    return -1;
}

// trainerbattle_rematch: the party the trainer battles with this time
u16 VsSeeker_GetRematchTrainerId(u16 trainerId)
{
    u8 i;
    u8 j = GetNextAvailableRematchTrainer(trainerId, &i);

    if (!j)
        return TRAINER_NONE;
    TryGetRematchTrainerIdGivenGameState(sRematches[i].trainerIdxs, &j);
    return sRematches[i].trainerIdxs[j];
}

// specialvar IsTrainerReadyForRematch, for the Kanto trainers
bool8 VsSeeker_IsTrainerReadyForRematch(u16 trainerId)
{
    if (LookupVsSeekerOpponentInArray(trainerId) == -1)
        return FALSE;
    return IsThisTrainerRematchable(gSpecialVar_LastTalked);
}

static bool8 ObjectEventIdIsSane(u8 objectEventId)
{
    struct ObjectEvent *objectEvent = &gObjectEvents[objectEventId];

    if (objectEventId < OBJECT_EVENTS_COUNT && objectEvent->active
     && gSprites[objectEvent->spriteId].data[0] == objectEventId)
        return TRUE;
    return FALSE;
}

static u8 GetRandomFaceDirectionMovementType(void)
{
    switch (Random() % 4)
    {
    case 0:
        return MOVEMENT_TYPE_FACE_UP;
    case 1:
        return MOVEMENT_TYPE_FACE_DOWN;
    case 2:
        return MOVEMENT_TYPE_FACE_LEFT;
    default:
        return MOVEMENT_TYPE_FACE_RIGHT;
    }
}

// The trainer id of "trainerbattle TYPE, TRAINER, ..." at the start of the script
static u16 GetTrainerFlagFromScript(const u8 *script)
{
    return script[2] | (script[3] << 8);
}

static int GetRematchIdx(u16 trainerFlagIdx)
{
    int i;

    for (i = 0; i < (int)ARRAY_COUNT(sRematches); i++)
    {
        if (sRematches[i].trainerIdxs[0] == trainerFlagIdx)
            return i;
    }

    return -1;
}

static bool32 IsThisTrainerRematchable(u32 localId)
{
    if (localId >= ARRAY_COUNT(sVsSeekerSave.rematchWanted) * 8)
        return FALSE;
    return (sVsSeekerSave.rematchWanted[localId / 8] >> (localId % 8)) & 1;
}

static void SetRematchWanted(u32 localId, bool32 wanted)
{
    if (localId >= ARRAY_COUNT(sVsSeekerSave.rematchWanted) * 8)
        return;
    if (wanted)
        sVsSeekerSave.rematchWanted[localId / 8] |= 1 << (localId % 8);
    else
        sVsSeekerSave.rematchWanted[localId / 8] &= ~(1 << (localId % 8));
}

static void ClearAllTrainerRematchStates(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sVsSeekerSave.rematchWanted); i++)
        sVsSeekerSave.rematchWanted[i] = 0;
}

static bool8 IsTrainerVisibleOnScreen(struct VsSeekerTrainerInfo *trainerInfo)
{
    s16 x;
    s16 y;

    PlayerGetDestCoords(&x, &y);
    x -= MAP_OFFSET;
    y -= MAP_OFFSET;

    if (x - 7 <= trainerInfo->xCoord
     && x + 7 >= trainerInfo->xCoord
     && y - 5 <= trainerInfo->yCoord
     && y + 5 >= trainerInfo->yCoord
     && ObjectEventIdIsSane(trainerInfo->objectEventId) == TRUE)
        return TRUE;
    return FALSE;
}

// The stage (index in sRematches) of the trainer's next rematch party, or 0 if it has none
static u8 GetNextAvailableRematchTrainer(u16 trainerFlagNo, u8 *idxPtr)
{
    int i, j;

    for (i = 0; i < (int)ARRAY_COUNT(sRematches); i++)
    {
        if (sRematches[i].trainerIdxs[0] == trainerFlagNo)
        {
            *idxPtr = i;
            for (j = 1; j < MAX_REMATCH_PARTIES; j++)
            {
                if (sRematches[i].trainerIdxs[j] == TRAINER_NONE)
                    return j - 1;
                if (sRematches[i].trainerIdxs[j] == SKIP)
                    continue;
                if (HasTrainerBeenFought(sRematches[i].trainerIdxs[j]))
                    continue;
                return j;
            }
            return j - 1;
        }
    }

    *idxPtr = 0;
    return 0;
}

static u8 GetRematchableTrainerLocalId(void)
{
    u8 idx;
    u8 i;

    for (i = 0; sVsSeeker->trainerInfo[i].localId != NO_REMATCH_LOCALID; i++)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[i]) == TRUE)
        {
            u16 trainerIdx = sVsSeeker->trainerInfo[i].trainerIdx;
            if (!IsKantoTrainer(trainerIdx))
                continue;
            if (HasTrainerBeenFought(trainerIdx) != TRUE || GetNextAvailableRematchTrainer(trainerIdx, &idx))
                return sVsSeeker->trainerInfo[i].localId;
        }
    }

    return NO_REMATCH_LOCALID;
}

static void StartTrainerObjectMovementScript(struct VsSeekerTrainerInfo *trainerInfo, const u8 *script)
{
    UnfreezeObjectEvent(&gObjectEvents[trainerInfo->objectEventId]);
    ScriptMovement_StartObjectMovementScript(trainerInfo->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, script);
}

// Two objects of the same trainer (twins) answer the same way
static u8 GetCurVsSeekerResponse(s32 vsSeekerIdx, u16 trainerIdx)
{
    s32 i;
    s32 j;

    for (i = 0; i < vsSeekerIdx; i++)
    {
        if (IsTrainerVisibleOnScreen(&sVsSeeker->trainerInfo[i]) == TRUE && sVsSeeker->trainerInfo[i].trainerIdx == trainerIdx)
        {
            for (j = 0; j < sVsSeeker->numRematchableTrainers; j++)
            {
                if (sVsSeeker->trainerIdxArray[j] == sVsSeeker->trainerInfo[i].trainerIdx)
                    return VSSEEKER_SINGLE_RESP_YES;
            }
            return VSSEEKER_SINGLE_RESP_NO;
        }
    }
    return VSSEEKER_SINGLE_RESP_RAND;
}

static void StartAllRespondantIdleMovements(void)
{
    s32 i;
    s32 j;

    for (i = 0; i < sVsSeeker->numRematchableTrainers; i++)
    {
        for (j = 0; sVsSeeker->trainerInfo[j].localId != NO_REMATCH_LOCALID; j++)
        {
            if (sVsSeeker->trainerInfo[j].trainerIdx == sVsSeeker->trainerIdxArray[i])
            {
                struct ObjectEvent *objectEvent = &gObjectEvents[sVsSeeker->trainerInfo[j].objectEventId];
                u8 movementType = sVsSeeker->runningBehaviourEtcArray[i];

                if (ObjectEventIdIsSane(sVsSeeker->trainerInfo[j].objectEventId) == TRUE)
                    SetTrainerMovementType(objectEvent, movementType);
                SetObjEventTemplateMovementType(sVsSeeker->trainerInfo[j].localId, movementType);
                SetRematchWanted(sVsSeeker->trainerInfo[j].localId, TRUE);
            }
        }
    }
}
