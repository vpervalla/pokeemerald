#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/card_game_cards.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// A test of a card drawn like the first Pokémon TCG cards (Base Set, 1999): a yellow border,
// a panel in the type's color, the stage, name and HP, a framed picture, the attacks with their
// damage on the right, then weakness and retreat cost (no resistance: this game has none).
// The small print is in a tiny font of its own, a pixel smaller than FONT_SMALL; the name and
// damage are in FONT_SMALL.
// Opened from the Test Area's card tester (special OpenVintageCardTest), on Pidgeotto;
// LEFT/RIGHT go through the other Pokémon cards, B goes back.
//
// The card is 14x20 tiles (112x160 pixels) in the middle of the screen, in two windows of the
// same place: the frame, drawn with rectangles on BG1 (FRAME_PALETTE), and the text and Energy
// icons on BG0 (TEXT_PALETTE). The picture is a sprite between them, clipped to its frame by
// the hardware window 0.

#define TEXT_PALETTE    15
#define FRAME_PALETTE   14

#define CARD_LEFT       8   // tiles
#define CARD_WIDTH      14
#define CARD_HEIGHT     20
#define CARD_X          (CARD_LEFT * 8)

// The frame's colors
enum {
    FRAME_TRANSPARENT,
    FRAME_BORDER,
    FRAME_BORDER_SHADE,
    FRAME_PANEL,        // the type's colors
    FRAME_PANEL_SHADE,
    FRAME_ART_EDGE,
    FRAME_ART_EDGE_SHADE,
    FRAME_SKY,
    FRAME_SKY_LIGHT,
    FRAME_GROUND,
    FRAME_LINE,
    FRAME_WHITE,
};

// Layout, in the card's pixels
#define BORDER          5
#define TEXT_LEFT       (BORDER + 2)
#define TEXT_RIGHT      (CARD_WIDTH * 8 - BORDER - 3)
#define STAGE_Y         (BORDER + 1)        // tiny font: the top of its capitals
#define NAME_Y          12                  // FONT_SMALL
#define NAME_TINY_Y     (NAME_Y + 5)        // tiny font, on the name's line (FONT_SMALL's capitals start 5 down)
#define ART_X           10
#define ART_Y           26
#define ART_WIDTH       92
#define ART_HEIGHT      62
#define ATTACKS_Y       (ART_Y + ART_HEIGHT + 6)
#define BOTTOM_Y        137                 // the weakness and retreat labels
#define BOTTOM_LINE_Y   (BOTTOM_Y - 3)

enum {
    WIN_FRAME,
    WIN_TEXT,
};

struct VintageCard
{
    void (*exitCallback)(void);
    u16 cardId;
    u16 picSprite;
    u8 state;
    u8 text[200];
};

static EWRAM_DATA struct VintageCard *sCard = NULL;

static void CB2_InitVintageCard(void);
static void Task_VintageCard(u8 taskId);
static void DrawCard(void);

static const u16 sBackdropColor = RGB(4, 10, 6);

static const u16 sFramePalette[] =
{
    [FRAME_TRANSPARENT]    = RGB(0, 0, 0),
    [FRAME_BORDER]         = RGB(31, 26, 5),
    [FRAME_BORDER_SHADE]   = RGB(27, 21, 2),
    [FRAME_PANEL]          = RGB(27, 27, 26),
    [FRAME_PANEL_SHADE]    = RGB(22, 22, 21),
    [FRAME_ART_EDGE]       = RGB(30, 28, 19),
    [FRAME_ART_EDGE_SHADE] = RGB(19, 15, 7),
    [FRAME_SKY]            = RGB(23, 27, 31),
    [FRAME_SKY_LIGHT]      = RGB(27, 29, 31),
    [FRAME_GROUND]         = RGB(20, 25, 15),
    [FRAME_LINE]           = RGB(15, 14, 12),
    [FRAME_WHITE]          = RGB(31, 31, 31),
};

