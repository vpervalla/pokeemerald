#include "global.h"
#include "iv_checker.h"
#include "battle_main.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The IV CHECKER, a key item from the SILPH CO. scientist on 8F once TEAM ROCKET is gone.
// One POKéMON at a time: its picture, name, level, nature and Hidden Power on the left, its six IVs
// on the right (a number and a bar each, the stats its nature raises and lowers in red and blue).
// Up/Down: the next POKéMON. L/R (or Left/Right): the party, then each PC box with POKéMON in it.
// Eggs and empty boxes are skipped.

#define GROUP_PARTY     0   // then 1 + the box number
#define GROUP_COUNT     (1 + TOTAL_BOXES_COUNT)
#define MAX_GROUP_MONS  max(PARTY_SIZE, IN_BOX_COUNT)

#define BG_PALETTE      1
#define TEXT_PALETTE    15

// sBgTiles
#define TILE_BACKDROP   0
#define TILE_PANEL      1
#define TILE_BAR        2

// Colors of TEXT_PALETTE
#define COLOR_DARK          2
#define COLOR_DARK_SHADOW   3
#define COLOR_RED           4
#define COLOR_RED_SHADOW    5
#define COLOR_BLUE          6
#define COLOR_BLUE_SHADOW   7
#define COLOR_WHITE         8
#define COLOR_WHITE_SHADOW  9
#define COLOR_IV_GOOD       10  // 26-30
#define COLOR_IV_FAIR       11  // 16-25
#define COLOR_IV_LOW        12  // 1-15
#define COLOR_TRACK         14
#define COLOR_IV_BEST       15  // 31

// The IV list: a row per stat, then the total
#define IV_ROW_Y(i)     (4 + (i) * 16)
#define IV_VALUE_RIGHT  68
#define IV_BAR_X        74
#define IV_BAR_Y        6   // in the row
#define IV_BAR_HEIGHT   5
#define IV_BAR_SCALE    2   // pixels per IV point

#define INFO_TEXT_WIDTH (10 * 8 - 3)

#define PIC_X           48
#define PIC_Y           56

enum
{
    WIN_HEADER,
    WIN_INFO,
    WIN_IVS,
    WIN_HELP,
};

enum
{
    STATE_FADE_IN,
    STATE_INPUT,
    STATE_FADE_OUT,
};

struct IvChecker
{
    MainCallback exitCallback;
    struct BoxPokemon *mons[MAX_GROUP_MONS];
    u8 count;
    u8 index;
    u8 group;
    u8 state;
    u16 picSprite;
    u8 text[64];
};

static EWRAM_DATA struct IvChecker *sIv = NULL;

static void CB2_InitIvChecker(void);
static void Task_IvChecker(u8 taskId);

static const u8 sText_Party[] = _("PARTY");
static const u8 sText_Slash[] = _("/");
static const u8 sText_Lv[] = _("Lv. ");
static const u8 sText_HiddenPower[] = _("H.POWER ");
static const u8 sText_Space[] = _(" ");
static const u8 sText_Total[] = _("TOTAL");
static const u8 sText_TotalMax[] = _("/186");
static const u8 sText_Plus[] = _("+");
static const u8 sText_Minus[] = _("-");
static const u8 sText_NoPokemon[] = _("No POKéMON.");
static const u8 sText_Help[] = _("{DPAD_UPDOWN}POKéMON  {L_BUTTON}{R_BUTTON}PARTY/BOXES  {B_BUTTON}EXIT");

static const u8 sText_StatHp[] = _("HP");
static const u8 sText_StatAttack[] = _("ATTACK");
static const u8 sText_StatDefense[] = _("DEFENSE");
static const u8 sText_StatSpAtk[] = _("SP. ATK");
static const u8 sText_StatSpDef[] = _("SP. DEF");
static const u8 sText_StatSpeed[] = _("SPEED");

// In the summary screen's order
static const u8 *const sStatNames[NUM_STATS] = {sText_StatHp, sText_StatAttack, sText_StatDefense, sText_StatSpAtk, sText_StatSpDef, sText_StatSpeed};
static const u8 sStatIds[NUM_STATS] = {STAT_HP, STAT_ATK, STAT_DEF, STAT_SPATK, STAT_SPDEF, STAT_SPEED};
static const u8 sStatIvData[NUM_STATS] = {MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV, MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV, MON_DATA_SPEED_IV};

