#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "gpu_regs.h"
#include "item_icon.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The card game's deck builder (the Card Case) and pack opening screen
// (docs/card_game_plan.md, phase 6). The decks are edited in place in the save.
//
// Deck builder: a list of the player's cards (A adds one to the deck), or of the deck's cards
// (A takes one out), or of the Energy types (A turns one on or off, 3 at most), with the
// highlighted card described on the right. L/R: the saved decks; START: duel with this deck.
// Left of the player's cards, the Card Player list: the card players met, where they are, and
// whether they were beaten.
//
// Pack screen: the cards of a pack, one at a time, with their picture.

enum
{
    WIN_HEADER,
    WIN_LIST,
    WIN_INFO,
    WIN_HELP,
    WIN_PACK,   // the pack screen's wider description
};

enum
{
    MODE_COLLECTION,
    MODE_DECK,
    MODE_ENERGY,
    MODE_PLAYERS,
    MODE_PACK,
};

enum
{
    STATE_FADE_IN,
    STATE_INPUT,
    STATE_FADE_OUT,
};

#define TEXT_PALETTE        15
#define TAG_ARROWS          0x4C10
#define TAG_INFO_ITEM       0x4C11
#define LABEL_LENGTH        24
#define MAX_ITEMS           CARD_COUNT
#define LIST_ROWS           7
#define PACK_WINDOW_HEIGHT  17  // tiles: the pack screen's card text goes down to the bottom
#define PACK_LINE_HEIGHT    12
#define PACK_MIN_LINE_HEIGHT 9
#define NOT_SHINY_PERSONALITY 0x0000FFFF

// The Energy types a deck can make
#define ZONE_TYPE_COUNT     (CARD_TYPE_METAL - CARD_TYPE_GRASS + 1)

struct DeckBuilder
{
    void (*exitCallback)(void);
    u8 mode;
    u8 state;
    u8 deckIndex;
    u8 listTaskId;
    u8 arrowTaskId;
    u16 scroll;
    u16 row;
    u16 itemCount;
    u16 infoCard;
    u16 infoSprite;
    u8 infoSpriteKind;      // 0: none, 1: icon, 2: item, 3: picture
    bool8 showingMessage;
    // Pack
    const u16 *packCards;
    u8 packCount;
    u8 packIndex;
    struct ListMenuItem items[MAX_ITEMS];
    u8 labels[MAX_ITEMS][LABEL_LENGTH];
    u8 text[400];
    u8 line[80];
};

static EWRAM_DATA struct DeckBuilder *sBuilder = NULL;

static void CB2_InitDeckBuilder(void);
static void Task_DeckBuilder(u8 taskId);
static void ShowList(void);
static void HideList(void);
static void DrawHeader(void);
static void DrawHelp(void);
static void ShowCardInfo(u16 cardId);
static void ShowPackCard(void);

static const u16 sBackdropColor = RGB(4, 10, 6);
static const u8 sHeaderTextColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static const u8 sText_Deck[] = _("DECK ");
static const u8 sText_InUse[] = _(" (IN USE)");
static const u8 sText_Slash[] = _("/");
static const u8 sText_Space[] = _(" ");
static const u8 sText_Cards[] = _(" cards  ENERGY ");
static const u8 sText_NoEnergy[] = _("-");
static const u8 sText_Times[] = _("x");
static const u8 sText_On[] = _("ON");
static const u8 sText_Hp[] = _(" HP ");
static const u8 sText_Basic[] = _("BASIC");
static const u8 sText_Stage1[] = _("STAGE 1");
static const u8 sText_Stage2[] = _("STAGE 2");
static const u8 sText_AbilityLabel[] = _("ABILITY");
static const u8 sText_YouHave[] = _("You have ");
static const u8 sText_PackCard[] = _("CARD ");
static const u8 sText_NoCards[] = _("No cards yet.");
static const u8 sText_EmptyDeck[] = _("The deck is empty.");
static const u8 sText_NoPlayers[] = _("No card players met.");
static const u8 sText_CardPlayers[] = _("CARD PLAYERS  BEATEN ");
static const u8 sText_Won[] = _("WON");
static const u8 sText_Beaten[] = _("Beaten!");
static const u8 sText_NotBeaten[] = _("Not beaten yet");
static const u8 sText_PlaysEnergy[] = _("Plays ");
static const u8 sText_PlaysYourDeck[] = _("Plays a copy of your deck");
static const u8 sText_Prize[] = _("Prize card:");
static const u8 sText_PrizeUnknown[] = _("???");