// The panel's colors by type, as on the Base Set cards: the panel and its edge
static const u16 sPanelColors[CARD_TYPE_COUNT][2] =
{
    [CARD_TYPE_NONE]      = {RGB(27, 27, 26), RGB(22, 22, 21)},
    [CARD_TYPE_GRASS]     = {RGB(22, 28, 15), RGB(15, 22, 8)},
    [CARD_TYPE_FIRE]      = {RGB(31, 21, 17), RGB(27, 13, 9)},
    [CARD_TYPE_WATER]     = {RGB(18, 25, 31), RGB(10, 18, 28)},
    [CARD_TYPE_LIGHTNING] = {RGB(31, 29, 15), RGB(28, 24, 6)},
    [CARD_TYPE_PSYCHIC]   = {RGB(26, 20, 29), RGB(20, 13, 24)},
    [CARD_TYPE_FIGHTING]  = {RGB(28, 22, 16), RGB(23, 16, 10)},
    [CARD_TYPE_DARKNESS]  = {RGB(19, 19, 21), RGB(14, 14, 16)},
    [CARD_TYPE_METAL]     = {RGB(23, 24, 25), RGB(17, 18, 20)},
    [CARD_TYPE_DRAGON]    = {RGB(28, 26, 17), RGB(23, 20, 10)},
    [CARD_TYPE_COLORLESS] = {RGB(29, 29, 28), RGB(23, 23, 22)},
};

// Colors of TEXT_PALETTE (the Energy icons' colors over the standard menu's)
#define COLOR_INK   4   // near-black
#define COLOR_HP    8   // dark red

static const u8 sTextColors[] = {TEXT_COLOR_TRANSPARENT, COLOR_INK, TEXT_COLOR_TRANSPARENT};

static const u8 sText_Basic[] = _("BASIC POKéMON");
static const u8 sText_BasicShort[] = _("BASIC");
static const u8 sText_Stage1[] = _("STAGE 1");
static const u8 sText_Stage2[] = _("STAGE 2");
static const u8 sText_EvolvesFrom[] = _("Evolves from ");
static const u8 sText_From[] = _("from ");
static const u8 sText_Hp[] = _(" HP");
static const u8 sText_Weakness[] = _("weakness");
static const u8 sText_Retreat[] = _("retreat");
static const u8 sText_Plus[] = _("+");

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .priority = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .priority = 2,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_FRAME] = {.bg = 1, .tilemapLeft = CARD_LEFT, .tilemapTop = 0, .width = CARD_WIDTH, .height = CARD_HEIGHT, .paletteNum = FRAME_PALETTE, .baseBlock = 1},
    [WIN_TEXT]  = {.bg = 0, .tilemapLeft = CARD_LEFT, .tilemapTop = 0, .width = CARD_WIDTH, .height = CARD_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};

// ---------------------------------------------------------------------------------------------
// Entry and exit

#define tState data[0]

static void Task_OpenFromField(u8 taskId)
{
    if (gTasks[taskId].tState == 0)
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].tState++;
    }
    else if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        OpenVintageCardScreen(CARD_PIDGEOTTO, CB2_ReturnToFieldContinueScriptPlayMapMusic);
        DestroyTask(taskId);
    }
}

// special OpenVintageCardTest (waitstate)
void OpenVintageCardTest(void)
{
    CreateTask(Task_OpenFromField, 0);
}

// The screen is black when it is called
void OpenVintageCardScreen(u16 cardId, void (*exitCallback)(void))
{
    sCard = AllocZeroed(sizeof(*sCard));
    sCard->exitCallback = exitCallback;
    sCard->cardId = cardId;
    sCard->picSprite = MAX_SPRITES;
    gMain.state = 0;
    SetMainCallback2(CB2_InitVintageCard);
}

static void CB2_VintageCard(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_VintageCard(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitVintageCard(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
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
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PALETTE), PLTT_SIZE_4BPP);
        CardGame_LoadEnergyIconPalette(TEXT_PALETTE);
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 32);
        PutWindowTilemap(WIN_FRAME);
        PutWindowTilemap(WIN_TEXT);
        gMain.state++;
        break;
    case 2:
        DrawCard();
        gMain.state++;
        break;
    case 3:
        // The picture's sprite only in its frame
        SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(CARD_X + ART_X - 2, CARD_X + ART_X + ART_WIDTH));
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(ART_Y - 2, ART_Y + ART_HEIGHT));
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP | DISPCNT_WIN0_ON);
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB_VintageCard);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_VintageCard, 0);
        SetMainCallback2(CB2_VintageCard);
        break;
    }
}

