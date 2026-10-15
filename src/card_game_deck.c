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
    u8 text[220];
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

static const u8 sHelp_Collection[] = _("A: add to the deck  {RIGHT_ARROW}: the deck  L/R: deck 1-3\nSELECT: Energy  START: duel with this deck  B: done");
static const u8 sHelp_Deck[] = _("A: take out  {LEFT_ARROW}: your cards  L/R: deck 1-3\nSELECT: Energy  START: duel with this deck  B: done");
static const u8 sHelp_Energy[] = _("A: on/off (3 at most)\nB: back");
static const u8 sHelp_Pack[] = _("A: next card");

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
    [WIN_PACK]   = {.bg = 0, .tilemapLeft = 12, .tilemapTop = 2, .width = 17, .height = 14, .paletteNum = TEXT_PALETTE, .baseBlock = 559},
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
        }
        DrawHelp();
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
        FreeAndDestroyMonPicSprite(sBuilder->infoSprite);
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
    static const u8 *const sHelps[] = {sHelp_Collection, sHelp_Deck, sHelp_Energy, sHelp_Pack};

    sBuilder->showingMessage = FALSE;
    PrintHelp(sHelps[sBuilder->mode]);
}

static void ShowHelpMessage(const u8 *text)
{
    PlaySE(SE_FAILURE);
    sBuilder->showingMessage = TRUE;
    PrintHelp(text);
}

// The highlighted card: its icon with the stage and ability beside it, then its name, type
// and HP, and its attacks (Trainers: what they do). The pack screen uses a wider window, with
// the picture instead of the icon.
static void ShowCardInfo(u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);
    bool8 isPack = sBuilder->mode == MODE_PACK;
    u8 windowId = isPack ? WIN_PACK : WIN_INFO;
    u8 width = isPack ? 130 : 84;
    u8 *text = sBuilder->text;
    u8 *end;
    u8 i, y;

    if (sBuilder->infoCard == cardId && !isPack)
        return;
    sBuilder->infoCard = cardId;
    DestroyInfoSprite();
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (cardId != CARD_NONE)
    {
        *text = EOS;
        CardGame_AppendCardName(text, cardId);
        TruncateToWidth(text, isPack ? FONT_NORMAL : FONT_SMALL, width);
        AddTextPrinterParameterized(windowId, isPack ? FONT_NORMAL : FONT_SMALL, text, 2, isPack ? 2 : 34, TEXT_SKIP_DRAW, NULL);
        y = isPack ? 20 : 46;
        if (pokemon != NULL)
        {
            // Beside the icon (or under the name on the pack screen): stage and ability
            StringCopy(text, pokemon->stage == CARD_STAGE_BASIC ? sText_Basic : (pokemon->stage == CARD_STAGE_1 ? sText_Stage1 : sText_Stage2));
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, isPack ? 2 : 38, isPack ? y : 4, TEXT_SKIP_DRAW, NULL);
            if (pokemon->ability != CARD_ABILITY_NONE)
                AddTextPrinterParameterized(windowId, FONT_SMALL, sText_AbilityLabel, isPack ? 60 : 38, isPack ? y : 16, TEXT_SKIP_DRAW, NULL);
            if (isPack)
                y += 12;
            end = StringCopy(text, gCardTypeNames[pokemon->type]);
            end = StringAppend(end, sText_Hp);
            AppendNumber(end, pokemon->hp);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
            y += 12;
            for (i = 0; i < pokemon->numAttacks; i++)
            {
                // On the pack screen with its effect; in the deck builder, cost, name and damage
                struct CardAttack attack = pokemon->attacks[i];

                if (!isPack)
                    attack.effect = CARD_EFFECT_NONE;
                *text = EOS;
                CardGame_AppendAttackText(text, &attack);
                WrapText(text, FONT_SMALL, width);
                AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
                for (end = text, y += 12; *end != EOS; end++)
                {
                    if (*end == CHAR_NEWLINE)
                        y += 12;
                }
            }
            if (!isPack)
            {
                sBuilder->infoSprite = CreateMonIcon(pokemon->isMega ? GetMegaBaseSpecies(pokemon->species) : pokemon->species,
                                                     SpriteCB_MonIcon, 18 * 8 + 18, 2 * 8 + 16, 0, 0, FALSE);
                gSprites[sBuilder->infoSprite].oam.priority = 0; // over the window
                sBuilder->infoSpriteKind = 1;
            }
        }
        else if (trainer != NULL)
        {
            *text = EOS;
            CardGame_AppendTrainerKind(text, trainer);
            WrapText(text, FONT_SMALL, isPack ? width : 48);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, isPack ? 2 : 38, isPack ? y : 4, TEXT_SKIP_DRAW, NULL);
            if (isPack)
                y += 24;
            *text = EOS;
            CardGame_AppendTrainerText(text, trainer);
            WrapText(text, FONT_SMALL, width);
            AddTextPrinterParameterized(windowId, FONT_SMALL, text, 2, y, TEXT_SKIP_DRAW, NULL);
            if (trainer->iconItem != ITEM_NONE)
            {
                sBuilder->infoSprite = AddItemIconSprite(TAG_INFO_ITEM, TAG_INFO_ITEM, trainer->iconItem);
                if (sBuilder->infoSprite < MAX_SPRITES)
                {
                    gSprites[sBuilder->infoSprite].x = isPack ? 52 : 18 * 8 + 18;
                    gSprites[sBuilder->infoSprite].y = isPack ? 72 : 2 * 8 + 16;
                    gSprites[sBuilder->infoSprite].oam.priority = 0;
                    sBuilder->infoSpriteKind = 2;
                }
            }
        }
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
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
    }
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, 104, y, TEXT_SKIP_DRAW, NULL);
}

static void MoveListCursor(s32 itemId, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
    if (sBuilder->mode != MODE_ENERGY && itemId >= 0)
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
        AddTextPrinterParameterized(WIN_LIST, FONT_NORMAL, sBuilder->mode == MODE_DECK ? sText_EmptyDeck : sText_NoCards, 8, 1, TEXT_SKIP_DRAW, NULL);
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
}

// ---------------------------------------------------------------------------------------------
// The pack

static void ShowPackCard(void)
{
    u16 cardId = sBuilder->packCards[sBuilder->packIndex];
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    u8 *end;

    ClearWindowTilemap(WIN_LIST);
    ShowCardInfo(cardId);
    // A big picture for a Pokémon, instead of the icon
    if (pokemon != NULL)
    {
        DestroyInfoSprite();
        sBuilder->infoSprite = CreateMonPicSprite_HandleDeoxys(pokemon->species, 0, NOT_SHINY_PERSONALITY, TRUE, 52, 72, 15, TAG_NONE);
        if (sBuilder->infoSprite < MAX_SPRITES)
            sBuilder->infoSpriteKind = 3;
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
    else if (JOY_NEW(DPAD_LEFT) && sBuilder->mode == MODE_DECK)
    {
        PlaySE(SE_SELECT);
        SetMode(MODE_COLLECTION);
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
