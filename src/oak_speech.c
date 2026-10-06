// The new game scene with Prof. Oak, ported from FireRed/LeafGreen (pokefirered's oak_speech.c).
// It replaces Prof. Birch's speech. FRLG's controls guide and Pikachu intro pages that come
// before Oak are not included, since they describe FRLG's help system.
// The player can be Brendan, May, Red or Leaf, and names Oak's grandson, their rival.
#include "global.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "naming_screen.h"
#include "oak_speech.h"
#include "overworld.h"
#include "palette.h"
#include "pokeball.h"
#include "random.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "util.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/species.h"
#include "constants/songs.h"

#define INTRO_SPECIES SPECIES_NIDORAN_F

#define Q_8_8_inv(n) ((s16)(0x10000 / (n)))

// Window 0 is the standard message box, created by InitStandardTextBoxWindows
#define WIN_INTRO_TEXTBOX 0

// The standard window frame, as loaded by InitTextBoxGfxAndPrinters (see menu.c)
#define STD_WINDOW_BASE_TILE_NUM 0x214
#define STD_WINDOW_PALETTE_NUM 14

struct OakSpeechResources
{
    u16 hasPlayerBeenNamed;
    u16 shrinkTimer;
    u8 bg2TilemapBuffer[0x400];
    u8 bg1TilemapBuffer[0x800];
};

static EWRAM_DATA struct OakSpeechResources *sOakSpeechResources = NULL;

static void Task_OakSpeech_Init(u8);
static void Task_OakSpeech_WelcomeToTheWorld(u8);
static void Task_OakSpeech_ThisWorld(u8);
static void Task_OakSpeech_ReleaseNidoranFFromPokeBall(u8);
static void Task_OakSpeech_IsInhabitedFarAndWide(u8);
static void Task_OakSpeech_IStudyPokemon(u8);
static void Task_OakSpeech_ReturnNidoranFToPokeBall(u8);
static void Task_OakSpeech_TellMeALittleAboutYourself(u8);
static void Task_OakSpeech_FadeOutOak(u8);
static void Task_OakSpeech_AskPlayerGender(u8);
static void Task_OakSpeech_ShowGenderOptions(u8);
static void Task_OakSpeech_HandleGenderInput(u8);
static void Task_OakSpeech_ClearGenderWindows(u8);
static void Task_OakSpeech_LoadPlayerPic(u8);
static void Task_OakSpeech_YourNameWhatIsIt(u8);
static void Task_OakSpeech_FadeOutForPlayerNamingScreen(u8);
static void Task_OakSpeech_HandleRivalNameInput(u8);
static void Task_OakSpeech_DoNamingScreen(u8);
static void Task_OakSpeech_ConfirmName(u8);
static void Task_OakSpeech_HandleConfirmNameInput(u8);
static void Task_OakSpeech_FadeOutPlayerPic(u8);
static void Task_OakSpeech_FadeOutRivalPic(u8);
static void Task_OakSpeech_FadeInRivalPic(u8);
static void Task_OakSpeech_AskRivalsName(u8);
static void Task_OakSpeech_ReshowPlayersPic(u8);
static void Task_OakSpeech_LetsGo(u8);
static void Task_OakSpeech_FadeOutBGM(u8);
static void Task_OakSpeech_SetUpExitAnimation(u8);
static void Task_OakSpeech_SetUpShrinkPlayerPic(u8);
static void Task_OakSpeech_ShrinkPlayerPic(u8);
static void Task_OakSpeech_SetUpDestroyPlatformSprites(u8);
static void Task_OakSpeech_DestroyPlatformSprites(u8);
static void Task_OakSpeech_SetUpFadePlayerPicWhite(u8);
static void Task_OakSpeech_FadePlayerPicWhite(u8);
static void Task_OakSpeech_FadePlayerPicToBlack(u8);
static void Task_OakSpeech_WaitForFade(u8);
static void Task_OakSpeech_FreeResources(u8);

static void CB2_ReturnFromNamingScreen(void);
static void CreateNidoranFSprite(u8);
static void CreatePlatformSprites(u8);
static void DestroyPlatformSprites(u8);
static void LoadTrainerPic(u16);
static void LoadPlayerPic(void);
static void ClearTrainerPic(void);
static void CreateFadeInTask(u8, u8);
static void CreateFadeOutTask(u8, u8);
static void PrintNameChoiceOptions(u8, u8);
static void GetDefaultName(u8, u8);

