#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "gpu_regs.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
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
// highlighted card on the right, drawn like the Base Set's (src/card_game_vintage.c). L/R: the
// saved decks; START: duel with this deck. Left of the player's cards, the Card Player list:
// the card players met, where they are, and whether they were beaten.
//
// Pack screen: the cards of a pack, one at a time, drawn the same way, with how many the player
// has, and the card's whole text left of it when the card can't show it all.
//
// The left half (SIDE_WIDTH tiles) has the header, the list and the help; the card is on the
// right, its frame on BG1.

enum
{
    WIN_HEADER,
    WIN_LIST,   // the pack screen: the card's whole text
    WIN_HELP,
    WIN_CARD,   // the card's text, or a card player's details
    WIN_FRAME,  // the card's frame
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
#define FRAME_PALETTE       14
#define TAG_ARROWS          0x4C10
#define LABEL_LENGTH        24
#define MAX_ITEMS           CARD_COUNT
#define LIST_ROWS           6
#define SIDE_WIDTH          (DISPLAY_TILE_WIDTH - VINTAGE_CARD_WIDTH)   // tiles
#define HEADER_HEIGHT       2
#define LIST_HEIGHT         (LIST_ROWS * 2)
#define HELP_HEIGHT         (DISPLAY_TILE_HEIGHT - HEADER_HEIGHT - LIST_HEIGHT)
#define HELP_TOP            8   // pixels: the list's scroll arrow is above
#define HELP_LINE_HEIGHT    9

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
    bool8 showingMessage;
    // Pack
    const u16 *packCards;
    u8 packCount;
    u8 packIndex;
    struct ListMenuItem items[MAX_ITEMS];
    u8 labels[MAX_ITEMS][LABEL_LENGTH];
    u8 text[400];
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
static const u8 sText_InUse[] = _(" IN USE");
static const u8 sText_Slash[] = _("/");
static const u8 sText_Space[] = _(" ");
static const u8 sText_NoEnergy[] = _("-");
static const u8 sText_Times[] = _("x");
static const u8 sText_On[] = _("ON");
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

static const u8 sHelp_Collection[] = _("A: add  L/R: deck\n{LEFT_ARROW}: players  {RIGHT_ARROW}: the deck\nSELECT: Energy  B: done\nSTART: duel with this deck");
static const u8 sHelp_Deck[] = _("A: take out  L/R: deck\n{LEFT_ARROW}: your cards\nSELECT: Energy  B: done\nSTART: duel with this deck");
static const u8 sHelp_Energy[] = _("A: on/off (3 at most)\nB: back");
static const u8 sHelp_Players[] = _("{RIGHT_ARROW}: your cards\nB: done");
static const u8 sHelp_Pack[] = _("A: next card");

static const u8 sMessage_DeckFull[] = _("The deck already has 20 cards.");
static const u8 sMessage_NoMoreCopies[] = _("No more copies of this card: you have them all in the deck, or 2 already.");
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
    {
        .bg = 1,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = SIDE_WIDTH, .height = HEADER_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    [WIN_LIST]   = {.bg = 0, .tilemapLeft = 0, .tilemapTop = HEADER_HEIGHT, .width = SIDE_WIDTH, .height = LIST_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 33},
    [WIN_HELP]   = {.bg = 0, .tilemapLeft = 0, .tilemapTop = HEADER_HEIGHT + LIST_HEIGHT, .width = SIDE_WIDTH, .height = HELP_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 225},
    [WIN_CARD]   = {.bg = 0, .tilemapLeft = SIDE_WIDTH, .tilemapTop = 0, .width = VINTAGE_CARD_WIDTH, .height = VINTAGE_CARD_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 321},
    [WIN_FRAME]  = {.bg = 1, .tilemapLeft = SIDE_WIDTH, .tilemapTop = 0, .width = VINTAGE_CARD_WIDTH, .height = VINTAGE_CARD_HEIGHT, .paletteNum = FRAME_PALETTE, .baseBlock = 1},
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
        VintageCard_Init(WIN_FRAME, WIN_CARD);
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
        if (sBuilder->mode == MODE_PACK)
        {
            ShowPackCard();
        }
        else
        {
            DrawHeader();
            ShowList();
        }
        DrawHelp();
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        VintageCard_EnableClipping();
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB_DeckBuilder);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_DeckBuilder, 0);
        SetMainCallback2(CB2_DeckBuilder);
        break;
    }
}

static void ExitScreen(void)
{
    HideList();
    VintageCard_Free();
    VintageCard_DisableClipping();
    SetMainCallback2(sBuilder->exitCallback);
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

    // "DECK 1 IN USE  18/20" and the Energy types' icons
    end = StringCopy(sBuilder->text, sText_Deck);
    end = AppendNumber(end, sBuilder->deckIndex + 1);
    if (gSaveBlock1Ptr->cardGame.activeDeck == sBuilder->deckIndex)
        end = StringAppend(end, sText_InUse);
    end = StringAppend(end, sText_Space);
    end = StringAppend(end, sText_Space);
    end = AppendNumber(end, CountDeck());
    end = StringAppend(end, sText_Slash);
    end = AppendNumber(end, CARD_DECK_SIZE);
    end = StringAppend(end, sText_Space);
    CardGame_GetDeckZone(sBuilder->deckIndex, zoneTypes);
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        if (zoneTypes[i] == CARD_TYPE_NONE)
            continue;
        end = CardGame_AppendTypeIcon(end, zoneTypes[i]);
        any = TRUE;
    }
    if (!any)
        StringAppend(end, sText_NoEnergy);
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, 4, 0, sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
    CardGame_DrawEnergyIcons(WIN_HEADER, sBuilder->text, 4, 0, 12);
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

