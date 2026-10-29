#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "gpu_regs.h"
#include "item_icon.h"
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
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The cards drawn like the first Pokémon TCG cards (Base Set, 1999): a yellow border, a panel
// in the type's color, the stage, name and HP, a framed picture, the attacks with their damage
// on the right, then weakness and retreat cost (no resistance: this game has none). A Trainer
// has a gray panel, its kind, its item in the frame and what it does. The small print is in a
// tiny font of its own, a pixel smaller than FONT_SMALL; the name and damage are in FONT_SMALL.
// The duel's card viewer, the CARD CASE and the pack screen show their cards this way.
//
// A card is 14x20 tiles (112x160 pixels), in two windows of the screen that uses it, at the
// same place: the frame, drawn with rectangles on a BG under the sprites (its window's palette
// becomes the frame's), and the text and Energy icons on a BG over them, whose palette is the
// standard menu's with CardGame_LoadEnergyIconPalette. The picture is a sprite between them
// (priority 1), clipped to its frame by the hardware window 0 (VintageCard_EnableClipping).
//
// What doesn't fit is left out: first the ex rule, then the attacks' effects and the ability's
// text. VintageCard_Draw says so, and a screen with room beside the card can then print the
// card's whole text with VintageCard_PrintFullText.
//
// The Test Area's card tester also opens a screen of its own (special OpenVintageCardTest), on
// Pidgeotto: LEFT/RIGHT go through the other cards, B goes back.

#define TEXT_PALETTE    15
#define FRAME_PALETTE   14

#define CARD_WIDTH      VINTAGE_CARD_WIDTH  // tiles
#define CARD_HEIGHT     VINTAGE_CARD_HEIGHT

#define TAG_ITEM        0x4C20
#define TAG_ITEM_2      0x4C21  // the next card's, loaded while the last is still shown

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
#define RULES_BOTTOM    (BOTTOM_LINE_Y - 2) // where a Pokémon's attacks must end
#define TRAINER_BOTTOM  (CARD_HEIGHT * 8 - BORDER - 2)

// What a Pokémon card shows besides the ability's name and the attacks' costs, names and damage
#define SHOW_EFFECTS    (1 << 0)    // the attacks' effects and the ability's text
#define SHOW_EX_RULE    (1 << 1)
#define SHOW_ALL        (SHOW_EFFECTS | SHOW_EX_RULE)

// The full text beside a card (VintageCard_PrintFullText)
#define FULL_LINE_HEIGHT        12
#define FULL_MIN_LINE_HEIGHT    9

struct VintageCard
{
    u8 frameWindow;
    u8 textWindow;
    u16 picSprite;
    bool8 picIsItem;
    u16 picItemTag;
    u8 frameType;   // The panel's colours, and the picture: shown with the card's windows (ShowCard)
    u16 picSpecies;
    u16 picItem;
    u8 text[400];
    u8 line[80];
};

// The Test Area's screen
struct VintageCardTest
{
    void (*exitCallback)(void);
    u16 cardId;
    u8 state;
};

enum {
    WIN_FRAME,
    WIN_TEXT,
};

static EWRAM_DATA struct VintageCard *sCard = NULL;
static EWRAM_DATA struct VintageCardTest *sTest = NULL;

static void CB2_InitVintageCard(void);
static void Task_VintageCard(u8 taskId);

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

// The panel's colors by type, as on the Base Set cards: the panel and its edge. The Trainers'
// are CARD_TYPE_NONE's.
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

// Colors of the text window's palette (the Energy icons' colors over the standard menu's)
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
static const u8 sText_Ability[] = _("Ability: ");
static const u8 sText_Trainer[] = _("TRAINER");

