#include "global.h"
#include "card_game.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_object_movement.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "gpu_regs.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "window.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The card game's duel screen (docs/card_game_plan.md): the table with the Pokémon in play, the
// rules engine's events played back one at a time (animated, with a message), a menu of the
// player's legal actions, and a viewer for the cards in hand and in play.
//
// Started from a script with `special PlayCardGame` (VAR_0x8004: the card player,
// CARD_OPPONENT_*), it sets VAR_RESULT to CARD_GAME_RESULT_* and returns to the field.
//
// The board is drawn from sScreen->display, a copy of the duel that follows the events as they
// are shown (the engine has already played the whole action), and catches up with the real
// duel once they are all shown.

// Screen layout, in tiles: the opponent's half on top, the player's below, the messages at the
// bottom. Each half has an info panel and the Bench on its outer row, and the Energy Zone, the
// Active Pokémon and its details on its inner row.
#define ROW_OPPONENT_OUTER  0
#define ROW_OPPONENT_INNER  4
#define ROW_PLAYER_INNER    8
#define ROW_PLAYER_OUTER    12
#define ROW_MESSAGE         16
#define SLOT_TILE_X         12  // the Active's slot, and the first Bench slot
#define SLOT_TILE_WIDTH     6
#define SLOT_TILE_HEIGHT    4

enum
{
    WIN_OPPONENT_INFO,
    WIN_OPPONENT_BENCH,
    WIN_OPPONENT_ZONE,
    WIN_OPPONENT_ACTIVE,
    WIN_PLAYER_INFO,
    WIN_PLAYER_BENCH,
    WIN_PLAYER_ZONE,
    WIN_PLAYER_ACTIVE,
    WIN_MESSAGE,
    WIN_VIEW_INFO,  // the card viewer: where the card is, and its state in play
    WIN_VIEW_CARD,  // the card's text (src/card_game_vintage.c)
    WIN_VIEW_FRAME, // the card's frame, on the board's BG
};

// Per side: info, Bench, Energy Zone, Active
static const u8 sSideWindows[CARD_SIDE_COUNT][4] =
{
    [CARD_SIDE_PLAYER] = {WIN_PLAYER_INFO, WIN_PLAYER_BENCH, WIN_PLAYER_ZONE, WIN_PLAYER_ACTIVE},
    [CARD_SIDE_OPPONENT] = {WIN_OPPONENT_INFO, WIN_OPPONENT_BENCH, WIN_OPPONENT_ZONE, WIN_OPPONENT_ACTIVE},
};
#define SIDE_WIN_INFO   0
#define SIDE_WIN_BENCH  1
#define SIDE_WIN_ZONE   2
#define SIDE_WIN_ACTIVE 3

#define TEXT_PALETTE    15
#define FRAME_PALETTE   14
#define BOARD_PALETTE   1

// Board tiles (graphics/card_game/board.png, made by tools/card_game/gen_graphics.py): per side
// a table tile, then the 9 tiles of a slot frame
#define BOARD_TILE_OPPONENT 0
#define BOARD_TILE_PLAYER   10
#define BOARD_TILE_COUNT    20

// The card viewer: the card on the right, where it is and its state on the left
#define VIEW_INFO_WIDTH     (DISPLAY_TILE_WIDTH - VINTAGE_CARD_WIDTH)
#define VIEW_HELP_Y         (DISPLAY_HEIGHT - 14)

#define TAG_PIPS            0x4C00
#define TAG_COIN            0x4C01
#define TAG_ARROWS          0x4C02
#define ENERGY_ICON_SIZE    9
#define ENERGY_ICON_SPACE   (ENERGY_ICON_SIZE + 1)
#define ENERGY_ICON_Y       2   // below the top of the line
#define TAG_CARD_PIC        0x4C05  // a Gold picture, here or in the deck builder (never both)

#define PIP_TROPHY          11  // pips.png: then the card types, a trophy won and one to win
#define PIP_TROPHY_EMPTY    12
#define MAX_PIPS            40
#define MAX_ACTIVE_PIPS     10

#define LABEL_LENGTH        64  // before truncating to the menu's width
#define MENU_VIEW_CARDS     (CARD_MAX_ACTIONS + 0)
#define MENU_FORFEIT        (CARD_MAX_ACTIONS + 1)
#define MENU_ITEMS_MAX      (CARD_MAX_ACTIONS + 2)

#define MESSAGE_FRAMES      45  // a message stays this long unless A or B skips it
#define AI_DELAY_FRAMES     20
#define COIN_SPIN_FRAMES    30

// With the Trainer ID 0, a personality whose halves don't cancel out: not shiny
#define NOT_SHINY_PERSONALITY 0x0000FFFF

// The cards the viewer shows: the hand, then the Pokémon in play
#define VIEW_MAX            (CARD_HAND_MAX + CARD_SLOT_COUNT * CARD_SIDE_COUNT)

enum
{
    STATE_FADE_IN,
    STATE_NEXT,
    STATE_ANIMATE,
    STATE_MESSAGE,
    STATE_MENU,
    STATE_CONFIRM_FORFEIT,
    STATE_VIEW_CARDS,
    STATE_AI_WAIT,
    STATE_RESULT,
    STATE_FADE_OUT,
};

enum
{
    ANIM_NONE,
    ANIM_LUNGE,     // the attacker moves towards the opponent
    ANIM_SHAKE,     // a damaged Pokémon
    ANIM_BLINK,     // a knocked out Pokémon, or one that just came in
    ANIM_COIN,
};

struct CardView
{
    u16 cardId;
    u8 side;
    u8 slot;    // CARD_SLOT_NONE: in the hand
};

struct CardGameScreen
{
    struct CardDuel duel;
    struct CardDuel display;            // what the board shows, see above
    MainCallback exitCallback;
    u8 opponentDeck;
    u8 opponentName[24];
    u8 state;
    u8 nextEvent;
    u8 listTaskId;
    u8 arrowTaskId;
    bool8 forfeited;
    bool8 menuShown;
    bool8 eventApplied;                 // the current event has been applied to the display
    u16 timer;
    u16 menuScroll;
    u16 menuRow;
    // Sprites
    u8 iconSprites[CARD_SIDE_COUNT][CARD_SLOT_COUNT];
    u16 iconCards[CARD_SIDE_COUNT][CARD_SLOT_COUNT];
    u8 pipSprites[MAX_PIPS];
    u8 pipCount;
    u8 coinSprite;
    // The current animation
    u8 anim;
    u8 animSide;
    u8 animSlot;
    u8 animFrame;
    u8 coinHeads;
    // Card viewer
    u8 viewCount;
    u8 viewIndex;
    struct CardView views[VIEW_MAX];
    // Action menu
    u8 actionCount;
    struct CardAction actions[CARD_MAX_ACTIONS];
    struct ListMenuItem items[MENU_ITEMS_MAX];
    u8 labels[MENU_ITEMS_MAX][LABEL_LENGTH];
    u8 text[200];
};

static EWRAM_DATA struct CardGameScreen *sScreen = NULL;

static void CB2_InitCardGame(void);
static void Task_FadeToCardGame(u8 taskId);
static void Task_CardGame(u8 taskId);
static void RefreshBoard(void);
static void ShowMessage(const u8 *text);
static void OpenActionMenu(void);
static void CloseActionMenu(void);
static void OpenCardViewer(void);
static void ShowViewedCard(void);
static void CloseCardViewer(void);

static const u32 sBoardTiles[] = INCGFX_U32("graphics/card_game/board.png", ".4bpp");
static const u16 sBoardPalette[] = INCGFX_U16("graphics/card_game/board.png", ".gbapal");
static const u32 sPipTiles[] = INCGFX_U32("graphics/card_game/pips.png", ".4bpp");
static const u16 sPipPalette[] = INCGFX_U16("graphics/card_game/pips.png", ".gbapal");
static const u32 sCoinTiles[] = INCGFX_U32("graphics/card_game/coin.png", ".4bpp");
static const u16 sCoinPalette[] = INCGFX_U16("graphics/card_game/coin.png", ".gbapal");
static const u8 sEnergyIconGfx[] = INCGFX_U8("graphics/card_game/energy_icons.png", ".4bpp");
static const u16 sEnergyIconPalette[] = INCGFX_U16("graphics/card_game/energy_icons.png", ".gbapal");

// The cards' pictures: the Pokémon Gold front sprites (made by tools/card_game/gen_retro_pics.py)
struct RetroPic
{
    const u32 *gfx;
    const u16 *palette;
};

#include "data/card_game/retro_pics.h"

static const struct OamData sOamData_CardPic =
{
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
};

