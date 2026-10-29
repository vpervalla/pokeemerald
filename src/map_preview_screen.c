// FRLG's map preview screens (pokefirered's map_preview_screen.c): on entering some Kanto and Sevii
// dungeons from another map section, a picture of the place with its name in the top-left corner.
// A cave's preview (MPS_TYPE_CAVE) is shown by fldeff_flash.c on the black screen of the map change,
// before the map loads and its cave transition; a forest's (MPS_TYPE_FOREST) is drawn on BG0 over the
// loaded map, which fades in under it, and is then blended away. The first visit shows it for longer.
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "malloc.h"
#include "map_preview_screen.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "region_map.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/region_map_sections.h"
#include "constants/weather.h"

#define DURATION_FIRST_VISIT 120
#define DURATION             40

struct MapPreviewScreen
{
    mapsec_u16_t mapsec;
    u8 type;
    u16 flagId; // Set once shown
    const u32 *tiles;
    const u32 *tilemap;
    const u16 *palette;
};

static void Task_RunMapPreviewScreenForest(u8 taskId);

static EWRAM_DATA bool8 sAllocedBg0TilemapBuffer = FALSE;
// The forest's preview takes BG palettes 13 and 14 from the field (14 is its windows' border)
static EWRAM_DATA u16 sSavedFieldPalettes[2 * 16] = {0};

