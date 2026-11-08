// Kanto (FRLG): SAFFRON CITY's POKéMON TRAINER FAN CLUB, from pret/pokefirered (src/trainer_fan_club.c).
// It works like LILYCOVE's (src/field_specials.c) but keeps its own fans, in the Kanto vars
// VAR_KANTO_FANCLUB_FAN_COUNTER (the same bit layout: FANCLUB_COUNTER, FANCLUB_GOT_FIRST_FANS and
// FANCLUB_MEMBER1-8) and VAR_KANTO_FANCLUB_LOSE_FAN_TIMER. It opens after Kanto's Hall of Fame
// (VAR_MAP_SCENE_SAFFRON_CITY_POKEMON_TRAINER_FAN_CLUB 1: the first fans' scene, 2: open), and
// beating LANCE again brings new fans while the player has fewer than 3.
#include "global.h"
#include "event_data.h"
#include "random.h"
#include "string_util.h"
#include "strings.h"
#include "international_string_util.h"
#include "constants/field_specials.h"

#define FANCLUB_BITS        (*GetVarPointer(VAR_KANTO_FANCLUB_FAN_COUNTER))
#define FANCLUB_COUNTER_MASK 0x007F

#define GET_FLAG(flag)  (FANCLUB_BITS >> (flag) & 1)
#define SET_FLAG(flag)  (FANCLUB_BITS |= 1 << (flag))
#define FLIP_FLAG(flag) (FANCLUB_BITS ^= 1 << (flag))

static u16 GetNumFans(void)
{
    u8 i, numFans = 0;

    for (i = 0; i < NUM_TRAINER_FAN_CLUB_MEMBERS; i++)
    {
        if (GET_FLAG(i + FANCLUB_MEMBER1))
            numFans++;
    }
    return numFans;
}

static void GainRandomFan(void)
{
    static const u8 sMemberIds[NUM_TRAINER_FAN_CLUB_MEMBERS] = {
        FANCLUB_MEMBER2, FANCLUB_MEMBER4, FANCLUB_MEMBER6, FANCLUB_MEMBER1,
        FANCLUB_MEMBER8, FANCLUB_MEMBER7, FANCLUB_MEMBER5, FANCLUB_MEMBER3,
    };
    u8 i, idx = 0;

    for (i = 0; i < ARRAY_COUNT(sMemberIds); i++)
    {
        if (!GET_FLAG(sMemberIds[i]))
        {
            idx = i;
            if (Random() % 2)
            {
                SET_FLAG(sMemberIds[i]);
                return;
            }
        }
    }
    SET_FLAG(sMemberIds[idx]);
}

static void LoseRandomFan(void)
{
    static const u8 sMemberIds[NUM_TRAINER_FAN_CLUB_MEMBERS] = {
        FANCLUB_MEMBER6, FANCLUB_MEMBER7, FANCLUB_MEMBER4, FANCLUB_MEMBER8,
        FANCLUB_MEMBER5, FANCLUB_MEMBER2, FANCLUB_MEMBER1, FANCLUB_MEMBER3,
    };
    u8 i, idx = 0;

    if (GetNumFans() == 1)
        return;
    for (i = 0; i < ARRAY_COUNT(sMemberIds); i++)
    {
        if (GET_FLAG(sMemberIds[i]))
        {
            idx = i;
            if (Random() % 2)
            {
                FLIP_FLAG(sMemberIds[i]);
                return;
            }
        }
    }
    if (GET_FLAG(sMemberIds[idx]))
        FLIP_FLAG(sMemberIds[idx]);
}

// Kanto's Hall of Fame: the first three fans, and the club's other members come
void KantoFanClub_UpdateGameClear(void)
{
    if (GET_FLAG(FANCLUB_GOT_FIRST_FANS))
        return;
    SET_FLAG(FANCLUB_GOT_FIRST_FANS);
    SET_FLAG(FANCLUB_MEMBER1);
    SET_FLAG(FANCLUB_MEMBER2);
    SET_FLAG(FANCLUB_MEMBER3);
    VarSet(VAR_KANTO_FANCLUB_LOSE_FAN_TIMER, gSaveBlock2Ptr->playTimeHours);
    FlagClear(FLAG_HIDE_SAFFRON_FAN_CLUB_BLACK_BELT);
    FlagClear(FLAG_HIDE_SAFFRON_FAN_CLUB_ROCKER);
    FlagClear(FLAG_HIDE_SAFFRON_FAN_CLUB_WOMAN);
    FlagClear(FLAG_HIDE_SAFFRON_FAN_CLUB_BEAUTY);
    VarSet(VAR_MAP_SCENE_SAFFRON_CITY_POKEMON_TRAINER_FAN_CLUB, 1);
}