static void DestroyPicture(void)
{
    if (sCard->picSprite != MAX_SPRITES)
        CardGame_DestroyPokemonPic(sCard->picSprite);
    sCard->picSprite = MAX_SPRITES;
}

// The next (or previous) Pokémon card
static void BrowseCards(s8 delta)
{
    u16 cardId = sCard->cardId;

    do
    {
        cardId += delta;
        if (cardId == 0)
            cardId = CARD_POKEMON_END - 1;
        else if (cardId >= CARD_POKEMON_END)
            cardId = 1;
    } while (CardGame_GetPokemonInfo(cardId) == NULL && cardId != sCard->cardId);
    sCard->cardId = cardId;
    PlaySE(SE_SELECT);
    DrawCard();
}

static void Task_VintageCard(u8 taskId)
{
    switch (sCard->state)
    {
    case 0:
        if (!gPaletteFade.active)
            sCard->state++;
        break;
    case 1:
        if (JOY_NEW(B_BUTTON | A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sCard->state++;
        }
        else if (JOY_NEW(DPAD_RIGHT))
        {
            BrowseCards(1);
        }
        else if (JOY_NEW(DPAD_LEFT))
        {
            BrowseCards(-1);
        }
        break;
    case 2:
        if (!gPaletteFade.active)
        {
            void (*exitCallback)(void) = sCard->exitCallback;

            DestroyPicture();
            SetGpuReg(REG_OFFSET_DISPCNT, 0);
            SetGpuReg(REG_OFFSET_WIN0H, 0);
            SetGpuReg(REG_OFFSET_WIN0V, 0);
            SetGpuReg(REG_OFFSET_WININ, 0);
            SetGpuReg(REG_OFFSET_WINOUT, 0);
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sCard);
            DestroyTask(taskId);
            SetMainCallback2(exitCallback);
        }
        break;
    }
}

// ---------------------------------------------------------------------------------------------
// The tiny font (made by tools/card_game/gen_vintage_font.py), drawn straight into the text
// window's pixels. The Energy icons of CardGame_AppendTypeIcon are drawn in their blanks.

#include "data/card_game/vintage_font.h"

#define TINY_LINE_HEIGHT    8
#define TINY_SPACING        1
#define TINY_ICON_Y         (-2)    // an icon's top, from the capitals' top

static void SetTextPixel(u8 x, u8 y, u8 color)
{
    u8 *tiles = (u8 *)GetWindowAttribute(WIN_TEXT, WINDOW_TILE_DATA);
    u8 *pixel;

    if (x >= CARD_WIDTH * 8 || y >= CARD_HEIGHT * 8)
        return;
    pixel = &tiles[((y / 8) * CARD_WIDTH + x / 8) * TILE_SIZE_4BPP + (y % 8) * 4 + (x % 8) / 2];
    if (x & 1)
        *pixel = (*pixel & 0x0F) | (color << 4);
    else
        *pixel = (*pixel & 0xF0) | color;
}

static u8 GetTinyGlyphWidth(u8 c)
{
    return sTinyGlyphWidths[c] != 0 ? sTinyGlyphWidths[c] : sTinyGlyphWidths[CHAR_SPACE];
}

// The width of the text's first line
static u16 GetTinyWidth(const u8 *text)
{
    u16 width = 0;

    for (; *text != EOS && *text != CHAR_NEWLINE; text++)
    {
        if (*text == EXT_CTRL_CODE_BEGIN)
        {
            // {PALETTE type} prints nothing, {CLEAR width} is the icon's blank
            if (text[1] == EXT_CTRL_CODE_CLEAR)
                width += text[2];
            text += 2;
            continue;
        }
        width += GetTinyGlyphWidth(*text) + TINY_SPACING;
    }
    return width;
}

