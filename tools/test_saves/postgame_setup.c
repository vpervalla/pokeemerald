// Test save: just after the first Kanto Hall of Fame. The league save (league_setup.c), plus what the
// Hall of Fame does (PokemonLeague_HallOfFame_EventScript_GameClear and GameClear): the game is clear,
// and the player wakes up in their room in PALLET TOWN. Downstairs, MOM has BLAINE's message: the
// start of the POKéMON MANSION raid (VAR_HOENN_STORY_STATE, data/scripts/hoenn_story.inc).
// The Hall of Fame itself holds no record.
#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "constants/game_stat.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

void TestSave_SetupLeague(void);

void TestSave_SetupPostgame(void)
{
    TestSave_SetupLeague();
    FlagSet(FLAG_SYS_GAME_CLEAR);
    FlagClear(FLAG_HIDE_POSTGAME_GOSSIPERS);
    FlagClear(FLAG_LUGIA_FLEW_AWAY);
    FlagClear(FLAG_HO_OH_FLEW_AWAY);
    FlagClear(FLAG_DEOXYS_FLEW_AWAY);
    SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, 1);
    SetLastHealLocationWarp(HEAL_LOCATION_PALLET_TOWN);
    SetWarpDestination(MAP_GROUP(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), MAP_NUM(MAP_PALLET_TOWN_PLAYERS_HOUSE_2F), WARP_ID_NONE, 6, 6);
    WarpIntoMap();
}