static const struct SpriteTemplate sSpriteTemplate_CardPic =
{
    .tileTag = TAG_CARD_PIC,
    .paletteTag = TAG_CARD_PIC,
    .oam = &sOamData_CardPic,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const u16 sBackdropColor = RGB(4, 10, 6);

// White text with a dark shadow, over the table
static const u8 sBoardTextColors[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

// ---------------------------------------------------------------------------------------------
// Texts

static const u8 sText_You[] = _("YOU");
static const u8 sText_Deck[] = _("DECK ");
static const u8 sText_Hand[] = _("  HAND ");
static const u8 sText_EnergyLabel[] = _("ENERGY");
static const u8 sText_NextLabel[] = _("NEXT");
static const u8 sText_Hp[] = _("HP ");
static const u8 sText_E[] = _("E");
static const u8 sText_Plus[] = _("+");
static const u8 sText_Slash[] = _("/");
static const u8 sText_Space[] = _(" ");
static const u8 sText_Mega[] = _("MEGA ");
static const u8 sText_Ex[] = _(" ex");
static const u8 sText_MegaX[] = _(" X");
static const u8 sText_MegaY[] = _(" Y");
static const u8 sText_Bench1[] = _("1");
static const u8 sText_Bench2[] = _("2");
static const u8 sText_Bench3[] = _("3");
static const u8 sText_ActiveSpot[] = _("ACTIVE");
static const u8 *const sSlotNames[] = {sText_ActiveSpot, sText_Bench1, sText_Bench2, sText_Bench3};

static const u8 sText_TypeNameNone[] = _("NONE");
static const u8 sText_TypeNameG[] = _("GRASS");
static const u8 sText_TypeNameR[] = _("FIRE");
static const u8 sText_TypeNameW[] = _("WATER");
static const u8 sText_TypeNameL[] = _("LIGHTNING");
static const u8 sText_TypeNameP[] = _("PSYCHIC");
static const u8 sText_TypeNameF[] = _("FIGHTING");
static const u8 sText_TypeNameD[] = _("DARKNESS");
static const u8 sText_TypeNameM[] = _("METAL");
static const u8 sText_TypeNameN[] = _("DRAGON");
static const u8 sText_TypeNameC[] = _("COLORLESS");
const u8 *const gCardTypeNames[CARD_TYPE_COUNT] =
{
    sText_TypeNameNone, sText_TypeNameG, sText_TypeNameR, sText_TypeNameW, sText_TypeNameL, sText_TypeNameP,
    sText_TypeNameF, sText_TypeNameD, sText_TypeNameM, sText_TypeNameN, sText_TypeNameC,
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
static const u8 sText_Evolve[] = _("Evolve ");
static const u8 sText_Into[] = _(" into ");
static const u8 sText_RareCandyOn[] = _(" on ");
static const u8 sText_On[] = _(" on ");
static const u8 sText_Play[] = _("Play ");
static const u8 sText_RetreatTo[] = _("Retreat to ");
static const u8 sText_Attack[] = _("Attack: ");
static const u8 sText_At[] = _(" at ");
static const u8 sText_Ability[] = _("Ability of ");
static const u8 sText_EndTurn[] = _("End turn");
static const u8 sText_SendOut[] = _("Send out ");
static const u8 sText_ViewCards[] = _("Look at the cards");
static const u8 sText_Forfeit[] = _("Forfeit");

// Messages
static const u8 sText_DuelStart[] = _("A card duel against ");
static const u8 sText_ChooseActive[] = _("! Choose your Active and Bench POKéMON.");
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
static const u8 sText_UsedAbility[] = _(" used ");
static const u8 sText_HandOf[] = _("Hand of ");
static const u8 sText_Colon[] = _(": ");
static const u8 sText_Comma[] = _(", ");
static const u8 sText_Dot[] = _(".");
static const u8 sText_WasSwitchedIn[] = _(" was switched in.");
static const u8 sText_ForfeitConfirm[] = _("Forfeit the duel?\nA: yes  B: no");
static const u8 sText_Exclamation[] = _("!");

// Card viewer
static const u8 sText_YourHand[] = _("YOUR HAND ");
static const u8 sText_YourPokemon[] = _("YOUR ");
static const u8 sText_TheirPokemon[] = _("OPPONENT'S ");
static const u8 sText_BenchSpot[] = _("BENCH ");
static const u8 sText_ViewerHelp[] = _("{LEFT_ARROW}{RIGHT_ARROW}: other cards  B: back");
static const u8 sText_ExRule[] = _("ex: knocked out, it gives 2 trophies.");
static const u8 sText_MegaExRule[] = _("MEGA ex: knocked out, it gives 3 trophies.");
static const u8 sText_AbilityLabel[] = _("ABILITY ");
static const u8 sText_Item[] = _("ITEM");
static const u8 sText_Supporter[] = _("SUPPORTER\n(1 a turn)");
static const u8 sText_Tool[] = _("POKéMON TOOL");
static const u8 sText_ToolAttached[] = _("TOOL: ");
static const u8 sViewerTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

// Attack effects: text before and after the effect's number (or status)
struct EffectText
{
    const u8 *before;
    const u8 *after;
    bool8 hasNumber;
    bool8 hasStatus;
};

static const u8 sEffect_Status[] = _("The defender is now ");
static const u8 sEffect_FlipStatus[] = _("Flip a coin. Heads: the defender is now ");
static const u8 sEffect_HealSelf[] = _("Heal ");
static const u8 sEffect_HealSelfAfter[] = _(" from this POKéMON.");
static const u8 sEffect_SelfDamage[] = _("This POKéMON also takes ");
static const u8 sEffect_DamageAfter[] = _(" damage.");
static const u8 sEffect_FlipBonus[] = _("Flip a coin. Heads: ");
static const u8 sEffect_MoreDamage[] = _(" more damage.");
static const u8 sEffect_FlipOrFail[] = _("Flip a coin. Tails: this attack does nothing.");
static const u8 sEffect_Discard[] = _("Discard ");
static const u8 sEffect_DiscardAfter[] = _(" Energy of its type from it.");
static const u8 sEffect_DiscardAll[] = _("Discard all Energy from this POKéMON.");
static const u8 sEffect_FlipMulti[] = _("Flip ");
static const u8 sEffect_FlipMultiAfter[] = _(" coins: this damage for each heads.");
static const u8 sEffect_Snipe[] = _("It hits 1 of your opponent's POKéMON.");
static const u8 sEffect_Spread[] = _("Also ");
static const u8 sEffect_SpreadAfter[] = _(" damage to each opponent's Benched POKéMON.");
static const u8 sEffect_SelfBenchAfter[] = _(" damage to each of your Benched POKéMON.");
static const u8 sEffect_FlipDiscard[] = _("Flip a coin. Heads: the defender discards an Energy.");
static const u8 sEffect_CantAttack[] = _("It can't attack during your next turn.");
static const u8 sEffect_ExtraEnergyAfter[] = _(" more damage with 2 extra Energy of its type.");
static const u8 sEffect_Draw[] = _("Draw ");
static const u8 sEffect_DrawAfter[] = _(" cards.");
static const u8 sEffect_Switch[] = _("Your opponent's Active is switched with a Benched POKéMON.");
static const u8 sEffect_IfDamagedAfter[] = _(" more damage if the defender is damaged.");
static const u8 sEffect_Dot[] = _(".");

static const struct EffectText sEffectTexts[CARD_EFFECT_COUNT] =
{
    [CARD_EFFECT_STATUS]              = {sEffect_Status, sEffect_Dot, FALSE, TRUE},
    [CARD_EFFECT_FLIP_STATUS]         = {sEffect_FlipStatus, sEffect_Dot, FALSE, TRUE},
    [CARD_EFFECT_HEAL_SELF]           = {sEffect_HealSelf, sEffect_HealSelfAfter, TRUE, FALSE},
    [CARD_EFFECT_SELF_DAMAGE]         = {sEffect_SelfDamage, sEffect_DamageAfter, TRUE, FALSE},
    [CARD_EFFECT_FLIP_BONUS]          = {sEffect_FlipBonus, sEffect_MoreDamage, TRUE, FALSE},
    [CARD_EFFECT_FLIP_OR_FAIL]        = {sEffect_FlipOrFail, NULL, FALSE, FALSE},
    [CARD_EFFECT_DISCARD_ENERGY]      = {sEffect_Discard, sEffect_DiscardAfter, TRUE, FALSE},
    [CARD_EFFECT_DISCARD_ALL_ENERGY]  = {sEffect_DiscardAll, NULL, FALSE, FALSE},
    [CARD_EFFECT_FLIP_MULTI]          = {sEffect_FlipMulti, sEffect_FlipMultiAfter, TRUE, FALSE},
    [CARD_EFFECT_SNIPE]               = {sEffect_Snipe, NULL, FALSE, FALSE},
    [CARD_EFFECT_SPREAD_BENCH]        = {sEffect_Spread, sEffect_SpreadAfter, TRUE, FALSE},
    [CARD_EFFECT_SELF_BENCH_DAMAGE]   = {sEffect_Spread, sEffect_SelfBenchAfter, TRUE, FALSE},
    [CARD_EFFECT_FLIP_DISCARD_ENERGY] = {sEffect_FlipDiscard, NULL, FALSE, FALSE},
    [CARD_EFFECT_CANT_ATTACK_NEXT]    = {sEffect_CantAttack, NULL, FALSE, FALSE},
    [CARD_EFFECT_BONUS_EXTRA_ENERGY]  = {NULL, sEffect_ExtraEnergyAfter, TRUE, FALSE},
    [CARD_EFFECT_DRAW]                = {sEffect_Draw, sEffect_DrawAfter, TRUE, FALSE},
    [CARD_EFFECT_SWITCH_DEFENDER]     = {sEffect_Switch, NULL, FALSE, FALSE},
    [CARD_EFFECT_BONUS_IF_DAMAGED]    = {NULL, sEffect_IfDamagedAfter, TRUE, FALSE},
};

// Abilities: name, and text before and after abilityArg (NULL after: no number)
static const u8 sAbility_HealAll[] = _("POWDER HEAL");
static const u8 sAbility_HealAllText[] = _("Once a turn: heal ");
static const u8 sAbility_HealAllAfter[] = _(" from each of your POKéMON.");
static const u8 sAbility_Attach[] = _("ENERGY SHADOW");
static const u8 sAbility_AttachText[] = _("Once a turn: attach an Energy of its type to your Active of that type.");
static const u8 sAbility_Poison[] = _("GAS LEAK");
static const u8 sAbility_PoisonText[] = _("Once a turn, as the Active: the opponent's Active is now POISONED.");
static const u8 sAbility_Sleep[] = _("SLEEP PENDULUM");
static const u8 sAbility_SleepText[] = _("Once a turn: flip a coin. Heads: the opponent's Active is now ASLEEP.");
static const u8 sAbility_DriveOff[] = _("DRIVE OFF");
static const u8 sAbility_DriveOffText[] = _("Once a turn, as the Active: the opponent's Active is switched out.");
static const u8 sAbility_Snipe[] = _("SHARP SHOT");
static const u8 sAbility_SnipeText[] = _("Once a turn: ");
static const u8 sAbility_SnipeAfter[] = _(" damage to 1 of your opponent's POKéMON.");
static const u8 sAbility_Reduce[] = _("HARD SHELL");
static const u8 sAbility_ReduceText[] = _("It takes ");
static const u8 sAbility_ReduceAfter[] = _(" less damage from attacks.");
static const u8 sAbility_Counter[] = _("ROUGH SKIN");
static const u8 sAbility_CounterText[] = _("As the Active, damaged by an attack: ");
static const u8 sAbility_CounterAfter[] = _(" damage to the attacker.");
static const u8 sAbility_FreeRetreat[] = _("LIGHT FEET");
static const u8 sAbility_FreeRetreatText[] = _("It retreats for free.");

static const u8 *const sAbilityTexts[CARD_ABILITY_COUNT][3] =
{
    [CARD_ABILITY_HEAL_ALL]        = {sAbility_HealAll, sAbility_HealAllText, sAbility_HealAllAfter},
    [CARD_ABILITY_ATTACH_OWN_TYPE] = {sAbility_Attach, sAbility_AttachText, NULL},
    [CARD_ABILITY_POISON_DEFENDER] = {sAbility_Poison, sAbility_PoisonText, NULL},
    [CARD_ABILITY_FLIP_SLEEP]      = {sAbility_Sleep, sAbility_SleepText, NULL},
    [CARD_ABILITY_DRIVE_OFF]       = {sAbility_DriveOff, sAbility_DriveOffText, NULL},
    [CARD_ABILITY_SNIPE]           = {sAbility_Snipe, sAbility_SnipeText, sAbility_SnipeAfter},
    [CARD_ABILITY_REDUCE_DAMAGE]   = {sAbility_Reduce, sAbility_ReduceText, sAbility_ReduceAfter},
    [CARD_ABILITY_COUNTER]         = {sAbility_Counter, sAbility_CounterText, sAbility_CounterAfter},
    [CARD_ABILITY_FREE_RETREAT]    = {sAbility_FreeRetreat, sAbility_FreeRetreatText, NULL},
};

// Trainers: text before and after effectArg, and the effect's type if it has one
static const u8 sTrainer_Draw[] = _("Draw ");
static const u8 sTrainer_DrawAfter[] = _(" cards.");
static const u8 sTrainer_Heal[] = _("Heal ");
static const u8 sTrainer_HealAfter[] = _(" from 1 of your POKéMON.");
static const u8 sTrainer_HealTypeAfter[] = _(" from 1 of your POKéMON of this type: ");
static const u8 sTrainer_PokeBall[] = _("Put a random Basic POKéMON from your deck into your hand.");
static const u8 sTrainer_XSpeed[] = _("This turn, your Active's retreat cost is ");
static const u8 sTrainer_XSpeedAfter[] = _(" less.");
static const u8 sTrainer_ToolHp[] = _("The POKéMON it is attached to gets ");
static const u8 sTrainer_ToolHpAfter[] = _(" more HP.");
static const u8 sTrainer_HandScope[] = _("Your opponent reveals their hand.");
static const u8 sTrainer_RedCard[] = _("Your opponent shuffles their hand into their deck and draws ");
static const u8 sTrainer_RareCandy[] = _("A Basic POKéMON evolves straight into a Stage 2 from your hand.");
static const u8 sTrainer_Helmet[] = _("Attached to your Active, damaged by an attack: ");
static const u8 sTrainer_HelmetAfter[] = _(" damage to the attacker.");
static const u8 sTrainer_AttachEnergy[] = _("Attach an Energy to 1 of your POKéMON of this type: ");
static const u8 sTrainer_FlipAttach[] = _("Flip coins until tails: an Energy for each heads, to 1 of your POKéMON of this type: ");
static const u8 sTrainer_Gather[] = _("Move all Energy of this type from your Bench to your Active of that type: ");
static const u8 sTrainer_Switch[] = _("Your opponent's Active is switched with a Benched POKéMON.");
static const u8 sTrainer_Bonus[] = _("This turn, your attacks do ");
static const u8 sTrainer_BonusTypeAfter[] = _(" more damage, with POKéMON of this type: ");
static const u8 sTrainer_BonusAfter[] = _(" more damage.");
static const u8 sTrainer_Status[] = _("Your opponent's Active POKéMON is now ");
static const u8 sTrainer_Search[] = _("Put a random POKéMON from your deck into your hand.");
static const u8 sTrainer_Shield[] = _("During your opponent's next turn, your POKéMON take ");
static const u8 sTrainer_ShieldAfter[] = _(" less damage from attacks.");
static const u8 sTrainer_HealAllAfter[] = _(" from each of your POKéMON.");
static const u8 sTrainer_DrawUntil[] = _("Draw cards until you have ");
static const u8 sTrainer_DrawUntilAfter[] = _(" cards in your hand.");
static const u8 sTrainer_Cure[] = _("Your Active POKéMON recovers from all Special Conditions.");
static const u8 sTrainer_ToolHeal[] = _("At the end of your turn, if it is your Active POKéMON, heal ");
static const u8 sTrainer_ToolHealAfter[] = _(" from it.");
static const u8 sTrainer_ToolDamage[] = _("Its attacks do ");
static const u8 sTrainer_ToolDamageAfter[] = _(" more damage to your opponent's Active POKéMON.");

static const u8 *const sTrainerTexts[][2] =
{
    [CARD_TRAINER_DRAW]               = {sTrainer_Draw, sTrainer_DrawAfter},
    [CARD_TRAINER_HEAL]               = {sTrainer_Heal, sTrainer_HealAfter},
    [CARD_TRAINER_POKE_BALL]          = {sTrainer_PokeBall, NULL},
    [CARD_TRAINER_RETREAT_DISCOUNT]   = {sTrainer_XSpeed, sTrainer_XSpeedAfter},
    [CARD_TRAINER_TOOL_HP]            = {sTrainer_ToolHp, sTrainer_ToolHpAfter},
    [CARD_TRAINER_HAND_SCOPE]         = {sTrainer_HandScope, NULL},
    [CARD_TRAINER_RED_CARD]           = {sTrainer_RedCard, sEffect_Dot},
    [CARD_TRAINER_RARE_CANDY]         = {sTrainer_RareCandy, NULL},
    [CARD_TRAINER_TOOL_COUNTER]       = {sTrainer_Helmet, sTrainer_HelmetAfter},
    [CARD_TRAINER_ATTACH_ENERGY]      = {sTrainer_AttachEnergy, NULL},
    [CARD_TRAINER_FLIP_ATTACH_ENERGY] = {sTrainer_FlipAttach, NULL},
    [CARD_TRAINER_GATHER_ENERGY]      = {sTrainer_Gather, NULL},
    [CARD_TRAINER_HEAL_TYPE]          = {sTrainer_Heal, sTrainer_HealTypeAfter},
    [CARD_TRAINER_SWITCH_OPPONENT]    = {sTrainer_Switch, NULL},
    [CARD_TRAINER_ATTACK_BONUS]       = {sTrainer_Bonus, sTrainer_BonusAfter},
    [CARD_TRAINER_STATUS_OPPONENT]    = {sTrainer_Status, NULL},
    [CARD_TRAINER_SEARCH_POKEMON]     = {sTrainer_Search, NULL},
    [CARD_TRAINER_SHIELD]             = {sTrainer_Shield, sTrainer_ShieldAfter},
    [CARD_TRAINER_HEAL_ALL]           = {sTrainer_Heal, sTrainer_HealAllAfter},
    [CARD_TRAINER_DRAW_UNTIL]         = {sTrainer_DrawUntil, sTrainer_DrawUntilAfter},
    [CARD_TRAINER_CURE]               = {sTrainer_Cure, NULL},
    [CARD_TRAINER_TOOL_HEAL]          = {sTrainer_ToolHeal, sTrainer_ToolHealAfter},
    [CARD_TRAINER_TOOL_DAMAGE]        = {sTrainer_ToolDamage, sTrainer_ToolDamageAfter},
};

// ---------------------------------------------------------------------------------------------
// Graphics

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

#define BOARD_WINDOW(left, top, w, base) {.bg = 0, .tilemapLeft = left, .tilemapTop = top, .width = w, .height = 4, .paletteNum = TEXT_PALETTE, .baseBlock = base}

// The card viewer covers the board and the messages, so it shares their windows' tiles
static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_OPPONENT_INFO]   = BOARD_WINDOW(0, ROW_OPPONENT_OUTER, 12, 1),
    [WIN_OPPONENT_BENCH]  = BOARD_WINDOW(SLOT_TILE_X, ROW_OPPONENT_OUTER, 18, 1 + 48),
    [WIN_OPPONENT_ZONE]   = BOARD_WINDOW(0, ROW_OPPONENT_INNER, 12, 1 + 48 + 72),
    [WIN_OPPONENT_ACTIVE] = BOARD_WINDOW(SLOT_TILE_X + SLOT_TILE_WIDTH, ROW_OPPONENT_INNER, 12, 1 + 48 * 2 + 72),
    [WIN_PLAYER_INFO]     = BOARD_WINDOW(0, ROW_PLAYER_OUTER, 12, 1 + 48 * 3 + 72),
    [WIN_PLAYER_BENCH]    = BOARD_WINDOW(SLOT_TILE_X, ROW_PLAYER_OUTER, 18, 1 + 48 * 4 + 72),
    [WIN_PLAYER_ZONE]     = BOARD_WINDOW(0, ROW_PLAYER_INNER, 12, 1 + 48 * 4 + 72 * 2),
    [WIN_PLAYER_ACTIVE]   = BOARD_WINDOW(SLOT_TILE_X + SLOT_TILE_WIDTH, ROW_PLAYER_INNER, 12, 1 + 48 * 5 + 72 * 2),
    [WIN_MESSAGE] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = ROW_MESSAGE,
        .width = 30,
        .height = 4,
        .paletteNum = TEXT_PALETTE,
        .baseBlock = 481,
    },
    [WIN_VIEW_INFO]  = {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = VIEW_INFO_WIDTH, .height = DISPLAY_TILE_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 1},
    [WIN_VIEW_CARD]  = {.bg = 0, .tilemapLeft = VIEW_INFO_WIDTH, .tilemapTop = 0, .width = VINTAGE_CARD_WIDTH, .height = VINTAGE_CARD_HEIGHT, .paletteNum = TEXT_PALETTE, .baseBlock = 1 + VIEW_INFO_WIDTH * DISPLAY_TILE_HEIGHT},
    [WIN_VIEW_FRAME] = {.bg = 1, .tilemapLeft = VIEW_INFO_WIDTH, .tilemapTop = 0, .width = VINTAGE_CARD_WIDTH, .height = VINTAGE_CARD_HEIGHT, .paletteNum = FRAME_PALETTE, .baseBlock = BOARD_TILE_COUNT},
    DUMMY_WIN_TEMPLATE,
};