static const u8 sHelp_Collection[] = _("A: add  {LEFT_ARROW}: players  {RIGHT_ARROW}: the deck  L/R: deck 1-3\nSELECT: Energy  START: duel with this deck  B: done");
static const u8 sHelp_Deck[] = _("A: take out  {LEFT_ARROW}: your cards  L/R: deck 1-3\nSELECT: Energy  START: duel with this deck  B: done");
static const u8 sHelp_Energy[] = _("A: on/off (3 at most)\nB: back");
static const u8 sHelp_Players[] = _("{RIGHT_ARROW}: your cards\nB: done");
static const u8 sHelp_Pack[] = _("A: next card");
static const u8 sText_PackStats[] = _("  ");

static const u8 sMessage_DeckFull[] = _("The deck already has 20 cards.");
static const u8 sMessage_NoMoreCopies[] = _("No more copies of this card:\nyou have them all in the deck, or 2 already.");
static const u8 sMessage_TooManyTypes[] = _("A deck makes 3 Energy types at most.");
static const u8 sMessage_InUse[] = _("You will duel with this deck.");
static const u8 sMessage_WrongSize[] = _("The deck needs exactly 20 cards.");
static const u8 sMessage_TooManyCopies[] = _("A deck can have 2 copies of a card at most.");
static const u8 sMessage_NoBasic[] = _("The deck needs a Basic POKéMON.");
static const u8 sMessage_NoEnergy[] = _("Choose the deck's Energy with SELECT.");
static const u8 sMessage_NotOwned[] = _("The deck has cards you don't have.");

static const u8 *const sDeckErrors[] =
{
    [CARD_DECK_WRONG_SIZE] = sMessage_WrongSize,
    [CARD_DECK_TOO_MANY_COPIES] = sMessage_TooManyCopies,
    [CARD_DECK_NO_BASIC] = sMessage_NoBasic,
    [CARD_DECK_UNKNOWN_CARD] = sMessage_WrongSize,
    [CARD_DECK_NO_ENERGY_TYPE] = sMessage_NoEnergy,
    [CARD_DECK_NOT_OWNED] = sMessage_NotOwned,
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
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 2, .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    [WIN_LIST]   = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 2, .width = 16, .height = 14, .paletteNum = TEXT_PALETTE, .baseBlock = 61},
    [WIN_INFO]   = {.bg = 0, .tilemapLeft = 18, .tilemapTop = 2, .width = 11, .height = 14, .paletteNum = TEXT_PALETTE, .baseBlock = 285},
    [WIN_HELP]   = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 16, .width = 30, .height = 4, .paletteNum = TEXT_PALETTE, .baseBlock = 439},
    [WIN_PACK]   = {.bg = 0, .tilemapLeft = 12, .tilemapTop = 2, .width = 17, .height = PACK_WINDOW_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 559},
    DUMMY_WIN_TEMPLATE,
};

// ---------------------------------------------------------------------------------------------
// Entry and exit

static void StartScreen(void (*exitCallback)(void), u8 mode)
{
    sBuilder = AllocZeroed(sizeof(*sBuilder));
    sBuilder->exitCallback = exitCallback;
    sBuilder->mode = mode;
    sBuilder->deckIndex = gSaveBlock1Ptr->cardGame.activeDeck < CARD_SAVED_DECKS ? gSaveBlock1Ptr->cardGame.activeDeck : 0;
    sBuilder->arrowTaskId = TASK_NONE;
    sBuilder->listTaskId = TASK_NONE;
    sBuilder->infoSprite = SPRITE_NONE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitDeckBuilder);
}

// The screen is black when they are called
void OpenCardDeckBuilder(void (*exitCallback)(void))
{
    StartScreen(exitCallback, MODE_COLLECTION);
}

void OpenCardPackScreen(const u16 *cards, u8 count, void (*exitCallback)(void))
{
    StartScreen(exitCallback, MODE_PACK);
    sBuilder->packCards = cards;
    sBuilder->packCount = count;
}

static void CB2_DeckBuilder(void)
{
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_DeckBuilder(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitDeckBuilder(void)
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
        gMain.state++;
        break;
    case 2:
        if (sBuilder->mode == MODE_PACK)
        {
            ShowPackCard();
        }
        else
        {
            DrawHeader();
            ShowList();
            DrawHelp();
        }
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        SetVBlankCallback(VBlankCB_DeckBuilder);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_DeckBuilder, 0);
        SetMainCallback2(CB2_DeckBuilder);
        break;
    }
}

static void DestroyInfoSprite(void)
{
    switch (sBuilder->infoSpriteKind)
    {
    case 1:
        FreeAndDestroyMonIconSprite(&gSprites[sBuilder->infoSprite]);
        break;
    case 2:
        DestroySprite(&gSprites[sBuilder->infoSprite]);
        FreeSpriteTilesByTag(TAG_INFO_ITEM);
        FreeSpritePaletteByTag(TAG_INFO_ITEM);
        break;
    case 3:
        CardGame_DestroyPokemonPic(sBuilder->infoSprite);
        break;
    }
    sBuilder->infoSpriteKind = 0;
    sBuilder->infoSprite = SPRITE_NONE;
}

