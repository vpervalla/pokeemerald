#include "global.h"
#include "event_data.h"
#include "field_door.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "task.h"
#include "tilesets.h"
#include "constants/songs.h"
#include "constants/metatile_labels.h"

#define DOOR_SOUND_NORMAL  0
#define DOOR_SOUND_SLIDING 1
#define DOOR_SOUND_ARENA   2

// Door sizes. FRLG's Kanto doors only animate their bottom metatile.
#define DOOR_SIZE_1x1 0
#define DOOR_SIZE_1x2 1
#define DOOR_SIZE_2x2 2

struct DoorGraphics
{
    u16 metatileNum;
    u8 sound;
    u8 size;
    const void *tiles;
    const void *palettes;
};

struct DoorAnimFrame
{
    u8 time;
    u16 offset;
};

static bool8 ShouldUseMultiCorridorDoor(void);

// The door last drawn opening, open or closing, and the frame of its animation graphics it's drawn
// with (or will be on the next frame, see AnimateDoorFrame), until it's drawn closed again. For
// day_night.c's glowing door glass, which follows the frame. In map coordinates with MAP_OFFSET.
static EWRAM_DATA s8 sOpenDoorFrame = -1;
static EWRAM_DATA s16 sOpenDoorX = 0;
static EWRAM_DATA s16 sOpenDoorY = 0;