static const struct OamData sOam_Pip =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .size = SPRITE_SIZE(8x8),
    .priority = 1,
};

static const struct OamData sOam_Coin =
{
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 0,
};

static const struct SpriteSheet sSpriteSheets[] =
{
    {sPipTiles, sizeof(sPipTiles), TAG_PIPS},
    {sCoinTiles, sizeof(sCoinTiles), TAG_COIN},
    {0},
};

static const struct SpritePalette sSpritePalettes[] =
{
    {sPipPalette, TAG_PIPS},
    {sCoinPalette, TAG_COIN},
    {0},
};

static const struct SpriteTemplate sSpriteTemplate_Pip =
{
    .tileTag = TAG_PIPS,
    .paletteTag = TAG_PIPS,
    .oam = &sOam_Pip,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sSpriteTemplate_Coin =
{
    .tileTag = TAG_COIN,
    .paletteTag = TAG_COIN,
    .oam = &sOam_Coin,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

// Where a slot's icon is, by side and slot (pixels, the icon's center)
static s16 GetSlotX(u8 slot)
{
    if (slot == CARD_SLOT_ACTIVE)
        return SLOT_TILE_X * 8 + SLOT_TILE_WIDTH * 4;
    return (SLOT_TILE_X + (slot - CARD_SLOT_BENCH_1) * SLOT_TILE_WIDTH) * 8 + 17;
}

static s16 GetSlotY(u8 side, u8 slot)
{
    u8 row;

    if (side == CARD_SIDE_PLAYER)
        row = slot == CARD_SLOT_ACTIVE ? ROW_PLAYER_INNER : ROW_PLAYER_OUTER;
    else
        row = slot == CARD_SLOT_ACTIVE ? ROW_OPPONENT_INNER : ROW_OPPONENT_OUTER;
    return row * 8 + 16;
}

// ---------------------------------------------------------------------------------------------
// Entry and exit

// The script's lock keeps the opponent still by the held movement faceplayer gave them, and coming
// back to the field clears held movements, so a wandering opponent walked off while they were still
// talking. They're frozen before the field shows, until the script's release.
static void FieldCB_ReturnFromCardGame(void)
{
    if (gObjectEvents[gSelectedObjectEvent].active && !gObjectEvents[gSelectedObjectEvent].isPlayer)
        FreezeObjectEvent(&gObjectEvents[gSelectedObjectEvent]);
    FieldCB_ContinueScriptHandleMusic();
}

static void CB2_ReturnFromCardGame(void)
{
    CB2_ReturnToFieldContinueScriptPlayMapMusic();
    gFieldCallback = FieldCB_ReturnFromCardGame;  // Runs once the map is loaded
}

void PlayCardGame(void)
{
    u8 opponentId = gSpecialVar_0x8004 < CARD_OPPONENTS_COUNT ? gSpecialVar_0x8004 : 0;

    sScreen = AllocZeroed(sizeof(*sScreen));
    sScreen->opponentDeck = gCardOpponents[opponentId].deck;
    CardGame_GetOpponentName(sScreen->opponentName, opponentId);
    sScreen->exitCallback = CB2_ReturnFromCardGame;
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

    // The player's deck, or the starter deck if it can't be played (scripts check it first,
    // with CardGame_CheckActiveDeck; this is for the debug duel)
    if (!CardGame_GetPlayerDeck(&player))
    {
        player.deck = gCardDecks[CARD_DECK_STARTER].cards;
        for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
            player.zoneTypes[i] = gCardDecks[CARD_DECK_STARTER].zoneTypes[i];
    }
    opponent.deck = gCardDecks[sScreen->opponentDeck].cards;
    for (i = 0; i < CARD_MAX_ZONE_TYPES; i++)
        opponent.zoneTypes[i] = gCardDecks[sScreen->opponentDeck].zoneTypes[i];
    // The Copycat plays a copy of the player's deck
    if (sScreen->opponentDeck == CARD_DECK_COPYCAT)
        opponent = player;
    Duel_Init(&sScreen->duel, &player, &opponent, Random32());
    Duel_ClearEvents(&sScreen->duel); // the opening draws and coin flip show on the board
    sScreen->display = sScreen->duel;
}

static void DrawSlotFrame(u8 x, u8 y, u8 width, u8 height, u16 baseTile)
{
    FillBgTilemapBufferRect(1, baseTile + 1, x, y, 1, 1, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 2, x + 1, y, width - 2, 1, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 3, x + width - 1, y, 1, 1, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 4, x, y + 1, 1, height - 2, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 5, x + 1, y + 1, width - 2, height - 2, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 6, x + width - 1, y + 1, 1, height - 2, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 7, x, y + height - 1, 1, 1, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 8, x + 1, y + height - 1, width - 2, 1, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, baseTile + 9, x + width - 1, y + height - 1, 1, 1, BOARD_PALETTE);
}

static void DrawTable(void)
{
    u8 i;

    FillBgTilemapBufferRect(1, BOARD_TILE_OPPONENT, 0, 0, 32, ROW_PLAYER_INNER, BOARD_PALETTE);
    FillBgTilemapBufferRect(1, BOARD_TILE_PLAYER, 0, ROW_PLAYER_INNER, 32, 32 - ROW_PLAYER_INNER, BOARD_PALETTE);
    DrawSlotFrame(SLOT_TILE_X, ROW_OPPONENT_INNER, SLOT_TILE_WIDTH, SLOT_TILE_HEIGHT, BOARD_TILE_OPPONENT);
    DrawSlotFrame(SLOT_TILE_X, ROW_PLAYER_INNER, SLOT_TILE_WIDTH, SLOT_TILE_HEIGHT, BOARD_TILE_PLAYER);
    for (i = 0; i < CARD_BENCH_MAX; i++)
    {
        DrawSlotFrame(SLOT_TILE_X + i * SLOT_TILE_WIDTH, ROW_OPPONENT_OUTER, SLOT_TILE_WIDTH, SLOT_TILE_HEIGHT, BOARD_TILE_OPPONENT);
        DrawSlotFrame(SLOT_TILE_X + i * SLOT_TILE_WIDTH, ROW_PLAYER_OUTER, SLOT_TILE_WIDTH, SLOT_TILE_HEIGHT, BOARD_TILE_PLAYER);
    }
    CopyBgTilemapBufferToVram(1);
}

static void InitSprites(void)
{
    u8 side, slot, i;

    LoadSpriteSheets(sSpriteSheets);
    LoadSpritePalettes(sSpritePalettes);
    LoadMonIconPalettes();
    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
        {
            sScreen->iconSprites[side][slot] = SPRITE_NONE;
            sScreen->iconCards[side][slot] = CARD_NONE;
        }
    }
    for (i = 0; i < MAX_PIPS; i++)
        sScreen->pipSprites[i] = SPRITE_NONE;
    sScreen->coinSprite = CreateSprite(&sSpriteTemplate_Coin, DISPLAY_WIDTH / 2, (ROW_PLAYER_INNER) * 8, 0);
    gSprites[sScreen->coinSprite].invisible = TRUE;
    sScreen->arrowTaskId = TASK_NONE;
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
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        InitWindows(sWindowTemplates);
        VintageCard_Init(WIN_VIEW_FRAME, WIN_VIEW_CARD);
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
        LoadPalette(sBoardPalette, BG_PLTT_ID(BOARD_PALETTE), PLTT_SIZE_4BPP);
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PALETTE), PLTT_SIZE_4BPP);
        CardGame_LoadEnergyIconPalette(TEXT_PALETTE);
        LoadBgTiles(1, sBoardTiles, sizeof(sBoardTiles), 0);
        InitSprites();
        gMain.state++;
        break;
    case 2:
        StartDuel();
        DrawTable();
        RefreshBoard();
        StringCopy(sScreen->text, sText_DuelStart);
        StringAppend(sScreen->text, sScreen->opponentName);
        StringAppend(sScreen->text, sText_ChooseActive);
        ShowMessage(sScreen->text);
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
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
    CardGame_RecordResult(gSpecialVar_Result);
    SetMainCallback2(sScreen->exitCallback);
    VintageCard_Free();
    FreeAllWindowBuffers();
    Free(GetBgTilemapBuffer(1));
    FreeMonIconPalettes();
    ResetSpriteData();
    FREE_AND_SET_NULL(sScreen);
}