static const u8 sTextColors_Dark[] = {TEXT_COLOR_TRANSPARENT, COLOR_DARK, COLOR_DARK_SHADOW};
static const u8 sTextColors_Raised[] = {TEXT_COLOR_TRANSPARENT, COLOR_RED, COLOR_RED_SHADOW};
static const u8 sTextColors_Lowered[] = {TEXT_COLOR_TRANSPARENT, COLOR_BLUE, COLOR_BLUE_SHADOW};
static const u8 sTextColors_White[] = {TEXT_COLOR_TRANSPARENT, COLOR_WHITE, COLOR_WHITE_SHADOW};

static const u16 sBackdropColor = RGB(9, 14, 20);

static const u16 sBgPalette[16] =
{
    [TILE_PANEL] = RGB(29, 29, 27),
    [TILE_BAR]   = RGB(4, 7, 12),
};

static const u16 sTextPalette[16] =
{
    [COLOR_DARK]         = RGB(7, 7, 7),
    [COLOR_DARK_SHADOW]  = RGB(22, 22, 20),
    [COLOR_RED]          = RGB(26, 5, 4),
    [COLOR_RED_SHADOW]   = RGB(30, 20, 18),
    [COLOR_BLUE]         = RGB(4, 9, 26),
    [COLOR_BLUE_SHADOW]  = RGB(19, 21, 30),
    [COLOR_WHITE]        = RGB(31, 31, 31),
    [COLOR_WHITE_SHADOW] = RGB(2, 4, 8),
    [COLOR_IV_GOOD]      = RGB(6, 23, 8),
    [COLOR_IV_FAIR]      = RGB(21, 25, 4),
    [COLOR_IV_LOW]       = RGB(29, 18, 4),
    [COLOR_TRACK]        = RGB(23, 23, 21),
    [COLOR_IV_BEST]      = RGB(31, 24, 2),
};

#define SOLID_TILE(color) {(color) * 0x11111111, (color) * 0x11111111, (color) * 0x11111111, (color) * 0x11111111, \
                           (color) * 0x11111111, (color) * 0x11111111, (color) * 0x11111111, (color) * 0x11111111}

static const u32 sBgTiles[][8] =
{
    [TILE_BACKDROP] = SOLID_TILE(0),
    [TILE_PANEL]    = SOLID_TILE(TILE_PANEL),
    [TILE_BAR]      = SOLID_TILE(TILE_BAR),
};

static const struct BgTemplate sBgTemplates[] =
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
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 0,  .width = 28, .height = 2,  .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    [WIN_INFO]   = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 11, .width = 10, .height = 7,  .paletteNum = TEXT_PALETTE, .baseBlock = 1 + 56},
    [WIN_IVS]    = {.bg = 0, .tilemapLeft = 12, .tilemapTop = 3,  .width = 17, .height = 15, .paletteNum = TEXT_PALETTE, .baseBlock = 1 + 56 + 70},
    [WIN_HELP]   = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 18, .width = 28, .height = 2,  .paletteNum = TEXT_PALETTE, .baseBlock = 1 + 56 + 70 + 255},
    DUMMY_WIN_TEMPLATE,
};

// ---------------------------------------------------------------------------------------------
// The POKéMON

static bool8 IsCheckable(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_SANITY_HAS_SPECIES, NULL)
        && !GetBoxMonData(mon, MON_DATA_SANITY_IS_BAD_EGG, NULL)
        && !GetBoxMonData(mon, MON_DATA_IS_EGG, NULL);
}

// The group's POKéMON in sIv->mons; returns how many
static u8 LoadGroup(u8 group)
{
    u8 i, count = 0;
    struct BoxPokemon *mon;

    if (group == GROUP_PARTY)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            mon = &gPlayerParty[i].box;
            if (IsCheckable(mon))
                sIv->mons[count++] = mon;
        }
    }
    else
    {
        for (i = 0; i < IN_BOX_COUNT; i++)
        {
            mon = GetBoxedMonPtr(group - 1, i);
            if (IsCheckable(mon))
                sIv->mons[count++] = mon;
        }
    }
    sIv->count = count;
    sIv->group = group;
    sIv->index = 0;
    return count;
}