static void ExitScreen(void)
{
    HideList();
    DestroyInfoSprite();
    SetMainCallback2(sBuilder->exitCallback);
    FreeMonIconPalettes();
    FreeAllWindowBuffers();
    FREE_AND_SET_NULL(sBuilder);
}

// ---------------------------------------------------------------------------------------------
// Text

static u8 *AppendNumber(u8 *dst, u16 number)
{
    u8 digits = number >= 100 ? 3 : (number >= 10 ? 2 : 1);

    return ConvertIntToDecimalStringN(dst, number, STR_CONV_MODE_LEFT_ALIGN, digits);
}

// Breaks the text into lines at spaces, so each fits in width pixels
static void WrapText(u8 *text, u8 fontId, u16 width)
{
    u8 *lineStart = text, *lastSpace = NULL, *p;
    u8 saved;

    for (p = text; ; p++)
    {
        if (*p != CHAR_SPACE && *p != CHAR_NEWLINE && *p != EOS)
            continue;
        saved = *p;
        *p = EOS;
        if (GetStringWidth(fontId, lineStart, 0) > width && lastSpace != NULL)
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

static void TruncateToWidth(u8 *text, u8 fontId, u16 width)
{
    u16 length = StringLength(text);

    while (length != 0 && GetStringWidth(fontId, text, 0) > width)
        text[--length] = EOS;
}

// ---------------------------------------------------------------------------------------------
// The deck being edited

static u16 *GetDeck(void)
{
    return CardGame_GetSavedDeck(sBuilder->deckIndex);
}

static u8 CountDeck(void)
{
    const u16 *deck = GetDeck();
    u8 i, count = 0;

    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (deck[i] != CARD_NONE)
            count++;
    }
    return count;
}

// The cards in order, empty places last
static void SortDeck(void)
{
    u16 *deck = GetDeck();
    u16 card;
    u8 i, j;

    for (i = 1; i < CARD_DECK_SIZE; i++)
    {
        card = deck[i];
        for (j = i; j > 0 && (deck[j - 1] == CARD_NONE || (card != CARD_NONE && deck[j - 1] > card)); j--)
            deck[j] = deck[j - 1];
        deck[j] = card;
    }
}

static bool8 AddToDeck(u16 cardId)
{
    u16 *deck = GetDeck();
    u8 copies = CardGame_CountCopies(deck, cardId);
    u8 i;

    if (CountDeck() >= CARD_DECK_SIZE)
        return FALSE;
    if (copies >= CARD_MAX_COPIES || copies >= CardGame_GetOwnedCopies(cardId))
        return FALSE;
    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (deck[i] == CARD_NONE)
        {
            deck[i] = cardId;
            break;
        }
    }
    SortDeck();
    return TRUE;
}

static void RemoveFromDeck(u16 cardId)
{
    u16 *deck = GetDeck();
    u8 i;

    for (i = 0; i < CARD_DECK_SIZE; i++)
    {
        if (deck[i] == cardId)
        {
            deck[i] = CARD_NONE;
            break;
        }
    }
    SortDeck();
}

static bool8 HasZoneType(const u8 *zoneTypes, u8 type)
{
    u8 i;

    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == type)
            return TRUE;
    }
    return FALSE;
}

// Turns an Energy type on or off; FALSE if there are already 3
static bool8 ToggleZoneType(u8 type)
{
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];
    u8 i;

    CardGame_GetDeckZone(sBuilder->deckIndex, zoneTypes);
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == type)
        {
            zoneTypes[i] = CARD_TYPE_NONE;
            CardGame_SetDeckZone(sBuilder->deckIndex, zoneTypes);
            return TRUE;
        }
    }
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == CARD_TYPE_NONE)
        {
            zoneTypes[i] = type;
            CardGame_SetDeckZone(sBuilder->deckIndex, zoneTypes);
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------
// Drawing

static void DrawHeader(void)
{
    u8 zoneTypes[CARD_MAX_ZONE_TYPES];
    u8 *end, i;
    bool8 any = FALSE;

    if (sBuilder->mode == MODE_PLAYERS)
    {
        u8 met = 0, beaten = 0;

        for (i = 0; i < CARD_OPPONENTS_COUNT; i++)
        {
            met += CardGame_HasMetOpponent(i);
            beaten += CardGame_HasBeatenOpponent(i);
        }
        end = StringCopy(sBuilder->text, sText_CardPlayers);
        end = AppendNumber(end, beaten);
        end = StringAppend(end, sText_Slash);
        AppendNumber(end, met);
        FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
        AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 4, 0, sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
        PutWindowTilemap(WIN_HEADER);
        CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
        return;
    }

    end = StringCopy(sBuilder->text, sText_Deck);
    end = AppendNumber(end, sBuilder->deckIndex + 1);
    if (gSaveBlock1Ptr->cardGame.activeDeck == sBuilder->deckIndex)
        end = StringAppend(end, sText_InUse);
    end = StringAppend(end, sText_Space);
    end = StringAppend(end, sText_Space);
    end = AppendNumber(end, CountDeck());
    end = StringAppend(end, sText_Slash);
    end = AppendNumber(end, CARD_DECK_SIZE);
    end = StringAppend(end, sText_Cards);
    CardGame_GetDeckZone(sBuilder->deckIndex, zoneTypes);
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == CARD_TYPE_NONE)
            continue;
        if (any)
            end = StringAppend(end, sText_Space);
        end = StringAppend(end, gCardTypeNames[zoneTypes[i]]);
        any = TRUE;
    }
    if (!any)
        StringAppend(end, sText_NoEnergy);
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 4, 0, sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void PrintHelp(const u8 *text)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 4, 4, sHeaderTextColors, TEXT_SKIP_DRAW, text);
    PutWindowTilemap(WIN_HELP);
    CopyWindowToVram(WIN_HELP, COPYWIN_FULL);
}