static const u16 sOakSpeech_Background_Pal[] = INCGFX_U16("graphics/oak_speech/bg.pal", ".gbapal");
static const u32 sOakSpeech_Background_Tiles[] = INCGFX_U32("graphics/oak_speech/oak_speech_bg.png", ".4bpp.lz");
static const u32 sOakSpeech_Background_Tilemap[] = INCGFX_U32("graphics/oak_speech/oak_speech_bg.bin", ".lz");
static const u16 sOakSpeech_Brendan_Pal[] = INCGFX_U16("graphics/oak_speech/brendan/pal.pal", ".gbapal");
static const u32 sOakSpeech_Brendan_Tiles[] = INCGFX_U32("graphics/oak_speech/brendan/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_May_Pal[] = INCGFX_U16("graphics/oak_speech/may/pal.pal", ".gbapal");
static const u32 sOakSpeech_May_Tiles[] = INCGFX_U32("graphics/oak_speech/may/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_Red_Pal[] = INCGFX_U16("graphics/oak_speech/red/pal.pal", ".gbapal");
static const u32 sOakSpeech_Red_Tiles[] = INCGFX_U32("graphics/oak_speech/red/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_Leaf_Pal[] = INCGFX_U16("graphics/oak_speech/leaf/pal.pal", ".gbapal");
static const u32 sOakSpeech_Leaf_Tiles[] = INCGFX_U32("graphics/oak_speech/leaf/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_Oak_Pal[] = INCGFX_U16("graphics/oak_speech/oak/pal.pal", ".gbapal");
static const u32 sOakSpeech_Oak_Tiles[] = INCGFX_U32("graphics/oak_speech/oak/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_Rival_Pal[] = INCGFX_U16("graphics/oak_speech/rival/pal.pal", ".gbapal");
static const u32 sOakSpeech_Rival_Tiles[] = INCGFX_U32("graphics/oak_speech/rival/pic.png", ".8bpp.lz");
static const u16 sOakSpeech_Platform_Pal[] = INCGFX_U16("graphics/oak_speech/platform.pal", ".gbapal");
static const u32 sOakSpeech_Platform_Gfx[] = INCGFX_U32("graphics/oak_speech/platform.png", ".4bpp.lz");

static const u8 sText_WelcomeToTheWorld[] = _("Hello, there!\nGlad to meet you!\pWelcome to the world of POKéMON!\pMy name is OAK.\pPeople affectionately refer to me\nas the POKéMON PROFESSOR.\p");
static const u8 sText_ThisWorld[] = _("This world…");
static const u8 sText_IsInhabitedFarAndWide[] = _("…is inhabited far and wide by\ncreatures called POKéMON.\p");
static const u8 sText_IStudyPokemon[] = _("For some people, POKéMON are pets.\nOthers use them for battling.\pAs for myself…\pI study POKéMON as a profession.\p");
static const u8 sText_TellMeALittleAboutYourself[] = _("But first, tell me a little about\nyourself.\p");
static const u8 sText_AskPlayerGender[] = _("Now tell me. Are you a boy?\nOr are you a girl?");
static const u8 sText_YourNameWhatIsIt[] = _("Let's begin with your name.\nWhat is it?\p");
static const u8 sText_SoYourNameIsPlayer[] = _("Right…\nSo your name is {PLAYER}.");
static const u8 sText_WhatWasHisName[] = _("This is my grandson.\pHe's been your rival since you both\nwere babies.\p…Erm, what was his name now?");
static const u8 sText_YourRivalsNameWhatWasIt[] = _("Your rival's name, what was it now?");
static const u8 sText_ConfirmRivalName[] = _("…Er, was it {STR_VAR_1}?");
static const u8 sText_RememberRivalsName[] = _("That's right! I remember now!\nHis name is {STR_VAR_1}!\p");
static const u8 sText_LetsGo[] = _("{PLAYER}!\pYour very own POKéMON legend is\nabout to unfold!\pA world of dreams and adventures\nwith POKéMON awaits! Let's go!");
static const u8 sText_NewName[] = _("NEW NAME");

static const u8 sText_Brendan[] = _("BRENDAN");
static const u8 sText_May[] = _("MAY");
static const u8 sText_Red[] = _("RED");
static const u8 sText_Leaf[] = _("LEAF");

static const u8 sNameChoice_Red[] = _("RED");
static const u8 sNameChoice_Fire[] = _("FIRE");
static const u8 sNameChoice_Ash[] = _("ASH");
static const u8 sNameChoice_Kene[] = _("KENE");
static const u8 sNameChoice_Geki[] = _("GEKI");
static const u8 sNameChoice_Jak[] = _("JAK");
static const u8 sNameChoice_Janne[] = _("JANNE");
static const u8 sNameChoice_Jonn[] = _("JONN");
static const u8 sNameChoice_Kamon[] = _("KAMON");
static const u8 sNameChoice_Karl[] = _("KARL");
static const u8 sNameChoice_Taylor[] = _("TAYLOR");
static const u8 sNameChoice_Oscar[] = _("OSCAR");
static const u8 sNameChoice_Hiro[] = _("HIRO");
static const u8 sNameChoice_Max[] = _("MAX");
static const u8 sNameChoice_Jon[] = _("JON");
static const u8 sNameChoice_Ralph[] = _("RALPH");
static const u8 sNameChoice_Kay[] = _("KAY");
static const u8 sNameChoice_Tosh[] = _("TOSH");
static const u8 sNameChoice_Roak[] = _("ROAK");
static const u8 sNameChoice_Omi[] = _("OMI");
static const u8 sNameChoice_Jodi[] = _("JODI");
static const u8 sNameChoice_Amanda[] = _("AMANDA");
static const u8 sNameChoice_Hillary[] = _("HILLARY");
static const u8 sNameChoice_Makey[] = _("MAKEY");
static const u8 sNameChoice_Michi[] = _("MICHI");
static const u8 sNameChoice_Paula[] = _("PAULA");
static const u8 sNameChoice_June[] = _("JUNE");
static const u8 sNameChoice_Cassie[] = _("CASSIE");
static const u8 sNameChoice_Rey[] = _("REY");
static const u8 sNameChoice_Seda[] = _("SEDA");
static const u8 sNameChoice_Kiko[] = _("KIKO");
static const u8 sNameChoice_Mina[] = _("MINA");
static const u8 sNameChoice_Norie[] = _("NORIE");
static const u8 sNameChoice_Sai[] = _("SAI");
static const u8 sNameChoice_Momo[] = _("MOMO");
static const u8 sNameChoice_Suzi[] = _("SUZI");
static const u8 sNameChoice_Green[] = _("GREEN");
static const u8 sNameChoice_Gary[] = _("GARY");
static const u8 sNameChoice_Kaz[] = _("KAZ");
static const u8 sNameChoice_Toru[] = _("TORU");

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 1,
        .paletteMode = 1,
        .priority = 1,
        .baseTile = 0
    }
};

enum
{
    WIN_INTRO_CHARACTERS,
    WIN_INTRO_YESNO,
    WIN_INTRO_NAMES,
};

static const struct WindowTemplate sIntro_WindowTemplates[] =
{
    [WIN_INTRO_CHARACTERS] =
    {
        .bg = 0,
        .tilemapLeft = 18,
        .tilemapTop = 5,
        .width = 9,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 1
    },
    [WIN_INTRO_YESNO] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x125
    },
    [WIN_INTRO_NAMES] =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 12,
        .height = 10,
        .paletteNum = 15,
        .baseBlock = 1
    },
};

// The player characters that can be chosen when Oak asks "Are you a boy or a girl?"
enum
{
    PLAYER_CHOICE_BRENDAN,
    PLAYER_CHOICE_MAY,
    PLAYER_CHOICE_RED,
    PLAYER_CHOICE_LEAF,
    PLAYER_CHOICE_COUNT
};

#define PLAYER_CHOICE_GENDER(choice)   ((choice) % GENDER_COUNT)
#define PLAYER_CHOICE_COSTUME(choice)  ((choice) / GENDER_COUNT)

static const struct MenuAction sMenuActions_PlayerChoice[PLAYER_CHOICE_COUNT] =
{
    [PLAYER_CHOICE_BRENDAN] = {sText_Brendan, {NULL}},
    [PLAYER_CHOICE_MAY]     = {sText_May,     {NULL}},
    [PLAYER_CHOICE_RED]     = {sText_Red,     {NULL}},
    [PLAYER_CHOICE_LEAF]    = {sText_Leaf,    {NULL}},
};

#define GFX_TAG_PLATFORM 0x1000
#define PAL_TAG_PLATFORM 0x1000

enum
{
    PLATFORM_LEFT,
    PLATFORM_MIDDLE,
    PLATFORM_RIGHT,
    NUM_PLATFORM_SPRITES,
};

static const struct CompressedSpriteSheet sOakSpeech_Platform_SpriteSheet =
{
    .data = sOakSpeech_Platform_Gfx,
    .size = 0x600,
    .tag = GFX_TAG_PLATFORM
};

static const struct SpritePalette sOakSpeech_Platform_SpritePalette =
{
    .data = sOakSpeech_Platform_Pal,
    .tag = PAL_TAG_PLATFORM
};

static const struct OamData sOam_Platform =
{
    .objMode = ST_OAM_OBJ_BLEND,
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
};

static const union AnimCmd sOakSpeech_PlatformLeft_Anim[] =
{
    ANIMCMD_FRAME( 0, 0),
    ANIMCMD_END
};

static const union AnimCmd sOakSpeech_PlatformMiddle_Anim[] =
{
    ANIMCMD_FRAME(16, 0),
    ANIMCMD_END
};

static const union AnimCmd sOakSpeech_PlatformRight_Anim[] =
{
    ANIMCMD_FRAME(32, 0),
    ANIMCMD_END
};

static const union AnimCmd *const sOakSpeech_PlatformLeft_Anims[] =
{
    sOakSpeech_PlatformLeft_Anim
};

static const union AnimCmd *const sOakSpeech_PlatformMiddle_Anims[] =
{
    sOakSpeech_PlatformMiddle_Anim
};

static const union AnimCmd *const sOakSpeech_PlatformRight_Anims[] =
{
    sOakSpeech_PlatformRight_Anim
};

static const struct SpriteTemplate sOakSpeech_Platform_SpriteTemplates[NUM_PLATFORM_SPRITES] =
{
    [PLATFORM_LEFT] =
    {
        .tileTag = GFX_TAG_PLATFORM,
        .paletteTag = PAL_TAG_PLATFORM,
        .oam = &sOam_Platform,
        .anims = sOakSpeech_PlatformLeft_Anims,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy
    },
    [PLATFORM_MIDDLE] =
    {
        .tileTag = GFX_TAG_PLATFORM,
        .paletteTag = PAL_TAG_PLATFORM,
        .oam = &sOam_Platform,
        .anims = sOakSpeech_PlatformMiddle_Anims,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy
    },
    [PLATFORM_RIGHT] =
    {
        .tileTag = GFX_TAG_PLATFORM,
        .paletteTag = PAL_TAG_PLATFORM,
        .oam = &sOam_Platform,
        .anims = sOakSpeech_PlatformRight_Anims,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy
    },
};

#define NUM_NAME_CHOICES 4

static const u8 *const sMaleNameChoices[] =
{
    sNameChoice_Red,
    sNameChoice_Fire,
    sNameChoice_Ash,
    sNameChoice_Kene,
    sNameChoice_Geki,
    sNameChoice_Jak,
    sNameChoice_Janne,
    sNameChoice_Jonn,
    sNameChoice_Kamon,
    sNameChoice_Karl,
    sNameChoice_Taylor,
    sNameChoice_Oscar,
    sNameChoice_Hiro,
    sNameChoice_Max,
    sNameChoice_Jon,
    sNameChoice_Ralph,
    sNameChoice_Kay,
    sNameChoice_Tosh,
    sNameChoice_Roak
};

static const u8 *const sFemaleNameChoices[] =
{
    sNameChoice_Red,
    sNameChoice_Fire,
    sNameChoice_Omi,
    sNameChoice_Jodi,
    sNameChoice_Amanda,
    sNameChoice_Hillary,
    sNameChoice_Makey,
    sNameChoice_Michi,
    sNameChoice_Paula,
    sNameChoice_June,
    sNameChoice_Cassie,
    sNameChoice_Rey,
    sNameChoice_Seda,
    sNameChoice_Kiko,
    sNameChoice_Mina,
    sNameChoice_Norie,
    sNameChoice_Sai,
    sNameChoice_Momo,
    sNameChoice_Suzi
};

static const u8 *const sRivalNameChoices[NUM_NAME_CHOICES] =
{
    sNameChoice_Green,
    sNameChoice_Gary,
    sNameChoice_Kaz,
    sNameChoice_Toru
};

enum
{
    BRENDAN_PIC,
    MAY_PIC,
    RED_PIC,
    LEAF_PIC,
    RIVAL_PIC,
    OAK_PIC
};

static void VBlankCB_NewGameScene(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_NewGameScene(void)
{
    RunTasks();
    RunTextPrinters();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

#define tSpriteTimer                data[0]
#define tTrainerPicPosX             data[1]
#define tTrainerPicFadeState        data[2]
#define tTimer                      data[3]
#define tNidoranFSpriteId           data[4]
#define tPokeBallSpriteId           data[6]
#define tPlatformSpriteId(i)        data[7 + i] // The platform is built of three sprites,
                                 // data[8]     // so these are used to hold their sprite IDs
                                 // data[9]     //
#define tMenuWindowId               data[13]

static void Task_NewGameScene(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        DmaFill16(3, 0, VRAM, VRAM_SIZE);
        DmaFill32(3, 0, OAM, OAM_SIZE);
        DmaFill16(3, 0, PLTT + sizeof(u16), PLTT_SIZE - 2);
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetAllPicSprites();
        ResetTempTileDataBuffers();
        break;
    case 1:
        sOakSpeechResources = AllocZeroed(sizeof(*sOakSpeechResources));
        break;
    case 2:
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_WININ, 0);
        SetGpuReg(REG_OFFSET_WINOUT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        break;
    case 3:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(1, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        SetBgTilemapBuffer(1, sOakSpeechResources->bg1TilemapBuffer);
        SetBgTilemapBuffer(2, sOakSpeechResources->bg2TilemapBuffer);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        gSpriteCoordOffsetX = 0;
        gSpriteCoordOffsetY = 0;
        break;
    case 4:
        gPaletteFade.bufferTransferDisabled = TRUE;
        InitStandardTextBoxWindows();
        InitTextBoxGfxAndPrinters();
        LoadPalette(sOakSpeech_Background_Pal, BG_PLTT_ID(0), sizeof(sOakSpeech_Background_Pal));
        break;
    case 5:
        gTextFlags.canABSpeedUpPrint = TRUE;
        DecompressAndCopyTileDataToVram(1, sOakSpeech_Background_Tiles, 0, 0, 0);
        break;
    case 6:
        if (FreeTempTileDataBuffersIfPossible())
            return;
        ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 32);
        CopyToBgTilemapBuffer(1, sOakSpeech_Background_Tilemap, 0, 0);
        CopyBgTilemapBufferToVram(1);
        break;
    case 7:
        gPaletteFade.bufferTransferDisabled = FALSE;
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(1);
        SetVBlankCallback(VBlankCB_NewGameScene);
        gTasks[taskId].tTimer = 30;
        gTasks[taskId].func = Task_OakSpeech_Init;
        gMain.state = 0;
        return;
    }

    gMain.state++;
}

void StartNewGameScene(void)
{
    gPlttBufferUnfaded[0] = RGB_BLACK;
    gPlttBufferFaded[0]   = RGB_BLACK;
    gMain.state = 0;
    CreateTask(Task_NewGameScene, 0);
    SetMainCallback2(CB2_NewGameScene);
}

static void Task_OakSpeech_Init(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTimer != 0)
    {
        tTimer--;
    }
    else
    {
        CreateNidoranFSprite(taskId);
        LoadTrainerPic(OAK_PIC);
        CreatePlatformSprites(taskId);
        PlayBGM(MUS_RG_ROUTE24);
        BeginNormalPaletteFade(PALETTES_ALL, 5, 16, 0, RGB_BLACK);
        tTimer = 80;
        ShowBg(2);
        gTasks[taskId].func = Task_OakSpeech_WelcomeToTheWorld;
    }
}

static void OakSpeechPrintMessage(const u8 *str)
{
    DrawDialogueFrame(WIN_INTRO_TEXTBOX, FALSE);
    StringExpandPlaceholders(gStringVar4, str);
    AddTextPrinterForMessage(TRUE);
    CopyWindowToVram(WIN_INTRO_TEXTBOX, COPYWIN_FULL);
}

static void Task_OakSpeech_WelcomeToTheWorld(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        if (tTimer != 0)
        {
            tTimer--;
        }
        else
        {
            OakSpeechPrintMessage(sText_WelcomeToTheWorld);
            gTasks[taskId].func = Task_OakSpeech_ThisWorld;
        }
    }
}