// ---------------------------------------------------------------------------------------------
// Text helpers

static u8 *AppendNumber(u8 *dst, u16 number)
{
    u8 digits = number >= 100 ? 3 : (number >= 10 ? 2 : 1);

    return ConvertIntToDecimalStringN(dst, number, STR_CONV_MODE_LEFT_ALIGN, digits);
}

u8 *CardGame_AppendCardName(u8 *dst, u16 cardId)
{
    const struct PokemonCardInfo *pokemon = CardGame_GetPokemonInfo(cardId);
    const struct TrainerCardInfo *trainer = CardGame_GetTrainerInfo(cardId);

    if (pokemon != NULL)
    {
        if (pokemon->isMega)
            dst = StringAppend(dst, sText_Mega);
        dst = StringAppend(dst, gSpeciesNames[pokemon->species]);
        // Two Megas of the same species
        if (cardId == CARD_CHARIZARD_MEGA_X_EX || cardId == CARD_MEWTWO_MEGA_X_EX)
            dst = StringAppend(dst, sText_MegaX);
        else if (cardId == CARD_CHARIZARD_MEGA_Y_EX || cardId == CARD_MEWTWO_MEGA_Y_EX)
            dst = StringAppend(dst, sText_MegaY);
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
    return StringAppend(dst, sScreen->opponentName);
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

static u8 *AppendStatusLetters(u8 *dst, u8 status)
{
    if (status & CARD_STATUS_ASLEEP)
        dst = StringAppend(dst, sText_Asleep);
    if (status & CARD_STATUS_PARALYZED)
        dst = StringAppend(dst, sText_Paralyzed);
    if (status & CARD_STATUS_CONFUSED)
        dst = StringAppend(dst, sText_Confused);
    if (status & CARD_STATUS_POISONED)
        dst = StringAppend(dst, sText_Poisoned);
    if (status & CARD_STATUS_BURNED)
        dst = StringAppend(dst, sText_Burned);
    return dst;
}

// The cost, as Energy icons: for each Energy, a blank of ENERGY_ICON_SPACE pixels that
// CardGame_DrawEnergyIcons fills, after a code that says its type and prints nothing
// (CardGame_AppendTypeIcon)
static u8 *AppendCost(u8 *dst, const struct CardAttack *attack)
{
    u8 i;

    for (i = 0; i < CARD_ATTACK_MAX_COST && attack->cost[i] != CARD_TYPE_NONE; i++)
        dst = CardGame_AppendTypeIcon(dst, attack->cost[i]);
    *dst++ = CHAR_SPACE;
    *dst = EOS;
    return dst;
}

// A type as its Energy icon (see AppendCost), or its name if it has none (NONE)
u8 *CardGame_AppendTypeIcon(u8 *dst, u8 type)
{
    if (type == CARD_TYPE_NONE || type >= CARD_TYPE_COUNT)
        return StringAppend(dst, gCardTypeNames[type < CARD_TYPE_COUNT ? type : CARD_TYPE_NONE]);
    *dst++ = EXT_CTRL_CODE_BEGIN;
    *dst++ = EXT_CTRL_CODE_PALETTE;
    *dst++ = type;
    *dst++ = EXT_CTRL_CODE_BEGIN;
    *dst++ = EXT_CTRL_CODE_CLEAR;
    *dst++ = ENERGY_ICON_SPACE;
    *dst = EOS;
    return dst;
}

// Colors 4-15 of a text palette, for the Energy icons (0-3 stay the text's)
void CardGame_LoadEnergyIconPalette(u8 paletteNum)
{
    LoadPalette(&sEnergyIconPalette[4], BG_PLTT_ID(paletteNum) + 4, PLTT_SIZEOF(12));
}

// One Energy icon (ENERGY_ICON_SIZE pixels), its top left at x, y
void CardGame_DrawEnergyIcon(u8 windowId, u8 type, u16 x, u16 y)
{
    BlitBitmapRectToWindow(windowId, sEnergyIconGfx, type * 16, 0, CARD_TYPE_COUNT * 16, 16, x, y, ENERGY_ICON_SIZE, ENERGY_ICON_SIZE);
}

// Draws the icons of the costs in a text already printed at x, y (FONT_SMALL, lineHeight pixels
// a line)
void CardGame_DrawEnergyIcons(u8 windowId, const u8 *text, u8 x, u8 y, u8 lineHeight)
{
    u8 line[64];
    u8 i = 0;

    for (; *text != EOS; text++)
    {
        if (*text == CHAR_NEWLINE)
        {
            i = 0;
            y += lineHeight;
            continue;
        }
        if (text[0] == EXT_CTRL_CODE_BEGIN && text[1] == EXT_CTRL_CODE_PALETTE && text[2] < CARD_TYPE_COUNT)
        {
            line[i] = EOS;
            CardGame_DrawEnergyIcon(windowId, text[2], x + GetStringWidth(FONT_SMALL, line, 0), y + ENERGY_ICON_Y);
        }
        if (i < sizeof(line) - 1)
            line[i++] = *text;
    }
}

static u16 GetRemainingHp(const struct CardInPlay *mon)
{
    u16 maxHp = Duel_GetMaxHp(mon);

    return mon->damage >= maxHp ? 0 : maxHp - mon->damage;
}

// Cuts the text so it fits in width pixels
static void TruncateToWidth(u8 *text, u8 fontId, u16 width)
{
    u16 length = StringLength(text);

    while (length != 0 && GetStringWidth(fontId, text, 0) > width)
        text[--length] = EOS;
}

// Breaks the text into lines at spaces, so each fits in width pixels
void CardGame_WrapText(u8 *text, u8 fontId, u16 width)
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

static void PrintOnBoard(u8 windowId, u8 *text, u8 x, u8 y, u16 width)
{
    TruncateToWidth(text, FONT_SMALL, width);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, x, y, sBoardTextColors, TEXT_SKIP_DRAW, text);
}

// ---------------------------------------------------------------------------------------------
// The board

static u16 GetIconSpeciesForCard(u16 cardId)
{
    const struct PokemonCardInfo *info = CardGame_GetPokemonInfo(cardId);

    if (info == NULL)
        return SPECIES_NONE;
    return info->isMega ? GetMegaBaseSpecies(info->species) : info->species;
}

static void RefreshIcons(void)
{
    u8 side, slot, spriteId;
    u16 cardId;

    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        for (slot = 0; slot < CARD_SLOT_COUNT; slot++)
        {
            cardId = sScreen->display.sides[side].slots[slot].cardId;
            if (cardId == sScreen->iconCards[side][slot])
                continue;
            if (sScreen->iconSprites[side][slot] != SPRITE_NONE)
                FreeAndDestroyMonIconSprite(&gSprites[sScreen->iconSprites[side][slot]]);
            sScreen->iconSprites[side][slot] = SPRITE_NONE;
            sScreen->iconCards[side][slot] = cardId;
            if (cardId == CARD_NONE)
                continue;
            // The Active Pokémon move; the Benched ones stay still
            spriteId = CreateMonIcon(GetIconSpeciesForCard(cardId),
                                     slot == CARD_SLOT_ACTIVE ? SpriteCB_MonIcon : SpriteCallbackDummy,
                                     GetSlotX(slot), GetSlotY(side, slot), 1, 0, FALSE);
            gSprites[spriteId].oam.priority = 1;
            sScreen->iconSprites[side][slot] = spriteId;
        }
    }
}