static void DrawHelp(void)
{
    static const u8 *const sHelps[] = {
        [MODE_COLLECTION] = sHelp_Collection,
        [MODE_DECK]       = sHelp_Deck,
        [MODE_ENERGY]     = sHelp_Energy,
        [MODE_PLAYERS]    = sHelp_Players,
        [MODE_PACK]       = sHelp_Pack,
    };

    sBuilder->showingMessage = FALSE;
    PrintHelp(sHelps[sBuilder->mode]);
}

static void ShowHelpMessage(const u8 *text)
{
    PlaySE(SE_FAILURE);
    sBuilder->showingMessage = TRUE;
    PrintHelp(text);
}

// Prints text (with newlines) one line at a time from y, lineHeight apart; returns the next y
static u8 PrintPackLines(const u8 *text, u8 y, u8 lineHeight)
{
    u8 *line = sBuilder->line;
    u8 i;

    while (TRUE)
    {
        for (i = 0; text[i] != EOS && text[i] != CHAR_NEWLINE && i < sizeof(sBuilder->line) - 1; i++)
            line[i] = text[i];
        line[i] = EOS;
        AddTextPrinterParameterized(WIN_PACK, FONT_SMALL, line, 2, y, TEXT_SKIP_DRAW, NULL);
        y += lineHeight;
        if (text[i] != CHAR_NEWLINE)
            return y;
        text += i + 1;
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

// The pack screen's card, right of its picture: the name, then the stage, type and HP on one
// line, then the ability, the attacks with their effects and the ex rule (Trainers: kind and
// what they do).
// The lines move closer together when a card has many.
static void ShowPackCardText(u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    u8 *text = sBuilder->text;
    u8 *end;
    u8 i, y, lines, lineHeight;
    const u8 top = 18, bottom = PACK_WINDOW_HEIGHT * 8 - 2;

    FillWindowPixelBuffer(WIN_PACK, PIXEL_FILL(1));
    *text = EOS;
    CardGame_AppendCardName(text, cardId);
    TruncateToWidth(text, FONT_NORMAL, 130);
    AddTextPrinterParameterized(WIN_PACK, FONT_NORMAL, text, 2, 2, TEXT_SKIP_DRAW, NULL);

    // Everything below the name, as lines
    end = text;
    *end = EOS;
    if (pokemon != NULL)
    {
        end = StringAppend(end, pokemon->stage == CARD_STAGE_BASIC ? sText_Basic : (pokemon->stage == CARD_STAGE_1 ? sText_Stage1 : sText_Stage2));
        end = StringAppend(end, sText_PackStats);
        end = StringAppend(end, gCardTypeNames[pokemon->type]);
        end = StringAppend(end, sText_Hp);
        end = AppendNumber(end, pokemon->hp);
        if (pokemon->ability != CARD_ABILITY_NONE)
        {
            *end++ = CHAR_NEWLINE;
            *end = EOS;
            end = CardGame_AppendAbilityText(end, pokemon);
            WrapText(text, FONT_SMALL, 130);
            end = text + StringLength(text);
        }
        for (i = 0; i < pokemon->numAttacks; i++)
        {
            *end++ = CHAR_NEWLINE;
            *end = EOS;
            end = CardGame_AppendAttackText(end, &pokemon->attacks[i]);
            WrapText(text, FONT_SMALL, 130);
            end = text + StringLength(text);
        }
        if (pokemon->isEx)
        {
            *end++ = CHAR_NEWLINE;
            *end = EOS;
            CardGame_AppendExRule(end, pokemon);
            WrapText(text, FONT_SMALL, 130);
        }
    }
    else if (trainer != NULL)
    {
        end = CardGame_AppendTrainerKind(end, trainer);
        *end++ = CHAR_NEWLINE;
        *end = EOS;
        CardGame_AppendTrainerText(end, trainer);
        WrapText(text, FONT_SMALL, 130);
    }
    lines = CountLines(text);
    lineHeight = PACK_LINE_HEIGHT;
    if (top + lines * lineHeight > bottom)
        lineHeight = max((bottom - top) / lines, PACK_MIN_LINE_HEIGHT);
    y = PrintPackLines(text, top, lineHeight);
    CardGame_DrawEnergyIcons(WIN_PACK, text, 2, top, lineHeight);
    PutWindowTilemap(WIN_PACK);
    CopyWindowToVram(WIN_PACK, COPYWIN_FULL);
}

// The highlighted card: its icon with the stage and ability beside it, then its name, type
// and HP, and its attacks (Trainers: what they do). The pack screen has its own layout
// (ShowPackCardText), with the picture instead of the icon.
static void ShowCardInfo(u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    u8 windowId = WIN_INFO;
    u8 width = 84;
    u8 *text = sBuilder->text;
    u8 *end;
    u8 i, y;

    if (sBuilder->infoCard == cardId)
        return;
    sBuilder->infoCard = cardId;
    DestroyInfoSprite();
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (cardId != CARD_NONE)
    {
        *text = EOS;
        CardGame_AppendCardName(text, cardId);
        TruncateToWidth(text, FONT_SMALL, width);
        AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, 34, TEXT_SKIP_DRAW, NULL);
        y = 46;
        if (pokemon != NULL)
        {
            // Beside the icon: stage and ability
            StringCopy(text, pokemon->stage == CARD_STAGE_BASIC ? sText_Basic : (pokemon->stage == CARD_STAGE_1 ? sText_Stage1 : sText_Stage2));
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 38, 4, TEXT_SKIP_DRAW, NULL);
            if (pokemon->ability != CARD_ABILITY_NONE)
                AddTextPrinterParameterized(windowId, FONT_SMALL, sText_AbilityLabel, 38, 16, TEXT_SKIP_DRAW, NULL);
            end = StringCopy(text, gCardTypeNames[pokemon->type]);
            end = StringAppend(end, sText_Hp);
            AppendNumber(end, pokemon->hp);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
            y += 12;
            for (i = 0; i < pokemon->numAttacks; i++)
            {
                // Cost, name and damage (the pack screen shows the effects)
                struct CardAttack attack = pokemon->attacks[i];

                attack.effect = CARD_EFFECT_NONE;
                *text = EOS;
                CardGame_AppendAttackText(text, &attack);
                WrapText(text, FONT_SMALL, width);
                AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
                CardGame_DrawEnergyIcons(windowId, text, 2, y, 12);
                for (end = text, y += 12; *end != EOS; end++)
                {
                    if (*end == CHAR_NEWLINE)
                        y += 12;
                }
            }
            sBuilder->infoSprite = CreateMonIcon(pokemon->isMega ? GetMegaBaseSpecies(pokemon->species) : pokemon->species,
                                                 SpriteCB_MonIcon, 18 * 8 + 18, 2 * 8 + 16, 0, 0, FALSE);
            gSprites[sBuilder->infoSprite].oam.priority = 0; // over the window
            sBuilder->infoSpriteKind = 1;
        }
        else if (trainer != NULL)
        {
            *text = EOS;
            CardGame_AppendTrainerKind(text, trainer);
            WrapText(text, FONT_SMALL, 48);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 38, 4, TEXT_SKIP_DRAW, NULL);
            *text = EOS;
            CardGame_AppendTrainerText(text, trainer);
            WrapText(text, FONT_SMALL, width);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
            if (trainer->iconItem != ITEM_NONE)
            {
                sBuilder->infoSprite = AddItemIconSprite(TAG_INFO_ITEM, TAG_INFO_ITEM, trainer->iconItem);
                if (sBuilder->infoSprite < MAX_SPRITES)
                {
                    gSprites[sBuilder->infoSprite].x = 18 * 8 + 18;
                    gSprites[sBuilder->infoSprite].y = 2 * 8 + 16;
                    gSprites[sBuilder->infoSprite].oam.priority = 0;
                    sBuilder->infoSpriteKind = 2;
                }
            }
        }
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

// A card player: where they are, their Energy, beaten or not, and their prize card (once won)
static void ShowPlayerInfo(u8 opponentId)
{
    const struct CardOpponent *opponent = &gCardOpponents[opponentId];
    const struct CardDeckData *deck = &gCardDecks[opponent->deck];
    bool8 beaten = CardGame_HasBeatenOpponent(opponentId);
    u8 *text = sBuilder->text;
    u8 *end;
    u8 i;

    sBuilder->infoCard = CARD_COUNT;
    DestroyInfoSprite();
    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    CardGame_GetOpponentName(text, opponentId);
    AddTextPrinterParameterized(WIN_INFO, FONT_NORMAL, text, 2, 2, TEXT_SKIP_DRAW, NULL);
    GetMapName(text, opponent->mapsec, 0);
    TruncateToWidth(text, FONT_SMALL, 84);
    AddTextPrinterParameterized(WIN_INFO, FONT_SMALL, text, 2, 18, TEXT_SKIP_DRAW, NULL);
    end = StringCopy(text, sText_PlaysEnergy);
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (deck->zoneTypes[i] == CARD_TYPE_NONE)
            continue;
        if (i != 0)
            end = StringAppend(end, sText_Space);
        end = StringAppend(end, gCardTypeNames[deck->zoneTypes[i]]);
    }
    if (opponent->deck == CARD_DECK_COPYCAT)
        StringCopy(text, sText_PlaysYourDeck);
    WrapText(text, FONT_SMALL, 84);
    AddTextPrinterParameterized(WIN_INFO, FONT_SMALL, text, 2, 34, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(WIN_INFO, FONT_SMALL, beaten ? sText_Beaten : sText_NotBeaten, 2, 62, TEXT_SKIP_DRAW, NULL);
    if (opponent->rewardCard != CARD_NONE)
    {
        AddTextPrinterParameterized(WIN_INFO, FONT_SMALL, sText_Prize, 2, 78, TEXT_SKIP_DRAW, NULL);
        if (beaten)
        {
            *text = EOS;
            CardGame_AppendCardName(text, opponent->rewardCard);
        }
        else
        {
            StringCopy(text, sText_PrizeUnknown);
        }
        WrapText(text, FONT_SMALL, 84);
        AddTextPrinterParameterized(WIN_INFO, FONT_SMALL, text, 2, 90, TEXT_SKIP_DRAW, NULL);
    }
    PutWindowTilemap(WIN_INFO);
    CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
}

// The counts right of each card: in the deck/owned, or in the deck, or ON
static void PrintListCounts(u8 windowId, u32 itemId, u8 y)
{
    u8 *text = sBuilder->text;
    u8 *end;

    if (itemId == LIST_CANCEL)
        return;
    switch (sBuilder->mode)
    {
    case MODE_COLLECTION:
        end = AppendNumber(text, CardGame_CountCopies(GetDeck(), itemId));
        end = StringAppend(end, sText_Slash);
        AppendNumber(end, CardGame_GetOwnedCopies(itemId));
        break;
    case MODE_DECK:
        end = StringCopy(text, sText_Times);
        AppendNumber(end, CardGame_CountCopies(GetDeck(), itemId));
        break;
    case MODE_ENERGY:
        {
            u8 zoneTypes[CARD_MAX_ZONE_TYPES];

            CardGame_GetDeckZone(sBuilder->deckIndex, zoneTypes);
            StringCopy(text, HasZoneType(zoneTypes, itemId) ? sText_On : sText_Space);
        }
        break;
    case MODE_PLAYERS:
        StringCopy(text, CardGame_HasBeatenOpponent(itemId) ? sText_Won : sText_Space);
        break;
    }
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, 104, y, TEXT_SKIP_DRAW, NULL);
}