static void Task_OakSpeech_ThisWorld(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        OakSpeechPrintMessage(sText_ThisWorld);
        gTasks[taskId].tTimer = 30;
        gTasks[taskId].func = Task_OakSpeech_ReleaseNidoranFFromPokeBall;
    }
}

static void Task_OakSpeech_ReleaseNidoranFFromPokeBall(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 spriteId;

    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        if (tTimer != 0)
            tTimer--;
        spriteId = gTasks[taskId].tNidoranFSpriteId;
        gSprites[spriteId].invisible = FALSE;
        gSprites[spriteId].tSpriteTimer = 0;
        // Emerald's version also plays the Pokémon's cry when it comes out
        CreatePokeballSpriteToReleaseMon(spriteId, gSprites[spriteId].oam.paletteNum, 100, 66, 0, 0, 32, 0xFFFF1FFF, INTRO_SPECIES);
        gTasks[taskId].func = Task_OakSpeech_IsInhabitedFarAndWide;
        gTasks[taskId].tTimer = 0;
    }
}

static void Task_OakSpeech_IsInhabitedFarAndWide(u8 taskId)
{
    if (IsCryFinished())
    {
        if (gTasks[taskId].tTimer >= 96)
            gTasks[taskId].func = Task_OakSpeech_IStudyPokemon;
    }
    if (gTasks[taskId].tTimer < 0x4000)
    {
        gTasks[taskId].tTimer++;
        if (gTasks[taskId].tTimer == 32)
            OakSpeechPrintMessage(sText_IsInhabitedFarAndWide);
    }
}