static const u16 sAlteringCave_Pal[] = INCGFX_U16("graphics/map_preview/altering_cave/tiles.png", ".gbapal");
static const u32 sAlteringCave_Gfx[] = INCGFX_U32("graphics/map_preview/altering_cave/tiles.png", ".4bpp.lz", "-num_tiles 391 -Wnum_tiles");
static const u32 sAlteringCave_Tilemap[] = INCGFX_U32("graphics/map_preview/altering_cave/tilemap.bin", ".lz");
static const u16 sBerryForest_Pal[] = INCGFX_U16("graphics/map_preview/berry_forest/tiles.png", ".gbapal");
static const u32 sBerryForest_Gfx[] = INCGFX_U32("graphics/map_preview/berry_forest/tiles.png", ".4bpp.lz", "-num_tiles 395 -Wnum_tiles");
static const u32 sBerryForest_Tilemap[] = INCGFX_U32("graphics/map_preview/berry_forest/tilemap.bin", ".lz");
static const u16 sCeruleanCave_Pal[] = INCGFX_U16("graphics/map_preview/cerulean_cave/tiles.png", ".gbapal");
static const u32 sCeruleanCave_Gfx[] = INCGFX_U32("graphics/map_preview/cerulean_cave/tiles.png", ".4bpp.lz");
static const u32 sCeruleanCave_Tilemap[] = INCGFX_U32("graphics/map_preview/cerulean_cave/tilemap.bin", ".lz");
static const u16 sDiglettsCave_Pal[] = INCGFX_U16("graphics/map_preview/digletts_cave/tiles.png", ".gbapal");
static const u32 sDiglettsCave_Gfx[] = INCGFX_U32("graphics/map_preview/digletts_cave/tiles.png", ".4bpp.lz", "-num_tiles 398 -Wnum_tiles");
static const u32 sDiglettsCave_Tilemap[] = INCGFX_U32("graphics/map_preview/digletts_cave/tilemap.bin", ".lz");
static const u16 sDottedHole_Pal[] = INCGFX_U16("graphics/map_preview/dotted_hole/tiles.png", ".gbapal");
static const u32 sDottedHole_Gfx[] = INCGFX_U32("graphics/map_preview/dotted_hole/tiles.png", ".4bpp.lz", "-num_tiles 317 -Wnum_tiles");
static const u32 sDottedHole_Tilemap[] = INCGFX_U32("graphics/map_preview/dotted_hole/tilemap.bin", ".lz");
static const u16 sIcefallCave_Pal[] = INCGFX_U16("graphics/map_preview/icefall_cave/tiles.png", ".gbapal");
static const u32 sIcefallCave_Gfx[] = INCGFX_U32("graphics/map_preview/icefall_cave/tiles.png", ".4bpp.lz", "-num_tiles 399 -Wnum_tiles");
static const u32 sIcefallCave_Tilemap[] = INCGFX_U32("graphics/map_preview/icefall_cave/tilemap.bin", ".lz");
static const u16 sLostCave_Pal[] = INCGFX_U16("graphics/map_preview/lost_cave/tiles.png", ".gbapal");
static const u32 sLostCave_Gfx[] = INCGFX_U32("graphics/map_preview/lost_cave/tiles.png", ".4bpp.lz", "-num_tiles 404 -Wnum_tiles");
static const u32 sLostCave_Tilemap[] = INCGFX_U32("graphics/map_preview/lost_cave/tilemap.bin", ".lz");
static const u16 sMoneanChamber_Pal[] = INCGFX_U16("graphics/map_preview/monean_chamber/tiles.png", ".gbapal");
static const u32 sMoneanChamber_Gfx[] = INCGFX_U32("graphics/map_preview/monean_chamber/tiles.png", ".4bpp.lz", "-num_tiles 326 -Wnum_tiles");
static const u32 sMoneanChamber_Tilemap[] = INCGFX_U32("graphics/map_preview/monean_chamber/tilemap.bin", ".lz");
static const u16 sMtEmber_Pal[] = INCGFX_U16("graphics/map_preview/mt_ember/tiles.png", ".gbapal");
static const u32 sMtEmber_Gfx[] = INCGFX_U32("graphics/map_preview/mt_ember/tiles.png", ".4bpp.lz", "-num_tiles 355 -Wnum_tiles");
static const u32 sMtEmber_Tilemap[] = INCGFX_U32("graphics/map_preview/mt_ember/tilemap.bin", ".lz");
static const u16 sMtMoon_Pal[] = INCGFX_U16("graphics/map_preview/mt_moon/tiles.png", ".gbapal");
static const u32 sMtMoon_Gfx[] = INCGFX_U32("graphics/map_preview/mt_moon/tiles.png", ".4bpp.lz", "-num_tiles 364 -Wnum_tiles");
static const u32 sMtMoon_Tilemap[] = INCGFX_U32("graphics/map_preview/mt_moon/tilemap.bin", ".lz");
static const u16 sPokemonMansion_Pal[] = INCGFX_U16("graphics/map_preview/pokemon_mansion/tiles.png", ".gbapal");
static const u32 sPokemonMansion_Gfx[] = INCGFX_U32("graphics/map_preview/pokemon_mansion/tiles.png", ".4bpp.lz", "-num_tiles 388 -Wnum_tiles");
static const u32 sPokemonMansion_Tilemap[] = INCGFX_U32("graphics/map_preview/pokemon_mansion/tilemap.bin", ".lz");
static const u16 sPokemonTower_Pal[] = INCGFX_U16("graphics/map_preview/pokemon_tower/tiles.png", ".gbapal");
static const u32 sPokemonTower_Gfx[] = INCGFX_U32("graphics/map_preview/pokemon_tower/tiles.png", ".4bpp.lz", "-num_tiles 290 -Wnum_tiles");
static const u32 sPokemonTower_Tilemap[] = INCGFX_U32("graphics/map_preview/pokemon_tower/tilemap.bin", ".lz");
static const u16 sPowerPlant_Pal[] = INCGFX_U16("graphics/map_preview/power_plant/tiles.png", ".gbapal");
static const u32 sPowerPlant_Gfx[] = INCGFX_U32("graphics/map_preview/power_plant/tiles.png", ".4bpp.lz", "-num_tiles 368 -Wnum_tiles");
static const u32 sPowerPlant_Tilemap[] = INCGFX_U32("graphics/map_preview/power_plant/tilemap.bin", ".lz");
static const u16 sRockTunnel_Pal[] = INCGFX_U16("graphics/map_preview/rock_tunnel/tiles.png", ".gbapal");
static const u32 sRockTunnel_Gfx[] = INCGFX_U32("graphics/map_preview/rock_tunnel/tiles.png", ".4bpp.lz", "-num_tiles 407 -Wnum_tiles");
static const u32 sRockTunnel_Tilemap[] = INCGFX_U32("graphics/map_preview/rock_tunnel/tilemap.bin", ".lz");
static const u16 sRocketHideout_Pal[] = INCGFX_U16("graphics/map_preview/rocket_hideout/tiles.png", ".gbapal");
static const u32 sRocketHideout_Gfx[] = INCGFX_U32("graphics/map_preview/rocket_hideout/tiles.png", ".4bpp.lz", "-num_tiles 194 -Wnum_tiles");
static const u32 sRocketHideout_Tilemap[] = INCGFX_U32("graphics/map_preview/rocket_hideout/tilemap.bin", ".lz");
static const u16 sRocketWarehouse_Pal[] = INCGFX_U16("graphics/map_preview/rocket_warehouse/tiles.png", ".gbapal");
static const u32 sRocketWarehouse_Gfx[] = INCGFX_U32("graphics/map_preview/rocket_warehouse/tiles.png", ".4bpp.lz", "-num_tiles 234 -Wnum_tiles");
static const u32 sRocketWarehouse_Tilemap[] = INCGFX_U32("graphics/map_preview/rocket_warehouse/tilemap.bin", ".lz");
static const u16 sSafariZone_Pal[] = INCGFX_U16("graphics/map_preview/safari_zone/tiles.png", ".gbapal");
static const u32 sSafariZone_Gfx[] = INCGFX_U32("graphics/map_preview/safari_zone/tiles.png", ".4bpp.lz", "-num_tiles 330 -Wnum_tiles");
static const u32 sSafariZone_Tilemap[] = INCGFX_U32("graphics/map_preview/safari_zone/tilemap.bin", ".lz");
static const u16 sSeafoamIslands_Pal[] = INCGFX_U16("graphics/map_preview/seafoam_islands/tiles.png", ".gbapal");
static const u32 sSeafoamIslands_Gfx[] = INCGFX_U32("graphics/map_preview/seafoam_islands/tiles.png", ".4bpp.lz", "-num_tiles 408 -Wnum_tiles");
static const u32 sSeafoamIslands_Tilemap[] = INCGFX_U32("graphics/map_preview/seafoam_islands/tilemap.bin", ".lz");
static const u16 sSilphCo_Pal[] = INCGFX_U16("graphics/map_preview/silph_co/tiles.png", ".gbapal");
static const u32 sSilphCo_Gfx[] = INCGFX_U32("graphics/map_preview/silph_co/tiles.png", ".4bpp.lz", "-num_tiles 355 -Wnum_tiles");
static const u32 sSilphCo_Tilemap[] = INCGFX_U32("graphics/map_preview/silph_co/tilemap.bin", ".lz");
static const u16 sVictoryRoad_Pal[] = INCGFX_U16("graphics/map_preview/victory_road/tiles.png", ".gbapal");
static const u32 sVictoryRoad_Gfx[] = INCGFX_U32("graphics/map_preview/victory_road/tiles.png", ".4bpp.lz", "-num_tiles 375 -Wnum_tiles");
static const u32 sVictoryRoad_Tilemap[] = INCGFX_U32("graphics/map_preview/victory_road/tilemap.bin", ".lz");
static const u16 sViridianForest_Pal[] = INCGFX_U16("graphics/map_preview/viridian_forest/tiles.png", ".gbapal");
static const u32 sViridianForest_Gfx[] = INCGFX_U32("graphics/map_preview/viridian_forest/tiles.png", ".4bpp.lz", "-num_tiles 389 -Wnum_tiles");
static const u32 sViridianForest_Tilemap[] = INCGFX_U32("graphics/map_preview/viridian_forest/tilemap.bin", ".lz");

