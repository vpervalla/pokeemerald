#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The card game's duel screen (phase 3 of docs/card_game_plan.md): a text board, the rules
// engine's events played back as messages, and a menu of the player's legal actions.
// Phase 4 replaces the text with graphics.
//
// Started from a script with `special PlayCardGame` (VAR_0x8004: the opponent's deck,
// CARD_DECK_*), it sets VAR_RESULT to CARD_GAME_RESULT_* and returns to the field.

enum
{
    WIN_OPPONENT,
    WIN_PLAYER,
    WIN_MESSAGE,
    WIN_MENU,
};

#define FRAME_BASE_TILE     0x3E0
#define FRAME_PALETTE       14
#define TEXT_PALETTE        15

#define LABEL_LENGTH        40
#define MENU_VIEW_HAND      (CARD_MAX_ACTIONS + 0)
#define MENU_FORFEIT        (CARD_MAX_ACTIONS + 1)
#define MENU_ITEMS_MAX      (CARD_MAX_ACTIONS + 2)

#define MESSAGE_FRAMES      50  // a message stays this long unless A or B skips it
#define AI_DELAY_FRAMES     20

enum
{
    STATE_FADE_IN,
    STATE_NEXT,
    STATE_MESSAGE,
    STATE_MENU,
    STATE_CONFIRM_FORFEIT,
    STATE_AI_WAIT,
    STATE_RESULT,
    STATE_FADE_OUT,
};

struct CardGameScreen
{
    struct CardDuel duel;
    MainCallback exitCallback;
    u8 opponentDeck;
    u8 state;
    u8 nextEvent;
    u8 listTaskId;
    u8 handPage;            // while showing the hand, the next card to show
    bool8 forfeited;
    bool8 menuShown;
    u16 timer;
    u16 menuScroll;
    u16 menuRow;
    u8 actionCount;
    struct CardAction actions[CARD_MAX_ACTIONS];
    struct ListMenuItem items[MENU_ITEMS_MAX];
    u8 labels[MENU_ITEMS_MAX][LABEL_LENGTH];
    u8 text[160];
};

static EWRAM_DATA struct CardGameScreen *sScreen = NULL;

static void CB2_InitCardGame(void);
static void Task_FadeToCardGame(u8 taskId);
static void Task_CardGame(u8 taskId);
static void DrawBoard(void);
static void ShowMessage(const u8 *text);
static void OpenActionMenu(void);
static void CloseActionMenu(void);

static const u8 sText_You[] = _("YOU");
static const u8 sText_Trophies[] = _("  TROPHIES ");
static const u8 sText_Deck[] = _("  DECK ");
static const u8 sText_Hand[] = _("  HAND ");
static const u8 sText_Energy[] = _("ENERGY ");
static const u8 sText_Next[] = _(" NEXT ");
static const u8 sText_Active[] = _("ACTIVE ");
static const u8 sText_Empty[] = _("-");
static const u8 sText_Slash[] = _("/");
static const u8 sText_Space[] = _(" ");
static const u8 sText_Mega[] = _("MEGA ");
static const u8 sText_Ex[] = _(" ex");
static const u8 sText_Tool[] = _(" +TOOL");
static const u8 sText_Bench1[] = _("1 ");
static const u8 sText_Bench2[] = _("2 ");
static const u8 sText_Bench3[] = _("3 ");
static const u8 *const sBenchLabels[] = {sText_Active, sText_Bench1, sText_Bench2, sText_Bench3};

static const u8 sText_TypeNone[] = _("");
static const u8 sText_TypeG[] = _("G");
static const u8 sText_TypeR[] = _("R");
static const u8 sText_TypeW[] = _("W");
static const u8 sText_TypeL[] = _("L");
static const u8 sText_TypeP[] = _("P");
static const u8 sText_TypeF[] = _("F");
static const u8 sText_TypeD[] = _("D");
static const u8 sText_TypeM[] = _("M");
static const u8 sText_TypeN[] = _("N");
static const u8 sText_TypeC[] = _("C");
static const u8 *const sTypeLetters[CARD_TYPE_COUNT] =
{
    sText_TypeNone, sText_TypeG, sText_TypeR, sText_TypeW, sText_TypeL, sText_TypeP,
    sText_TypeF, sText_TypeD, sText_TypeM, sText_TypeN, sText_TypeC,
};

static const u8 sText_Asleep[] = _(" SLP");
static const u8 sText_Paralyzed[] = _(" PAR");
static const u8 sText_Confused[] = _(" CNF");
static const u8 sText_Poisoned[] = _(" PSN");
static const u8 sText_Burned[] = _(" BRN");