// Prints at x, y (the top of the capitals)
static void TinyPrint(u8 x, u8 y, u8 color, const u8 *text)
{
    u8 left = x;
    u8 i, row, width;

    for (; *text != EOS; text++)
    {
        if (*text == CHAR_NEWLINE)
        {
            x = left;
            y += TINY_LINE_HEIGHT;
            continue;
        }
        if (*text == EXT_CTRL_CODE_BEGIN)
        {
            if (text[1] == EXT_CTRL_CODE_PALETTE)
                CardGame_DrawEnergyIcon(WIN_TEXT, text[2], x, y + TINY_ICON_Y);
            else if (text[1] == EXT_CTRL_CODE_CLEAR)
                x += text[2];
            text += 2;
            continue;
        }
        width = GetTinyGlyphWidth(*text);
        for (row = 0; row < 8; row++)
        {
            for (i = 0; i < width; i++)
            {
                if (sTinyGlyphs[*text][row] & (0x80 >> i))
                    SetTextPixel(x + i, y + row, color);
            }
        }
        x += width + TINY_SPACING;
    }
}

static void TinyPrintRight(u8 right, u8 y, u8 color, const u8 *text)
{
    TinyPrint(right + TINY_SPACING - GetTinyWidth(text), y, color, text);
}

// Breaks the text into lines of width pixels at most, at its spaces
static void TinyWrap(u8 *text, u16 width)
{
    u8 *lineStart = text, *lastSpace = NULL, *p;
    u8 saved;

    for (p = text; ; p++)
    {
        if (*p == EXT_CTRL_CODE_BEGIN)
        {
            p += 2;
            continue;
        }
        if (*p != CHAR_SPACE && *p != CHAR_NEWLINE && *p != EOS)
            continue;
        saved = *p;
        *p = EOS;
        if (GetTinyWidth(lineStart) > width && lastSpace != NULL)
        {
            *lastSpace = CHAR_NEWLINE;
            lineStart = lastSpace + 1;
        }
        *p = saved;
        if (saved == EOS)
            break;
        if (saved == CHAR_NEWLINE)
        {
            lineStart = p + 1;
            lastSpace = NULL;
        }
        else
        {
            lastSpace = p;
        }
    }
}

static u8 CountLines(const u8 *text)
{
    u8 lines = 1;

    for (; *text != EOS; text++)
    {
        if (*text == CHAR_NEWLINE)
            lines++;
    }
    return lines;
}

// ---------------------------------------------------------------------------------------------
// Drawing

static void FillFrame(u8 color, u8 x, u8 y, u8 width, u8 height)
{
    FillWindowPixelRect(WIN_FRAME, PIXEL_FILL(color), x, y, width, height);
}

// The border, the panel, the picture's frame and background and the line above the weakness
static void DrawFrame(u8 type)
{
    const u8 w = CARD_WIDTH * 8, h = CARD_HEIGHT * 8;
    u8 i;

    LoadPalette(sFramePalette, BG_PLTT_ID(FRAME_PALETTE), sizeof(sFramePalette));
    LoadPalette(sPanelColors[type], BG_PLTT_ID(FRAME_PALETTE) + FRAME_PANEL, PLTT_SIZEOF(2));

    FillWindowPixelBuffer(WIN_FRAME, PIXEL_FILL(FRAME_BORDER));
    // Rounded corners
    for (i = 0; i < 3; i++)
    {
        u8 len = 3 - i;

        FillFrame(FRAME_TRANSPARENT, 0, i, len, 1);
        FillFrame(FRAME_TRANSPARENT, w - len, i, len, 1);
        FillFrame(FRAME_TRANSPARENT, 0, h - 1 - i, len, 1);
        FillFrame(FRAME_TRANSPARENT, w - len, h - 1 - i, len, 1);
    }
    // The panel, with a shaded edge
    FillFrame(FRAME_BORDER_SHADE, BORDER - 1, BORDER - 1, w - 2 * BORDER + 2, h - 2 * BORDER + 2);
    FillFrame(FRAME_PANEL_SHADE, BORDER, BORDER, w - 2 * BORDER, h - 2 * BORDER);
    FillFrame(FRAME_PANEL, BORDER + 1, BORDER + 1, w - 2 * BORDER - 2, h - 2 * BORDER - 2);

    // The picture: a frame, the sky (lighter at the top) and the ground
    FillFrame(FRAME_ART_EDGE_SHADE, ART_X - 2, ART_Y - 2, ART_WIDTH + 4, ART_HEIGHT + 4);
    FillFrame(FRAME_ART_EDGE, ART_X - 2, ART_Y - 2, ART_WIDTH + 3, ART_HEIGHT + 3);
    FillFrame(FRAME_ART_EDGE_SHADE, ART_X - 1, ART_Y - 1, ART_WIDTH + 1, ART_HEIGHT + 1);
    FillFrame(FRAME_SKY, ART_X, ART_Y, ART_WIDTH, ART_HEIGHT);
    for (i = 0; i < 4; i++)
        FillFrame(FRAME_SKY_LIGHT, ART_X, ART_Y + i * 6, ART_WIDTH, 4 - i);
    FillFrame(FRAME_GROUND, ART_X, ART_Y + ART_HEIGHT - 14, ART_WIDTH, 14);

    // The line above weakness and retreat
    FillFrame(FRAME_LINE, BORDER + 3, BOTTOM_LINE_Y, w - 2 * BORDER - 6, 1);
    CopyWindowToVram(WIN_FRAME, COPYWIN_FULL);
}