// The next group with POKéMON in that direction, back to the same one if there's no other
static bool8 ChangeGroup(s8 delta)
{
    u8 start = sIv->group;
    u8 group = start;

    do
    {
        group = (group + GROUP_COUNT + delta) % GROUP_COUNT;
        if (group == start)
            break;
    } while (!LoadGroup(group));

    if (group == start)
    {
        u8 index = sIv->index;

        LoadGroup(start);
        sIv->index = index;
        return FALSE;
    }
    return TRUE;
}

// ivs[] are in the screen's order: each IV's bit goes where Cmd_hiddenpowercalc puts it, by STAT_*
static u8 GetHiddenPowerType(const u8 *ivs)
{
    u8 type = 0, i;

    for (i = 0; i < NUM_STATS; i++)
        type |= (ivs[i] & 1) << sStatIds[i];
    // As Cmd_hiddenpowercalc: Fighting to Dark, skipping Normal and Mystery
    type = ((TYPE_DARK - 2) * type) / 63 + 1;
    if (type >= TYPE_MYSTERY)
        type++;
    return type;
}

static u8 GetHiddenPowerPower(const u8 *ivs)
{
    u8 power = 0, i;

    for (i = 0; i < NUM_STATS; i++)
        power |= ((ivs[i] & 2) >> 1) << sStatIds[i];
    return (40 * power) / 63 + 30;
}

// ---------------------------------------------------------------------------------------------
// The screen

static void DrawBackground(void)
{
    FillBgTilemapBufferRect(1, TILE_BACKDROP, 0, 0, 32, 32, BG_PALETTE);
    FillBgTilemapBufferRect(1, TILE_BAR, 0, 0, 30, 2, BG_PALETTE);
    FillBgTilemapBufferRect(1, TILE_BAR, 0, 18, 30, 2, BG_PALETTE);
    FillBgTilemapBufferRect(1, TILE_PANEL, 1, 3, 10, 15, BG_PALETTE);
    FillBgTilemapBufferRect(1, TILE_PANEL, 12, 3, 17, 15, BG_PALETTE);
    CopyBgTilemapBufferToVram(1);
}

static void PrintHelp(void)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 0, 3, sTextColors_White, TEXT_SKIP_DRAW, sText_Help);
    PutWindowTilemap(WIN_HELP);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

static void PrintHeader(void)
{
    u8 *end;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
    StringCopy(sIv->text, sIv->group == GROUP_PARTY ? sText_Party : GetBoxNamePtr(sIv->group - 1));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 0, 1, sTextColors_White, TEXT_SKIP_DRAW, sIv->text);
    if (sIv->count != 0)
    {
        end = ConvertIntToDecimalStringN(sIv->text, sIv->index + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        end = StringAppend(end, sText_Slash);
        ConvertIntToDecimalStringN(end, sIv->count, STR_CONV_MODE_LEFT_ALIGN, 2);
        AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, sIv->text, 28 * 8 - 2),
                                     1, sTextColors_White, TEXT_SKIP_DRAW, sIv->text);
    }
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static u8 GetIvBarColor(u8 iv)
{
    if (iv == MAX_PER_STAT_IVS)
        return COLOR_IV_BEST;
    if (iv >= 26)
        return COLOR_IV_GOOD;
    if (iv >= 16)
        return COLOR_IV_FAIR;
    return COLOR_IV_LOW;
}