static const u8 sText_StatusAsleep[] = _("ASLEEP");
static const u8 sText_StatusParalyzed[] = _("PARALYZED");
static const u8 sText_StatusConfused[] = _("CONFUSED");
static const u8 sText_StatusPoisoned[] = _("POISONED");
static const u8 sText_StatusBurned[] = _("BURNED");

// Menu labels
static const u8 sText_ToActive[] = _("Active: ");
static const u8 sText_ToBench[] = _("Bench: ");
static const u8 sText_StartDuel[] = _("Done, start the duel");
static const u8 sText_Attach[] = _("Attach ");
static const u8 sText_EnergyTo[] = _(" Energy to ");
static const u8 sText_Evolve[] = _("Evolve into ");
static const u8 sText_On[] = _(" on ");
static const u8 sText_Play[] = _("Play ");
static const u8 sText_RetreatTo[] = _("Retreat to ");
static const u8 sText_Attack[] = _("Attack: ");
static const u8 sText_At[] = _(" at ");
static const u8 sText_Ability[] = _("Ability of ");
static const u8 sText_EndTurn[] = _("End turn");
static const u8 sText_SendOut[] = _("Send out ");
static const u8 sText_ViewHand[] = _("Look at your hand");
static const u8 sText_Forfeit[] = _("Forfeit");

// Messages
static const u8 sText_DuelStart[] = _("A card duel against ");
static const u8 sText_ChooseActive[] = _("!\nChoose your Active and Bench POKéMON.");
static const u8 sText_GoesFirst[] = _(" goes first!");
static const u8 sText_TurnOf[] = _("Turn ");
static const u8 sText_TurnSeparator[] = _(": ");
static const u8 sText_PlayersTurn[] = _("'s turn.");
static const u8 sText_YouDrew[] = _("You drew ");
static const u8 sText_DrewACard[] = _(" drew a card.");
static const u8 sText_Gets[] = _(" gets a ");
static const u8 sText_EnergyDot[] = _(" Energy.");
static const u8 sText_Heads[] = _("Coin flip... HEADS!");
static const u8 sText_Tails[] = _("Coin flip... TAILS!");
static const u8 sText_Played[] = _(" played ");
static const u8 sText_Attached[] = _(" attached ");
static const u8 sText_Evolution[] = _("Evolution: ");
static const u8 sText_RetreatedTo[] = _(" retreated to ");
static const u8 sText_Used[] = _(" used ");
static const u8 sText_ItFailed[] = _("But it failed!");
static const u8 sText_Took[] = _(" took ");
static const u8 sText_Damage[] = _(" damage.");
static const u8 sText_Healed[] = _(" healed ");
static const u8 sText_IsNow[] = _(" is now ");
static const u8 sText_IsNoLonger[] = _(" is no longer ");
static const u8 sText_Discarded[] = _(" discarded ");
static const u8 sText_EnergyCount[] = _(" Energy.");
static const u8 sText_KnockedOut[] = _(" was knocked out!");
static const u8 sText_Earned[] = _(" earned ");
static const u8 sText_Trophy[] = _(" trophy!");
static const u8 sText_TrophiesEarned[] = _(" trophies!");
static const u8 sText_SentOut[] = _(" sent out ");
static const u8 sText_DeckEmpty[] = _("'s deck is empty.");
static const u8 sText_HandFull[] = _("'s hand is full.");
static const u8 sText_YouWin[] = _("You won the duel!");
static const u8 sText_YouLose[] = _("You lost the duel...");
static const u8 sText_ItsADraw[] = _("The duel is a draw.");
static const u8 sText_UsedAbility[] = _(" used its ability!");
static const u8 sText_HandOf[] = _("Hand of ");
static const u8 sText_Colon[] = _(": ");
static const u8 sText_Comma[] = _(", ");
static const u8 sText_Dot[] = _(".");
static const u8 sText_YourHand[] = _("Your hand: ");
static const u8 sText_HandEmpty[] = _("Your hand is empty.");
static const u8 sText_WasSwitchedIn[] = _(" was switched in.");
static const u8 sText_ForfeitConfirm[] = _("Forfeit the duel?\nA: yes  B: no");
static const u8 sText_Exclamation[] = _("!");