static const u8 sDoorAnimTiles_Littleroot[] = INCGFX_U8("graphics/door_anims/littleroot.png", ".4bpp");
static const u16 sDoorNullPalette1[16] = {};
static const u8 sDoorAnimTiles_BirchsLab[] = INCGFX_U8("graphics/door_anims/birchs_lab.png", ".4bpp");
static const u16 sDoorNullPalette2[16] = {};
static const u8 sDoorAnimTiles_FallarborLightRoof[] = INCGFX_U8("graphics/door_anims/fallarbor_light_roof.png", ".4bpp");
static const u16 sDoorNullPalette3[16] = {};
static const u8 sDoorAnimTiles_Lilycove[] = INCGFX_U8("graphics/door_anims/lilycove.png", ".4bpp");
static const u16 sDoorNullPalette4[16] = {};
static const u8 sDoorAnimTiles_LilycoveWooden[] = INCGFX_U8("graphics/door_anims/lilycove_wooden.png", ".4bpp");
static const u16 sDoorNullPalette5[16] = {};
static const u8 sDoorAnimTiles_General[] = INCGFX_U8("graphics/door_anims/general.png", ".4bpp");
static const u16 sDoorNullPalette6[16] = {};
static const u8 sDoorAnimTiles_PokeCenter[] = INCGFX_U8("graphics/door_anims/poke_center.png", ".4bpp");
static const u8 sDoorAnimTiles_Gym[] = INCGFX_U8("graphics/door_anims/gym.png", ".4bpp");
static const u8 sDoorAnimTiles_PokeMart[] = INCGFX_U8("graphics/door_anims/poke_mart.png", ".4bpp");
static const u8 sDoorAnimTiles_RustboroTan[] = INCGFX_U8("graphics/door_anims/rustboro_tan.png", ".4bpp");
static const u16 sDoorNullPalette7[16] = {};
static const u8 sDoorAnimTiles_RustboroGray[] = INCGFX_U8("graphics/door_anims/rustboro_gray.png", ".4bpp");
static const u16 sDoorNullPalette8[16] = {};
static const u8 sDoorAnimTiles_Oldale[] = INCGFX_U8("graphics/door_anims/oldale.png", ".4bpp");
static const u8 sFiller1[0x5900] = {};
static const u8 sDoorAnimTiles_UnusedTops[] = INCGFX_U8("graphics/door_anims/unused_top.png", ".4bpp");
static const u8 sFiller2[0x140] = {};
static const u8 sDoorAnimTiles_UnusedBottoms[] = INCGFX_U8("graphics/door_anims/unused_bottom.png", ".4bpp");
static const u16 sDoorNullPalette11[16] = {};
static const u8 sDoorAnimTiles_Mauville[] = INCGFX_U8("graphics/door_anims/mauville.png", ".4bpp");
static const u16 sDoorNullPalette12[16] = {};
static const u8 sDoorAnimTiles_Verdanturf[] = INCGFX_U8("graphics/door_anims/verdanturf.png", ".4bpp");
static const u16 sDoorNullPalette13[16] = {};
static const u8 sDoorAnimTiles_Slateport[] = INCGFX_U8("graphics/door_anims/slateport.png", ".4bpp");
static const u16 sDoorNullPalette14[16] = {};
static const u8 sDoorAnimTiles_Dewford[] = INCGFX_U8("graphics/door_anims/dewford.png", ".4bpp");
static const u16 sDoorNullPalette15[16] = {};
static const u8 sDoorAnimTiles_Contest[] = INCGFX_U8("graphics/door_anims/contest.png", ".4bpp");
static const u16 sDoorNullPalette16[16] = {};
static const u8 sDoorAnimTiles_Mossdeep[] = INCGFX_U8("graphics/door_anims/mossdeep.png", ".4bpp");
static const u16 sDoorNullPalette17[16] = {};
static const u8 sDoorAnimTiles_SootopolisPeakedRoof[] = INCGFX_U8("graphics/door_anims/sootopolis_peaked_roof.png", ".4bpp");
static const u16 sDoorNullPalette18[16] = {};
static const u8 sDoorAnimTiles_Sootopolis[] = INCGFX_U8("graphics/door_anims/sootopolis.png", ".4bpp");
static const u16 sDoorNullPalette19[16] = {};
static const u8 sDoorAnimTiles_PokemonLeague[] = INCGFX_U8("graphics/door_anims/pokemon_league.png", ".4bpp");
static const u16 sDoorNullPalette20[16] = {};
static const u8 sDoorAnimTiles_Pacifidlog[] = INCGFX_U8("graphics/door_anims/pacifidlog.png", ".4bpp");
static const u16 sDoorNullPalette21[16] = {};
static const u8 sDoorAnimTiles_PetalburgGym[] = INCGFX_U8("graphics/door_anims/petalburg_gym.png", ".4bpp");
static const u16 sDoorNullPalette22[16] = {};
static const u8 sDoorAnimTiles_CyclingRoad[] = INCGFX_U8("graphics/door_anims/cycling_road.png", ".4bpp");
static const u16 sDoorNullPalette23[16] = {};
static const u8 sDoorAnimTiles_LilycoveDeptStore[] = INCGFX_U8("graphics/door_anims/lilycove_dept_store.png", ".4bpp");
static const u16 sDoorNullPalette24[16] = {};
static const u8 sDoorAnimTiles_SafariZone[] = INCGFX_U8("graphics/door_anims/safari_zone.png", ".4bpp");
static const u16 sDoorNullPalette25[16] = {};
static const u8 sDoorAnimTiles_MossdeepSpaceCenter[] = INCGFX_U8("graphics/door_anims/mossdeep_space_center.png", ".4bpp");
static const u16 sDoorNullPalette26[16] = {};
static const u8 sDoorAnimTiles_CableClub[] = INCGFX_U8("graphics/door_anims/cable_club.png", ".4bpp");
static const u16 sDoorNullPalette27[16] = {};
static const u8 sDoorAnimTiles_AbandonedShip[] = INCGFX_U8("graphics/door_anims/abandoned_ship.png", ".4bpp");
static const u16 sDoorNullPalette28[16] = {};
static const u8 sDoorAnimTiles_FallarborDarkRoof[] = INCGFX_U8("graphics/door_anims/fallarbor_dark_roof.png", ".4bpp");
static const u16 sDoorNullPalette29[16] = {};
static const u8 sDoorAnimTiles_AbandonedShipRoom[] = INCGFX_U8("graphics/door_anims/abandoned_ship_room.png", ".4bpp");
static const u16 sDoorNullPalette30[16] = {};
static const u8 sDoorAnimTiles_LilycoveDeptStoreElevator[] = INCGFX_U8("graphics/door_anims/lilycove_dept_store_elevator.png", ".4bpp");
static const u16 sDoorNullPalette31[16] = {};
static const u8 sDoorAnimTiles_BattleTowerOld[] = INCGFX_U8("graphics/door_anims/battle_tower_old.png", ".4bpp");
static const u16 sDoorNullPalette32[16] = {};
static const u8 sDoorAnimTiles_BattleTowerElevator[] = INCGFX_U8("graphics/door_anims/battle_tower_elevator.png", ".4bpp");
static const u16 sDoorNullPalette33[16] = {};
static const u8 sDoorAnimTiles_UnusedBattleFrontier[] = INCGFX_U8("graphics/door_anims/unused_battle_frontier.png", ".4bpp");
static const u16 sDoorNullPalette34[16] = {};
static const u8 sDoorAnimTiles_BattleDome[] = INCGFX_U8("graphics/door_anims/battle_dome.png", ".4bpp");
static const u16 sDoorNullPalette35[16] = {};
static const u8 sDoorAnimTiles_BattleFactory[] = INCGFX_U8("graphics/door_anims/battle_factory.png", ".4bpp");
static const u16 sDoorNullPalette36[16] = {};
static const u8 sDoorAnimTiles_BattleTower[] = INCGFX_U8("graphics/door_anims/battle_tower.png", ".4bpp");
static const u16 sDoorNullPalette37[16] = {};
static const u8 sDoorAnimTiles_BattleArena[] = INCGFX_U8("graphics/door_anims/battle_arena.png", ".4bpp");
static const u16 sDoorNullPalette38[16] = {};
static const u8 sDoorAnimTiles_BattleArenaLobby[] = INCGFX_U8("graphics/door_anims/battle_arena_lobby.png", ".4bpp");
static const u16 sDoorNullPalette39[16] = {};
static const u8 sDoorAnimTiles_BattleDomeLobby[] = INCGFX_U8("graphics/door_anims/battle_dome_lobby.png", ".4bpp");
static const u16 sDoorNullPalette40[16] = {};
static const u8 sDoorAnimTiles_BattlePalaceLobby[] = INCGFX_U8("graphics/door_anims/battle_palace_lobby.png", ".4bpp");
static const u8 sDoorAnimTiles_BattleTent[] = INCGFX_U8("graphics/door_anims/battle_tent.png", ".4bpp");
static const u16 sDoorNullPalette41[16] = {};
static const u8 sDoorAnimTiles_BattleDomeCorridor[] = INCGFX_U8("graphics/door_anims/battle_dome_corridor.png", ".4bpp");
static const u16 sDoorNullPalette42[16] = {};
static const u8 sDoorAnimTiles_BattleTowerMultiCorridor[] = INCGFX_U8("graphics/door_anims/battle_tower_multi_corridor.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u16 sDoorNullPalette43[16] = {};
static const u8 sDoorAnimTiles_BattleFrontier[] = INCGFX_U8("graphics/door_anims/battle_frontier.png", ".4bpp");
static const u16 sDoorNullPalette44[16] = {};
static const u8 sDoorAnimTiles_BattleFrontierSliding[] = INCGFX_U8("graphics/door_anims/battle_frontier_sliding.png", ".4bpp");
static const u16 sDoorNullPalette45[16] = {};
static const u8 sDoorAnimTiles_BattleDomePreBattleRoom[] = INCGFX_U8("graphics/door_anims/battle_dome_pre_battle_room.png", ".4bpp");
static const u16 sDoorNullPalette46[16] = {};
static const u8 sDoorAnimTiles_BattleTentInterior[] = INCGFX_U8("graphics/door_anims/battle_tent_interior.png", ".4bpp");
static const u16 sDoorNullPalette47[16] = {};
static const u8 sDoorAnimTiles_TrainerHillLobbyElevator[] = INCGFX_U8("graphics/door_anims/trainer_hill_lobby_elevator.png", ".4bpp");
static const u16 sDoorNullPalette48[16] = {};
static const u8 sDoorAnimTiles_TrainerHillRoofElevator[] = INCGFX_U8("graphics/door_anims/trainer_hill_roof_elevator.png", ".4bpp");
static const u16 sDoorNullPalette49[16] = {};

// FRLG's door animations, used by the Kanto tilesets
static const u8 sDoorAnimTiles_Kanto_General[] = INCGFX_U8("graphics/door_anims/kanto/general.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SlidingSingle[] = INCGFX_U8("graphics/door_anims/kanto/sliding_single.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SlidingDouble[] = INCGFX_U8("graphics/door_anims/kanto/sliding_double.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Pallet[] = INCGFX_U8("graphics/door_anims/kanto/pallet.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_OaksLab[] = INCGFX_U8("graphics/door_anims/kanto/oaks_lab.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Viridian[] = INCGFX_U8("graphics/door_anims/kanto/viridian.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Pewter[] = INCGFX_U8("graphics/door_anims/kanto/pewter.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Saffron[] = INCGFX_U8("graphics/door_anims/kanto/saffron.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SilphCo[] = INCGFX_U8("graphics/door_anims/kanto/silph_co.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Cerulean[] = INCGFX_U8("graphics/door_anims/kanto/cerulean.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Lavender[] = INCGFX_U8("graphics/door_anims/kanto/lavender.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Vermilion[] = INCGFX_U8("graphics/door_anims/kanto/vermilion.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_PokemonFanClub[] = INCGFX_U8("graphics/door_anims/kanto/pokemon_fan_club.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_DeptStore[] = INCGFX_U8("graphics/door_anims/kanto/dept_store.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Fuchsia[] = INCGFX_U8("graphics/door_anims/kanto/fuchsia.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SafariZone[] = INCGFX_U8("graphics/door_anims/kanto/safari_zone.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_CinnabarLab[] = INCGFX_U8("graphics/door_anims/kanto/cinnabar_lab.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_DeptStoreElevator[] = INCGFX_U8("graphics/door_anims/kanto/dept_store_elevator.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_CableClub[] = INCGFX_U8("graphics/door_anims/kanto/cable_club.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_HideoutElevator[] = INCGFX_U8("graphics/door_anims/kanto/hideout_elevator.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SSAnne[] = INCGFX_U8("graphics/door_anims/kanto/ss_anne.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_SilphCoElevator[] = INCGFX_U8("graphics/door_anims/kanto/silph_co_elevator.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Sevii123[] = INCGFX_U8("graphics/door_anims/kanto/sevii_123.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_JoyfulGameCorner[] = INCGFX_U8("graphics/door_anims/kanto/joyful_game_corner.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_OneIslandPokeCenter[] = INCGFX_U8("graphics/door_anims/kanto/one_island_poke_center.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Sevii45[] = INCGFX_U8("graphics/door_anims/kanto/sevii_45.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_FourIslandDayCare[] = INCGFX_U8("graphics/door_anims/kanto/four_island_day_care.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_RocketWarehouse[] = INCGFX_U8("graphics/door_anims/kanto/rocket_warehouse.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Sevii67[] = INCGFX_U8("graphics/door_anims/kanto/sevii_67.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_Teleporter[] = INCGFX_U8("graphics/door_anims/kanto/teleporter.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_TrainerTowerLobbyElevator[] = INCGFX_U8("graphics/door_anims/kanto/trainer_tower_lobby_elevator.png", ".4bpp");
static const u8 sDoorAnimTiles_Kanto_TrainerTowerRoofElevator[] = INCGFX_U8("graphics/door_anims/kanto/trainer_tower_roof_elevator.png", ".4bpp");

static const struct DoorAnimFrame sDoorOpenAnimFrames[] =
{
    {4, -1},
    {4, 0},
    {4, 0x100},
    {4, 0x200},
    {0, 0},
};

static const struct DoorAnimFrame sDoorCloseAnimFrames[] =
{
    {4, 0x200},
    {4, 0x100},
    {4, 0},
    {4, -1},
    {0, 0},
};

static const struct DoorAnimFrame sBigDoorOpenAnimFrames[] =
{
    {4, -1},
    {4, 0},
    {4, 0x200},
    {4, 0x400},
    {0, 0},
};

static const struct DoorAnimFrame sBigDoorCloseAnimFrames[] =
{
    {4, 0x400},
    {4, 0x200},
    {4, 0},
    {4, -1},
    {0, 0},
};

// A 1x1 door's frames are a single metatile (4 tiles) each
static const struct DoorAnimFrame sSmallDoorOpenAnimFrames[] =
{
    {4, -1},
    {4, 0},
    {4, 0x80},
    {4, 0x100},
    {0, 0},
};

static const struct DoorAnimFrame sSmallDoorCloseAnimFrames[] =
{
    {4, 0x100},
    {4, 0x80},
    {4, 0},
    {4, -1},
    {0, 0},
};

static const u8 sDoorAnimPalettes_General[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_PokeCenter[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_Gym[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_PokeMart[] = {0, 0, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_Littleroot[] = {10, 10, 6, 6, 6, 6, 6, 6};
static const u8 sDoorAnimPalettes_BirchsLab[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_RustboroTan[] = {11, 11, 11, 11, 11, 11, 11, 11};
static const u8 sDoorAnimPalettes_RustboroGray[] = {10, 10, 10, 10, 10, 10, 10, 10};
static const u8 sDoorAnimPalettes_FallarborLightRoof[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_Lilycove[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Oldale[] = {10, 10, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_Mossdeep[] = {9, 9, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_PokemonLeague[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Pacifidlog[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_SootopolisPeakedRoof[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const u8 sDoorAnimPalettes_Sootopolis[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const u8 sDoorAnimPalettes_Dewford[] = {0, 0, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_Slateport[] = {6, 6, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_Mauville[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_Verdanturf[] = {6, 6, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_LilycoveWooden[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_Contest[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_PetalburgGym[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const u8 sDoorAnimPalettes_CyclingRoad[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_LilycoveDeptStore[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_SafariZone[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_MossdeepSpaceCenter[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_CableClub[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const u8 sDoorAnimPalettes_AbandonedShip[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_FallarborDarkRoof[] = {11, 11, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_AbandonedShipRoom[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_LilycoveDeptStoreElevator[] = {6, 6, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattleTowerOld[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_BattleTowerElevator[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_UnusedBattleFrontier[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_BattleDome[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_BattleFactory[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_BattleTower[] = {0, 0, 0, 0, 0, 0, 0, 0};
static const u8 sDoorAnimPalettes_BattleArena[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_BattleArenaLobby[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattleDomeLobby[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattlePalaceLobby[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattleTent[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_BattleDomeCorridor[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattleTowerMultiCorridor[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_Unused[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_BattleFrontier[] = {1, 1, 1, 1, 1, 1, 1, 1};
static const u8 sDoorAnimPalettes_BattleDomePreBattleRoom[] = {9, 9, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_BattleTentInterior[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_TrainerHillLobbyElevator[] = {7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_TrainerHillRoofElevator[] = {9, 9, 7, 7, 7, 7, 7, 7};

static const u8 sDoorAnimPalettes_Kanto_General[] = {2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sDoorAnimPalettes_Kanto_SlidingSingle[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_SlidingDouble[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_Pallet[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_OaksLab[] = {10, 10, 10, 10, 10, 10, 10, 10};
static const u8 sDoorAnimPalettes_Kanto_Viridian[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_Pewter[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_Saffron[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_SilphCo[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_Cerulean[] = {12, 12, 12, 12, 12, 12, 12, 12};
static const u8 sDoorAnimPalettes_Kanto_Lavender[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_Kanto_Vermilion[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_Kanto_PokemonFanClub[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_Kanto_DeptStore[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_Fuchsia[] = {8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_SafariZone[] = {9, 9, 9, 9, 9, 9, 9, 9};
static const u8 sDoorAnimPalettes_Kanto_CinnabarLab[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_DeptStoreElevator[] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_CableClub[] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_HideoutElevator[] = {12, 12, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sDoorAnimPalettes_Kanto_SSAnne[] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
static const u8 sDoorAnimPalettes_Kanto_SilphCoElevator[] = {8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sDoorAnimPalettes_Kanto_Sevii123[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_Kanto_JoyfulGameCorner[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_OneIslandPokeCenter[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_Sevii45[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_Kanto_FourIslandDayCare[] = {3, 3, 3, 3, 3, 3, 3, 3};
static const u8 sDoorAnimPalettes_Kanto_RocketWarehouse[] = {10, 10, 10, 10, 10, 10, 10, 10};
static const u8 sDoorAnimPalettes_Kanto_Sevii67[] = {5, 5, 5, 5, 5, 5, 5, 5};
static const u8 sDoorAnimPalettes_Kanto_Teleporter[] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
static const u8 sDoorAnimPalettes_Kanto_TrainerTowerLobbyElevator[] = {8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
static const u8 sDoorAnimPalettes_Kanto_TrainerTowerRoofElevator[] = {11, 11, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

static const struct DoorGraphics sDoorAnimGraphicsTable[] =
{
    {METATILE_General_Door,                                 DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_General, sDoorAnimPalettes_General},
    {METATILE_General_Door_PokeCenter,                      DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_PokeCenter, sDoorAnimPalettes_PokeCenter},
    {METATILE_General_Door_Gym,                             DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_Gym, sDoorAnimPalettes_Gym},
    {METATILE_General_Door_PokeMart,                        DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_PokeMart, sDoorAnimPalettes_PokeMart},
    {METATILE_Petalburg_Door_Littleroot,                    DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Littleroot, sDoorAnimPalettes_Littleroot},
    {METATILE_Petalburg_Door_BirchsLab,                     DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_BirchsLab, sDoorAnimPalettes_BirchsLab},
    {METATILE_Rustboro_Door_Tan,                            DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_RustboroTan, sDoorAnimPalettes_RustboroTan},
    {METATILE_Rustboro_Door_Gray,                           DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_RustboroGray, sDoorAnimPalettes_RustboroGray},
    {METATILE_Fallarbor_Door_LightRoof,                     DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_FallarborLightRoof, sDoorAnimPalettes_FallarborLightRoof},
    {METATILE_Petalburg_Door_Oldale,                        DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Oldale, sDoorAnimPalettes_Oldale},
    {METATILE_Mauville_Door,                                DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Mauville, sDoorAnimPalettes_Mauville},
    {METATILE_Mauville_Door_Verdanturf,                     DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Verdanturf, sDoorAnimPalettes_Verdanturf},
    {METATILE_Slateport_Door,                               DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Slateport, sDoorAnimPalettes_Slateport},
    {METATILE_Dewford_Door,                                 DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Dewford, sDoorAnimPalettes_Dewford},
    {METATILE_General_Door_Contest,                         DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_Contest, sDoorAnimPalettes_Contest},
    {METATILE_Lilycove_Door,                                DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Lilycove, sDoorAnimPalettes_Lilycove},
    {METATILE_Lilycove_Door_Wooden,                         DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_LilycoveWooden, sDoorAnimPalettes_LilycoveWooden},
    {METATILE_Mossdeep_Door,                                DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Mossdeep, sDoorAnimPalettes_Mossdeep},
    {METATILE_Sootopolis_Door_PeakedRoof,                   DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_SootopolisPeakedRoof, sDoorAnimPalettes_SootopolisPeakedRoof},
    {METATILE_Sootopolis_Door,                              DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Sootopolis, sDoorAnimPalettes_Sootopolis},
    {METATILE_EverGrande_Door_PokemonLeague,                DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_PokemonLeague, sDoorAnimPalettes_PokemonLeague},
    {METATILE_Pacifidlog_Door,                              DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_Pacifidlog, sDoorAnimPalettes_Pacifidlog},
    {METATILE_PetalburgGym_Door,                            DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_PetalburgGym, sDoorAnimPalettes_PetalburgGym},
    {METATILE_Mauville_Door_CyclingRoad,                    DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_CyclingRoad, sDoorAnimPalettes_CyclingRoad},
    {METATILE_Lilycove_Door_DeptStore,                      DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_LilycoveDeptStore, sDoorAnimPalettes_LilycoveDeptStore},
    {METATILE_Lilycove_Door_SafariZone,                     DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_SafariZone, sDoorAnimPalettes_SafariZone},
    {METATILE_Mossdeep_Door_SpaceCenter,                    DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_MossdeepSpaceCenter, sDoorAnimPalettes_MossdeepSpaceCenter},
    {METATILE_PokemonCenter_Door_CableClub,                 DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_CableClub, sDoorAnimPalettes_CableClub},
    {METATILE_InsideShip_IntactDoor_Bottom_Unlocked,        DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_AbandonedShip, sDoorAnimPalettes_AbandonedShip},
    {METATILE_Fallarbor_Door_DarkRoof,                      DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_FallarborDarkRoof, sDoorAnimPalettes_FallarborDarkRoof},
    {METATILE_InsideShip_IntactDoor_Bottom_Interior,        DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_AbandonedShipRoom, sDoorAnimPalettes_AbandonedShipRoom},
    {METATILE_Shop_Door_Elevator,                           DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_LilycoveDeptStoreElevator, sDoorAnimPalettes_LilycoveDeptStoreElevator},
    {METATILE_Dewford_Door_BattleTower,                     DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTowerOld, sDoorAnimPalettes_BattleTowerOld},
    {METATILE_BattleFrontier_Door_Elevator,                 DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTowerElevator, sDoorAnimPalettes_BattleTowerElevator},
    // The metatile for this door doesn't seem to correspond to a door in any Emerald tileset. Given the surrounding door animations, it was likely cut from the Battle Frontier.
    // From the palettes array we know it uses palette 9, and the door's shadow looks correct using either the Battle Tent or Battle Frontier Outside's 9th palette.
    {0x3B0,                                                 DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_UnusedBattleFrontier, sDoorAnimPalettes_UnusedBattleFrontier},
    {METATILE_BattleFrontierOutsideWest_Door_BattleDome,    DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleDome, sDoorAnimPalettes_BattleDome},
    {METATILE_BattleFrontierOutsideWest_Door_BattleFactory, DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleFactory, sDoorAnimPalettes_BattleFactory},
    {METATILE_BattleFrontierOutsideEast_Door_BattleTower,   DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTower, sDoorAnimPalettes_BattleTower},
    {METATILE_BattleFrontierOutsideEast_Door_BattleArena,   DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_BattleArena, sDoorAnimPalettes_BattleArena},
    {METATILE_BattleArena_Door,                             DOOR_SOUND_ARENA,   1, sDoorAnimTiles_BattleArenaLobby, sDoorAnimPalettes_BattleArenaLobby},
    {METATILE_BattleDome_Door_Lobby,                        DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleDomeLobby, sDoorAnimPalettes_BattleDomeLobby},
    {METATILE_BattlePalace_Door,                            DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_BattlePalaceLobby, sDoorAnimPalettes_BattlePalaceLobby},
    {METATILE_Slateport_Door_BattleTent,                    DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTent, sDoorAnimPalettes_BattleTent},
    {METATILE_Mauville_Door_BattleTent,                     DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTent, sDoorAnimPalettes_BattleTent},
    {METATILE_Fallarbor_Door_BattleTent,                    DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTent, sDoorAnimPalettes_BattleTent},
    {METATILE_BattleDome_Door_Corridor,                     DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleDomeCorridor, sDoorAnimPalettes_BattleDomeCorridor},
    {METATILE_BattleFrontier_Door_MultiCorridor,            DOOR_SOUND_SLIDING, 2, sDoorAnimTiles_BattleTowerMultiCorridor, sDoorAnimPalettes_BattleTowerMultiCorridor},
    {METATILE_BattleFrontierOutsideWest_Door,               DOOR_SOUND_NORMAL,  1, sDoorAnimTiles_BattleFrontier, sDoorAnimPalettes_BattleFrontier},
    {METATILE_BattleFrontierOutsideWest_Door_Sliding,       DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleFrontierSliding, sDoorAnimPalettes_BattleFrontier},
    {METATILE_BattleDome_Door_PreBattleRoom,                DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleDomePreBattleRoom, sDoorAnimPalettes_BattleDomePreBattleRoom},
    {METATILE_BattleTent_Door,                              DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_BattleTentInterior, sDoorAnimPalettes_BattleTentInterior},
    {METATILE_TrainerHill_Door_Elevator_Lobby,              DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_TrainerHillLobbyElevator, sDoorAnimPalettes_TrainerHillLobbyElevator},
    {METATILE_TrainerHill_Door_Elevator_Roof,               DOOR_SOUND_SLIDING, 1, sDoorAnimTiles_TrainerHillRoofElevator, sDoorAnimPalettes_TrainerHillRoofElevator},
    {},
};

// FRLG's door metatiles. Their IDs overlap with Hoenn's (Oak's Lab's door is Mauville's
// door ID), so the Kanto tilesets need their own table.
static const struct DoorGraphics sKantoDoorAnimGraphicsTable[] =
{
    {0x03D, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_General, sDoorAnimPalettes_Kanto_General},                         // General_Door
    {0x062, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_SlidingSingle, sDoorAnimPalettes_Kanto_SlidingSingle},             // General_SlidingSingleDoor
    {0x15B, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_SlidingDouble, sDoorAnimPalettes_Kanto_SlidingDouble},             // General_SlidingDoubleDoor
    {0x2A3, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Pallet, sDoorAnimPalettes_Kanto_Pallet},                           // PalletTown_Door
    {0x2AC, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_OaksLab, sDoorAnimPalettes_Kanto_OaksLab},                         // PalletTown_OaksLabDoor
    {0x299, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Viridian, sDoorAnimPalettes_Kanto_Viridian},                       // ViridianCity_Door
    {0x2CE, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Pewter, sDoorAnimPalettes_Kanto_Pewter},                           // PewterCity_Door
    {0x284, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Saffron, sDoorAnimPalettes_Kanto_Saffron},                         // SaffronCity_Door
    {0x2BC, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_SilphCo, sDoorAnimPalettes_Kanto_SilphCo},                         // SaffronCity_SilphCoDoor
    {0x298, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Cerulean, sDoorAnimPalettes_Kanto_Cerulean},                       // CeruleanCity_Door
    {0x2A2, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Lavender, sDoorAnimPalettes_Kanto_Lavender},                       // LavenderTown_Door
    {0x29E, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Vermilion, sDoorAnimPalettes_Kanto_Vermilion},                     // VermilionCity_Door
    {0x2E1, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_PokemonFanClub, sDoorAnimPalettes_Kanto_PokemonFanClub},           // VermilionCity_SSAnneWarp
    {0x294, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_DeptStore, sDoorAnimPalettes_Kanto_DeptStore},                     // CeladonCity_DeptStoreDoor
    {0x2BF, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Fuchsia, sDoorAnimPalettes_Kanto_Fuchsia},                         // FuchsiaCity_Door
    {0x2D2, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_SafariZone, sDoorAnimPalettes_Kanto_SafariZone},                   // FuchsiaCity_SafariZoneDoor
    {0x2AD, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_CinnabarLab, sDoorAnimPalettes_Kanto_CinnabarLab},                 // CinnabarIsland_LabDoor
    {0x297, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Sevii123, sDoorAnimPalettes_Kanto_Sevii123},                       // SeviiIslands123_Door
    {0x29B, DOOR_SOUND_SLIDING, DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_JoyfulGameCorner, sDoorAnimPalettes_Kanto_JoyfulGameCorner},       // SeviiIslands123_GameCornerDoor
    {0x2EB, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_OneIslandPokeCenter, sDoorAnimPalettes_Kanto_OneIslandPokeCenter}, // SeviiIslands123_PokeCenterDoor
    {0x29A, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Sevii45, sDoorAnimPalettes_Kanto_Sevii45},                         // SeviiIslands45_Door
    {0x2B9, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_FourIslandDayCare, sDoorAnimPalettes_Kanto_FourIslandDayCare},     // SeviiIslands45_DayCareDoor
    {0x2AF, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_RocketWarehouse, sDoorAnimPalettes_Kanto_RocketWarehouse},         // SeviiIslands45_RocketWarehouseDoor_Unlocked
    {0x30C, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x1, sDoorAnimTiles_Kanto_Sevii67, sDoorAnimPalettes_Kanto_Sevii67},                         // SeviiIslands67_Door
    {0x28D, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_DeptStoreElevator, sDoorAnimPalettes_Kanto_DeptStoreElevator},     // DepartmentStore_ElevatorDoor
    {0x2DE, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_CableClub, sDoorAnimPalettes_Kanto_CableClub},                     // PokemonCenter_CableClubDoor
    {0x2AB, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_HideoutElevator, sDoorAnimPalettes_Kanto_HideoutElevator},         // SilphCo_HideoutElevatorDoor
    {0x281, DOOR_SOUND_NORMAL,  DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_SSAnne, sDoorAnimPalettes_Kanto_SSAnne},                           // SSAnne_Door
    {0x2E2, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_SilphCoElevator, sDoorAnimPalettes_Kanto_SilphCoElevator},         // SilphCo_ElevatorDoor
    {0x296, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_Teleporter, sDoorAnimPalettes_Kanto_Teleporter},                   // SeaCottage_Teleporter_Door
    {0x2C3, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_TrainerTowerLobbyElevator, sDoorAnimPalettes_Kanto_TrainerTowerLobbyElevator}, // TrainerTower_LobbyElevatorDoor
    {0x356, DOOR_SOUND_SLIDING, DOOR_SIZE_1x2, sDoorAnimTiles_Kanto_TrainerTowerRoofElevator, sDoorAnimPalettes_Kanto_TrainerTowerRoofElevator},   // TrainerTower_RoofElevatorDoor
    {},
};

// NOTE: The tiles of a door's animation must be copied to VRAM because they are not already part of any given tileset.
//       This means that if there are any pre-existing tiles in this copied region that are visible when the door
//       animation is played they will be overwritten.
#define DOOR_TILE_START_SIZE1 (NUM_TILES_TOTAL - 8)
#define DOOR_TILE_START_SIZE2 (NUM_TILES_TOTAL - 16)

// Bytes of animation graphics per frame
static u32 GetDoorFrameSize(const struct DoorGraphics *gfx)
{
    if (gfx->size == DOOR_SIZE_2x2)
        return 16 * TILE_SIZE_4BPP;
    else if (gfx->size == DOOR_SIZE_1x1)
        return 4 * TILE_SIZE_4BPP;
    else
        return 8 * TILE_SIZE_4BPP;
}

static void CopyDoorTilesToVram(const struct DoorGraphics *gfx, const struct DoorAnimFrame *frame)
{
    if (gfx->size == DOOR_SIZE_2x2)
        CpuFastCopy(gfx->tiles + frame->offset, (void *)(VRAM + TILE_OFFSET_4BPP(DOOR_TILE_START_SIZE2)), 16 * TILE_SIZE_4BPP);
    else if (gfx->size == DOOR_SIZE_1x1)
        CpuFastCopy(gfx->tiles + frame->offset, (void *)(VRAM + TILE_OFFSET_4BPP(DOOR_TILE_START_SIZE1)), 4 * TILE_SIZE_4BPP);
    else
        CpuFastCopy(gfx->tiles + frame->offset, (void *)(VRAM + TILE_OFFSET_4BPP(DOOR_TILE_START_SIZE1)), 8 * TILE_SIZE_4BPP);
}

static void BuildDoorTiles(u16 *tiles, u16 tileNum, const u8 *paletteNums)
{
    int i;
    u16 tile;

    // Only the first 4 tiles of each metatile (bottom layer) actually use the door tiles
    for (i = 0; i < 4; i++)
    {
        tile = *(paletteNums++) << 12;
        tiles[i] = tile | (tileNum + i);
    }

    // The remaining layers are left as tile 0 (with the same palette)
    for (; i < 8; i++)
    {
        tile = *(paletteNums++) << 12;
        tiles[i] = tile;
    }
}

static void DrawCurrentDoorAnimFrame(const struct DoorGraphics *gfx, u32 x, u32 y, const u8 *paletteNums)
{
    u16 tiles[24];

    if (gfx->size == DOOR_SIZE_1x1)
    {
        // Only the bottom metatile animates
        BuildDoorTiles(&tiles[0], DOOR_TILE_START_SIZE1 + 0, &paletteNums[0]);
        DrawDoorMetatileAt(x, y, &tiles[0]);
    }
    else if (gfx->size == DOOR_SIZE_2x2)
    {
        // Top left metatile
        BuildDoorTiles(&tiles[8], DOOR_TILE_START_SIZE2 + 0, &paletteNums[0]);
        DrawDoorMetatileAt(x, y - 1, &tiles[8]);

        // Bottom left metatile
        BuildDoorTiles(&tiles[8], DOOR_TILE_START_SIZE2 + 4, &paletteNums[4]);
        DrawDoorMetatileAt(x, y, &tiles[8]);

        // Top right metatile
        BuildDoorTiles(&tiles[8], DOOR_TILE_START_SIZE2 + 8, &paletteNums[0]);
        DrawDoorMetatileAt(x + 1, y - 1, &tiles[8]);

        // Bottom right metatile
        BuildDoorTiles(&tiles[8], DOOR_TILE_START_SIZE2 + 12, &paletteNums[4]);
        DrawDoorMetatileAt(x + 1, y, &tiles[8]);
    }
    else
    {
        // Top metatile
        BuildDoorTiles(&tiles[0], DOOR_TILE_START_SIZE1 + 0, &paletteNums[0]);
        DrawDoorMetatileAt(x, y - 1, &tiles[0]);

        // Bottom metatile
        BuildDoorTiles(&tiles[0], DOOR_TILE_START_SIZE1 + 4, &paletteNums[4]);
        DrawDoorMetatileAt(x, y, &tiles[0]);
    }
}

static void DrawClosedDoorTiles(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    if (gfx->size != DOOR_SIZE_1x1)
        CurrentMapDrawMetatileAt(x, y - 1);
    CurrentMapDrawMetatileAt(x, y);

    if (gfx->size == DOOR_SIZE_2x2)
    {
        CurrentMapDrawMetatileAt(x + 1, y - 1);
        CurrentMapDrawMetatileAt(x + 1, y);
    }
}

static void SetOpenDoorFrame(const struct DoorGraphics *gfx, const struct DoorAnimFrame *frame, u32 x, u32 y)
{
    if (frame->offset == 0xFFFF)
    {
        sOpenDoorFrame = -1;
    }
    else
    {
        sOpenDoorFrame = frame->offset / GetDoorFrameSize(gfx);
        sOpenDoorX = x;
        sOpenDoorY = y;
    }
}

static void DrawDoor(const struct DoorGraphics *gfx, const struct DoorAnimFrame *frame, u32 x, u32 y)
{
    SetOpenDoorFrame(gfx, frame, x, y);
    if (frame->offset == 0xFFFF)
    {
        DrawClosedDoorTiles(gfx, x, y);
        if (ShouldUseMultiCorridorDoor())
            DrawClosedDoorTiles(gfx, gSpecialVar_0x8004 + MAP_OFFSET, gSpecialVar_0x8005 + MAP_OFFSET);
    }
    else
    {
        CopyDoorTilesToVram(gfx, frame);
        DrawCurrentDoorAnimFrame(gfx, x, y, gfx->palettes);
        if (ShouldUseMultiCorridorDoor())
            DrawCurrentDoorAnimFrame(gfx, gSpecialVar_0x8004 + MAP_OFFSET, gSpecialVar_0x8005 + MAP_OFFSET, gfx->palettes);
    }
}

#define tFramesHi data[0]
#define tFramesLo data[1]
#define tGfxHi data[2]
#define tGfxLo data[3]
#define tFrameId data[4]
#define tCounter data[5]
#define tX data[6]
#define tY data[7]

// Draws a single frame of the door animation, or skips drawing to wait between frames.
// Returns FALSE when the final frame has been reached
static bool32 AnimateDoorFrame(struct DoorGraphics *gfx, struct DoorAnimFrame *frames, s16 *data)
{
    if (tCounter == 0)
        DrawDoor(gfx, &frames[tFrameId], tX, tY);

    if (tCounter == frames[tFrameId].time)
    {
        tCounter = 0;
        tFrameId++;
        if (frames[tFrameId].time == 0)
            return FALSE;
        // The next frame is drawn on the next run. Between two frames of animation graphics only the
        // tiles change, and they reach VRAM straight away, while sprites change at the VBlank, so the
        // glowing door glass is told a frame early to change with them. (To or from the closed door,
        // the tilemap changes, at the VBlank like the sprites.)
        if (frames[tFrameId].offset != 0xFFFF && frames[tFrameId - 1].offset != 0xFFFF)
            SetOpenDoorFrame(gfx, &frames[tFrameId], tX, tY);
        return TRUE;
    }
    tCounter++;
    return TRUE;
}

static void Task_AnimateDoor(u8 taskId)
{
    u16 *data = (u16*) gTasks[taskId].data;
    struct DoorAnimFrame *frames = (struct DoorAnimFrame *)(tFramesHi << 16 | tFramesLo);
    struct DoorGraphics *gfx = (struct DoorGraphics *)(tGfxHi << 16 | tGfxLo);

    if (AnimateDoorFrame(gfx, frames, data) == FALSE)
        DestroyTask(taskId);
}

static const struct DoorAnimFrame *GetLastDoorFrame(const struct DoorAnimFrame *frame, const void *unused)
{
    while (frame->time != 0)
        frame++;
    return frame - 1;
}

static const struct DoorGraphics *GetDoorGraphics(const struct DoorGraphics *gfx, u16 metatileNum)
{
    while (gfx->tiles != NULL)
    {
        if (gfx->metatileNum == metatileNum)
            return gfx;
        gfx++;
    }
    return NULL;
}

static const struct DoorGraphics *GetDoorGraphicsTable(void)
{
    const struct Tileset *primaryTileset = gMapHeader.mapLayout->primaryTileset;

    if (primaryTileset == &gTileset_KantoGeneral || primaryTileset == &gTileset_KantoBuilding)
        return sKantoDoorAnimGraphicsTable;
    return sDoorAnimGraphicsTable;
}

static s8 StartDoorAnimationTask(const struct DoorGraphics *gfx, const struct DoorAnimFrame *frames, u32 x, u32 y)
{
    if (FuncIsActiveTask(Task_AnimateDoor) == TRUE)
    {
        return -1;
    }
    else
    {
        u8 taskId = CreateTask(Task_AnimateDoor, 0x50);
        s16 *data = gTasks[taskId].data;

        tX = x;
        tY = y;

        tFramesLo = (u32)frames;
        tFramesHi = (u32)frames >> 16;

        tGfxLo = (u32)gfx;
        tGfxHi = (u32)gfx >> 16;

        return taskId;
    }
}

static void DrawClosedDoor(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    sOpenDoorFrame = -1;
    DrawClosedDoorTiles(gfx, x, y);
}

static void DrawOpenedDoor(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    gfx = GetDoorGraphics(gfx, MapGridGetMetatileIdAt(x, y));
    if (gfx != NULL)
    {
        if (gfx->size == DOOR_SIZE_1x1)
            DrawDoor(gfx, GetLastDoorFrame(sSmallDoorOpenAnimFrames, sSmallDoorOpenAnimFrames), x, y);
        else
            DrawDoor(gfx, GetLastDoorFrame(sDoorOpenAnimFrames, sDoorOpenAnimFrames), x, y);
    }
}

static s8 StartDoorOpenAnimation(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    gfx = GetDoorGraphics(gfx, MapGridGetMetatileIdAt(x, y));
    if (gfx == NULL)
    {
        return -1;
    }
    else
    {
        if (gfx->size == DOOR_SIZE_2x2)
            return StartDoorAnimationTask(gfx, sBigDoorOpenAnimFrames, x, y);
        else if (gfx->size == DOOR_SIZE_1x1)
            return StartDoorAnimationTask(gfx, sSmallDoorOpenAnimFrames, x, y);
        else
            return StartDoorAnimationTask(gfx, sDoorOpenAnimFrames, x, y);
    }
}

static s8 StartDoorCloseAnimation(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    gfx = GetDoorGraphics(gfx, MapGridGetMetatileIdAt(x, y));
    if (gfx == NULL)
        return -1;
    else if (gfx->size == DOOR_SIZE_1x1)
        return StartDoorAnimationTask(gfx, sSmallDoorCloseAnimFrames, x, y);
    else
        return StartDoorAnimationTask(gfx, sDoorCloseAnimFrames, x, y);
}

static s8 GetDoorSoundType(const struct DoorGraphics *gfx, u32 x, u32 y)
{
    gfx = GetDoorGraphics(gfx, MapGridGetMetatileIdAt(x, y));
    if (gfx == NULL)
        return -1;
    else
        return gfx->sound;
}

// Debug? Same as FieldAnimateDoorOpen but doesnt return or check if metatile is actually a door
static void UNUSED Debug_FieldAnimateDoorOpen(u32 x, u32 y)
{
    StartDoorOpenAnimation(GetDoorGraphicsTable(), x, y);
}

// The frame of its animation graphics the door at (x, y) is drawn with, or -1 if it's drawn closed.
s8 FieldGetDoorAnimFrame(s16 x, s16 y)
{
    if (x != sOpenDoorX || y != sOpenDoorY)
        return -1;
    return sOpenDoorFrame;
}

void FieldSetDoorOpened(u32 x, u32 y)
{
    if (MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x, y)))
        DrawOpenedDoor(GetDoorGraphicsTable(), x, y);
}

void FieldSetDoorClosed(u32 x, u32 y)
{
    if (MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x, y)))
        DrawClosedDoor(GetDoorGraphicsTable(), x, y);
}

s8 FieldAnimateDoorClose(u32 x, u32 y)
{
    if (!MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x, y)))
        return -1;
    else
        return StartDoorCloseAnimation(GetDoorGraphicsTable(), x, y);
}

s8 FieldAnimateDoorOpen(u32 x, u32 y)
{
    if (!MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x, y)))
        return -1;
    else
        return StartDoorOpenAnimation(GetDoorGraphicsTable(), x, y);
}

bool8 FieldIsDoorAnimationRunning(void)
{
    return FuncIsActiveTask(Task_AnimateDoor);
}

u32 GetDoorSoundEffect(u32 x, u32 y)
{
    int sound = GetDoorSoundType(GetDoorGraphicsTable(), x, y);

    if (sound == DOOR_SOUND_NORMAL)
        return SE_DOOR;
    else if (sound == DOOR_SOUND_SLIDING)
        return SE_SLIDING_DOOR;
    else if (sound == DOOR_SOUND_ARENA)
        return SE_REPEL;
    else
        return SE_DOOR;
}

// Opens the Battle Tower multi partner's door in sync with the player's door
static bool8 ShouldUseMultiCorridorDoor(void)
{
    if (FlagGet(FLAG_ENABLE_MULTI_CORRIDOR_DOOR))
    {
        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_CORRIDOR)
            && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_BATTLE_FRONTIER_BATTLE_TOWER_MULTI_CORRIDOR))
        {
            return TRUE;
        }
    }
    return FALSE;
}