// A Trainer's item, twice its size
static const struct OamData sOamData_Item =
{
    .affineMode = ST_OAM_AFFINE_DOUBLE,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const union AffineAnimCmd sAffineAnim_Item[] =
{
    AFFINEANIMCMD_FRAME(0x200, 0x200, 0, 0),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sAffineAnims_Item[] = {sAffineAnim_Item};

static const struct SpriteTemplate sSpriteTemplate_Item =
{
    .tileTag = TAG_ITEM,
    .paletteTag = TAG_ITEM,
    .oam = &sOamData_Item,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = sAffineAnims_Item,
    .callback = SpriteCallbackDummy,
};

// ---------------------------------------------------------------------------------------------
// Use from another screen

// The windows must be CARD_WIDTH x CARD_HEIGHT tiles at the same place, the frame's under the
// sprites and the text's over them
void VintageCard_Init(u8 frameWindowId, u8 textWindowId)
{
    sCard = AllocZeroed(sizeof(*sCard));
    sCard->frameWindow = frameWindowId;
    sCard->textWindow = textWindowId;
    sCard->picSprite = MAX_SPRITES;
}

static u8 GetCardX(void)
{
    return GetWindowAttribute(sCard->textWindow, WINDOW_TILEMAP_LEFT) * 8;
}

static u8 GetCardY(void)
{
    return GetWindowAttribute(sCard->textWindow, WINDOW_TILEMAP_TOP) * 8;
}

static void DestroyPictureSprite(u16 spriteId, bool8 isItem, u16 itemTag)
{
    if (spriteId == MAX_SPRITES)
        return;
    if (isItem)
    {
        // Its template was a copy, freed already: its tags are itemTag
        FreeSpriteOamMatrix(&gSprites[spriteId]);
        DestroySprite(&gSprites[spriteId]);
        FreeSpriteTilesByTag(itemTag);
        FreeSpritePaletteByTag(itemTag);
    }
    else
        CardGame_DestroyPokemonPic(spriteId);
}

static void DestroyPicture(void)
{
    DestroyPictureSprite(sCard->picSprite, sCard->picIsItem, sCard->picItemTag);
    sCard->picSprite = MAX_SPRITES;
}

// No card: the windows empty
void VintageCard_Clear(void)
{
    DestroyPicture();
    FillWindowPixelBuffer(sCard->frameWindow, PIXEL_FILL(FRAME_TRANSPARENT));
    FillWindowPixelBuffer(sCard->textWindow, PIXEL_FILL(0));
    PutWindowTilemap(sCard->frameWindow);
    PutWindowTilemap(sCard->textWindow);
    CopyWindowToVram(sCard->frameWindow, COPYWIN_FULL);
    CopyWindowToVram(sCard->textWindow, COPYWIN_FULL);
}

void VintageCard_Free(void)
{
    if (sCard == NULL)
        return;
    DestroyPicture();
    FREE_AND_SET_NULL(sCard);
}

// The sprites show in the picture's frame, and in window 1: the screen left (or right) of the
// card, for the screen's own sprites. Over the rest of the card they don't.
void VintageCard_EnableClipping(void)
{
    u8 x = GetCardX(), y = GetCardY();

    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(x + ART_X - 2, x + ART_X + ART_WIDTH));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(y + ART_Y - 2, y + ART_Y + ART_HEIGHT));
    if (x >= DISPLAY_WIDTH - x - CARD_WIDTH * 8)
        SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(0, x));
    else
        SetGpuReg(REG_OFFSET_WIN1H, WIN_RANGE(x + CARD_WIDTH * 8, DISPLAY_WIDTH));
    SetGpuReg(REG_OFFSET_WIN1V, WIN_RANGE(0, DISPLAY_HEIGHT));
    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL | WININ_WIN1_ALL);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_CLR);
    SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
}

void VintageCard_DisableClipping(void)
{
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WIN1H, 0);
    SetGpuReg(REG_OFFSET_WIN1V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
}

// ---------------------------------------------------------------------------------------------
// The Test Area's screen

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

#define TEST_CARD_LEFT  ((DISPLAY_TILE_WIDTH - CARD_WIDTH) / 2)

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_FRAME] = {.bg = 1, .tilemapLeft = TEST_CARD_LEFT, .tilemapTop = 0, .width = CARD_WIDTH, .height = CARD_HEIGHT, .paletteNum = FRAME_PALETTE, .baseBlock = 1},
    [WIN_TEXT]  = {.bg = 0, .tilemapLeft = TEST_CARD_LEFT, .tilemapTop = 0, .width = CARD_WIDTH, .height = CARD_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};