static void AddPip(u8 pip, s16 x, s16 y)
{
    u8 spriteId;

    if (sScreen->pipCount >= MAX_PIPS)
        return;
    spriteId = CreateSprite(&sSpriteTemplate_Pip, x, y, 0);
    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].oam.tileNum = GetSpriteTileStartByTag(TAG_PIPS) + pip;
    sScreen->pipSprites[sScreen->pipCount++] = spriteId;
}

static void RefreshPips(void)
{
    u8 i, side, type, count;
    s16 x, top;

    for (i = 0; i < sScreen->pipCount; i++)
        DestroySprite(&gSprites[sScreen->pipSprites[i]]);
    sScreen->pipCount = 0;

    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        const struct CardDuelSide *s = &sScreen->display.sides[side];
        const struct CardInPlay *active = &s->slots[CARD_SLOT_ACTIVE];

        // Trophies, in the info panel
        top = (side == CARD_SIDE_PLAYER ? ROW_PLAYER_OUTER : ROW_OPPONENT_OUTER) * 8;
        for (i = 0; i < CARD_WIN_TROPHIES; i++)
            AddPip(i < s->trophies ? PIP_TROPHY : PIP_TROPHY_EMPTY, 8 + i * 10, top + 28);

        // This turn's Energy and the next one
        top = (side == CARD_SIDE_PLAYER ? ROW_PLAYER_INNER : ROW_OPPONENT_INNER) * 8;
        if (s->zoneEnergy != CARD_TYPE_NONE)
            AddPip(s->zoneEnergy, 50, top + 7);
        if (s->nextEnergy != CARD_TYPE_NONE)
            AddPip(s->nextEnergy, 50, top + 23);

        // The Active's Energy, under its details
        count = 0;
        x = (SLOT_TILE_X + SLOT_TILE_WIDTH) * 8 + 6;
        for (type = CARD_TYPE_GRASS; type < CARD_TYPE_COUNT; type++)
        {
            for (i = 0; i < active->energy[type] && count < MAX_ACTIVE_PIPS; i++, count++)
                AddPip(type, x + count * 9, top + 28);
        }
    }
}