static const u16 sBackdropColor = RGB(4, 12, 7);

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
    [WIN_OPPONENT] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 6,
        .paletteNum = TEXT_PALETTE,
        .baseBlock = 1,
    },
    [WIN_PLAYER] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 6,
        .width = 30,
        .height = 6,
        .paletteNum = TEXT_PALETTE,
        .baseBlock = 1 + 30 * 6,
    },
    [WIN_MESSAGE] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 13,
        .width = 28,
        .height = 6,
        .paletteNum = TEXT_PALETTE,
        .baseBlock = 1 + 30 * 6 * 2,
    },
    [WIN_MENU] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 13,
        .width = 28,
        .height = 6,
        .paletteNum = TEXT_PALETTE,
        .baseBlock = 1 + 30 * 6 * 2 + 28 * 6,
    },
    DUMMY_WIN_TEMPLATE,
};

// ---------------------------------------------------------------------------------------------
// Entry and exit

void PlayCardGame(void)
{
    sScreen = AllocZeroed(sizeof(*sScreen));
    sScreen->opponentDeck = gSpecialVar_0x8004 < CARD_DECKS_COUNT ? gSpecialVar_0x8004 : CARD_DECK_BUG_CATCHER;
    sScreen->exitCallback = CB2_ReturnToFieldContinueScriptPlayMapMusic;
    CreateTask(Task_FadeToCardGame, 0);
}

static void Task_FadeToCardGame(u8 taskId)
{
    if (gTasks[taskId].data[0] == 0)
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].data[0]++;
    }
    else if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitCardGame);
        DestroyTask(taskId);
    }
}

static void CB2_CardGame(void)
{
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_CardGame(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void StartDuel(void)
{
    struct CardDuelSetup player, opponent;
    u8 i;

    player.deck = gCardDecks[CARD_DECK_STARTER].cards;
    opponent.deck = gCardDecks[sScreen->opponentDeck].cards;
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
    {
        player.zoneTypes[i] = gCardDecks[CARD_DECK_STARTER].zoneTypes[i];
        opponent.zoneTypes[i] = gCardDecks[sScreen->opponentDeck].zoneTypes[i];
    }
    Duel_Init(&sScreen->duel, &player, &opponent, Random32());
}

static void CB2_InitCardGame(void)
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
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 1:
        LoadPalette(&sBackdropColor, BG_PLTT_ID(0), PLTT_SIZEOF(1));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PALETTE), PLTT_SIZE_4BPP);
        LoadUserWindowBorderGfx(WIN_MESSAGE, FRAME_BASE_TILE, BG_PLTT_ID(FRAME_PALETTE));
        gMain.state++;
        break;
    case 2:
        StartDuel();
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 32, 32);
        DrawBoard();
        DrawStdFrameWithCustomTileAndPalette(WIN_MESSAGE, FALSE, FRAME_BASE_TILE, FRAME_PALETTE);
        StringCopy(sScreen->text, sText_DuelStart);
        StringAppend(sScreen->text, gCardDecks[sScreen->opponentDeck].name);
        StringAppend(sScreen->text, sText_ChooseActive);
        ShowMessage(sScreen->text);
        Duel_ClearEvents(&sScreen->duel); // the opening draws and coin flip are shown on the board
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        SetVBlankCallback(VBlankCB_CardGame);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        CreateTask(Task_CardGame, 0);
        SetMainCallback2(CB2_CardGame);
        break;
    }
}

static void ExitCardGame(void)
{
    const struct CardDuel *duel = &sScreen->duel;

    if (sScreen->forfeited)
        gSpecialVar_Result = CARD_GAME_RESULT_FORFEITED;
    else if (duel->winner == CARD_SIDE_PLAYER)
        gSpecialVar_Result = CARD_GAME_RESULT_WON;
    else if (duel->winner == CARD_RESULT_DRAW)
        gSpecialVar_Result = CARD_GAME_RESULT_DRAW;
    else
        gSpecialVar_Result = CARD_GAME_RESULT_LOST;
    SetMainCallback2(sScreen->exitCallback);
    FreeAllWindowBuffers();
    FREE_AND_SET_NULL(sScreen);
}

// ---------------------------------------------------------------------------------------------
// Text

static u8 *AppendNumber(u8 *dst, u16 number)
{
    u8 digits = number >= 100 ? 3 : (number >= 10 ? 2 : 1);

    return ConvertIntToDecimalStringN(dst, number, STR_CONV_MODE_LEFT_ALIGN, digits);
}