static void MoveListCursor(s32 itemId, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
    if (sBuilder->mode == MODE_PLAYERS && itemId >= 0)
        ShowPlayerInfo(itemId);
    else if (sBuilder->mode != MODE_ENERGY && itemId >= 0)
        ShowCardInfo(itemId);
}

static void AddItem(u16 id, const u8 *name)
{
    u16 n = sBuilder->itemCount++;

    StringCopy(sBuilder->labels[n], name);
    TruncateToWidth(sBuilder->labels[n], FONT_NORMAL, 92);
    sBuilder->items[n].name = sBuilder->labels[n];
    sBuilder->items[n].id = id;
}

static void BuildItems(void)
{
    const u16 *deck = GetDeck();
    u8 name[LABEL_LENGTH * 2];
    u16 cardId;
    u8 i;

    sBuilder->itemCount = 0;
    switch (sBuilder->mode)
    {
    case MODE_COLLECTION:
        for (cardId = 1; cardId < CARD_COUNT; cardId++)
        {
            if (CardGame_GetOwnedCopies(cardId) == 0)
                continue;
            *name = EOS;
            CardGame_AppendCardName(name, cardId);
            AddItem(cardId, name);
        }
        break;
    case MODE_DECK:
        for (i = 0; i < CARD_DECK_SIZE; i++)
        {
            if (deck[i] == CARD_NONE || (i != 0 && deck[i - 1] == deck[i]))
                continue;
            *name = EOS;
            CardGame_AppendCardName(name, deck[i]);
            AddItem(deck[i], name);
        }
        break;
    case MODE_ENERGY:
        for (i = CARD_TYPE_GRASS; i < CARD_TYPE_GRASS + ZONE_TYPE_COUNT; i++)
            AddItem(i, gCardTypeNames[i]);
        break;
    case MODE_PLAYERS:
        for (i = 0; i < CARD_OPPONENTS_COUNT; i++)
        {
            if (CardGame_HasMetOpponent(i))
            {
                CardGame_GetOpponentName(name, i);
                AddItem(i, name);
            }
        }
        break;
    }
}