// The screen is black when it is called
void OpenVintageCardScreen(u16 cardId, void (*exitCallback)(void))
{
    sTest = AllocZeroed(sizeof(*sTest));
    sTest->exitCallback = exitCallback;
    sTest->cardId = cardId;
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
        VintageCard_Init(WIN_FRAME, WIN_TEXT);
        gMain.state++;
        break;
    case 1:
        LoadPalette(&sBackdropColor, BG_PLTT_ID(0), PLTT_SIZEOF(1));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PALETTE), PLTT_SIZE_4BPP);
        CardGame_LoadEnergyIconPalette(TEXT_PALETTE);
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 32);
        gMain.state++;
        break;
    case 2:
        VintageCard_Draw(sTest->cardId);
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        VintageCard_EnableClipping();
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB_VintageCard);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_VintageCard, 0);
        SetMainCallback2(CB2_VintageCard);
        break;
    }
}

// The next (or previous) card
static void BrowseCards(s8 delta)
{
    u16 cardId = sTest->cardId;

    do
    {
        cardId += delta;
        if (cardId == 0)
            cardId = CARD_COUNT - 1;
        else if (cardId >= CARD_COUNT)
            cardId = 1;
    } while (CardGame_GetPokemonInfo(cardId) == NULL && CardGame_GetTrainerInfo(cardId) == NULL && cardId != sTest->cardId);
    sTest->cardId = cardId;
    PlaySE(SE_SELECT);
    VintageCard_Draw(cardId);
}

static void Task_VintageCard(u8 taskId)
{
    switch (sTest->state)
    {
    case 0:
        if (!gPaletteFade.active)
            sTest->state++;
        break;
    case 1:
        if (JOY_NEW(B_BUTTON | A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sTest->state++;
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
            void (*exitCallback)(void) = sTest->exitCallback;

            VintageCard_Free();
            SetGpuReg(REG_OFFSET_DISPCNT, 0);
            VintageCard_DisableClipping();
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sTest);
            DestroyTask(taskId);
            SetMainCallback2(exitCallback);
        }
        break;
    }
}

#undef tState

// ---------------------------------------------------------------------------------------------
// The tiny font (made by tools/card_game/gen_vintage_font.py), drawn straight into the text
// window's pixels. The Energy icons of CardGame_AppendTypeIcon are drawn in their blanks.

#include "data/card_game/vintage_font.h"

#define TINY_LINE_HEIGHT    8
#define TINY_SPACING        1
#define TINY_ICON_Y         (-2)    // an icon's top, from the capitals' top