static u8 *AppendCardName(u8 *dst, u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);

    if (pokemon != NULL)
    {
        if (pokemon->isMega)
            dst = StringAppend(dst, sText_Mega);
        dst = StringAppend(dst, gSpeciesNames[pokemon->species]);
        if (pokemon->isEx)
            dst = StringAppend(dst, sText_Ex);
    }
    else if (trainer != NULL)
    {
        dst = StringAppend(dst, trainer->name);
    }
    return dst;
}

static u8 *AppendSideName(u8 *dst, u8 side)
{
    if (side == CARD_SIDE_PLAYER)
        return StringCopy_PlayerName(dst, gSaveBlock2Ptr->playerName);
    return StringAppend(dst, gCardDecks[sScreen->opponentDeck].name);
}

static u8 *AppendStatusName(u8 *dst, u8 status)
{
    if (status & CARD_STATUS_ASLEEP)
        return StringAppend(dst, sText_StatusAsleep);
    if (status & CARD_STATUS_PARALYZED)
        return StringAppend(dst, sText_StatusParalyzed);
    if (status & CARD_STATUS_CONFUSED)
        return StringAppend(dst, sText_StatusConfused);
    if (status & CARD_STATUS_POISONED)
        return StringAppend(dst, sText_StatusPoisoned);
    return StringAppend(dst, sText_StatusBurned);
}

static u8 *AppendEnergyLetters(u8 *dst, const struct CardInPlay *mon)
{
    u8 type, i;

    for (type = CARD_TYPE_GRASS; type < CARD_TYPE_COUNT; type++)
    {
        for (i = 0; i < mon->energy[type]; i++)
            dst = StringAppend(dst, sTypeLetters[type]);
    }
    return dst;
}

// "PIDGEY 40/50 GGC PSN +TOOL"
static void FormatPokemonInPlay(u8 *dst, const struct CardInPlay *mon)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(mon->cardId);
    u16 maxHp;

    *dst = EOS;
    if (info == NULL)
    {
        StringAppend(dst, sText_Empty);
        return;
    }
    maxHp = Duel_GetMaxHp(mon);
    dst = AppendCardName(dst, mon->cardId);
    dst = StringAppend(dst, sText_Space);
    dst = AppendNumber(dst, mon->damage >= maxHp ? 0 : maxHp - mon->damage);
    dst = StringAppend(dst, sText_Slash);
    dst = AppendNumber(dst, maxHp);
    if (Duel_CountEnergy(mon) != 0)
    {
        dst = StringAppend(dst, sText_Space);
        dst = AppendEnergyLetters(dst, mon);
    }
    if (mon->status & CARD_STATUS_ASLEEP)
        dst = StringAppend(dst, sText_Asleep);
    if (mon->status & CARD_STATUS_PARALYZED)
        dst = StringAppend(dst, sText_Paralyzed);
    if (mon->status & CARD_STATUS_CONFUSED)
        dst = StringAppend(dst, sText_Confused);
    if (mon->status & CARD_STATUS_POISONED)
        dst = StringAppend(dst, sText_Poisoned);
    if (mon->status & CARD_STATUS_BURNED)
        dst = StringAppend(dst, sText_Burned);
    if (mon->tool != CARD_NONE)
        dst = StringAppend(dst, sText_Tool);
}

// ---------------------------------------------------------------------------------------------
// Board

// Cuts the text so it fits in width pixels
static void TruncateToWidth(u8 *text, u8 fontId, u16 width)
{
    u16 length = StringLength(text);

    while (length != 0 && GetStringWidth(fontId, text, 0) > width)
        text[--length] = EOS;
}