static void HideList(void)
{
    if (sBuilder->listTaskId != TASK_NONE)
    {
        DestroyListMenuTask(sBuilder->listTaskId, &sBuilder->scroll, &sBuilder->row);
        sBuilder->listTaskId = TASK_NONE;
    }
    if (sBuilder->arrowTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sBuilder->arrowTaskId);
        sBuilder->arrowTaskId = TASK_NONE;
    }
}

// (Re)builds the list, keeping the cursor where it was if possible
static void ShowList(void)
{
    struct ListMenuTemplate template;

    HideList();
    BuildItems();
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    PutWindowTilemap(WIN_LIST);
    if (sBuilder->itemCount == 0)
    {
        const u8 *empty = sText_NoCards;

        if (sBuilder->mode == MODE_DECK)
            empty = sText_EmptyDeck;
        else if (sBuilder->mode == MODE_PLAYERS)
            empty = sText_NoPlayers;
        AddTextPrinterParameterized(WIN_LIST, FONT_NORMAL, empty, 8, 1, TEXT_SKIP_DRAW, NULL);
        CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
        sBuilder->infoCard = CARD_COUNT;
        ShowCardInfo(CARD_NONE);
        return;
    }
    if (sBuilder->scroll + sBuilder->row >= sBuilder->itemCount)
    {
        sBuilder->row = (sBuilder->itemCount - 1) < LIST_ROWS - 1 ? sBuilder->itemCount - 1 : LIST_ROWS - 1;
        sBuilder->scroll = sBuilder->itemCount - 1 - sBuilder->row;
    }
    if (sBuilder->mode == MODE_ENERGY)
        ShowCardInfo(CARD_NONE);

    template.items = sBuilder->items;
    template.moveCursorFunc = MoveListCursor;
    template.itemPrintFunc = PrintListCounts;
    template.totalItems = sBuilder->itemCount;
    template.maxShowed = LIST_ROWS;
    template.windowId = WIN_LIST;
    template.header_X = 0;
    template.item_X = 8;
    template.cursor_X = 0;
    template.upText_Y = 1;
    template.cursorPal = 2;
    template.fillValue = 1;
    template.cursorShadowPal = 3;
    template.lettersSpacing = 0;
    template.itemVerticalPadding = 0;
    template.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    template.fontId = FONT_NORMAL;
    template.cursorKind = 0;
    sBuilder->listTaskId = ListMenuInit(&template, sBuilder->scroll, sBuilder->row);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
    if (sBuilder->itemCount > LIST_ROWS)
        sBuilder->arrowTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 72, 14, 132, sBuilder->itemCount - LIST_ROWS,
                                                                         TAG_ARROWS, TAG_ARROWS, &sBuilder->scroll);
}

