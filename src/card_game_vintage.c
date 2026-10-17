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
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/card_game_cards.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// A test of a card drawn like the first Pokémon TCG cards (Base Set, 1999): a yellow border,
// a panel in the type's color, the name and HP, a framed picture with the Pokédex strip under
// it, the attacks with their damage on the right, then weakness and retreat cost (no
// resistance: this game has none).
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
    FRAME_STRIP,
    FRAME_LINE,
    FRAME_WHITE,
};

// Layout, in the card's pixels
#define BORDER          5
#define STAGE_Y         (BORDER - 2)
#define NAME_Y          12
#define ART_X           10
#define ART_Y           28
#define ART_WIDTH       92
#define ART_HEIGHT      52
#define STRIP_Y         (ART_Y + ART_HEIGHT + 2)
#define STRIP_HEIGHT    9
#define ATTACKS_Y       (STRIP_Y + STRIP_HEIGHT + 2)
#define BOTTOM_Y        136
#define TEXT_RIGHT      (CARD_WIDTH * 8 - BORDER - 3)

enum {
    WIN_FRAME,
    WIN_TEXT,
};

struct VintageCard
{
    void (*exitCallback)(void);
    u16 cardId;
    u16 picSprite;
    u8 iconSprite;      // the previous stage, in the picture's corner
    u8 state;
    u8 text[160];
};

extern const struct PokedexEntry gPokedexEntries[];

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
    [FRAME_STRIP]          = RGB(31, 28, 15),
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

static const u8 sTextColors[] = {TEXT_COLOR_TRANSPARENT, 4, TEXT_COLOR_TRANSPARENT};  // icon palette's near-black
static const u8 sHpColors[] = {TEXT_COLOR_TRANSPARENT, 8, TEXT_COLOR_TRANSPARENT};    // icon palette's dark red

static const u8 sText_Basic[] = _("BASIC POKéMON");
static const u8 sText_BasicShort[] = _("BASIC");
static const u8 sText_Stage1[] = _("STAGE 1");
static const u8 sText_Stage2[] = _("STAGE 2");
static const u8 sText_EvolvesFrom[] = _("Evolves from ");
static const u8 sText_From[] = _("from ");
static const u8 sText_Hp[] = _(" HP");
static const u8 sText_PokemonDot[] = _(" POKéMON. ");
static const u8 sText_Inches[] = _("” ");
static const u8 sText_Lbs[] = _(" lbs.");
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
    sCard->iconSprite = MAX_SPRITES;
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
        LoadMonIconPalettes();
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
        // Sprites only in the picture (and the previous stage's box, at its corner)
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
    if (sCard->iconSprite != MAX_SPRITES)
        FreeAndDestroyMonIconSprite(&gSprites[sCard->iconSprite]);
    sCard->iconSprite = MAX_SPRITES;
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
            FreeMonIconPalettes();
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
// Drawing

static void FillFrame(u8 color, u8 x, u8 y, u8 width, u8 height)
{
    FillWindowPixelRect(WIN_FRAME, PIXEL_FILL(color), x, y, width, height);
}

// The border, the panel, the picture's frame and background, the strip and the line above the
// weakness
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
    FillFrame(FRAME_GROUND, ART_X, ART_Y + ART_HEIGHT - 12, ART_WIDTH, 12);

    // The Pokédex strip
    FillFrame(FRAME_BORDER_SHADE, ART_X - 2, STRIP_Y, ART_WIDTH + 4, STRIP_HEIGHT);
    FillFrame(FRAME_STRIP, ART_X - 1, STRIP_Y + 1, ART_WIDTH + 2, STRIP_HEIGHT - 2);

    // The line above weakness, resistance and retreat
    FillFrame(FRAME_LINE, BORDER + 3, BOTTOM_Y - 2, w - 2 * BORDER - 6, 1);
    CopyWindowToVram(WIN_FRAME, COPYWIN_FULL);
}

static void Print(u8 fontId, const u8 *colors, u8 x, u8 y, const u8 *text)
{
    AddTextPrinterParameterized3(WIN_TEXT, fontId, x, y, colors, TEXT_SKIP_DRAW, text);
    CardGame_DrawEnergyIcons(WIN_TEXT, text, x, y, 12);
}

static void PrintRight(u8 fontId, const u8 *colors, u8 right, u8 y, const u8 *text)
{
    Print(fontId, colors, right - GetStringWidth(fontId, text, 0), y, text);
}