static void PrintSmall(u8 windowId, u8 *text, u8 x, u8 y, u16 width)
{
    TruncateToWidth(text, FONT_SMALL, width);
    AddTextPrinterParameterized(windowId, FONT_SMALL, text, x, y, TEXT_SKIP_DRAW, NULL);
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

static void DrawSide(u8 sideId)
{
    const struct CardDuelSide *side = &sScreen->duel.sides[sideId];
    u8 windowId = sideId == CARD_SIDE_PLAYER ? WIN_PLAYER : WIN_OPPONENT;
    u8 *text = sScreen->text;
    u8 *end;
    u8 slot;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));

    // Who, trophies, deck, hand
    end = sideId == CARD_SIDE_PLAYER ? StringCopy(text, sText_You) : StringCopy(text, gCardDecks[sScreen->opponentDeck].name);
    end = StringAppend(end, sText_Trophies);
    end = AppendNumber(end, side->trophies);
    end = StringAppend(end, sText_Deck);
    end = AppendNumber(end, side->deckCount);
    end = StringAppend(end, sText_Hand);
    AppendNumber(end, side->handCount);
    PrintSmall(windowId, text, 2, 0, 236);

    // The Active, then the Bench in two columns, and the Energy Zone in the last place
    for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
    {
        u8 x = (slot == 2) ? 122 : 2;
        u8 y = (slot == 0) ? 12 : (slot == 3 ? 36 : 24);

        end = StringCopy(text, sBenchLabels[slot]);
        FormatPokemonInPlay(end, &side->slots[slot]);
        PrintSmall(windowId, text, x, y, slot == 0 ? 236 : 116);
    }
    end = StringCopy(text, sText_Energy);
    end = StringAppend(end, side->zoneEnergy != CARD_TYPE_NONE ? sTypeLetters[side->zoneEnergy] : sText_Empty);
    end = StringAppend(end, sText_Next);
    StringAppend(end, sTypeLetters[side->nextEnergy]);
    PrintSmall(windowId, text, 122, 36, 116);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawBoard(void)
{
    DrawSide(CARD_SIDE_OPPONENT);
    DrawSide(CARD_SIDE_PLAYER);
}

static void ShowMessage(const u8 *text)
{
    if (text != sScreen->text)
        StringCopy(sScreen->text, text);
    WrapText(sScreen->text, FONT_NORMAL, 220);
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_MESSAGE, FONT_NORMAL, sScreen->text, 0, 1, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(WIN_MESSAGE);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------------------------
// Events as messages

// Writes the message for an event, or returns FALSE if it has none
static bool8 FormatEvent(u8 *dst, const struct CardDuelEvent *event)
{
    const struct CardDuel *duel = &sScreen->duel;
    const struct PokemonCardInfo *info;

    *dst = EOS;
    switch (event->type)
    {
    case CARD_EVENT_FIRST_SIDE:
        dst = AppendSideName(dst, event->side);
        StringAppend(dst, sText_GoesFirst);
        return TRUE;
    case CARD_EVENT_TURN_START:
        dst = StringAppend(dst, sText_TurnOf);
        dst = AppendNumber(dst, event->value);
        dst = StringAppend(dst, sText_TurnSeparator);
        dst = AppendSideName(dst, event->side);
        StringAppend(dst, sText_PlayersTurn);
        return TRUE;
    case CARD_EVENT_DRAW:
        if (duel->phase == CARD_PHASE_SETUP)
            return FALSE;
        if (event->side == CARD_SIDE_PLAYER)
        {
            dst = StringAppend(dst, sText_YouDrew);
            dst = AppendCardName(dst, event->value);
            StringAppend(dst, sText_Dot);
        }
        else
        {
            dst = AppendSideName(dst, event->side);
            StringAppend(dst, sText_DrewACard);
        }
        return TRUE;
    case CARD_EVENT_ENERGY_ZONE:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Gets);
        dst = StringAppend(dst, sTypeLetters[event->value]);
        StringAppend(dst, sText_EnergyDot);
        return TRUE;
    case CARD_EVENT_COIN:
        StringAppend(dst, event->value ? sText_Heads : sText_Tails);
        return TRUE;
    case CARD_EVENT_PLAY_POKEMON:
    case CARD_EVENT_PLAY_TRAINER:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Played);
        dst = AppendCardName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_ATTACH_ENERGY:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Attached);
        dst = StringAppend(dst, sTypeLetters[event->value]);
        dst = StringAppend(dst, sText_EnergyTo);
        dst = AppendCardName(dst, event->cardId);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_EVOLVE:
        dst = StringAppend(dst, sText_Evolution);
        dst = AppendCardName(dst, event->value);
        StringAppend(dst, sText_Exclamation);
        return TRUE;
    case CARD_EVENT_RETREAT:
    case CARD_EVENT_PROMOTE:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, event->type == CARD_EVENT_RETREAT ? sText_RetreatedTo : sText_SentOut);
        dst = AppendCardName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_ATTACK:
        info = CardGame_GetPokemonInfo(event->cardId);
        dst = AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, sText_Used);
        dst = StringAppend(dst, info->attacks[event->value].name);
        StringAppend(dst, sText_Exclamation);
        return TRUE;
    case CARD_EVENT_ATTACK_FAILED:
        StringAppend(dst, sText_ItFailed);
        return TRUE;
    case CARD_EVENT_DAMAGE:
    case CARD_EVENT_HEAL:
        dst = AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, event->type == CARD_EVENT_DAMAGE ? sText_Took : sText_Healed);
        dst = AppendNumber(dst, event->value);
        StringAppend(dst, event->type == CARD_EVENT_DAMAGE ? sText_Damage : sText_Dot);
        return TRUE;
    case CARD_EVENT_STATUS:
    case CARD_EVENT_STATUS_CURED:
        dst = AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, event->type == CARD_EVENT_STATUS ? sText_IsNow : sText_IsNoLonger);
        dst = AppendStatusName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_DISCARD_ENERGY:
        dst = AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, sText_Discarded);
        dst = AppendNumber(dst, event->value);
        StringAppend(dst, sText_EnergyCount);
        return TRUE;
    case CARD_EVENT_KNOCK_OUT:
        dst = AppendCardName(dst, event->cardId);
        StringAppend(dst, sText_KnockedOut);
        return TRUE;
    case CARD_EVENT_TROPHY:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Earned);
        dst = AppendNumber(dst, event->value);
        StringAppend(dst, event->value == 1 ? sText_Trophy : sText_TrophiesEarned);
        return TRUE;
    case CARD_EVENT_DECK_EMPTY:
    case CARD_EVENT_HAND_FULL:
        dst = AppendSideName(dst, event->side);
        StringAppend(dst, event->type == CARD_EVENT_DECK_EMPTY ? sText_DeckEmpty : sText_HandFull);
        return TRUE;
    case CARD_EVENT_GAME_OVER:
        return FALSE; // shown by STATE_RESULT
    case CARD_EVENT_ABILITY:
        dst = AppendCardName(dst, event->cardId);
        StringAppend(dst, sText_UsedAbility);
        return TRUE;
    case CARD_EVENT_REVEAL_HAND:
        {
            const struct CardDuelSide *side = &duel->sides[event->side];
            u8 i;

            dst = StringAppend(dst, sText_HandOf);
            dst = AppendSideName(dst, event->side);
            dst = StringAppend(dst, sText_Colon);
            for (i = 0; i < side->handCount && i < 4; i++)
            {
                if (i == 2)
                {
                    *dst++ = CHAR_NEWLINE;
                    *dst = EOS;
                }
                else if (i != 0)
                    dst = StringAppend(dst, sText_Comma);
                dst = AppendCardName(dst, side->hand[i]);
            }
            *dst = EOS;
        }
        return TRUE;
    case CARD_EVENT_SWITCH:
        dst = AppendCardName(dst, event->value);
        StringAppend(dst, sText_WasSwitchedIn);
        return TRUE;
    }
    return FALSE;
}