#define PREVIEW(sec, typ, flag, gfx) {MAPSEC_##sec, MPS_TYPE_##typ, FLAG_SEEN_MAP_PREVIEW_##flag, s##gfx##_Gfx, s##gfx##_Tilemap, s##gfx##_Pal}

static const struct MapPreviewScreen sMapPreviewScreenData[] =
{
    PREVIEW(VIRIDIAN_FOREST,    FOREST, VIRIDIAN_FOREST,  ViridianForest),
    PREVIEW(MT_MOON,            CAVE,   MT_MOON,          MtMoon),
    PREVIEW(DIGLETTS_CAVE,      CAVE,   DIGLETTS_CAVE,    DiglettsCave),
    PREVIEW(ROCK_TUNNEL,        CAVE,   ROCK_TUNNEL,      RockTunnel),
    PREVIEW(POKEMON_TOWER,      CAVE,   POKEMON_TOWER,    PokemonTower),
    PREVIEW(KANTO_SAFARI_ZONE,  FOREST, SAFARI_ZONE,      SafariZone),
    PREVIEW(SEAFOAM_ISLANDS,    CAVE,   SEAFOAM_ISLANDS,  SeafoamIslands),
    PREVIEW(POKEMON_MANSION,    FOREST, POKEMON_MANSION,  PokemonMansion),
    PREVIEW(ROCKET_HIDEOUT,     FOREST, ROCKET_HIDEOUT,   RocketHideout),
    PREVIEW(SILPH_CO,           CAVE,   SILPH_CO,         SilphCo),
    PREVIEW(KANTO_VICTORY_ROAD, CAVE,   VICTORY_ROAD,     VictoryRoad),
    PREVIEW(CERULEAN_CAVE,      CAVE,   CERULEAN_CAVE,    CeruleanCave),
    PREVIEW(POWER_PLANT,        FOREST, POWER_PLANT,      PowerPlant),
    PREVIEW(MT_EMBER,           CAVE,   MT_EMBER,         MtEmber),
    PREVIEW(ROCKET_WAREHOUSE,   FOREST, ROCKET_WAREHOUSE, RocketWarehouse),
    PREVIEW(MONEAN_CHAMBER,     CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(DOTTED_HOLE,        CAVE,   DOTTED_HOLE,      DottedHole),
    PREVIEW(BERRY_FOREST,       FOREST, BERRY_FOREST,     BerryForest),
    PREVIEW(ICEFALL_CAVE,       CAVE,   ICEFALL_CAVE,     IcefallCave),
    PREVIEW(LOST_CAVE,          CAVE,   LOST_CAVE,        LostCave),
    PREVIEW(ALTERING_CAVE_FRLG, CAVE,   ALTERING_CAVE,    AlteringCave),
    PREVIEW(PATTERN_BUSH,       FOREST, PATTERN_BUSH,     ViridianForest), // As in FRLG
    // The other TANOBY chambers show the MONEAN CHAMBER's, as in FRLG
    PREVIEW(LIPTOO_CHAMBER,     CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(WEEPTH_CHAMBER,     CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(DILFORD_CHAMBER,    CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(SCUFIB_CHAMBER,     CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(RIXY_CHAMBER,       CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
    PREVIEW(VIAPOIS_CHAMBER,    CAVE,   TANOBY_CHAMBERS,  MoneanChamber),
};

#undef PREVIEW

// In the preview's palette 14: the box's white, the name's red and its light grey shadow
static const struct WindowTemplate sMapNameWindow =
{
    .bg = 0,
    .tilemapLeft = 0,
    .tilemapTop = 0,
    .width = 13,
    .height = 2,
    .paletteNum = 14,
    .baseBlock = 0x1C2, // After the pictures' tiles (at most 408)
};

static const struct BgTemplate sMapPreviewBgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static const struct MapPreviewScreen *GetMapPreviewScreen(u8 mapsec)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMapPreviewScreenData); i++)
    {
        if (sMapPreviewScreenData[i].mapsec == mapsec)
            return &sMapPreviewScreenData[i];
    }
    return NULL;
}

bool8 MapHasPreviewScreen(u8 mapsec, u8 type)
{
    const struct MapPreviewScreen *preview = GetMapPreviewScreen(mapsec);

    if (preview == NULL)
        return FALSE;
    return type == MPS_TYPE_ANY || preview->type == type;
}

// A forest's preview is drawn over the map, so it goes without weathers drawn over the screen too
// (the SAFARI ZONE's): their sprites would cover it, and fog and the like take the blend it fades with.
static bool8 IsWeatherClearOfPreview(void)
{
    switch (gWeatherPtr->nextWeather)
    {
    case WEATHER_NONE:
    case WEATHER_SUNNY:
    case WEATHER_SHADE:
    case WEATHER_DROUGHT:
        return TRUE;
    }
    return FALSE;
}

// The warp that loaded the current map came from another map section, into one with a preview of this type
bool8 MapHasPreviewScreenOnWarp(u8 type)
{
    const struct MapHeader *lastMap = Overworld_GetMapHeaderByGroupAndId(gLastUsedWarp.mapGroup, gLastUsedWarp.mapNum);

    return lastMap->regionMapSectionId != gMapHeader.regionMapSectionId
        && MapHasPreviewScreen(gMapHeader.regionMapSectionId, type)
        && (type != MPS_TYPE_FOREST || IsWeatherClearOfPreview());
}

void MapPreview_InitBgs(void)
{
    InitBgsFromTemplates(0, sMapPreviewBgTemplate, ARRAY_COUNT(sMapPreviewBgTemplate));
    ShowBg(0);
}

void MapPreview_LoadGfx(u8 mapsec)
{
    const struct MapPreviewScreen *preview = GetMapPreviewScreen(mapsec);

    if (preview == NULL)
        return;
    ResetTempTileDataBuffers();
    LoadPalette(preview->palette, BG_PLTT_ID(13), 2 * PLTT_SIZE_4BPP);
    DecompressAndCopyTileDataToVram(0, preview->tiles, 0, 0, 0);
    if (GetBgTilemapBuffer(0) == NULL)
    {
        SetBgTilemapBuffer(0, Alloc(BG_SCREEN_SIZE));
        sAllocedBg0TilemapBuffer = TRUE;
    }
    else
    {
        sAllocedBg0TilemapBuffer = FALSE;
    }
    CopyToBgTilemapBuffer(0, preview->tilemap, 0, 0);
    CopyBgTilemapBufferToVram(0);
}

bool32 MapPreview_IsGfxLoadFinished(void)
{
    return FreeTempTileDataBuffersIfPossible();
}

u16 MapPreview_CreateMapNameWindow(u8 mapsec)
{
    static const u8 color[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_GRAY};
    u16 windowId = AddWindow(&sMapNameWindow);
    u32 x;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    PutWindowTilemap(windowId);
    GetMapName(gStringVar4, mapsec, 0);
    x = 104 - GetStringWidth(FONT_NORMAL, gStringVar4, 0);
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, x / 2, 2, 0, 0, color, TEXT_SKIP_DRAW, gStringVar4);
    return windowId;
}

void MapPreview_Unload(s32 windowId)
{
    RemoveWindow(windowId);
    if (sAllocedBg0TilemapBuffer)
    {
        Free(GetBgTilemapBuffer(0));
        SetBgTilemapBuffer(0, NULL);
        sAllocedBg0TilemapBuffer = FALSE;
    }
}

// How long the preview stays, in frames: longer the first time. That's this time, from now on.
u16 MapPreview_GetDuration(u8 mapsec)
{
    const struct MapPreviewScreen *preview = GetMapPreviewScreen(mapsec);

    if (preview == NULL)
        return 0;
    if (!FlagGet(preview->flagId))
    {
        FlagSet(preview->flagId);
        return DURATION_FIRST_VISIT;
    }
    return DURATION;
}

#define tState      data[0]
#define tTimer      data[1]
#define tBg0Prio    data[2]
#define tDispCnt    data[3]
#define tBldCnt     data[4]
#define tBldAlpha   data[5]
#define tWinIn      data[6]
#define tWinOut     data[7]
#define tEva        data[8]
#define tEvb        data[9]
#define tDuration   data[10]
#define tWindowId   data[11]

// Called by the map load, once the map is drawn, with the screen black: the warp's fade in follows.
void MapPreview_StartForestTransition(u8 mapsec)
{
    u8 taskId = CreateTask(Task_RunMapPreviewScreenForest, 0);
    s16 *data = gTasks[taskId].data;

    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(13)], sSavedFieldPalettes, sizeof(sSavedFieldPalettes));
    MapPreview_LoadGfx(mapsec);
    tBg0Prio = GetBgAttribute(0, BG_ATTR_PRIORITY);
    tBldCnt = GetGpuReg(REG_OFFSET_BLDCNT);
    tBldAlpha = GetGpuReg(REG_OFFSET_BLDALPHA);
    tDispCnt = GetGpuReg(REG_OFFSET_DISPCNT);
    tWinIn = GetGpuReg(REG_OFFSET_WININ);
    tWinOut = GetGpuReg(REG_OFFSET_WINOUT);
    tDuration = MapPreview_GetDuration(mapsec);
    tEva = 16;
    tEvb = 0;
    SetBgAttribute(0, BG_ATTR_PRIORITY, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2
                               | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_OBJ | BLDCNT_TGT2_BD);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
    SetGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_CLR | WININ_WIN1_CLR);
    SetGpuRegBits(REG_OFFSET_WINOUT, WINOUT_WIN01_CLR);
    tWindowId = MapPreview_CreateMapNameWindow(mapsec);
}