static void Task_OakSpeech_IStudyPokemon(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        OakSpeechPrintMessage(sText_IStudyPokemon);
        gTasks[taskId].func = Task_OakSpeech_ReturnNidoranFToPokeBall;
    }
}

static void Task_OakSpeech_ReturnNidoranFToPokeBall(u8 taskId)
{
    u8 spriteId;

    spriteId = gTasks[taskId].tNidoranFSpriteId;
    // Wait for Nidoran's front animation to finish so it doesn't fight the return anim
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX) && gSprites[spriteId].callback == SpriteCallbackDummy)
    {
        ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
        // Emerald's release plays the front animation, which leaves the sprite with the
        // animation's own affine anims, paused. Restore the battle sprite affine anims
        // so the shrink back into the Poke Ball plays.
        gSprites[spriteId].affineAnims = gAffineAnims_BattleSpriteOpponentSide;
        gSprites[spriteId].affineAnimPaused = FALSE;
        gTasks[taskId].tPokeBallSpriteId = CreateTradePokeballSprite(spriteId, gSprites[spriteId].oam.paletteNum, 100, 66, 0, 0, 32, 0xFFFF1F3F);
        gTasks[taskId].tTimer = 48;
        gTasks[taskId].tSpriteTimer = 64;
        gTasks[taskId].func = Task_OakSpeech_TellMeALittleAboutYourself;
    }
}

static void Task_OakSpeech_TellMeALittleAboutYourself(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tSpriteTimer != 0)
    {
        if (tSpriteTimer < 24)
            gSprites[tNidoranFSpriteId].y--;
        tSpriteTimer--;
    }
    else
    {
        if (tTimer == 48)
        {
            FreeAndDestroyMonPicSprite(tNidoranFSpriteId);
            DestroySprite(&gSprites[tPokeBallSpriteId]);
        }
        if (tTimer != 0)
        {
            tTimer--;
        }
        else
        {
            OakSpeechPrintMessage(sText_TellMeALittleAboutYourself);
            gTasks[taskId].func = Task_OakSpeech_FadeOutOak;
        }
    }
}

static void Task_OakSpeech_FadeOutOak(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
        CreateFadeInTask(taskId, 2);
        tTimer = 48;
        gTasks[taskId].func = Task_OakSpeech_AskPlayerGender;
    }
}

static void Task_OakSpeech_AskPlayerGender(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTrainerPicFadeState != 0)
    {
        if (tTimer != 0)
        {
            tTimer--;
        }
        else
        {
            tTrainerPicPosX = -60;
            ClearTrainerPic();
            OakSpeechPrintMessage(sText_AskPlayerGender);
            gTasks[taskId].func = Task_OakSpeech_ShowGenderOptions;
        }
    }
}

static void Task_OakSpeech_ShowGenderOptions(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        u8 windowId = AddWindow(&sIntro_WindowTemplates[WIN_INTRO_CHARACTERS]);

        gTasks[taskId].tMenuWindowId = windowId;
        DrawStdWindowFrame(windowId, FALSE);
        PrintMenuTable(windowId, PLAYER_CHOICE_COUNT, sMenuActions_PlayerChoice);
        InitMenuInUpperLeftCornerNormal(windowId, PLAYER_CHOICE_COUNT, 0);
        CopyWindowToVram(windowId, COPYWIN_FULL);
        gTasks[taskId].func = Task_OakSpeech_HandleGenderInput;
    }
}

static void Task_OakSpeech_HandleGenderInput(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrap();

    switch (input)
    {
    case PLAYER_CHOICE_BRENDAN:
    case PLAYER_CHOICE_MAY:
    case PLAYER_CHOICE_RED:
    case PLAYER_CHOICE_LEAF:
        PlaySE(SE_SELECT);
        gSaveBlock2Ptr->playerGender = PLAYER_CHOICE_GENDER(input);
        gSaveBlock2Ptr->playerCostume = PLAYER_CHOICE_COSTUME(input);
        gTasks[taskId].func = Task_OakSpeech_ClearGenderWindows;
        break;
    }
}