// Each list starts at the top (HideList first: it gives back the old list's cursor)
static void SetMode(u8 mode)
{
    HideList();
    sBuilder->mode = mode;
    sBuilder->scroll = sBuilder->row = 0;
    sBuilder->infoCard = CARD_COUNT;
    ShowList();
    DrawHelp();
    DrawHeader();
}

// ---------------------------------------------------------------------------------------------
// The pack

static void ShowPackCard(void)
{
    u16 cardId = sBuilder->packCards[sBuilder->packIndex];
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    u8 *end;

    ClearWindowTilemap(WIN_LIST);
    DestroyInfoSprite();
    ShowPackCardText(cardId);
    // Left of the text: the Pokémon's picture, or the Trainer's item
    if (pokemon != NULL)
    {
        sBuilder->infoSprite = CardGame_CreatePokemonPic(pokemon->species, 52, 72);
        if (sBuilder->infoSprite < MAX_SPRITES)
            sBuilder->infoSpriteKind = 3;
    }
    else if (trainer != NULL && trainer->iconItem != ITEM_NONE)
    {
        sBuilder->infoSprite = AddItemIconSprite(TAG_INFO_ITEM, TAG_INFO_ITEM, trainer->iconItem);
        if (sBuilder->infoSprite < MAX_SPRITES)
        {
            gSprites[sBuilder->infoSprite].x = 52;
            gSprites[sBuilder->infoSprite].y = 72;
            gSprites[sBuilder->infoSprite].oam.priority = 0;
            sBuilder->infoSpriteKind = 2;
        }
    }

    end = StringCopy(sBuilder->text, sText_PackCard);
    end = AppendNumber(end, sBuilder->packIndex + 1);
    end = StringAppend(end, sText_Slash);
    end = AppendNumber(end, sBuilder->packCount);
    end = StringAppend(end, sText_Space);
    end = StringAppend(end, sText_Space);
    end = StringAppend(end, sText_YouHave);
    AppendNumber(end, CardGame_GetOwnedCopies(cardId));
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 4, 0, sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
    // The help goes in the header: the card takes the rest of the screen
    AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 240 - 4 - GetStringWidth(FONT_SMALL, sHelp_Pack, 0), 2, sHeaderTextColors, TEXT_SKIP_DRAW, sHelp_Pack);
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
    PlaySE(SE_SELECT);
}