// VAR_0x8004: FANCOUNTER_*. With fewer than 3 fans, a new one comes every 20 points (LANCE: 2)
void KantoFanClub_TryGainNewFanFromCounter(void)
{
    static const u8 sCounterIncrements[] = {2, 1, 2, 1};
    u16 counter = FANCLUB_BITS & FANCLUB_COUNTER_MASK;

    if (VarGet(VAR_MAP_SCENE_SAFFRON_CITY_POKEMON_TRAINER_FAN_CLUB) != 2 || gSpecialVar_0x8004 >= ARRAY_COUNT(sCounterIncrements))
        return;
    if (counter + sCounterIncrements[gSpecialVar_0x8004] >= 20)
    {
        if (GetNumFans() < 3)
        {
            GainRandomFan();
            counter = 0;
        }
        else
        {
            counter = 20;
        }
    }
    else
    {
        counter += sCounterIncrements[gSpecialVar_0x8004];
    }
    FANCLUB_BITS = (FANCLUB_BITS & ~FANCLUB_COUNTER_MASK) | counter;
}

// With 5 fans or more, one leaves every 12 hours of play
void KantoFanClub_TryLoseFansFromPlayTime(void)
{
    u8 i;

    if (gSaveBlock2Ptr->playTimeHours >= 999)
        return;
    for (i = 0; i < NUM_TRAINER_FAN_CLUB_MEMBERS; i++)
    {
        if (GetNumFans() < 5)
        {
            VarSet(VAR_KANTO_FANCLUB_LOSE_FAN_TIMER, gSaveBlock2Ptr->playTimeHours);
            return;
        }
        if (gSaveBlock2Ptr->playTimeHours - VarGet(VAR_KANTO_FANCLUB_LOSE_FAN_TIMER) < 12)
            return;
        LoseRandomFan();
        VarSet(VAR_KANTO_FANCLUB_LOSE_FAN_TIMER, VarGet(VAR_KANTO_FANCLUB_LOSE_FAN_TIMER) + 12);
    }
}

// VAR_0x8004: FANCLUB_MEMBER*
bool8 KantoFanClub_IsMemberFanOfPlayer(void)
{
    return GET_FLAG(gSpecialVar_0x8004);
}

u16 KantoFanClub_GetNumFans(void)
{
    return GetNumFans();
}

// The trainer a member who isn't a fan of the player prefers: one the player met in a link battle,
// or else the rival, LT. SURGE or KOGA
void KantoFanClub_BufferTrainerName(void)
{
    u8 whichLinkTrainer = 0, whichNPCTrainer = 0;
    struct LinkBattleRecords *linkRecords = &gSaveBlock1Ptr->linkBattleRecords;

    switch (gSpecialVar_0x8004)
    {
    case FANCLUB_MEMBER5:
        whichNPCTrainer = 1;
        break;
    case FANCLUB_MEMBER6:
        whichLinkTrainer = 1;
        break;
    case FANCLUB_MEMBER7:
        whichNPCTrainer = 2;
        whichLinkTrainer = 1;
        break;
    }
    if (linkRecords->entries[whichLinkTrainer].name[0] != EOS)
    {
        StringCopyN(gStringVar1, linkRecords->entries[whichLinkTrainer].name, PLAYER_NAME_LENGTH);
        gStringVar1[PLAYER_NAME_LENGTH] = EOS;
        ConvertInternationalString(gStringVar1, linkRecords->languages[whichLinkTrainer]);
    }
    else if (whichNPCTrainer == 1)
    {
        StringCopy(gStringVar1, gText_KantoFanClub_LtSurge);
    }
    else if (whichNPCTrainer == 2)
    {
        StringCopy(gStringVar1, gText_KantoFanClub_Koga);
    }
    else
    {
        StringCopy(gStringVar1, gSaveBlock2Ptr->rivalName);
    }
}