static void Task_OakSpeech_ClearGenderWindows(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ClearStdWindowAndFrameToTransparent(tMenuWindowId, TRUE);
    RemoveWindow(tMenuWindowId);
    tMenuWindowId = WIN_INTRO_TEXTBOX;
    ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
    gTasks[taskId].func = Task_OakSpeech_LoadPlayerPic;
}

static void Task_OakSpeech_LoadPlayerPic(u8 taskId)
{
    LoadPlayerPic();
    CreateFadeOutTask(taskId, 2);
    gTasks[taskId].tTimer = 32;
    gTasks[taskId].func = Task_OakSpeech_YourNameWhatIsIt;
}

static void Task_OakSpeech_YourNameWhatIsIt(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTrainerPicFadeState != 0)
    {
        if (tTimer != 0)
        {
            tTimer--;
        }
        else
        {
            tTrainerPicPosX = 0;
            OakSpeechPrintMessage(sText_YourNameWhatIsIt);
            gTasks[taskId].func = Task_OakSpeech_FadeOutForPlayerNamingScreen;
        }
    }
}

static void Task_OakSpeech_FadeOutForPlayerNamingScreen(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        sOakSpeechResources->hasPlayerBeenNamed = FALSE;
        gTasks[taskId].func = Task_OakSpeech_DoNamingScreen;
    }
}

static void Task_OakSpeech_MoveRivalDisplayNameOptions(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        if (tTrainerPicPosX > -60)
        {
            tTrainerPicPosX -= 2;
            gSpriteCoordOffsetX += 2;
            ChangeBgX(2, 0x200, BG_COORD_SUB);
        }
        else
        {
            tTrainerPicPosX = -60;
            PrintNameChoiceOptions(taskId, sOakSpeechResources->hasPlayerBeenNamed);
            gTasks[taskId].func = Task_OakSpeech_HandleRivalNameInput;
        }
    }
}

static void Task_OakSpeech_RepeatNameQuestion(u8 taskId)
{
    PrintNameChoiceOptions(taskId, sOakSpeechResources->hasPlayerBeenNamed);
    if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
        OakSpeechPrintMessage(sText_YourNameWhatIsIt);
    else
        OakSpeechPrintMessage(sText_YourRivalsNameWhatWasIt);
    gTasks[taskId].func = Task_OakSpeech_HandleRivalNameInput;
}

#define tNameNotConfirmed data[15]

static void Task_OakSpeech_HandleRivalNameInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s8 input = Menu_ProcessInput();

    switch (input)
    {
    case 0: // NEW NAME
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_OakSpeech_DoNamingScreen;
        break;
    case 1: // Default name options
    case 2: //
    case 3: //
    case 4: //
        PlaySE(SE_SELECT);
        ClearStdWindowAndFrameToTransparent(tMenuWindowId, TRUE);
        RemoveWindow(tMenuWindowId);
        GetDefaultName(sOakSpeechResources->hasPlayerBeenNamed, input - 1);
        tNameNotConfirmed = TRUE;
        gTasks[taskId].func = Task_OakSpeech_ConfirmName;
        break;
    case MENU_B_PRESSED:
        break;
    }
}

static void Task_OakSpeech_DoNamingScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        GetDefaultName(sOakSpeechResources->hasPlayerBeenNamed, 0);
        if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
        {
            DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName, gSaveBlock2Ptr->playerGender, 0, 0, CB2_ReturnFromNamingScreen);
        }
        else
        {
            ClearStdWindowAndFrameToTransparent(gTasks[taskId].tMenuWindowId, TRUE);
            RemoveWindow(gTasks[taskId].tMenuWindowId);
            DoNamingScreen(NAMING_SCREEN_RIVAL, gSaveBlock2Ptr->rivalName, 0, 0, 0, CB2_ReturnFromNamingScreen);
        }
        DestroyPlatformSprites(taskId);
        FreeAllWindowBuffers();
    }
}

static void Task_OakSpeech_ConfirmName(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        if (tNameNotConfirmed == TRUE)
        {
            if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
            {
                OakSpeechPrintMessage(sText_SoYourNameIsPlayer);
            }
            else
            {
                StringCopy(gStringVar1, gSaveBlock2Ptr->rivalName);
                OakSpeechPrintMessage(sText_ConfirmRivalName);
            }
            tNameNotConfirmed = FALSE;
            tTimer = 25;
        }
        else if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
        {
            if (tTimer != 0)
            {
                tTimer--;
            }
            else
            {
                // In the top left corner, away from the trainer pic
                CreateYesNoMenu(&sIntro_WindowTemplates[WIN_INTRO_YESNO], STD_WINDOW_BASE_TILE_NUM, STD_WINDOW_PALETTE_NUM, 0);
                gTasks[taskId].func = Task_OakSpeech_HandleConfirmNameInput;
            }
        }
    }
}

static void Task_OakSpeech_HandleConfirmNameInput(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrapClearOnChoose();

    switch (input)
    {
    case 0: // YES
        PlaySE(SE_SELECT);
        gTasks[taskId].tTimer = 40;
        if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
        {
            ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
            CreateFadeInTask(taskId, 2);
            gTasks[taskId].func = Task_OakSpeech_FadeOutPlayerPic;
        }
        else
        {
            StringCopy(gStringVar1, gSaveBlock2Ptr->rivalName);
            OakSpeechPrintMessage(sText_RememberRivalsName);
            gTasks[taskId].func = Task_OakSpeech_FadeOutRivalPic;
        }
        break;
    case 1: // NO
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
            gTasks[taskId].func = Task_OakSpeech_FadeOutForPlayerNamingScreen;
        else
            gTasks[taskId].func = Task_OakSpeech_RepeatNameQuestion;
        break;
    }
}

static void Task_OakSpeech_FadeOutPlayerPic(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTrainerPicFadeState != 0)
    {
        ClearTrainerPic();
        if (tTimer != 0)
            tTimer--;
        else
            gTasks[taskId].func = Task_OakSpeech_FadeInRivalPic;
    }
}

static void Task_OakSpeech_FadeOutRivalPic(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        ClearDialogWindowAndFrame(WIN_INTRO_TEXTBOX, TRUE);
        CreateFadeInTask(taskId, 2);
        gTasks[taskId].func = Task_OakSpeech_ReshowPlayersPic;
    }
}

static void Task_OakSpeech_FadeInRivalPic(u8 taskId)
{
    ChangeBgX(2, 0, BG_COORD_SET);
    gTasks[taskId].tTrainerPicPosX = 0;
    gSpriteCoordOffsetX = 0;
    LoadTrainerPic(RIVAL_PIC);
    CreateFadeOutTask(taskId, 2);
    gTasks[taskId].func = Task_OakSpeech_AskRivalsName;
}