static void SetTextPixel(u8 x, u8 y, u8 color)
{
    u8 *tiles = (u8 *)GetWindowAttribute(sCard->textWindow, WINDOW_TILE_DATA);
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
                CardGame_DrawEnergyIcon(sCard->textWindow, text[2], x, y + TINY_ICON_Y);
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

// Wraps the text, prints it at TEXT_LEFT + 2, y if draw; returns the y after it
static u8 TinyParagraph(u8 *text, u8 y, bool8 draw)
{
    TinyWrap(text, TEXT_RIGHT - TEXT_LEFT - 2);
    if (draw)
        TinyPrint(TEXT_LEFT + 2, y, COLOR_INK, text);
    return y + CountLines(text) * TINY_LINE_HEIGHT + 2;
}

// ---------------------------------------------------------------------------------------------
// Drawing

static void FillFrame(u8 color, u8 x, u8 y, u8 width, u8 height)
{
    FillWindowPixelRect(sCard->frameWindow, PIXEL_FILL(color), x, y, width, height);
}

// The border, the panel, the picture's frame and background (the ground only under a
// Pokémon) and the line above the weakness (Pokémon only). Only in the window's buffer: ShowCard
// puts it on screen with the rest of the card.
static void DrawFrame(u8 type, bool8 isPokemon)
{
    const u8 w = CARD_WIDTH * 8, h = CARD_HEIGHT * 8;
    u8 i;

    sCard->frameType = type;
    FillWindowPixelBuffer(sCard->frameWindow, PIXEL_FILL(FRAME_BORDER));
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

    if (isPokemon)
    {
        FillFrame(FRAME_GROUND, ART_X, ART_Y + ART_HEIGHT - 14, ART_WIDTH, 14);
        // The line above weakness and retreat
        FillFrame(FRAME_LINE, BORDER + 3, BOTTOM_LINE_Y, w - 2 * BORDER - 6, 1);
    }
}

// The panel's colours, both windows and the picture at once, once the whole card is drawn: drawing
// its text takes several frames, and what was loaded first showed on the old card in the meantime
static void ShowCard(void)
{
    u8 palette = GetWindowAttribute(sCard->frameWindow, WINDOW_PALETTE_NUM);
    u16 oldSprite = sCard->picSprite;
    bool8 oldIsItem = sCard->picIsItem;
    u16 oldItemTag = sCard->picItemTag;

    // All of it from the start of a frame, so that the next VBlank shows it at once: the colours and
    // the sprites go out at every VBlank, the windows' tiles when queued in time
    VBlankIntrWait();
    // The new picture in its own tiles and palette first, the old one destroyed after: its sprite is
    // on screen until the next frame, and would show the new one's tiles if they took its place
    sCard->picSprite = MAX_SPRITES;
    if (sCard->picSpecies != SPECIES_NONE)
    {
        // Its feet on the ground (the sprite's bottom rows are empty); a big one fits whole, under
        // the frame's top
        sCard->picSprite = CardGame_CreatePokemonPic(sCard->picSpecies, GetCardX() + ART_X + ART_WIDTH / 2, GetCardY() + ART_Y + ART_HEIGHT - 3 - 28);
        sCard->picIsItem = FALSE;
        if (sCard->picSprite != MAX_SPRITES)
            gSprites[sCard->picSprite].oam.priority = 1;
    }
    else if (sCard->picItem != ITEM_NONE)
    {
        sCard->picItemTag = (oldIsItem && oldSprite != MAX_SPRITES && oldItemTag == TAG_ITEM) ? TAG_ITEM_2 : TAG_ITEM;
        sCard->picSprite = AddCustomItemIconSprite(&sSpriteTemplate_Item, sCard->picItemTag, sCard->picItemTag, sCard->picItem);
        sCard->picIsItem = TRUE;
        if (sCard->picSprite != MAX_SPRITES)
        {
            gSprites[sCard->picSprite].x = GetCardX() + ART_X + ART_WIDTH / 2;
            gSprites[sCard->picSprite].y = GetCardY() + ART_Y + ART_HEIGHT / 2;
        }
    }
    DestroyPictureSprite(oldSprite, oldIsItem, oldItemTag);
    LoadPalette(sFramePalette, BG_PLTT_ID(palette), sizeof(sFramePalette));
    LoadPalette(sPanelColors[sCard->frameType], BG_PLTT_ID(palette) + FRAME_PANEL, PLTT_SIZEOF(2));
    PutWindowTilemap(sCard->frameWindow);
    PutWindowTilemap(sCard->textWindow);
    CopyWindowToVram(sCard->frameWindow, COPYWIN_FULL);
    CopyWindowToVram(sCard->textWindow, COPYWIN_FULL);
}

static void PrintSmall(u8 x, u8 y, const u8 *text)
{
    AddTextPrinterParameterized3(sCard->textWindow, FONT_SMALL, x, y, sTextColors, TEXT_SKIP_DRAW, text);
}

// The name at NAME_Y, in FONT_SMALL, or in the tiny font if it doesn't fit left of right; if it
// still doesn't, in FONT_SMALL anyway. FALSE if it doesn't fit.
static bool8 DrawName(u16 cardId, u8 right)
{
    u8 *text = sCard->text;

    *text = EOS;
    CardGame_AppendCardName(text, cardId);
    if (TEXT_LEFT + GetStringWidth(FONT_SMALL, text, 0) <= right)
    {
        PrintSmall(TEXT_LEFT, NAME_Y, text);
        return TRUE;
    }
    if (TEXT_LEFT + GetTinyWidth(text) <= right)
    {
        TinyPrint(TEXT_LEFT, NAME_TINY_Y, COLOR_INK, text);
        return TRUE;
    }
    PrintSmall(TEXT_LEFT, NAME_Y, text);
    return FALSE;
}

// The ability (its name in red, like the Base Set's Pokémon Powers), the attacks (cost and name,
// under the cost if it doesn't fit beside the damage, the damage on the right), and what show
// asks for of the rest. Returns where they end; draws them only if draw.
static u8 DrawRules(const struct PokemonCardInfo *info, u8 show, bool8 draw)
{
    u8 *text = sCard->text;
    u8 *end;
    u8 i, y = ATTACKS_Y;

    if (info->ability != CARD_ABILITY_NONE)
    {
        end = StringCopy(text, sText_Ability);
        CardGame_AppendAbilityName(end, info);
        if (draw)
            TinyPrint(TEXT_LEFT, y, COLOR_HP, text);
        y += TINY_LINE_HEIGHT + 2;
        if (show & SHOW_EFFECTS)
        {
            *text = EOS;
            CardGame_AppendAbilityEffect(text, info);
            y = TinyParagraph(text, y, draw);
        }
    }
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
        if (show & SHOW_EFFECTS)
            CardGame_AppendAttackEffect(text, attack);
        if (*text != EOS)
            y = TinyParagraph(text, y, draw);
    }
    if (info->isEx && (show & SHOW_EX_RULE))
    {
        *text = EOS;
        CardGame_AppendExRule(text, info);
        y = TinyParagraph(text, y, draw);
    }
    return y;
}