static void DrawSideTexts(u8 sideId)
{
    const struct CardDuelSide *side = &sScreen->display.sides[sideId];
    const struct CardInPlay *active = &side->slots[CARD_SLOT_ACTIVE];
    u8 *text = sScreen->text;
    u8 *end;
    u8 windowId, slot;

    // Info: who, deck and hand (the trophies are pips)
    windowId = sSideWindows[sideId][SIDE_WIN_INFO];
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    if (sideId == CARD_SIDE_PLAYER)
        StringCopy_PlayerName(text, gSaveBlock2Ptr->playerName);
    else
        StringCopy(text, sScreen->opponentName);
    PrintOnBoard(windowId, text, 2, 1, 92);
    end = StringCopy(text, sText_Deck);
    end = AppendNumber(end, side->deckCount);
    end = StringAppend(end, sText_Hand);
    AppendNumber(end, side->handCount);
    PrintOnBoard(windowId, text, 2, 12, 92);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    // Energy Zone (the Energy are pips)
    windowId = sSideWindows[sideId][SIDE_WIN_ZONE];
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    StringCopy(text, sText_EnergyLabel);
    PrintOnBoard(windowId, text, 2, 1, 40);
    StringCopy(text, sText_NextLabel);
    PrintOnBoard(windowId, text, 2, 17, 40);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    // The Active: name, HP and conditions (the Energy are pips)
    windowId = sSideWindows[sideId][SIDE_WIN_ACTIVE];
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    if (active->cardId != CARD_NONE)
    {
        *text = EOS;
        CardGame_AppendCardName(text, active->cardId);
        if (active->tool != CARD_NONE)
            StringAppend(text, sText_Plus);
        PrintOnBoard(windowId, text, 2, 1, 92);
        end = StringCopy(text, sText_Hp);
        end = AppendNumber(end, GetRemainingHp(active));
        end = StringAppend(end, sText_Slash);
        end = AppendNumber(end, Duel_GetMaxHp(active));
        AppendStatusLetters(end, active->status);
        PrintOnBoard(windowId, text, 2, 12, 92);
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    // The Bench: HP and Energy count, right of each icon
    windowId = sSideWindows[sideId][SIDE_WIN_BENCH];
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    for (slot = CARD_SLOT_BENCH_1; slot < CARD_SLOT_COUNT; slot++)
    {
        const struct CardInPlay *mon = &side->slots[slot];
        u8 x = (slot - CARD_SLOT_BENCH_1) * SLOT_TILE_WIDTH * 8 + 32;

        if (mon->cardId == CARD_NONE)
            continue;
        AppendNumber(text, GetRemainingHp(mon));
        PrintOnBoard(windowId, text, x, 3, 15);
        if (Duel_CountEnergy(mon) != 0)
        {
            end = StringCopy(text, sText_E);
            AppendNumber(end, Duel_CountEnergy(mon));
            PrintOnBoard(windowId, text, x, 16, 15);
        }
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void RefreshBoard(void)
{
    DrawSideTexts(CARD_SIDE_OPPONENT);
    DrawSideTexts(CARD_SIDE_PLAYER);
    RefreshIcons();
    RefreshPips();
}

static void ShowMessage(const u8 *text)
{
    if (text != sScreen->text)
        StringCopy(sScreen->text, text);
    CardGame_WrapText(sScreen->text, FONT_NORMAL, 228);
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_MESSAGE, FONT_NORMAL, sScreen->text, 4, 0, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(WIN_MESSAGE);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------------------------
// The display follows the events

static void ClearInPlay(struct CardInPlay *mon)
{
    u8 *bytes = (u8 *)mon;
    u8 i;

    for (i = 0; i < sizeof(*mon); i++)
        bytes[i] = 0;
}

static void SwapWithActive(struct CardDuelSide *side, u8 slot)
{
    struct CardInPlay temp = side->slots[CARD_SLOT_ACTIVE];

    side->slots[CARD_SLOT_ACTIVE] = side->slots[slot];
    side->slots[slot] = temp;
}

static void ApplyEventToDisplay(const struct CardDuelEvent *event)
{
    struct CardDuelSide *side;
    struct CardInPlay *mon = NULL;
    u8 type, count;

    if (event->side >= CARD_SIDE_COUNT)
        return;
    side = &sScreen->display.sides[event->side];
    if (event->slot < CARD_SLOT_COUNT)
        mon = &side->slots[event->slot];

    switch (event->type)
    {
    case CARD_EVENT_DRAW:
        if (side->deckCount != 0)
            side->deckCount--;
        side->handCount++;
        break;
    case CARD_EVENT_ENERGY_ZONE:
        side->zoneEnergy = event->value;
        side->nextEnergy = sScreen->duel.sides[event->side].nextEnergy;
        break;
    case CARD_EVENT_PLAY_POKEMON:
        ClearInPlay(mon);
        mon->cardId = event->value;
        if (side->handCount != 0)
            side->handCount--;
        break;
    case CARD_EVENT_PLAY_TRAINER:
        if (side->handCount != 0)
            side->handCount--;
        break;
    case CARD_EVENT_ATTACH_ENERGY:
        mon->energy[event->value]++;
        if (side->zoneEnergy == event->value)
            side->zoneEnergy = CARD_TYPE_NONE;
        break;
    case CARD_EVENT_EVOLVE:
        mon->cardId = event->value;
        mon->status = 0;
        if (side->handCount != 0)
            side->handCount--;
        break;
    case CARD_EVENT_RETREAT:
    case CARD_EVENT_PROMOTE:
    case CARD_EVENT_SWITCH:
        SwapWithActive(side, event->slot);
        side->slots[event->slot].status = 0;
        break;
    case CARD_EVENT_DAMAGE:
        mon->damage += event->value;
        break;
    case CARD_EVENT_HEAL:
        mon->damage = mon->damage > event->value ? mon->damage - event->value : 0;
        break;
    case CARD_EVENT_STATUS:
        if (event->value & CARD_STATUS_ROTATION)
            mon->status &= ~CARD_STATUS_ROTATION;
        mon->status |= event->value;
        break;
    case CARD_EVENT_STATUS_CURED:
        mon->status &= ~event->value;
        break;
    case CARD_EVENT_DISCARD_ENERGY:
        count = event->value;
        for (type = CARD_TYPE_COUNT - 1; type > CARD_TYPE_NONE && count != 0; type--)
        {
            while (mon->energy[type] != 0 && count != 0)
            {
                mon->energy[type]--;
                count--;
            }
        }
        break;
    case CARD_EVENT_KNOCK_OUT:
        ClearInPlay(mon);
        break;
    case CARD_EVENT_TROPHY:
        side->trophies += event->value;
        break;
    }
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
        if (event->side == CARD_SIDE_PLAYER)
        {
            dst = StringAppend(dst, sText_YouDrew);
            dst = CardGame_AppendCardName(dst, event->value);
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
        dst = StringAppend(dst, gCardTypeNames[event->value]);
        StringAppend(dst, sText_EnergyDot);
        return TRUE;
    case CARD_EVENT_COIN:
        StringAppend(dst, event->value ? sText_Heads : sText_Tails);
        return TRUE;
    case CARD_EVENT_PLAY_POKEMON:
    case CARD_EVENT_PLAY_TRAINER:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Played);
        dst = CardGame_AppendCardName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_ATTACH_ENERGY:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, sText_Attached);
        dst = StringAppend(dst, gCardTypeNames[event->value]);
        dst = StringAppend(dst, sText_EnergyTo);
        if (event->slot == CARD_SLOT_ACTIVE)
        {
            dst = StringAppend(dst, sText_ActiveSpot);
            dst = StringAppend(dst, sText_Space);
        }
        dst = CardGame_AppendCardName(dst, event->cardId);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_EVOLVE:
        dst = StringAppend(dst, sText_Evolution);
        dst = CardGame_AppendCardName(dst, event->value);
        StringAppend(dst, sText_Exclamation);
        return TRUE;
    case CARD_EVENT_RETREAT:
    case CARD_EVENT_PROMOTE:
        dst = AppendSideName(dst, event->side);
        dst = StringAppend(dst, event->type == CARD_EVENT_RETREAT ? sText_RetreatedTo : sText_SentOut);
        dst = CardGame_AppendCardName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_ATTACK:
        info = CardGame_GetPokemonInfo(event->cardId);
        dst = CardGame_AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, sText_Used);
        dst = StringAppend(dst, info->attacks[event->value].name);
        StringAppend(dst, sText_Exclamation);
        return TRUE;
    case CARD_EVENT_ATTACK_FAILED:
        StringAppend(dst, sText_ItFailed);
        return TRUE;
    case CARD_EVENT_DAMAGE:
    case CARD_EVENT_HEAL:
        dst = CardGame_AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, event->type == CARD_EVENT_DAMAGE ? sText_Took : sText_Healed);
        dst = AppendNumber(dst, event->value);
        StringAppend(dst, event->type == CARD_EVENT_DAMAGE ? sText_Damage : sText_Dot);
        return TRUE;
    case CARD_EVENT_STATUS:
    case CARD_EVENT_STATUS_CURED:
        dst = CardGame_AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, event->type == CARD_EVENT_STATUS ? sText_IsNow : sText_IsNoLonger);
        dst = AppendStatusName(dst, event->value);
        StringAppend(dst, sText_Dot);
        return TRUE;
    case CARD_EVENT_DISCARD_ENERGY:
        dst = CardGame_AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, sText_Discarded);
        dst = AppendNumber(dst, event->value);
        StringAppend(dst, sText_EnergyDot);
        return TRUE;
    case CARD_EVENT_KNOCK_OUT:
        dst = CardGame_AppendCardName(dst, event->cardId);
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
        dst = CardGame_AppendCardName(dst, event->cardId);
        dst = StringAppend(dst, sText_UsedAbility);
        dst = StringAppend(dst, sAbilityTexts[event->value][0]);
        StringAppend(dst, sText_Exclamation);
        return TRUE;
    case CARD_EVENT_REVEAL_HAND:
        {
            const struct CardDuelSide *side = &duel->sides[event->side];
            u8 i;

            dst = StringAppend(dst, sText_HandOf);
            dst = AppendSideName(dst, event->side);
            dst = StringAppend(dst, sText_Colon);
            for (i = 0; i < side->handCount && i < 5; i++)
            {
                if (i != 0)
                    dst = StringAppend(dst, sText_Comma);
                dst = CardGame_AppendCardName(dst, side->hand[i]);
            }
        }
        return TRUE;
    case CARD_EVENT_SWITCH:
        dst = CardGame_AppendCardName(dst, event->value);
        StringAppend(dst, sText_WasSwitchedIn);
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------------------------
// Animations

static struct Sprite *GetAnimIcon(void)
{
    u8 spriteId = sScreen->iconSprites[sScreen->animSide][sScreen->animSlot];

    return spriteId == SPRITE_NONE ? NULL : &gSprites[spriteId];
}

static void StartAnimation(const struct CardDuelEvent *event)
{
    sScreen->anim = ANIM_NONE;
    sScreen->animFrame = 0;
    sScreen->animSide = event->side;
    sScreen->animSlot = event->slot;
    switch (event->type)
    {
    case CARD_EVENT_ATTACK:
        sScreen->anim = ANIM_LUNGE;
        break;
    case CARD_EVENT_DAMAGE:
        sScreen->anim = ANIM_SHAKE;
        PlaySE(SE_M_COMET_PUNCH);
        break;
    case CARD_EVENT_KNOCK_OUT:
    case CARD_EVENT_PLAY_POKEMON:
    case CARD_EVENT_EVOLVE:
    case CARD_EVENT_PROMOTE:
    case CARD_EVENT_RETREAT:
    case CARD_EVENT_SWITCH:
        sScreen->anim = ANIM_BLINK;
        if (event->type == CARD_EVENT_RETREAT || event->type == CARD_EVENT_PROMOTE || event->type == CARD_EVENT_SWITCH)
            sScreen->animSlot = CARD_SLOT_ACTIVE;
        break;
    case CARD_EVENT_COIN:
        sScreen->anim = ANIM_COIN;
        sScreen->coinHeads = event->value;
        gSprites[sScreen->coinSprite].invisible = FALSE;
        PlaySE(SE_BALL_BOUNCE_1);
        break;
    case CARD_EVENT_HEAL:
        PlaySE(SE_USE_ITEM);
        break;
    case CARD_EVENT_TROPHY:
        PlaySE(SE_SUCCESS);
        break;
    }
}

// Runs a frame of the animation; FALSE when it's over
static bool8 RunAnimation(void)
{
    struct Sprite *icon = GetAnimIcon();
    u8 frame = sScreen->animFrame++;
    struct Sprite *coin = &gSprites[sScreen->coinSprite];

    switch (sScreen->anim)
    {
    case ANIM_LUNGE:
        if (icon == NULL || frame >= 16)
        {
            if (icon != NULL)
                icon->y2 = 0;
            return FALSE;
        }
        // Towards the other side and back
        icon->y2 = (frame < 8 ? frame : 16 - frame) * (sScreen->animSide == CARD_SIDE_PLAYER ? -2 : 2);
        return TRUE;
    case ANIM_SHAKE:
        if (icon == NULL || frame >= 18)
        {
            if (icon != NULL)
                icon->x2 = 0;
            return FALSE;
        }
        icon->x2 = (frame & 2) ? 3 : -3;
        return TRUE;
    case ANIM_BLINK:
        if (icon == NULL || frame >= 20)
        {
            if (icon != NULL)
                icon->invisible = FALSE;
            return FALSE;
        }
        icon->invisible = (frame & 4) != 0;
        return TRUE;
    case ANIM_COIN:
        if (frame >= COIN_SPIN_FRAMES)
        {
            coin->oam.tileNum = GetSpriteTileStartByTag(TAG_COIN) + (sScreen->coinHeads ? 0 : 32);
            return FALSE;
        }
        // Heads, edge, tails, edge...
        switch ((frame / 3) % 4)
        {
        case 0:
            coin->oam.tileNum = GetSpriteTileStartByTag(TAG_COIN);
            break;
        case 2:
            coin->oam.tileNum = GetSpriteTileStartByTag(TAG_COIN) + 32;
            break;
        default:
            coin->oam.tileNum = GetSpriteTileStartByTag(TAG_COIN) + 16;
            break;
        }
        coin->y2 = -(s16)((frame * (COIN_SPIN_FRAMES - frame)) / 8);
        return TRUE;
    }
    return FALSE;
}