static void Task_OakSpeech_AskRivalsName(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTrainerPicFadeState != 0)
    {
        OakSpeechPrintMessage(sText_WhatWasHisName);
        sOakSpeechResources->hasPlayerBeenNamed = TRUE;
        gTasks[taskId].func = Task_OakSpeech_MoveRivalDisplayNameOptions;
    }
}

static void Task_OakSpeech_ReshowPlayersPic(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (tTrainerPicFadeState != 0)
    {
        ClearTrainerPic();
        if (tTimer != 0)
        {
            tTimer--;
        }
        else
        {
            LoadPlayerPic();
            gTasks[taskId].tTrainerPicPosX = 0;
            gSpriteCoordOffsetX = 0;
            ChangeBgX(2, 0, BG_COORD_SET);
            CreateFadeOutTask(taskId, 2);
            gTasks[taskId].func = Task_OakSpeech_LetsGo;
        }
    }
}

static void Task_OakSpeech_LetsGo(u8 taskId)
{
    if (gTasks[taskId].tTrainerPicFadeState != 0)
    {
        OakSpeechPrintMessage(sText_LetsGo);
        gTasks[taskId].tTimer = 30;
        gTasks[taskId].func = Task_OakSpeech_FadeOutBGM;
    }
}

static void Task_OakSpeech_FadeOutBGM(u8 taskId)
{
    if (!IsTextPrinterActive(WIN_INTRO_TEXTBOX))
    {
        if (gTasks[taskId].tTimer != 0)
        {
            gTasks[taskId].tTimer--;
        }
        else
        {
            FadeOutBGM(4);
            gTasks[taskId].func = Task_OakSpeech_SetUpExitAnimation;
        }
    }
}

static void Task_OakSpeech_SetUpExitAnimation(u8 taskId)
{
    sOakSpeechResources->shrinkTimer = 0;
    Task_OakSpeech_SetUpDestroyPlatformSprites(taskId);
    Task_OakSpeech_SetUpFadePlayerPicWhite(taskId);
    Task_OakSpeech_SetUpShrinkPlayerPic(taskId);
}

#define tPlayerPicFadeOutTimer data[0]
#define tScaleDelta            data[2]
#define tPlayerIsShrunk        data[15]

static void Task_OakSpeech_SetUpShrinkPlayerPic(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    SetBgAttribute(2, BG_ATTR_WRAPAROUND, 1);
    tPlayerPicFadeOutTimer = 0;
    tScaleDelta = 256;
    tPlayerIsShrunk = FALSE;
    gTasks[taskId].func = Task_OakSpeech_ShrinkPlayerPic;
}

static void Task_OakSpeech_ShrinkPlayerPic(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s16 x, y;
    u16 oldScaleDelta;

    sOakSpeechResources->shrinkTimer++;
    if (sOakSpeechResources->shrinkTimer % 20 == 0)
    {
        if (sOakSpeechResources->shrinkTimer == 40)
            PlaySE(SE_WARP_IN);
        oldScaleDelta = tScaleDelta;
        tScaleDelta -= 32;
        x = Q_8_8_inv(oldScaleDelta - 8);
        y = Q_8_8_inv(tScaleDelta - 16);
        SetBgAffine(2, 0x7800, 0x5400, 120, 84, x, y, 0);
        if (tScaleDelta <= 96)
        {
            tPlayerIsShrunk = TRUE;
            tPlayerPicFadeOutTimer = 36;
            gTasks[taskId].func = Task_OakSpeech_FadePlayerPicToBlack;
        }
    }
}

#define tParentTaskId  data[0]
#define tBGFadeStarted data[1]

static void Task_OakSpeech_SetUpDestroyPlatformSprites(u8 taskId)
{
    u8 i;
    u8 taskId2 = CreateTask(Task_OakSpeech_DestroyPlatformSprites, 1);

    gTasks[taskId2].tParentTaskId = taskId;
    gTasks[taskId2].tBGFadeStarted = 0;
    for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
        gTasks[taskId2].tPlatformSpriteId(i) = gTasks[taskId].tPlatformSpriteId(i);
    BeginNormalPaletteFade(PALETTES_OBJECTS | 0x0FCF, 4, 0, 16, RGB_BLACK);
}

static void Task_OakSpeech_DestroyPlatformSprites(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        if (tBGFadeStarted != 0)
        {
            DestroyPlatformSprites(taskId);
            DestroyTask(taskId);
        }
        else
        {
            tBGFadeStarted++;
            BeginNormalPaletteFade(0x0000 | 0xF000, 0, 0, 16, RGB_BLACK);
        }
    }
}

#undef tParentTaskId
#undef tBGFadeStarted

#define tPlayerPicFadeWhiteTimer data[0]
#define tUnderflowingTimer       data[1]
#define tSecondaryTimer          data[2]
#define tBlendCoefficient        data[14]

static void Task_OakSpeech_SetUpFadePlayerPicWhite(u8 taskId)
{
    u8 taskId2 = CreateTask(Task_OakSpeech_FadePlayerPicWhite, 2);
    s16 *data = gTasks[taskId2].data;

    tPlayerPicFadeWhiteTimer = 8;
    tUnderflowingTimer = 0;
    tSecondaryTimer = 8;
    tBlendCoefficient = 0;
}

static void Task_OakSpeech_FadePlayerPicWhite(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 i;

    if (tPlayerPicFadeWhiteTimer != 0)
    {
        tPlayerPicFadeWhiteTimer--;
    }
    else
    {
        if (tUnderflowingTimer <= 0 && tSecondaryTimer != 0)
            tSecondaryTimer--;
        BlendPalette(BG_PLTT_ID(4), 0x20, tBlendCoefficient, RGB_WHITE);
        tBlendCoefficient++;
        tUnderflowingTimer--;
        tPlayerPicFadeWhiteTimer = tSecondaryTimer;
        if (tBlendCoefficient > 14)
        {
            for (i = 0; i < 32; i++)
            {
                gPlttBufferFaded[i + BG_PLTT_ID(4)] = RGB_WHITE;
                gPlttBufferUnfaded[i + BG_PLTT_ID(4)] = RGB_WHITE;
            }
            DestroyTask(taskId);
        }
    }
}

static void Task_OakSpeech_FadePlayerPicToBlack(u8 taskId)
{
    if (gTasks[taskId].tPlayerPicFadeOutTimer != 0)
    {
        gTasks[taskId].tPlayerPicFadeOutTimer--;
    }
    else
    {
        BeginNormalPaletteFade(0x0000 | 0x0030, 2, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_OakSpeech_WaitForFade;
    }
}

static void Task_OakSpeech_WaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OakSpeech_FreeResources;
}