// The help, or a message: 4 lines at most once wrapped, closer together than FONT_SMALL's,
// under the list's scroll arrow
static void PrintHelp(const u8 *text)
{
    u8 *line = sBuilder->text;
    u8 *end;
    u8 y = HELP_TOP;
    bool8 last;

    StringCopy(sBuilder->text, text);
    WrapText(sBuilder->text, FONT_SMALL, SIDE_WIDTH * 8 - 8);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(0));
    do
    {
        for (end = line; *end != CHAR_NEWLINE && *end != EOS; end++)
            ;
        last = (*end == EOS);
        *end = EOS;
        AddTextPrinterParameterized3(WIN_HELP, FONT_SMALL, 4, y, sHeaderTextColors, TEXT_SKIP_DRAW, line);
        line = end + 1;
        y += HELP_LINE_HEIGHT;
    } while (!last);
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

// The highlighted card, drawn whole on the right (nothing for CARD_NONE)
static void ShowCardInfo(u16 cardId)
{
    if (sBuilder->infoCard == cardId)
        return;
    sBuilder->infoCard = cardId;
    VintageCard_Draw(cardId);
}

// A card player, where the card is: where they are, their Energy, beaten or not, and their
// prize card (once won)
#define PLAYER_INFO_GAP     4   // pixels left of the panel, between it and the list
#define PLAYER_INFO_X       (PLAYER_INFO_GAP + 4)
#define PLAYER_INFO_WIDTH   (VINTAGE_CARD_WIDTH * 8 - PLAYER_INFO_X - 4)

static void ShowPlayerInfo(u8 opponentId)
{
    const struct CardOpponent *opponent = &gCardOpponents[opponentId];
    const struct CardDeckData *deck = &gCardDecks[opponent->deck];
    bool8 beaten = CardGame_HasBeatenOpponent(opponentId);
    u8 *text = sBuilder->text;
    u8 *end;
    u8 i;

    sBuilder->infoCard = CARD_COUNT;
    VintageCard_Clear();
    FillWindowPixelRect(WIN_CARD, PIXEL_FILL(1), PLAYER_INFO_GAP, 0, VINTAGE_CARD_WIDTH * 8 - PLAYER_INFO_GAP, VINTAGE_CARD_HEIGHT * 8);
    CardGame_GetOpponentName(text, opponentId);
    AddTextPrinterParameterized(WIN_CARD, FONT_NORMAL, text, PLAYER_INFO_X, 2, TEXT_SKIP_DRAW, NULL);
    GetMapName(text, opponent->mapsec, 0);
    TruncateToWidth(text, FONT_SMALL, PLAYER_INFO_WIDTH);
    AddTextPrinterParameterized(WIN_CARD, FONT_SMALL, text, PLAYER_INFO_X, 18, TEXT_SKIP_DRAW, NULL);
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
    WrapText(text, FONT_SMALL, PLAYER_INFO_WIDTH);
    AddTextPrinterParameterized(WIN_CARD, FONT_SMALL, text, PLAYER_INFO_X, 34, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(WIN_CARD, FONT_SMALL, beaten ? sText_Beaten : sText_NotBeaten, PLAYER_INFO_X, 62, TEXT_SKIP_DRAW, NULL);
    if (opponent->rewardCard != CARD_NONE)
    {
        AddTextPrinterParameterized(WIN_CARD, FONT_SMALL, sText_Prize, PLAYER_INFO_X, 78, TEXT_SKIP_DRAW, NULL);
        if (beaten)
        {
            *text = EOS;
            CardGame_AppendCardName(text, opponent->rewardCard);
        }
        else
        {
            StringCopy(text, sText_PrizeUnknown);
        }
        WrapText(text, FONT_SMALL, PLAYER_INFO_WIDTH);
        AddTextPrinterParameterized(WIN_CARD, FONT_SMALL, text, PLAYER_INFO_X, 90, TEXT_SKIP_DRAW, NULL);
    }
    PutWindowTilemap(WIN_CARD);
    CopyWindowToVram(WIN_CARD, COPYWIN_FULL);
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
        sBuilder->arrowTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, SIDE_WIDTH * 4, HEADER_HEIGHT * 8 - 2, (HEADER_HEIGHT + LIST_HEIGHT) * 8 + 4, sBuilder->itemCount - LIST_ROWS,
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
    u8 *end;

    // The header: "CARD 1/5", and how many the player has on the right
    end = StringCopy(sBuilder->text, sText_PackCard);
    end = AppendNumber(end, sBuilder->packIndex + 1);
    end = StringAppend(end, sText_Slash);
    AppendNumber(end, sBuilder->packCount);
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, 4, 0, sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
    end = StringCopy(sBuilder->text, sText_YouHave);
    AppendNumber(end, CardGame_GetOwnedCopies(cardId));
    AddTextPrinterParameterized3(WIN_HEADER, FONT_SMALL, SIDE_WIDTH * 8 - 4 - GetStringWidth(FONT_SMALL, sBuilder->text, 0), 2,
                                 sHeaderTextColors, TEXT_SKIP_DRAW, sBuilder->text);
    PutWindowTilemap(WIN_HEADER);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);

    // Left of the card: its whole text, if it can't show it all
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(0));
    if (!VintageCard_Draw(cardId))
        VintageCard_PrintFullText(WIN_LIST, cardId, 4, 2, SIDE_WIDTH * 8 - 8, LIST_HEIGHT * 8, sHeaderTextColors);
    PutWindowTilemap(WIN_LIST);
    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
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