// ---------------------------------------------------------------------------------------------
// Input

static void HandleListChoice(s32 input)
{
    switch (sBuilder->mode)
    {
    case MODE_COLLECTION:
        if (CountDeck() >= CARD_DECK_SIZE)
            ShowHelpMessage(sMessage_DeckFull);
        else if (!AddToDeck(input))
            ShowHelpMessage(sMessage_NoMoreCopies);
        else
            PlaySE(SE_SELECT);
        break;
    case MODE_DECK:
        RemoveFromDeck(input);
        PlaySE(SE_SELECT);
        break;
    case MODE_ENERGY:
        if (!ToggleZoneType(input))
            ShowHelpMessage(sMessage_TooManyTypes);
        else
            PlaySE(SE_SELECT);
        break;
    case MODE_PLAYERS:
        return;
    }
    DrawHeader();
    ShowList();
}

static void HandleDeckInput(void)
{
    s32 input = sBuilder->listTaskId != TASK_NONE ? ListMenu_ProcessInput(sBuilder->listTaskId) : LIST_NOTHING_CHOSEN;
    u8 result;

    if (sBuilder->listTaskId != TASK_NONE)
        ListMenuGetScrollAndRow(sBuilder->listTaskId, &sBuilder->scroll, &sBuilder->row);
    if (sBuilder->showingMessage && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY | L_BUTTON | R_BUTTON | START_BUTTON | SELECT_BUTTON))
        DrawHelp();

    if (input >= 0)
    {
        HandleListChoice(input);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sBuilder->mode == MODE_ENERGY)
        {
            SetMode(MODE_DECK);
        }
        else
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sBuilder->state = STATE_FADE_OUT;
        }
    }
    else if (JOY_NEW(DPAD_RIGHT) && sBuilder->mode == MODE_COLLECTION)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_DECK);
    }
    else if (JOY_NEW(DPAD_RIGHT) && sBuilder->mode == MODE_PLAYERS)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_COLLECTION);
    }
    else if (JOY_NEW(DPAD_LEFT) && sBuilder->mode == MODE_DECK)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_COLLECTION);
    }
    else if (JOY_NEW(DPAD_LEFT) && sBuilder->mode == MODE_COLLECTION)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_PLAYERS);
    }
    else if (sBuilder->mode == MODE_PLAYERS)
    {
        // Nothing else to do on the list
    }
    else if (JOY_NEW(SELECT_BUTTON) && sBuilder->mode != MODE_ENERGY)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_ENERGY);
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON) && sBuilder->mode != MODE_ENERGY)
    {
        PlaySE(SE_SELECT);
        sBuilder->deckIndex = (sBuilder->deckIndex + (JOY_NEW(R_BUTTON) ? 1 : CARD_SAVED_DECKS - 1)) % CARD_SAVED_DECKS;
        HideList();
        sBuilder->scroll = sBuilder->row = 0;
        sBuilder->infoCard = CARD_COUNT;
        DrawHeader();
        ShowList();
    }
    else if (JOY_NEW(START_BUTTON))
    {
        result = CardGame_ValidateSavedDeck(sBuilder->deckIndex);
        if (result == CARD_DECK_OK)
        {
            PlaySE(SE_SUCCESS);
            gSaveBlock1Ptr->cardGame.activeDeck = sBuilder->deckIndex;
            DrawHeader();
            PrintHelp(sMessage_InUse);
            sBuilder->showingMessage = TRUE;
        }
        else
        {
            ShowHelpMessage(sDeckErrors[result]);
        }
    }
}

static void Task_DeckBuilder(u8 taskId)
{
    switch (sBuilder->state)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
            sBuilder->state = STATE_INPUT;
        break;
    case STATE_INPUT:
        if (sBuilder->mode != MODE_PACK)
        {
            HandleDeckInput();
        }
        else if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            if (++sBuilder->packIndex < sBuilder->packCount)
            {
                ShowPackCard();
            }
            else
            {
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
                sBuilder->state = STATE_FADE_OUT;
            }
        }
        break;
    case STATE_FADE_OUT:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            ExitScreen();
        }
        break;
    }
}