// Shows the next event's message; FALSE when there are none left
static bool8 ShowNextEvent(void)
{
    struct CardDuel *duel = &sScreen->duel;

    while (sScreen->nextEvent < duel->eventCount)
    {
        if (FormatEvent(sScreen->text, &duel->events[sScreen->nextEvent++]))
        {
            ShowMessage(sScreen->text);
            return TRUE;
        }
    }
    Duel_ClearEvents(duel);
    sScreen->nextEvent = 0;
    return FALSE;
}

// The player's hand, a few cards per message
static bool8 ShowHandPage(void)
{
    const struct CardDuelSide *side = &sScreen->duel.sides[CARD_SIDE_PLAYER];
    u8 *dst = sScreen->text;
    u8 i;

    if (side->handCount == 0 && sScreen->handPage == 0)
    {
        StringCopy(dst, sText_HandEmpty);
        sScreen->handPage = 1;
        ShowMessage(sScreen->text);
        return TRUE;
    }
    if (sScreen->handPage >= side->handCount)
        return FALSE;
    dst = StringCopy(dst, sText_YourHand);
    for (i = 0; i < 4 && sScreen->handPage < side->handCount; i++, sScreen->handPage++)
    {
        if (i == 2)
        {
            *dst++ = CHAR_NEWLINE;
            *dst = EOS;
        }
        else if (i != 0)
            dst = StringAppend(dst, sText_Comma);
        dst = AppendCardName(dst, side->hand[sScreen->handPage]);
    }
    *dst = EOS;
    ShowMessage(sScreen->text);
    return TRUE;
}

// ---------------------------------------------------------------------------------------------
// Action menu