static void PrintIvs(struct BoxPokemon *mon, const u8 *ivs, u8 nature)
{
    const u8 *colors;
    u16 total = 0;
    u8 i, y;
    s8 effect;

    FillWindowPixelBuffer(WIN_IVS, PIXEL_FILL(0));
    for (i = 0; i < NUM_STATS; i++)
    {
        y = IV_ROW_Y(i);
        effect = sStatIds[i] == STAT_HP ? 0 : gNatureStatTable[nature][sStatIds[i] - 1];
        colors = effect > 0 ? sTextColors_Raised : effect < 0 ? sTextColors_Lowered : sTextColors_Dark;

        // The stat, "+" or "-" if the nature raises or lowers it
        StringCopy(sIv->text, sStatNames[i]);
        if (effect > 0)
            StringAppend(sIv->text, sText_Plus);
        else if (effect < 0)
            StringAppend(sIv->text, sText_Minus);
        AddTextPrinterParameterized3(WIN_IVS, FONT_NORMAL, 2, y, colors, TEXT_SKIP_DRAW, sIv->text);

        // The IV, and its bar
        ConvertIntToDecimalStringN(sIv->text, ivs[i], STR_CONV_MODE_LEFT_ALIGN, 2);
        AddTextPrinterParameterized3(WIN_IVS, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, sIv->text, IV_VALUE_RIGHT),
                                     y, sTextColors_Dark, TEXT_SKIP_DRAW, sIv->text);
        FillWindowPixelRect(WIN_IVS, PIXEL_FILL(COLOR_TRACK), IV_BAR_X, y + IV_BAR_Y, MAX_PER_STAT_IVS * IV_BAR_SCALE, IV_BAR_HEIGHT);
        if (ivs[i] != 0)
            FillWindowPixelRect(WIN_IVS, PIXEL_FILL(GetIvBarColor(ivs[i])), IV_BAR_X, y + IV_BAR_Y, ivs[i] * IV_BAR_SCALE, IV_BAR_HEIGHT);
        total += ivs[i];
    }

    y = IV_ROW_Y(NUM_STATS) + 4;
    AddTextPrinterParameterized3(WIN_IVS, FONT_NORMAL, 2, y, sTextColors_Dark, TEXT_SKIP_DRAW, sText_Total);
    StringAppend(ConvertIntToDecimalStringN(sIv->text, total, STR_CONV_MODE_LEFT_ALIGN, 3), sText_TotalMax);
    AddTextPrinterParameterized3(WIN_IVS, FONT_NORMAL, GetStringRightAlignXOffset(FONT_NORMAL, sIv->text, 17 * 8 - 4),
                                 y, sTextColors_Dark, TEXT_SKIP_DRAW, sIv->text);
    PutWindowTilemap(WIN_IVS);
    CopyWindowToVram(WIN_IVS, COPYWIN_FULL);
}

static void PrintInfo(struct BoxPokemon *mon, const u8 *ivs, u8 nature)
{
    u8 *end;
    u8 font;

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
    GetBoxMonData(mon, MON_DATA_NICKNAME, sIv->text);
    StringGet_Nickname(sIv->text);
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 2, 0, sTextColors_Dark, TEXT_SKIP_DRAW, sIv->text);

    end = StringCopy(sIv->text, sText_Lv);
    ConvertIntToDecimalStringN(end, GetLevelFromBoxMonExp(mon), STR_CONV_MODE_LEFT_ALIGN, 3);
    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 2, 14, sTextColors_Dark, TEXT_SKIP_DRAW, sIv->text);

    AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 2, 28, sTextColors_Dark, TEXT_SKIP_DRAW, gNatureNamePointers[nature]);

    end = StringCopy(sIv->text, sText_HiddenPower);
    end = StringAppend(end, gTypeNames[GetHiddenPowerType(ivs)]);
    end = StringAppend(end, sText_Space);
    ConvertIntToDecimalStringN(end, GetHiddenPowerPower(ivs), STR_CONV_MODE_LEFT_ALIGN, 2);
    // The six-letter types only fit in the narrower font
    font = GetStringWidth(FONT_SMALL, sIv->text, 0) <= INFO_TEXT_WIDTH ? FONT_SMALL : FONT_SMALL_NARROW;
    AddTextPrinterParameterized3(WIN_INFO, font, 2, 44, sTextColors_Dark, TEXT_SKIP_DRAW, sIv->text);
    PutWindowTilemap(WIN_INFO);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