bool32 MapPreview_IsForestTransitionRunning(void)
{
    return FuncIsActiveTask(Task_RunMapPreviewScreenForest);
}

static void Task_RunMapPreviewScreenForest(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        if (!MapPreview_IsGfxLoadFinished() && !IsDma3ManagerBusyWithBgCopy())
        {
            CopyWindowToVram(tWindowId, COPYWIN_FULL);
            tState++;
        }
        break;
    case 1:
        // The warp's fade in, under the preview
        if (!IsDma3ManagerBusyWithBgCopy() && IsWeatherNotFadingIn())
            tState++;
        break;
    case 2:
        if (++tTimer > tDuration)
        {
            tTimer = 0;
            tState++;
        }
        break;
    case 3:
        // The preview fades out over the map: one step of each weight in turn, every third frame
        switch (tTimer)
        {
        case 0:
            if (++tEvb > 16)
                tEvb = 16;
            break;
        case 1:
            if (--tEva < 0)
                tEva = 0;
            break;
        }
        tTimer = (tTimer + 1) % 3;
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(tEva, tEvb));
        if (tEva == 0 && tEvb == 16)
        {
            FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
            CopyBgTilemapBufferToVram(0);
            tState++;
        }
        break;
    case 4:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            MapPreview_Unload(tWindowId);
            LoadPalette(sSavedFieldPalettes, BG_PLTT_ID(13), sizeof(sSavedFieldPalettes));
            SetBgAttribute(0, BG_ATTR_PRIORITY, tBg0Prio);
            SetGpuReg(REG_OFFSET_DISPCNT, tDispCnt);
            SetGpuReg(REG_OFFSET_BLDCNT, tBldCnt);
            SetGpuReg(REG_OFFSET_BLDALPHA, tBldAlpha);
            SetGpuReg(REG_OFFSET_WININ, tWinIn);
            SetGpuReg(REG_OFFSET_WINOUT, tWinOut);
            DestroyTask(taskId);
        }
        break;
    }
}