static bool8 IsAppliedAfterAnimation(u8 eventType)
{
    return eventType == CARD_EVENT_DAMAGE || eventType == CARD_EVENT_KNOCK_OUT;
}

// Starts showing the next event; FALSE when there are none left
static bool8 StartNextEvent(void)
{
    struct CardDuel *duel = &sScreen->duel;
    const struct CardDuelEvent *event;

    while (sScreen->nextEvent < duel->eventCount)
    {
        event = &duel->events[sScreen->nextEvent++];
        if (!FormatEvent(sScreen->text, event))
        {
            ApplyEventToDisplay(event);
            continue;
        }
        ShowMessage(sScreen->text);
        sScreen->eventApplied = !IsAppliedAfterAnimation(event->type);
        if (sScreen->eventApplied)
        {
            ApplyEventToDisplay(event);
            RefreshBoard();
        }
        StartAnimation(event);
        return TRUE;
    }
    // All shown: the board catches up with the duel
    Duel_ClearEvents(duel);
    sScreen->nextEvent = 0;
    sScreen->display = *duel;
    RefreshBoard();
    return FALSE;
}

static void FinishEvent(void)
{
    if (!sScreen->eventApplied)
    {
        ApplyEventToDisplay(&sScreen->duel.events[sScreen->nextEvent - 1]);
        RefreshBoard();
        sScreen->eventApplied = TRUE;
    }
    gSprites[sScreen->coinSprite].invisible = TRUE;
    gSprites[sScreen->coinSprite].y2 = 0;
}

// ---------------------------------------------------------------------------------------------
// The card viewer

static void AddView(u16 cardId, u8 side, u8 slot)
{
    struct CardView *view;

    if (cardId == CARD_NONE || sScreen->viewCount >= VIEW_MAX)
        return;
    view = &sScreen->views[sScreen->viewCount++];
    view->cardId = cardId;
    view->side = side;
    view->slot = slot;
}

static void OpenCardViewer(void)
{
    const struct CardDuel *duel = &sScreen->duel;
    u8 i, side;

    sScreen->viewCount = 0;
    sScreen->viewIndex = 0;
    for (i = 0; i < duel->sides[CARD_SIDE_PLAYER].handCount; i++)
        AddView(duel->sides[CARD_SIDE_PLAYER].hand[i], CARD_SIDE_PLAYER, CARD_SLOT_NONE);
    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        for (i = 0; i < CARD_SLOT_COUNT; i++)
            AddView(duel->sides[side].slots[i].cardId, side, i);
    }
    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        for (i = 0; i < CARD_SLOT_COUNT; i++)
        {
            if (sScreen->iconSprites[side][i] != SPRITE_NONE)
                gSprites[sScreen->iconSprites[side][i]].invisible = TRUE;
        }
    }
    for (i = 0; i < sScreen->pipCount; i++)
        gSprites[sScreen->pipSprites[i]].invisible = TRUE;
    VintageCard_EnableClipping();
    ShowViewedCard();
}

static void CloseCardViewer(void)
{
    u8 side, i;

    VintageCard_Clear();
    VintageCard_DisableClipping();
    ClearWindowTilemap(WIN_VIEW_INFO);
    ClearWindowTilemap(WIN_VIEW_CARD);
    for (side = 0; side < CARD_SIDE_COUNT; side++)
    {
        for (i = 0; i < CARD_SLOT_COUNT; i++)
        {
            if (sScreen->iconSprites[side][i] != SPRITE_NONE)
                gSprites[sScreen->iconSprites[side][i]].invisible = FALSE;
        }
    }
    // The viewer used the board's and the message's windows' tiles, and the card's frame was
    // on the table: draw them again
    DrawTable();
    RefreshBoard();
    PutWindowTilemap(WIN_MESSAGE);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_FULL);
}

// Appends "before N after" or "before STATUS after"
static u8 *AppendEffectText(u8 *dst, const u8 *before, u16 number, const u8 *after)
{
    if (before != NULL)
        dst = StringAppend(dst, before);
    dst = AppendNumber(dst, number);
    if (after != NULL)
        dst = StringAppend(dst, after);
    return dst;
}

u8 *CardGame_AppendAttackText(u8 *dst, const struct CardAttack *attack)
{
    dst = AppendCost(dst, attack);
    dst = StringAppend(dst, attack->name);
    if (attack->damage != 0)
    {
        dst = StringAppend(dst, sText_Space);
        dst = AppendNumber(dst, attack->damage);
        if (attack->effect == CARD_EFFECT_FLIP_MULTI)
            dst = StringAppend(dst, sText_Plus);
    }
    if (attack->effect == CARD_EFFECT_NONE)
        return dst;
    dst = StringAppend(dst, sText_Space);
    return CardGame_AppendAttackEffect(dst, attack);
}

// What the attack does besides its damage (nothing for CARD_EFFECT_NONE)
u8 *CardGame_AppendAttackEffect(u8 *dst, const struct CardAttack *attack)
{
    const struct EffectText *effect = &sEffectTexts[attack->effect];

    if (attack->effect == CARD_EFFECT_NONE)
        return dst;
    if (effect->hasStatus)
    {
        dst = StringAppend(dst, effect->before);
        dst = AppendStatusName(dst, attack->effectArg);
        return StringAppend(dst, effect->after);
    }
    if (effect->hasNumber)
        return AppendEffectText(dst, effect->before, attack->effectArg, effect->after);
    return StringAppend(dst, effect->before);
}

// What knocking it out is worth, for the ex cards
u8 *CardGame_AppendExRule(u8 *dst, const struct PokemonCardInfo *info)
{
    return StringAppend(dst, info->isMega ? sText_MegaExRule : sText_ExRule);
}

// "ABILITY Name: what it does"
u8 *CardGame_AppendAbilityText(u8 *dst, const struct PokemonCardInfo *info)
{
    dst = StringAppend(dst, sText_AbilityLabel);
    dst = CardGame_AppendAbilityName(dst, info);
    dst = StringAppend(dst, sText_Colon);
    return CardGame_AppendAbilityEffect(dst, info);
}

u8 *CardGame_AppendAbilityName(u8 *dst, const struct PokemonCardInfo *info)
{
    return StringAppend(dst, sAbilityTexts[info->ability][0]);
}

// What the ability does
u8 *CardGame_AppendAbilityEffect(u8 *dst, const struct PokemonCardInfo *info)
{
    if (sAbilityTexts[info->ability][2] != NULL)
        return AppendEffectText(dst, sAbilityTexts[info->ability][1], info->abilityArg, sAbilityTexts[info->ability][2]);
    return StringAppend(dst, sAbilityTexts[info->ability][1]);
}

u8 *CardGame_AppendTrainerKind(u8 *dst, const struct TrainerCardInfo *info)
{
    return StringAppend(dst, info->kind == CARD_KIND_SUPPORTER ? sText_Supporter : (info->kind == CARD_KIND_TOOL ? sText_Tool : sText_Item));
}

u8 *CardGame_AppendTrainerText(u8 *dst, const struct TrainerCardInfo *info)
{
    const u8 *after = sTrainerTexts[info->effect][1];

    // Blaine: only for the Pokémon of his type
    if (info->effect == CARD_TRAINER_ATTACK_BONUS && info->effectType != CARD_TYPE_NONE)
        after = sTrainer_BonusTypeAfter;
    dst = StringAppend(dst, sTrainerTexts[info->effect][0]);
    // Koga, Lorelei, Agatha: the Special Condition's name
    if (info->effect == CARD_TRAINER_STATUS_OPPONENT)
        return StringAppend(AppendStatusName(dst, info->effectArg), sEffect_Dot);
    if (after != NULL)
        dst = AppendEffectText(dst, NULL, info->effectArg, after);
    if (info->effectType != CARD_TYPE_NONE)
    {
        dst = CardGame_AppendTypeIcon(dst, info->effectType);
        dst = StringAppend(dst, sEffect_Dot);
    }
    return dst;
}

// A Pokémon card's picture: its Gold sprite, or its front pic for the species Gold doesn't have
// (Megas, Hoenn). One at a time. Returns MAX_SPRITES if it can't be made.
u16 CardGame_CreatePokemonPic(u16 species, s16 x, s16 y)
{
    u16 spriteId;

    if (species < ARRAY_COUNT(sRetroPics) && sRetroPics[species].gfx != NULL)
    {
        struct CompressedSpriteSheet sheet;
        struct SpritePalette palette;

        sheet.data = sRetroPics[species].gfx;
        sheet.size = MON_PIC_SIZE;
        sheet.tag = TAG_CARD_PIC;
        palette.data = sRetroPics[species].palette;
        palette.tag = TAG_CARD_PIC;
        LoadCompressedSpriteSheet(&sheet);
        LoadSpritePalette(&palette);
        spriteId = CreateSprite(&sSpriteTemplate_CardPic, x, y, 0);
        if (spriteId == MAX_SPRITES)
        {
            FreeSpriteTilesByTag(TAG_CARD_PIC);
            FreeSpritePaletteByTag(TAG_CARD_PIC);
        }
        return spriteId;
    }
    spriteId = CreateMonPicSprite_HandleDeoxys(species, 0, NOT_SHINY_PERSONALITY, TRUE, x, y, 15, TAG_NONE);
    return spriteId < MAX_SPRITES ? spriteId : MAX_SPRITES;
}

void CardGame_DestroyPokemonPic(u16 spriteId)
{
    if (gSprites[spriteId].template == &sSpriteTemplate_CardPic)
    {
        DestroySprite(&gSprites[spriteId]);
        FreeSpriteTilesByTag(TAG_CARD_PIC);
        FreeSpritePaletteByTag(TAG_CARD_PIC);
    }
    else
    {
        FreeAndDestroyMonPicSprite(spriteId);
    }
}