static void ShowMon(void)
{
    struct BoxPokemon *mon;
    u8 ivs[NUM_STATS];
    u8 nature, i;
    u32 personality;

    if (sIv->picSprite != SPRITE_NONE)
    {
        FreeAndDestroyMonPicSprite(sIv->picSprite);
        sIv->picSprite = SPRITE_NONE;
    }
    PrintHeader();
    if (sIv->count == 0)
    {
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, 2, 0, sTextColors_Dark, TEXT_SKIP_DRAW, sText_NoPokemon);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        FillWindowPixelBuffer(WIN_IVS, PIXEL_FILL(0));
        CopyWindowToVram(WIN_IVS, COPYWIN_FULL);
        return;
    }

    mon = sIv->mons[sIv->index];
    personality = GetBoxMonData(mon, MON_DATA_PERSONALITY, NULL);
    nature = GetNatureFromPersonality(personality);
    for (i = 0; i < NUM_STATS; i++)
        ivs[i] = GetBoxMonData(mon, sStatIvData[i], NULL);

    sIv->picSprite = CreateMonPicSprite_HandleDeoxys(GetBoxMonData(mon, MON_DATA_SPECIES, NULL),
                                                     GetBoxMonData(mon, MON_DATA_OT_ID, NULL),
                                                     personality, TRUE, PIC_X, PIC_Y, 0, TAG_NONE);
    if (sIv->picSprite >= MAX_SPRITES)
        sIv->picSprite = SPRITE_NONE;
    PrintInfo(mon, ivs, nature);
    PrintIvs(mon, ivs, nature);
}

// ---------------------------------------------------------------------------------------------
// Entry, main loop and exit

void OpenIvChecker(MainCallback exitCallback)
{
    sIv = AllocZeroed(sizeof(*sIv));
    sIv->exitCallback = exitCallback;
    sIv->picSprite = SPRITE_NONE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitIvChecker);
}

static void CB2_IvChecker(void)
{
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_IvChecker(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitIvChecker(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetAllPicSprites();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        LoadPalette(&sBackdropColor, BG_PLTT_ID(0), PLTT_SIZEOF(1));
        LoadPalette(sBgPalette, BG_PLTT_ID(BG_PALETTE), PLTT_SIZE_4BPP);
        LoadPalette(sTextPalette, BG_PLTT_ID(TEXT_PALETTE), PLTT_SIZE_4BPP);
        LoadBgTiles(1, sBgTiles, sizeof(sBgTiles), 0);
        DrawBackground();
        PrintHelp();
        gMain.state++;
        break;
    case 2:
        if (!LoadGroup(GROUP_PARTY))
            ChangeGroup(1);
        ShowMon();
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB_IvChecker);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_IvChecker, 0);
        SetMainCallback2(CB2_IvChecker);
        break;
    }
}

static void ExitIvChecker(void)
{
    if (sIv->picSprite != SPRITE_NONE)
        FreeAndDestroyMonPicSprite(sIv->picSprite);
    SetMainCallback2(sIv->exitCallback);
    FreeAllWindowBuffers();
    Free(GetBgTilemapBuffer(1));
    ResetSpriteData();
    FREE_AND_SET_NULL(sIv);
}

static void Task_IvChecker(u8 taskId)
{
    switch (sIv->state)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
            sIv->state = STATE_INPUT;
        break;
    case STATE_INPUT:
        if (JOY_NEW(B_BUTTON | A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sIv->state = STATE_FADE_OUT;
        }
        else if (JOY_NEW(DPAD_DOWN) && sIv->count > 1)
        {
            PlaySE(SE_SELECT);
            sIv->index = (sIv->index + 1) % sIv->count;
            ShowMon();
        }
        else if (JOY_NEW(DPAD_UP) && sIv->count > 1)
        {
            PlaySE(SE_SELECT);
            sIv->index = (sIv->index + sIv->count - 1) % sIv->count;
            ShowMon();
        }
        else if (JOY_NEW(R_BUTTON | DPAD_RIGHT))
        {
            if (ChangeGroup(1))
            {
                PlaySE(SE_SELECT);
                ShowMon();
            }
        }
        else if (JOY_NEW(L_BUTTON | DPAD_LEFT))
        {
            if (ChangeGroup(-1))
            {
                PlaySE(SE_SELECT);
                ShowMon();
            }
        }
        break;
    case STATE_FADE_OUT:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            ExitIvChecker();
        }
        break;
    }
}