// Returns FALSE if some of its text didn't fit
static bool8 DrawPokemonCard(u16 cardId, const struct PokemonCardInfo *info)
{
    static const u8 sShowLevels[] = {SHOW_ALL, SHOW_EFFECTS, SHOW_EX_RULE, 0};
    const u8 *stage;
    u8 *text = sCard->text;
    u8 *end;
    u8 i, x, hpY, show;

    DrawFrame(info->type, TRUE);

    // The HP and type, then the name before them; if it doesn't fit, the HP goes up to the
    // stage's line
    *text = EOS;
    end = CardGame_AppendTypeIcon(text, info->type);
    end = StringAppend(end, gText_Space);
    end = ConvertIntToDecimalStringN(end, info->hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(end, sText_Hp);
    hpY = DrawName(cardId, TEXT_RIGHT - GetTinyWidth(text) - 2) ? NAME_TINY_Y : STAGE_Y;
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

    // The picture, put up by ShowCard with the rest
    sCard->picSpecies = info->species;

    // The ability and attacks, with all they can of the rest
    for (i = 0; i < ARRAY_COUNT(sShowLevels) - 1; i++)
    {
        if (DrawRules(info, sShowLevels[i], FALSE) <= RULES_BOTTOM)
            break;
    }
    show = sShowLevels[i];
    DrawRules(info, show, TRUE);

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
    return DrawRules(info, show, FALSE) == DrawRules(info, SHOW_ALL, FALSE);
}

// "TRAINER" and the kind, the name, the item in the frame, and what the card does; a Supporter's
// "(1 a turn)" at the end. Returns FALSE if the text doesn't fit.
static bool8 DrawTrainerCard(u16 cardId, const struct TrainerCardInfo *info)
{
    u8 *text = sCard->text;
    u8 *rule;
    u8 y;

    DrawFrame(CARD_TYPE_NONE, FALSE);
    TinyPrint(TEXT_LEFT, STAGE_Y, COLOR_INK, sText_Trainer);
    *text = EOS;
    CardGame_AppendTrainerKind(text, info);
    for (rule = text; *rule != CHAR_NEWLINE && *rule != EOS; rule++)
        ;
    if (*rule == CHAR_NEWLINE)
        *rule++ = EOS;
    TinyPrintRight(TEXT_RIGHT, STAGE_Y, COLOR_INK, text);
    // The rule after what the card does
    StringCopy(sCard->line, rule);
    DrawName(cardId, TEXT_RIGHT);

    sCard->picItem = info->iconItem;

    *text = EOS;
    CardGame_AppendTrainerText(text, info);
    TinyWrap(text, TEXT_RIGHT - TEXT_LEFT);
    TinyPrint(TEXT_LEFT, ATTACKS_Y, COLOR_INK, text);
    y = ATTACKS_Y + CountLines(text) * TINY_LINE_HEIGHT + 4;
    if (sCard->line[0] != EOS)
    {
        TinyPrint(TEXT_LEFT, y, COLOR_INK, sCard->line);
        y += TINY_LINE_HEIGHT;
    }
    return y <= TRAINER_BOTTOM;
}

// Returns FALSE if some of the card's text didn't fit: VintageCard_PrintFullText has it all
bool8 VintageCard_Draw(u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    bool8 complete = TRUE;

    if (pokemon == NULL && trainer == NULL)
    {
        VintageCard_Clear();
        return TRUE;
    }
    sCard->picSpecies = SPECIES_NONE;
    sCard->picItem = ITEM_NONE;
    FillWindowPixelBuffer(sCard->textWindow, PIXEL_FILL(0));
    if (pokemon != NULL)
        complete = DrawPokemonCard(cardId, pokemon);
    else
        complete = DrawTrainerCard(cardId, trainer);
    ShowCard();
    return complete;
}

// The card's whole text in FONT_SMALL, in lines of width pixels from x, y, closer together if
// they don't fit above bottom: the ability, the attacks with their effects and the ex rule
// (Trainers: their kind and what they do)
void VintageCard_PrintFullText(u8 windowId, u16 cardId, u8 x, u8 y, u8 width, u8 bottom, const u8 *colors)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    u8 *text = sCard->text;
    u8 *line = sCard->line;
    u8 *end = text;
    const u8 *p;
    u8 i, lines, lineHeight, top = y;

    *end = EOS;
    if (pokemon != NULL)
    {
        if (pokemon->ability != CARD_ABILITY_NONE)
            end = CardGame_AppendAbilityText(end, pokemon);
        for (i = 0; i < pokemon->numAttacks; i++)
        {
            if (end != text)
                *end++ = CHAR_NEWLINE;
            *end = EOS;
            end = CardGame_AppendAttackText(end, &pokemon->attacks[i]);
        }
        if (pokemon->isEx)
        {
            *end++ = CHAR_NEWLINE;
            *end = EOS;
            CardGame_AppendExRule(end, pokemon);
        }
    }
    else if (trainer != NULL)
    {
        end = CardGame_AppendTrainerKind(end, trainer);
        *end++ = CHAR_NEWLINE;
        *end = EOS;
        CardGame_AppendTrainerText(end, trainer);
    }
    CardGame_WrapText(text, FONT_SMALL, width);

    lines = CountLines(text);
    lineHeight = FULL_LINE_HEIGHT;
    if (y + lines * lineHeight > bottom)
        lineHeight = max((bottom - y) / lines, FULL_MIN_LINE_HEIGHT);
    for (p = text; ; p += i + 1)
    {
        for (i = 0; p[i] != EOS && p[i] != CHAR_NEWLINE && i < sizeof(sCard->line) - 1; i++)
            line[i] = p[i];
        line[i] = EOS;
        AddTextPrinterParameterized3(windowId, FONT_SMALL, x, y, colors, TEXT_SKIP_DRAW, line);
        y += lineHeight;
        if (p[i] != CHAR_NEWLINE)
            break;
    }
    CardGame_DrawEnergyIcons(windowId, text, x, top, lineHeight);
}