static void FormatAction(u8 *dst, const struct CardAction *action)
{
    const struct CardDuel *duel = &sScreen->duel;
    const struct CardDuelSide *side = &duel->sides[CARD_SIDE_PLAYER];
    const struct CardDuelSide *opponent = &duel->sides[CARD_SIDE_OPPONENT];
    const struct PokemonCardInfo *info;
    u16 handCard = action->handIndex < side->handCount ? side->hand[action->handIndex] : CARD_NONE;

    *dst = EOS;
    switch (action->type)
    {
    case CARD_ACTION_PLAY_BASIC:
        dst = StringAppend(dst, action->slot == CARD_SLOT_ACTIVE ? sText_ToActive : sText_ToBench);
        AppendCardName(dst, handCard);
        break;
    case CARD_ACTION_END_SETUP:
        StringAppend(dst, sText_StartDuel);
        break;
    case CARD_ACTION_ATTACH_ENERGY:
        dst = StringAppend(dst, sText_Attach);
        dst = StringAppend(dst, sTypeLetters[side->zoneEnergy]);
        dst = StringAppend(dst, sText_EnergyTo);
        AppendCardName(dst, side->slots[action->slot].cardId);
        break;
    case CARD_ACTION_EVOLVE:
        dst = StringAppend(dst, sText_Evolve);
        dst = AppendCardName(dst, handCard);
        if (action->slot != CARD_SLOT_ACTIVE)
        {
            dst = StringAppend(dst, sText_At);
            StringAppend(dst, sBenchLabels[action->slot]);
        }
        break;
    case CARD_ACTION_PLAY_TRAINER:
        dst = StringAppend(dst, sText_Play);
        dst = AppendCardName(dst, handCard);
        if (action->slot < CARD_SLOT_COUNT)
        {
            dst = StringAppend(dst, sText_On);
            if (CardGame_GetTrainerInfo(handCard)->effect == CARD_TRAINER_RARE_CANDY)
            {
                dst = AppendCardName(dst, side->hand[action->arg]);
                dst = StringAppend(dst, sText_Space);
            }
            AppendCardName(dst, side->slots[action->slot].cardId);
        }
        break;
    case CARD_ACTION_RETREAT:
        dst = StringAppend(dst, sText_RetreatTo);
        AppendCardName(dst, side->slots[action->slot].cardId);
        break;
    case CARD_ACTION_ATTACK:
        info = CardGame_GetPokemonInfo(side->slots[CARD_SLOT_ACTIVE].cardId);
        dst = StringAppend(dst, sText_Attack);
        dst = StringAppend(dst, info->attacks[action->arg].name);
        dst = StringAppend(dst, sText_Space);
        dst = AppendNumber(dst, info->attacks[action->arg].damage);
        if (info->attacks[action->arg].effect == CARD_EFFECT_SNIPE)
        {
            dst = StringAppend(dst, sText_At);
            AppendCardName(dst, opponent->slots[action->slot].cardId);
        }
        break;
    case CARD_ACTION_USE_ABILITY:
        dst = StringAppend(dst, sText_Ability);
        dst = AppendCardName(dst, side->slots[action->slot].cardId);
        info = CardGame_GetPokemonInfo(side->slots[action->slot].cardId);
        if (info->ability == CARD_ABILITY_SNIPE)
        {
            dst = StringAppend(dst, sText_At);
            AppendCardName(dst, opponent->slots[action->arg].cardId);
        }
        break;
    case CARD_ACTION_END_TURN:
        StringAppend(dst, sText_EndTurn);
        break;
    case CARD_ACTION_PROMOTE:
        dst = StringAppend(dst, sText_SendOut);
        AppendCardName(dst, side->slots[action->slot].cardId);
        break;
    }
}

static void OpenActionMenu(void)
{
    struct ListMenuTemplate template;
    u8 i, count;

    count = Duel_GetLegalActions(&sScreen->duel, CARD_SIDE_PLAYER, sScreen->actions);
    sScreen->actionCount = count;
    for (i = 0; i < count; i++)
    {
        FormatAction(sScreen->labels[i], &sScreen->actions[i]);
        TruncateToWidth(sScreen->labels[i], FONT_NORMAL, 212);
        sScreen->items[i].name = sScreen->labels[i];
        sScreen->items[i].id = i;
    }
    StringCopy(sScreen->labels[count], sText_ViewHand);
    sScreen->items[count].name = sScreen->labels[count];
    sScreen->items[count].id = MENU_VIEW_HAND;
    count++;
    if (sScreen->duel.phase != CARD_PHASE_SETUP)
    {
        StringCopy(sScreen->labels[count], sText_Forfeit);
        sScreen->items[count].name = sScreen->labels[count];
        sScreen->items[count].id = MENU_FORFEIT;
        count++;
    }

    template.items = sScreen->items;
    template.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    template.itemPrintFunc = NULL;
    template.totalItems = count;
    template.maxShowed = 3;
    template.windowId = WIN_MENU;
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

    // The actions change every time, so the cursor starts at the top
    sScreen->menuScroll = sScreen->menuRow = 0;
    ClearWindowTilemap(WIN_MESSAGE);
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    PutWindowTilemap(WIN_MENU);
    sScreen->listTaskId = ListMenuInit(&template, sScreen->menuScroll, sScreen->menuRow);
    CopyWindowToVram(WIN_MENU, COPYWIN_FULL);
    sScreen->menuShown = TRUE;
}