static void Task_OakSpeech_FreeResources(u8 taskId)
{
    FreeAllWindowBuffers();
    ResetAllPicSprites();
    FREE_AND_SET_NULL(sOakSpeechResources);
    gTextFlags.canABSpeedUpPrint = FALSE;
    SetMainCallback2(CB2_NewGame);
    DestroyTask(taskId);
}

static void CB2_ReturnFromNamingScreen(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        DmaFill16(3, 0, VRAM, VRAM_SIZE);
        DmaFill32(3, 0, OAM, OAM_SIZE);
        DmaFill16(3, RGB_BLACK, PLTT + sizeof(u16), PLTT_SIZE - sizeof(u16));
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetTempTileDataBuffers();
        break;
    case 1:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(1, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        SetBgTilemapBuffer(1, sOakSpeechResources->bg1TilemapBuffer);
        SetBgTilemapBuffer(2, sOakSpeechResources->bg2TilemapBuffer);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        break;
    case 2:
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, 0);
        SetGpuReg(REG_OFFSET_WINOUT, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        break;
    case 3:
        FreeAllWindowBuffers();
        InitStandardTextBoxWindows();
        InitTextBoxGfxAndPrinters();
        LoadPalette(sOakSpeech_Background_Pal, BG_PLTT_ID(0), sizeof(sOakSpeech_Background_Pal));
        break;
    case 4:
        DecompressAndCopyTileDataToVram(1, sOakSpeech_Background_Tiles, 0, 0, 0);
        break;
    case 5:
        if (FreeTempTileDataBuffersIfPossible())
            return;
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
        CopyToBgTilemapBuffer(1, sOakSpeech_Background_Tilemap, 0, 0);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 30, 20);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        break;
    case 6:
        taskId = CreateTask(Task_OakSpeech_ConfirmName, 0);
        if (sOakSpeechResources->hasPlayerBeenNamed == FALSE)
            LoadPlayerPic();
        else
            LoadTrainerPic(RIVAL_PIC);
        gTasks[taskId].tTrainerPicPosX = -60;
        gSpriteCoordOffsetX += 60;
        ChangeBgX(2, 0xFFFFC400, BG_COORD_SET);
        CreatePlatformSprites(taskId);
        gTasks[taskId].tNameNotConfirmed = TRUE;
        break;
    case 7:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        EnableInterrupts(INTR_FLAG_VBLANK);
        SetVBlankCallback(VBlankCB_NewGameScene);
        gTextFlags.canABSpeedUpPrint = TRUE;
        SetMainCallback2(CB2_NewGameScene);
        gMain.state = 0;
        return;
    }

    gMain.state++;
}

static void CreateNidoranFSprite(u8 taskId)
{
    u8 spriteId = CreateMonPicSprite_Affine(INTRO_SPECIES, SHINY_ODDS, 0, MON_PIC_AFFINE_FRONT, 96, 96, 14, TAG_NONE);

    gSprites[spriteId].callback = SpriteCallbackDummy;
    gSprites[spriteId].oam.priority = 1;
    gSprites[spriteId].invisible = TRUE;
    gTasks[taskId].tNidoranFSpriteId = spriteId;
}

static void CreatePlatformSprites(u8 taskId)
{
    u8 spriteId;
    u8 i;

    LoadCompressedSpriteSheet(&sOakSpeech_Platform_SpriteSheet);
    LoadSpritePalette(&sOakSpeech_Platform_SpritePalette);
    for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
    {
        spriteId = CreateSprite(&sOakSpeech_Platform_SpriteTemplates[i], i * 32 + 88, 112, 1);
        gSprites[spriteId].oam.priority = 2;
        gSprites[spriteId].animPaused = TRUE;
        gSprites[spriteId].coordOffsetEnabled = TRUE;
        gTasks[taskId].tPlatformSpriteId(i) = spriteId;
    }
}

static void DestroyPlatformSprites(u8 taskId)
{
    u8 i;

    for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
        DestroySprite(&gSprites[gTasks[taskId].tPlatformSpriteId(i)]);
    FreeSpriteTilesByTag(GFX_TAG_PLATFORM);
    FreeSpritePaletteByTag(PAL_TAG_PLATFORM);
}

// Draws one of the 64x96 trainer pics on BG2.
// The player pics use BG palettes 4-5, Oak's and the rival's palettes 6-7.
static void LoadTrainerPic(u16 whichPic)
{
    u8 tilemap[8 * 12];
    u32 i;

    switch (whichPic)
    {
    case BRENDAN_PIC:
        LoadPalette(sOakSpeech_Brendan_Pal, BG_PLTT_ID(4), sizeof(sOakSpeech_Brendan_Pal));
        LZ77UnCompVram(sOakSpeech_Brendan_Tiles, (void *)VRAM + 0x600);
        break;
    case MAY_PIC:
        LoadPalette(sOakSpeech_May_Pal, BG_PLTT_ID(4), sizeof(sOakSpeech_May_Pal));
        LZ77UnCompVram(sOakSpeech_May_Tiles, (void *)VRAM + 0x600);
        break;
    case RED_PIC:
        LoadPalette(sOakSpeech_Red_Pal, BG_PLTT_ID(4), sizeof(sOakSpeech_Red_Pal));
        LZ77UnCompVram(sOakSpeech_Red_Tiles, (void *)VRAM + 0x600);
        break;
    case LEAF_PIC:
        LoadPalette(sOakSpeech_Leaf_Pal, BG_PLTT_ID(4), sizeof(sOakSpeech_Leaf_Pal));
        LZ77UnCompVram(sOakSpeech_Leaf_Tiles, (void *)VRAM + 0x600);
        break;
    case RIVAL_PIC:
        LoadPalette(sOakSpeech_Rival_Pal, BG_PLTT_ID(6), sizeof(sOakSpeech_Rival_Pal));
        LZ77UnCompVram(sOakSpeech_Rival_Tiles, (void *)VRAM + 0x600);
        break;
    case OAK_PIC:
        LoadPalette(sOakSpeech_Oak_Pal, BG_PLTT_ID(6), sizeof(sOakSpeech_Oak_Pal));
        LZ77UnCompVram(sOakSpeech_Oak_Tiles, (void *)VRAM + 0x600);
        break;
    default:
        return;
    }

    for (i = 0; i < ARRAY_COUNT(tilemap); i++)
        tilemap[i] = i;
    FillBgTilemapBufferRect(2, 0, 0, 0, 32, 32, 16);
    // The 8bpp pic's tiles start at tile 24 (VRAM + 0x600)
    CopyRectToBgTilemapBufferRect(2, tilemap, 0, 0, 8, 12, 11, 2, 8, 12, 16, 24, 0);
    CopyBgTilemapBufferToVram(2);
}

