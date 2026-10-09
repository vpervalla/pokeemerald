#include "global.h"
#include "main.h"
#include "credits.h"
#include "event_data.h"
#include "hall_of_fame.h"
#include "load_save.h"
#include "overworld.h"
#include "script_pokemon_util.h"
#include "tv.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

int GameClear(void)
{
    int i;
    bool32 ribbonGet;
    struct RibbonCounter {
        u8 partyIndex;
        u8 count;
    } ribbonCounts[6];

    HealPlayerParty();

    if (FlagGet(FLAG_SYS_GAME_CLEAR) == TRUE)
    {
        gHasHallOfFameRecords = TRUE;
    }
    else
    {
        gHasHallOfFameRecords = FALSE;
        FlagSet(FLAG_SYS_GAME_CLEAR);
    }

    if (GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME) == 0)
        SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, (gSaveBlock2Ptr->playTimeHours << 16) | (gSaveBlock2Ptr->playTimeMinutes << 8) | gSaveBlock2Ptr->playTimeSeconds);

    SetContinueGameWarpStatus();

    // Kanto: after the Indigo Plateau's Hall of Fame, wake up in the player's room, as in FRLG
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_POKEMON_LEAGUE_HALL_OF_FAME)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_POKEMON_LEAGUE_HALL_OF_FAME))
        SetContinueGameWarp(MAP_GROUP(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), MAP_NUM(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), WARP_ID_NONE, 6, 6);
    else if (gSaveBlock2Ptr->playerGender == MALE)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);

    ribbonGet = FALSE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];

        ribbonCounts[i].partyIndex = i;
        ribbonCounts[i].count = 0;

        if (GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES)
         && !GetMonData(mon, MON_DATA_SANITY_IS_EGG)
         && !GetMonData(mon, MON_DATA_CHAMPION_RIBBON))
        {
            u8 val[1] = {TRUE};
            SetMonData(mon, MON_DATA_CHAMPION_RIBBON, val);
            ribbonCounts[i].count = GetRibbonCount(mon);
            ribbonGet = TRUE;
        }
    }

    if (ribbonGet == TRUE)
    {
        IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
        FlagSet(FLAG_SYS_RIBBON_GET);

        for (i = 1; i < 6; i++)
        {
            if (ribbonCounts[i].count > ribbonCounts[0].count)
            {
                struct RibbonCounter prevBest = ribbonCounts[0];
                ribbonCounts[0] = ribbonCounts[i];
                ribbonCounts[i] = prevBest;
            }
        }

        if (ribbonCounts[0].count > NUM_CUTIES_RIBBONS)
        {
            TryPutSpotTheCutiesOnAir(&gPlayerParty[ribbonCounts[0].partyIndex], MON_DATA_CHAMPION_RIBBON);
        }
    }

    SetMainCallback2(CB2_DoHallOfFameScreen);
    return 0;
}

bool8 SetCB2WhiteOut(void)
{
    SetMainCallback2(CB2_WhiteOut);
    return FALSE;
}