static void PrintSmall(u8 x, u8 y, const u8 *text)
{
    AddTextPrinterParameterized3(WIN_TEXT, FONT_SMALL, x, y, sTextColors, TEXT_SKIP_DRAW, text);
}

// The attacks: cost and name (under the cost if it doesn't fit beside the damage), the damage
// on the right, then the effect if withEffects. Returns where they end; draws them only if draw.
static u8 DrawAttacks(const struct PokemonCardInfo *info, bool8 withEffects, bool8 draw)
{
    u8 *text = sCard->text;
    u8 *end;
    u8 i, y = ATTACKS_Y;

    for (i = 0; i < info->numAttacks; i++)
    {
        const struct CardAttack *attack = &info->attacks[i];
        struct CardAttack nameOnly = *attack;
        u8 damageWidth = 0;

        if (attack->damage != 0)
        {
            end = ConvertIntToDecimalStringN(text, attack->damage, STR_CONV_MODE_LEFT_ALIGN, 3);
            if (attack->effect == CARD_EFFECT_FLIP_MULTI)
                StringAppend(end, sText_Plus);
            damageWidth = GetStringWidth(FONT_SMALL, text, 0);
            if (draw)
                PrintSmall(TEXT_RIGHT - damageWidth, y - 5, text);
            damageWidth += 4;
        }
        nameOnly.damage = 0;
        nameOnly.effect = CARD_EFFECT_NONE;
        *text = EOS;
        CardGame_AppendAttackText(text, &nameOnly);
        if (TEXT_LEFT + GetTinyWidth(text) > TEXT_RIGHT - damageWidth)
        {
            // The cost, then the name under it
            for (end = text; *end != CHAR_SPACE && *end != EOS; end++)
            {
                if (*end == EXT_CTRL_CODE_BEGIN)
                    end += 2;
            }
            *end = CHAR_NEWLINE;
        }
        if (draw)
            TinyPrint(TEXT_LEFT, y, COLOR_INK, text);
        y += (CountLines(text) - 1) * (TINY_LINE_HEIGHT + 1) + 10;
        *text = EOS;
        if (withEffects)
            CardGame_AppendAttackEffect(text, attack);
        if (*text != EOS)
        {
            TinyWrap(text, TEXT_RIGHT - TEXT_LEFT - 2);
            if (draw)
                TinyPrint(TEXT_LEFT + 2, y, COLOR_INK, text);
            y += CountLines(text) * TINY_LINE_HEIGHT + 2;
        }
    }
    return y;
}