// "BIRD POKéMON. 3’7” 66 lbs." from the Pokédex, in feet and pounds as on the cards (which
// say "Length: 3’ 7”, Weight: 66 lbs.", too long here); just the category if it doesn't fit
static void AppendDexStrip(u8 *dst, u16 species)
{
    const struct PokedexEntry *entry = &gPokedexEntries[SpeciesToNationalPokedexNum(species)];
    u32 inches = (entry->height * 3937 + 5000) / 10000;   // decimeters
    u32 pounds = (entry->weight * 2205 + 5000) / 10000;   // hectograms

    dst = StringCopy(dst, entry->categoryName);
    dst = StringAppend(dst, sText_PokemonDot);
    dst = ConvertIntToDecimalStringN(dst, inches / 12, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dst++ = CHAR_SGL_QUOTE_RIGHT;
    dst = ConvertIntToDecimalStringN(dst, inches % 12, STR_CONV_MODE_LEFT_ALIGN, 2);
    dst = StringCopy(dst, sText_Inches);
    dst = ConvertIntToDecimalStringN(dst, pounds, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringCopy(dst, sText_Lbs);
}

// The attacks: cost and name, the damage on the right, then the effect if withEffects. Returns
// where they end; draws them only if draw.
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
            if (draw)
                PrintRight(FONT_NORMAL, sTextColors, TEXT_RIGHT, y - 2, text);
            damageWidth = GetStringWidth(FONT_NORMAL, text, 0) + 4;
        }
        nameOnly.damage = 0;
        nameOnly.effect = CARD_EFFECT_NONE;
        *text = EOS;
        CardGame_AppendAttackText(text, &nameOnly);
        if (BORDER + 2 + GetStringWidth(FONT_SMALL, text, 0) > TEXT_RIGHT - damageWidth)
        {
            // Too long beside the damage: the cost, then the name under it
            for (end = text; *end != CHAR_SPACE && *end != EOS; end++)
            {
                if (*end == EXT_CTRL_CODE_BEGIN)
                    end += 2;
            }
            *end = EOS;
            if (draw)
                Print(FONT_SMALL, sTextColors, BORDER + 2, y, text);
            y += 10;
            *text = EOS;
            CardGame_AppendAttackText(text, &nameOnly);
            if (draw)
                Print(FONT_SMALL, sTextColors, BORDER + 2, y, end + 1);
        }
        else if (draw)
        {
            Print(FONT_SMALL, sTextColors, BORDER + 2, y, text);
        }
        y += 12;
        *text = EOS;
        if (withEffects)
            CardGame_AppendAttackEffect(text, attack);
        if (*text != EOS)
        {
            CardGame_WrapText(text, FONT_SMALL_NARROW, TEXT_RIGHT - BORDER - 4);
            if (draw)
                Print(FONT_SMALL_NARROW, sTextColors, BORDER + 4, y - 1, text);
            for (end = text; *end != EOS; end++)
            {
                if (*end == CHAR_NEWLINE)
                    y += 9;
            }
            y += 10;
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
    u8 i, y, x;

    DestroyPicture();
    DrawFrame(info->type);
    FillWindowPixelBuffer(WIN_TEXT, PIXEL_FILL(0));

    // The name, smaller if it doesn't fit before the HP; if it still doesn't, the HP goes up
    // to the stage's line
    *text = EOS;
    end = CardGame_AppendTypeIcon(text, info->type);
    x = GetStringWidth(FONT_SMALL, text, 0) + 2;
    end = ConvertIntToDecimalStringN(text, info->hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(end, sText_Hp);
    x = TEXT_RIGHT - x - GetStringWidth(FONT_SMALL, text, 0) - 2;
    *text = EOS;
    CardGame_AppendCardName(text, sCard->cardId);
    if (BORDER + 2 + GetStringWidth(FONT_NORMAL, text, 0) <= x)
    {
        Print(FONT_NORMAL, sTextColors, BORDER + 2, NAME_Y, text);
        y = NAME_Y + 2;
    }
    else
    {
        Print(FONT_SMALL, sTextColors, BORDER + 2, NAME_Y + 2, text);
        y = BORDER + 2 + GetStringWidth(FONT_SMALL, text, 0) <= x ? NAME_Y + 2 : STAGE_Y;
    }
    *text = EOS;
    CardGame_AppendTypeIcon(text, info->type);
    PrintRight(FONT_SMALL, sTextColors, TEXT_RIGHT, y, text);
    x = TEXT_RIGHT - GetStringWidth(FONT_SMALL, text, 0) - 2;
    end = ConvertIntToDecimalStringN(text, info->hp, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(end, sText_Hp);
    PrintRight(FONT_SMALL, sHpColors, x, y, text);

    // The stage ("BASIC" only beside the HP), and what it evolves from: "Evolves from X", or
    // "from X", if it fits (the previous stage's icon, in the picture's corner, says it anyway)
    if (info->stage == CARD_STAGE_BASIC)
        stage = y == STAGE_Y ? sText_BasicShort : sText_Basic;
    else
        stage = info->stage == CARD_STAGE_1 ? sText_Stage1 : sText_Stage2;
    Print(FONT_SMALL_NARROW, sTextColors, BORDER + 2, STAGE_Y, stage);
    if (info->evolvesFrom != CARD_NONE && y != STAGE_Y)
    {
        x = BORDER + 2 + GetStringWidth(FONT_SMALL_NARROW, stage, 0) + 4;
        end = StringCopy(text, sText_EvolvesFrom);
        CardGame_AppendCardName(end, info->evolvesFrom);
        if (TEXT_RIGHT - GetStringWidth(FONT_SMALL_NARROW, text, 0) < x)
        {
            end = StringCopy(text, sText_From);
            CardGame_AppendCardName(end, info->evolvesFrom);
        }
        if (TEXT_RIGHT - GetStringWidth(FONT_SMALL_NARROW, text, 0) >= x)
            PrintRight(FONT_SMALL_NARROW, sTextColors, TEXT_RIGHT, STAGE_Y, text);
    }

    // The picture, its feet on the ground
    sCard->picSprite = CardGame_CreatePokemonPic(info->species, CARD_X + ART_X + ART_WIDTH / 2, ART_Y + ART_HEIGHT - 32 - 1);
    if (sCard->picSprite != MAX_SPRITES)
        gSprites[sCard->picSprite].oam.priority = 1;
    // The previous stage in a small box over the picture's corner, as on the Base Set
    if (info->evolvesFrom != CARD_NONE)
    {
        const struct PokemonCardInfo *previous = CardGame_GetPokemonInfo(info->evolvesFrom);

        // On the text's layer, so the picture goes under it; the icon goes over it
        FillWindowPixelRect(WIN_TEXT, PIXEL_FILL(4), ART_X - 2, ART_Y - 2, 25, 23);
        FillWindowPixelRect(WIN_TEXT, PIXEL_FILL(15), ART_X - 2, ART_Y - 2, 24, 22);
        FillWindowPixelRect(WIN_TEXT, PIXEL_FILL(1), ART_X - 1, ART_Y - 1, 22, 20);
        if (previous != NULL)
        {
            sCard->iconSprite = CreateMonIcon(previous->isMega ? GetMegaBaseSpecies(previous->species) : previous->species,
                                              SpriteCallbackDummy, CARD_X + ART_X + 10, ART_Y + 4, 0, 0, FALSE);
            gSprites[sCard->iconSprite].oam.priority = 0;
        }
    }

    // The Pokédex strip
    AppendDexStrip(text, info->species);
    if (GetStringWidth(FONT_SMALL_NARROW, text, 0) > ART_WIDTH)
    {
        for (end = text; *end != CHAR_PERIOD && *end != EOS; end++)
            ;
        if (*end == CHAR_PERIOD)
            end[1] = EOS;
    }
    Print(FONT_SMALL_NARROW, sTextColors, ART_X, STRIP_Y - 2, text);

    // The attacks, without their effects if they don't all fit
    DrawAttacks(info, DrawAttacks(info, TRUE, FALSE) <= BOTTOM_Y - 4, TRUE);

    // Weakness and retreat cost, their icons under them (the Base Set's resistance doesn't
    // exist in this game)
    Print(FONT_SMALL_NARROW, sTextColors, BORDER + 3, BOTTOM_Y - 3, sText_Weakness);
    PrintRight(FONT_SMALL_NARROW, sTextColors, TEXT_RIGHT, BOTTOM_Y - 3, sText_Retreat);
    *text = EOS;
    CardGame_AppendTypeIcon(text, info->weakness);
    Print(FONT_SMALL, sTextColors, BORDER + 8, BOTTOM_Y + 6, text);
    *text = EOS;
    end = text;
    for (i = 0; i < info->retreatCost; i++)
        end = CardGame_AppendTypeIcon(end, CARD_TYPE_COLORLESS);
    PrintRight(FONT_SMALL, sTextColors, TEXT_RIGHT, BOTTOM_Y + 6, text);
    CopyWindowToVram(WIN_TEXT, COPYWIN_FULL);
}