static void LoadPlayerPic(void)
{
    if (gSaveBlock2Ptr->playerCostume == PLAYER_COSTUME_FRLG)
        LoadTrainerPic(gSaveBlock2Ptr->playerGender == MALE ? RED_PIC : LEAF_PIC);
    else
        LoadTrainerPic(gSaveBlock2Ptr->playerGender == MALE ? BRENDAN_PIC : MAY_PIC);
}

static void ClearTrainerPic(void)
{
    FillBgTilemapBufferRect(2, 0, 11, 1, 8, 12, 16);
    CopyBgTilemapBufferToVram(2);
}

#define tParentTaskId data[0]
#define tBlendTarget1 data[1]
#define tBlendTarget2 data[2]
#define tFadeTimer    data[4]

static void Task_SlowFadeIn(u8 taskId)
{
    u8 i;

    if (gTasks[taskId].tBlendTarget1 == 0)
    {
        gTasks[gTasks[taskId].tParentTaskId].tTrainerPicFadeState = 1;
        DestroyTask(taskId);
        for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
            gSprites[gTasks[taskId].tPlatformSpriteId(i)].invisible = TRUE;
    }
    else
    {
        if (gTasks[taskId].tFadeTimer != 0)
        {
            gTasks[taskId].tFadeTimer--;
        }
        else
        {
            gTasks[taskId].tFadeTimer = gTasks[taskId].tTimer;
            gTasks[taskId].tBlendTarget1--;
            gTasks[taskId].tBlendTarget2++;
            if (gTasks[taskId].tBlendTarget1 == 8)
            {
                for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
                    gSprites[gTasks[taskId].tPlatformSpriteId(i)].invisible ^= TRUE;
            }
            SetGpuReg(REG_OFFSET_BLDALPHA, (gTasks[taskId].tBlendTarget2 * 256) + gTasks[taskId].tBlendTarget1);
        }
    }
}

static void CreateFadeInTask(u8 taskId, u8 delay)
{
    u8 taskId2;
    u8 i;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG2 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_OBJ);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(16, 0));
    SetGpuReg(REG_OFFSET_BLDY, 0);
    gTasks[taskId].tTrainerPicFadeState = 0;
    taskId2 = CreateTask(Task_SlowFadeIn, 0);
    gTasks[taskId2].tParentTaskId = taskId;
    gTasks[taskId2].tBlendTarget1 = 16;
    gTasks[taskId2].tBlendTarget2 = 0;
    gTasks[taskId2].tTimer = delay; // How many frames each step of the fade takes
    gTasks[taskId2].tFadeTimer = delay;
    for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
        gTasks[taskId2].tPlatformSpriteId(i) = gTasks[taskId].tPlatformSpriteId(i);
}

static void Task_SlowFadeOut(u8 taskId)
{
    u8 i;

    if (gTasks[taskId].tBlendTarget1 == 16)
    {
        if (!gPaletteFade.active)
        {
            gTasks[gTasks[taskId].tParentTaskId].tTrainerPicFadeState = 1;
            DestroyTask(taskId);
        }
    }
    else
    {
        if (gTasks[taskId].tFadeTimer != 0)
        {
            gTasks[taskId].tFadeTimer--;
        }
        else
        {
            gTasks[taskId].tFadeTimer = gTasks[taskId].tTimer;
            gTasks[taskId].tBlendTarget1 += 2;
            gTasks[taskId].tBlendTarget2 -= 2;
            if (gTasks[taskId].tBlendTarget1 == 8)
            {
                for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
                    gSprites[gTasks[taskId].tPlatformSpriteId(i)].invisible ^= TRUE;
            }
            SetGpuReg(REG_OFFSET_BLDALPHA, (gTasks[taskId].tBlendTarget2 * 256) + gTasks[taskId].tBlendTarget1);
        }
    }
}

static void CreateFadeOutTask(u8 taskId, u8 delay)
{
    u8 taskId2;
    u8 i;

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG2 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_OBJ);
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(0, 16));
    SetGpuReg(REG_OFFSET_BLDY, 0);
    gTasks[taskId].tTrainerPicFadeState = 0;
    taskId2 = CreateTask(Task_SlowFadeOut, 0);
    gTasks[taskId2].tParentTaskId = taskId;
    gTasks[taskId2].tBlendTarget1 = 0;
    gTasks[taskId2].tBlendTarget2 = 16;
    gTasks[taskId2].tTimer = delay; // How many frames each step of the fade takes
    gTasks[taskId2].tFadeTimer = delay;
    for (i = 0; i < NUM_PLATFORM_SPRITES; i++)
        gTasks[taskId2].tPlatformSpriteId(i) = gTasks[taskId].tPlatformSpriteId(i);
}

#undef tParentTaskId
#undef tBlendTarget1
#undef tBlendTarget2
#undef tFadeTimer

static void PrintNameChoiceOptions(u8 taskId, u8 hasPlayerBeenNamed)
{
    s16 *data = gTasks[taskId].data;
    const u8 *const *textPtrs;
    u8 i;

    tMenuWindowId = AddWindow(&sIntro_WindowTemplates[WIN_INTRO_NAMES]);
    DrawStdWindowFrame(tMenuWindowId, FALSE);
    AddTextPrinterParameterized(tMenuWindowId, FONT_NORMAL, sText_NewName, 8, 1, 0, NULL);
    if (hasPlayerBeenNamed == FALSE)
        textPtrs = gSaveBlock2Ptr->playerGender == MALE ? sMaleNameChoices : sFemaleNameChoices;
    else
        textPtrs = sRivalNameChoices;
    for (i = 0; i < NUM_NAME_CHOICES; i++)
        AddTextPrinterParameterized(tMenuWindowId, FONT_NORMAL, textPtrs[i], 8, 16 * (i + 1) + 1, 0, NULL);
    InitMenuNormal(tMenuWindowId, FONT_NORMAL, 0, 1, 16, NUM_NAME_CHOICES + 1, 0);
    CopyWindowToVram(tMenuWindowId, COPYWIN_FULL);
}

static void GetDefaultName(u8 hasPlayerBeenNamed, u8 rivalNameChoice)
{
    const u8 *src;
    u8 *dest;
    u8 i;

    if (hasPlayerBeenNamed == FALSE)
    {
        if (gSaveBlock2Ptr->playerGender == MALE)
            src = sMaleNameChoices[Random() % ARRAY_COUNT(sMaleNameChoices)];
        else
            src = sFemaleNameChoices[Random() % ARRAY_COUNT(sFemaleNameChoices)];
        dest = gSaveBlock2Ptr->playerName;
    }
    else
    {
        src = sRivalNameChoices[rivalNameChoice];
        dest = gSaveBlock2Ptr->rivalName;
    }
    for (i = 0; i < PLAYER_NAME_LENGTH && src[i] != EOS; i++)
        dest[i] = src[i];
    for (; i < PLAYER_NAME_LENGTH + 1; i++)
        dest[i] = EOS;
}