static void DrawCard(void)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(sCard->cardId);
    const u8 *stage;
    u8 *text = sCard->text;
    u8 *end;
    u8 i, x, hpY;

    DestroyPicture();
    DrawFrame(info->type);
    FillWindowPixelBuffer(WIN_TEXT, PIXEL_FILL(0));

    // The HP and type, then the name before them: in FONT_SMALL, or in the tiny font if it
    // doesn't fit; if it still doesn't, the HP goes up to the stage's line
    *text = EOS;
    end = CardGame_AppendTypeIcon(text, info->type);
    end = StringAppend(end, gText_Space);
    end = ConvertIntToDecimalStringN(end, info->hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(end, sText_Hp);
    x = TEXT_RIGHT - GetTinyWidth(text) - 2;
    *text = EOS;
    CardGame_AppendCardName(text, sCard->cardId);
    hpY = NAME_TINY_Y;
    if (TEXT_LEFT + GetStringWidth(FONT_SMALL, text, 0) <= x)
        PrintSmall(TEXT_LEFT, NAME_Y, text);
    else if (TEXT_LEFT + GetTinyWidth(text) <= x)
        TinyPrint(TEXT_LEFT, NAME_TINY_Y, COLOR_INK, text);
    else
    {
        PrintSmall(TEXT_LEFT, NAME_Y, text);
        hpY = STAGE_Y;
    }
    // "130 HP" in red, then the type
    *text = EOS;
    CardGame_AppendTypeIcon(text, info->type);
    TinyPrintRight(TEXT_RIGHT, hpY, COLOR_INK, text);
    x = TEXT_RIGHT - GetTinyWidth(text) - 2;
    end = ConvertIntToDecimalStringN(text, info->hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(end, sText_Hp);
    TinyPrintRight(x, hpY, COLOR_HP, text);

    // The stage ("BASIC" only beside the HP), and what it evolves from: "Evolves from X", or
    // "from X", if it fits
    if (info->stage == CARD_STAGE_BASIC)
        stage = hpY == STAGE_Y ? sText_BasicShort : sText_Basic;
    else
        stage = info->stage == CARD_STAGE_1 ? sText_Stage1 : sText_Stage2;
    TinyPrint(TEXT_LEFT, STAGE_Y, COLOR_INK, stage);
    if (info->evolvesFrom != CARD_NONE && hpY != STAGE_Y)
    {
        x = TEXT_LEFT + GetTinyWidth(stage) + 4;
        end = StringCopy(text, sText_EvolvesFrom);
        CardGame_AppendCardName(end, info->evolvesFrom);
        if (TEXT_RIGHT - GetTinyWidth(text) < x)
        {
            end = StringCopy(text, sText_From);
            CardGame_AppendCardName(end, info->evolvesFrom);
        }
        if (TEXT_RIGHT - GetTinyWidth(text) >= x)
            TinyPrintRight(TEXT_RIGHT, STAGE_Y, COLOR_INK, text);
    }

    // The picture, its feet on the ground (the sprite's bottom rows are empty); a big one fits
    // whole, under the frame's top
    sCard->picSprite = CardGame_CreatePokemonPic(info->species, CARD_X + ART_X + ART_WIDTH / 2, ART_Y + ART_HEIGHT - 3 - 28);
    if (sCard->picSprite != MAX_SPRITES)
        gSprites[sCard->picSprite].oam.priority = 1;

    // The attacks, without their effects if they don't all fit
    DrawAttacks(info, DrawAttacks(info, TRUE, FALSE) <= BOTTOM_LINE_Y - 2, TRUE);

    // Weakness and retreat cost, their icons under them (the Base Set's resistance doesn't
    // exist in this game)
    TinyPrint(TEXT_LEFT + 1, BOTTOM_Y, COLOR_INK, sText_Weakness);
    TinyPrintRight(TEXT_RIGHT, BOTTOM_Y, COLOR_INK, sText_Retreat);
    *text = EOS;
    CardGame_AppendTypeIcon(text, info->weakness);
    TinyPrint(TEXT_LEFT + 6, BOTTOM_Y + 9, COLOR_INK, text);
    *text = EOS;
    end = text;
    for (i = 0; i < info->retreatCost; i++)
        end = CardGame_AppendTypeIcon(end, CARD_TYPE_COLORLESS);
    TinyPrintRight(TEXT_RIGHT, BOTTOM_Y + 9, COLOR_INK, text);
    CopyWindowToVram(WIN_TEXT, COPYWIN_FULL);
}