static void CloseActionMenu(void)
{
    if (!sScreen->menuShown)
        return;
    DestroyListMenuTask(sScreen->listTaskId, &sScreen->menuScroll, &sScreen->menuRow);
    ClearWindowTilemap(WIN_MENU);
    sScreen->menuShown = FALSE;
    // An empty message box until the next message
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
    PutWindowTilemap(WIN_MESSAGE);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------------------------
// The duel's flow

static bool8 PressedAorB(void)
{
    return JOY_NEW(A_BUTTON | B_BUTTON) != 0;
}

static void ShowResult(void)
{
    const struct CardDuel *duel = &sScreen->duel;

    if (sScreen->forfeited || duel->winner == CARD_SIDE_OPPONENT)
        ShowMessage(sText_YouLose);
    else if (duel->winner == CARD_SIDE_PLAYER)
        ShowMessage(sText_YouWin);
    else
        ShowMessage(sText_ItsADraw);
}

static void Task_CardGame(u8 taskId)
{
    struct CardDuel *duel = &sScreen->duel;
    s32 input;

    switch (sScreen->state)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
        {
            sScreen->timer = 0;
            sScreen->state = STATE_MESSAGE;
        }
        break;
    case STATE_NEXT:
        DrawBoard();
        if (ShowNextEvent())
        {
            sScreen->timer = 0;
            sScreen->state = STATE_MESSAGE;
        }
        else if (duel->phase == CARD_PHASE_OVER)
        {
            ShowResult();
            sScreen->state = STATE_RESULT;
        }
        else if (Duel_CanAct(duel, CARD_SIDE_PLAYER))
        {
            OpenActionMenu();
            sScreen->state = STATE_MENU;
        }
        else
        {
            sScreen->timer = 0;
            sScreen->state = STATE_AI_WAIT;
        }
        break;
    case STATE_MESSAGE:
        if (++sScreen->timer >= MESSAGE_FRAMES || PressedAorB())
        {
            if (sScreen->handPage != 0 && ShowHandPage())
            {
                sScreen->timer = 0;
                break;
            }
            sScreen->handPage = 0;
            sScreen->state = STATE_NEXT;
        }
        break;
    case STATE_MENU:
        input = ListMenu_ProcessInput(sScreen->listTaskId);
        if (input == LIST_NOTHING_CHOSEN || input == LIST_CANCEL)
            break;
        PlaySE(SE_SELECT);
        CloseActionMenu();
        if (input == MENU_VIEW_HAND)
        {
            sScreen->handPage = 0;
            ShowHandPage();
            sScreen->timer = 0;
            sScreen->state = STATE_MESSAGE;
        }
        else if (input == MENU_FORFEIT)
        {
            ShowMessage(sText_ForfeitConfirm);
            sScreen->state = STATE_CONFIRM_FORFEIT;
        }
        else
        {
            Duel_DoAction(duel, CARD_SIDE_PLAYER, &sScreen->actions[input]);
            sScreen->state = STATE_NEXT;
        }
        break;
    case STATE_CONFIRM_FORFEIT:
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            sScreen->forfeited = TRUE;
            Duel_Forfeit(duel, CARD_SIDE_PLAYER);
            Duel_ClearEvents(duel);
            sScreen->state = STATE_NEXT;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sScreen->state = STATE_NEXT;
        }
        break;
    case STATE_AI_WAIT:
        if (++sScreen->timer >= AI_DELAY_FRAMES)
        {
            struct CardAction action;

            CardAi_ChooseAction(duel, CARD_SIDE_OPPONENT, sScreen->actions, &action);
            Duel_DoAction(duel, CARD_SIDE_OPPONENT, &action);
            sScreen->state = STATE_NEXT;
        }
        break;
    case STATE_RESULT:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sScreen->state = STATE_FADE_OUT;
        }
        break;
    case STATE_FADE_OUT:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            ExitCardGame();
        }
        break;
    }
}