// The card drawn whole on the right; on the left, where it is, a Pokémon in play's HP left,
// Special Conditions and Tool, and the card's whole text if it can't show it all
static void ShowViewedCard(void)
{
    const struct CardView *view = &sScreen->views[sScreen->viewIndex];
    const struct CardInPlay *mon = view->slot != CARD_SLOT_NONE ? &sScreen->duel.sides[view->side].slots[view->slot] : NULL;
    u8 *text = sScreen->text;
    u8 *end;
    u8 y = 18;

    FillWindowPixelBuffer(WIN_VIEW_INFO, PIXEL_FILL(1));
    if (view->slot == CARD_SLOT_NONE)
    {
        end = StringCopy(text, sText_YourHand);
        end = AppendNumber(end, sScreen->viewIndex + 1);
        end = StringAppend(end, sText_Slash);
        AppendNumber(end, sScreen->duel.sides[CARD_SIDE_PLAYER].handCount);
    }
    else
    {
        end = StringCopy(text, view->side == CARD_SIDE_PLAYER ? sText_YourPokemon : sText_TheirPokemon);
        if (view->slot == CARD_SLOT_ACTIVE)
        {
            StringAppend(end, sText_ActiveSpot);
        }
        else
        {
            end = StringAppend(end, sText_BenchSpot);
            StringAppend(end, sSlotNames[view->slot]);
        }
    }
    CardGame_WrapText(text, FONT_NORMAL, VIEW_INFO_WIDTH * 8 - 8);
    AddTextPrinterParameterized(WIN_VIEW_INFO, FONT_NORMAL, text, 4, 2, TEXT_SKIP_DRAW, NULL);
    for (end = text; *end != EOS; end++)
    {
        if (*end == CHAR_NEWLINE)
            y += 16;
    }

    if (mon != NULL)
    {
        end = StringCopy(text, sText_Hp);
        end = AppendNumber(end, GetRemainingHp(mon));
        end = StringAppend(end, sText_Slash);
        end = AppendNumber(end, Duel_GetMaxHp(mon));
        AppendStatusLetters(end, mon->status);
        AddTextPrinterParameterized(WIN_VIEW_INFO, FONT_SMALL, text, 4, y, TEXT_SKIP_DRAW, NULL);
        y += 12;
        if (mon->tool != CARD_NONE)
        {
            end = StringCopy(text, sText_ToolAttached);
            CardGame_AppendCardName(end, mon->tool);
            AddTextPrinterParameterized(WIN_VIEW_INFO, FONT_SMALL, text, 4, y, TEXT_SKIP_DRAW, NULL);
            y += 12;
        }
    }
    if (!VintageCard_Draw(view->cardId))
        VintageCard_PrintFullText(WIN_VIEW_INFO, view->cardId, 4, y + 4, VIEW_INFO_WIDTH * 8 - 8, VIEW_HELP_Y - 2, sViewerTextColors);
    AddTextPrinterParameterized(WIN_VIEW_INFO, FONT_SMALL, sText_ViewerHelp, 4, VIEW_HELP_Y, TEXT_SKIP_DRAW, NULL);
    PutWindowTilemap(WIN_VIEW_INFO);
    CopyWindowToVram(WIN_VIEW_INFO, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------------------------
// Action menu

// A Pokémon in play by name, "ACTIVE " before the Active one's: two of the same Pokémon can be
// in play, and the Active one is what most cards are about
static u8 *AppendPokemonInPlay(u8 *dst, const struct CardDuelSide *side, u8 slot)
{
    if (slot == CARD_SLOT_ACTIVE)
    {
        dst = StringAppend(dst, sText_ActiveSpot);
        dst = StringAppend(dst, sText_Space);
    }
    return CardGame_AppendCardName(dst, side->slots[slot].cardId);
}

// A spot: "ACTIVE" or "BENCH 1"
static u8 *AppendSpot(u8 *dst, u8 slot)
{
    if (slot != CARD_SLOT_ACTIVE)
        dst = StringAppend(dst, sText_BenchSpot);
    return StringAppend(dst, sSlotNames[slot]);
}

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
        CardGame_AppendCardName(dst, handCard);
        break;
    case CARD_ACTION_END_SETUP:
        StringAppend(dst, sText_StartDuel);
        break;
    case CARD_ACTION_ATTACH_ENERGY:
        dst = StringAppend(dst, sText_Attach);
        dst = StringAppend(dst, gCardTypeNames[side->zoneEnergy]);
        dst = StringAppend(dst, sText_EnergyTo);
        AppendPokemonInPlay(dst, side, action->slot);
        break;
    case CARD_ACTION_EVOLVE:
        // "Evolve ACTIVE into IVYSAUR", "Evolve BENCH 1 into IVYSAUR"
        dst = StringAppend(dst, sText_Evolve);
        dst = AppendSpot(dst, action->slot);
        dst = StringAppend(dst, sText_Into);
        CardGame_AppendCardName(dst, handCard);
        break;
    case CARD_ACTION_PLAY_TRAINER:
        if (CardGame_GetTrainerInfo(handCard)->effect == CARD_TRAINER_RARE_CANDY)
        {
            // "RARE CANDY: VENUSAUR on ACTIVE": the Stage 2 tells which Basic it is for
            dst = CardGame_AppendCardName(dst, handCard);
            dst = StringAppend(dst, sText_Colon);
            dst = CardGame_AppendCardName(dst, side->hand[action->arg]);
            dst = StringAppend(dst, sText_RareCandyOn);
            AppendSpot(dst, action->slot);
            break;
        }
        dst = StringAppend(dst, sText_Play);
        dst = CardGame_AppendCardName(dst, handCard);
        if (action->slot < CARD_SLOT_COUNT)
        {
            dst = StringAppend(dst, sText_On);
            AppendPokemonInPlay(dst, side, action->slot);
        }
        break;
    case CARD_ACTION_RETREAT:
        dst = StringAppend(dst, sText_RetreatTo);
        CardGame_AppendCardName(dst, side->slots[action->slot].cardId);
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
            AppendPokemonInPlay(dst, opponent, action->slot);
        }
        break;
    case CARD_ACTION_USE_ABILITY:
        dst = StringAppend(dst, sText_Ability);
        dst = AppendPokemonInPlay(dst, side, action->slot);
        info = CardGame_GetPokemonInfo(side->slots[action->slot].cardId);
        if (info->ability == CARD_ABILITY_SNIPE)
        {
            dst = StringAppend(dst, sText_At);
            AppendPokemonInPlay(dst, opponent, action->arg);
        }
        break;
    case CARD_ACTION_END_TURN:
        StringAppend(dst, sText_EndTurn);
        break;
    case CARD_ACTION_PROMOTE:
        dst = StringAppend(dst, sText_SendOut);
        CardGame_AppendCardName(dst, side->slots[action->slot].cardId);
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
        TruncateToWidth(sScreen->labels[i], FONT_NORMAL, 210);
        sScreen->items[i].name = sScreen->labels[i];
        sScreen->items[i].id = i;
    }
    StringCopy(sScreen->labels[count], sText_ViewCards);
    sScreen->items[count].name = sScreen->labels[count];
    sScreen->items[count].id = MENU_VIEW_CARDS;
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
    template.maxShowed = 2;
    template.windowId = WIN_MESSAGE;
    template.header_X = 0;
    template.item_X = 12;
    template.cursor_X = 4;
    template.upText_Y = 0;
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
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
    PutWindowTilemap(WIN_MESSAGE);
    sScreen->listTaskId = ListMenuInit(&template, 0, 0);
    CopyWindowToVram(WIN_MESSAGE, COPYWIN_FULL);
    if (count > 2)
        sScreen->arrowTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 230, ROW_MESSAGE * 8 + 4, DISPLAY_HEIGHT - 4,
                                                                         count - 2, TAG_ARROWS, TAG_ARROWS, &sScreen->menuScroll);
    sScreen->menuShown = TRUE;
}

static void CloseActionMenu(void)
{
    if (!sScreen->menuShown)
        return;
    DestroyListMenuTask(sScreen->listTaskId, &sScreen->menuScroll, &sScreen->menuRow);
    if (sScreen->arrowTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sScreen->arrowTaskId);
        sScreen->arrowTaskId = TASK_NONE;
    }
    sScreen->menuShown = FALSE;
    // An empty message box until the next message
    FillWindowPixelBuffer(WIN_MESSAGE, PIXEL_FILL(1));
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
            sScreen->eventApplied = TRUE;
            sScreen->state = STATE_MESSAGE;
        }
        break;
    case STATE_NEXT:
        if (StartNextEvent())
        {
            sScreen->timer = 0;
            sScreen->state = STATE_ANIMATE;
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
    case STATE_ANIMATE:
        sScreen->timer++;
        if (!RunAnimation())
        {
            FinishEvent();
            sScreen->state = STATE_MESSAGE;
        }
        break;
    case STATE_MESSAGE:
        if (++sScreen->timer >= MESSAGE_FRAMES || PressedAorB())
        {
            gSprites[sScreen->coinSprite].invisible = TRUE;
            sScreen->state = STATE_NEXT;
        }
        break;
    case STATE_MENU:
        input = ListMenu_ProcessInput(sScreen->listTaskId);
        ListMenuGetScrollAndRow(sScreen->listTaskId, &sScreen->menuScroll, &sScreen->menuRow);
        if (JOY_NEW(SELECT_BUTTON))
            input = MENU_VIEW_CARDS;
        if (input == LIST_NOTHING_CHOSEN || input == LIST_CANCEL)
            break;
        PlaySE(SE_SELECT);
        CloseActionMenu();
        if (input == MENU_VIEW_CARDS)
        {
            OpenCardViewer();
            sScreen->state = STATE_VIEW_CARDS;
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
    case STATE_VIEW_CARDS:
        if (JOY_NEW(B_BUTTON | A_BUTTON | SELECT_BUTTON))
        {
            PlaySE(SE_SELECT);
            CloseCardViewer();
            sScreen->state = STATE_NEXT;
        }
        else if (JOY_NEW(DPAD_RIGHT | DPAD_DOWN) && sScreen->viewCount > 1)
        {
            PlaySE(SE_SELECT);
            sScreen->viewIndex = (sScreen->viewIndex + 1) % sScreen->viewCount;
            ShowViewedCard();
        }
        else if (JOY_NEW(DPAD_LEFT | DPAD_UP) && sScreen->viewCount > 1)
        {
            PlaySE(SE_SELECT);
            sScreen->viewIndex = (sScreen->viewIndex + sScreen->viewCount - 1) % sScreen->viewCount;
            ShowViewedCard();
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

            CardAi_ChooseAction(duel, CARD_SIDE_OPPONENT, gCardDecks[sScreen->opponentDeck].aiLevel, sScreen->actions, &action);
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
