// The S.S. ANNE's departure after the player has gotten HM01 CUT, from FRLG (pokefirered's
// src/ss_anne.c): the ship sails off to the left, with its wake behind and smoke from its funnel,
// then the script (SSAnne_Exterior_ExitSSAnne) goes on.
#include "global.h"
#include "event_object_movement.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "constants/songs.h"

#define TAG_WAKE  4000
#define TAG_SMOKE 4001

#define LOCALID_SHIP 1  // LOCALID_SS_ANNE on SSAnne_Exterior

static void Task_SSAnneInit(u8 taskId);
static void Task_SSAnneRun(u8 taskId);
static void Task_SSAnneFinish(u8 taskId);
static void CreateWakeBehindBoat(void);
static void WakeSpriteCallback(struct Sprite *sprite);
static void CreateSmokeSprite(void);
static void SmokeSpriteCallback(struct Sprite *sprite);

static const u16 sWakeTiles[] = INCGFX_U16("graphics/ss_anne/wake.png", ".4bpp");
static const u16 sSmokeTiles[] = INCGFX_U16("graphics/ss_anne/smoke.png", ".4bpp");

static const struct SpriteSheet sSpriteSheets[] = {
    {sWakeTiles, sizeof(sWakeTiles), TAG_WAKE},
    {sSmokeTiles, sizeof(sSmokeTiles), TAG_SMOKE},
    {0}
};

static const union AnimCmd sWakeAnim[] = {
    ANIMCMD_FRAME(0, 12),
    ANIMCMD_FRAME(8, 12),
    ANIMCMD_JUMP(0)
};

static const union AnimCmd *const sWakeAnimTable[] = {
    sWakeAnim
};

static const struct OamData sWakeOamData = {
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
};

static const struct SpriteTemplate sWakeSpriteTemplate = {
    .tileTag = TAG_WAKE,
    .paletteTag = TAG_NONE,
    .oam = &sWakeOamData,
    .anims = sWakeAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = WakeSpriteCallback,
};

static const union AnimCmd sSmokeAnim[] = {
    ANIMCMD_FRAME( 0, 10),
    ANIMCMD_FRAME( 4, 20),
    ANIMCMD_FRAME( 8, 20),
    ANIMCMD_FRAME(12, 30),
    ANIMCMD_END
};

static const union AnimCmd *const sSmokeAnimTable[] = {
    sSmokeAnim
};

static const struct OamData sSmokeOamData = {
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
};

static const struct SpriteTemplate sSmokeSpriteTemplate = {
    .tileTag = TAG_SMOKE,
    .paletteTag = TAG_NONE,
    .oam = &sSmokeOamData,
    .anims = sSmokeAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SmokeSpriteCallback,
};

// The ship's sprite, or NULL if it isn't there
static struct Sprite *GetShipSprite(void)
{
    u8 objectEventId;

    if (TryGetObjectEventIdByLocalIdAndMap(LOCALID_SHIP, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &objectEventId))
        return NULL;
    return &gSprites[gObjectEvents[objectEventId].spriteId];
}

void DoSSAnneDepartureCutscene(void)
{
    u8 taskId;

    PlaySE(SE_RG_SS_ANNE_HORN);
    taskId = CreateTask(Task_SSAnneInit, 8);
    gTasks[taskId].data[0] = 50;
}

static void Task_SSAnneInit(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (--data[0] == 0)
    {
        LoadSpriteSheets(sSpriteSheets);
        CreateWakeBehindBoat();
        gTasks[taskId].func = Task_SSAnneRun;
    }
}

static void Task_SSAnneRun(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    struct Sprite *ship = GetShipSprite();

    data[1]++;
    data[2]++;
    if (data[1] == 70)
    {
        data[1] = 0;
        CreateSmokeSprite();
    }
    // Without the ship there's nothing to watch: let the script go on rather than wait forever
    if (ship == NULL || ship->x + ship->x2 < -120)
    {
        PlaySE(SE_RG_SS_ANNE_HORN);
        gTasks[taskId].func = Task_SSAnneFinish;
    }
    else
    {
        ship->x2 = -(data[2] / 5);
    }
}

static void Task_SSAnneFinish(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (++data[3] == 40)
    {
        FreeSpriteTilesByTag(TAG_WAKE);
        FreeSpriteTilesByTag(TAG_SMOKE);
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// The wake and the smoke are drawn in the ship's colours (FRLG put the ship's palette in slot 10)
static void CreateWakeBehindBoat(void)
{
    struct Sprite *ship = GetShipSprite();
    u8 spriteId;

    if (ship == NULL)
        return;
    spriteId = CreateSprite(&sWakeSpriteTemplate, ship->x + ship->x2 + 80, 109, 0xFF);
    if (spriteId != MAX_SPRITES)
    {
        gSprites[spriteId].oam.priority = 2;
        gSprites[spriteId].oam.paletteNum = ship->oam.paletteNum;
    }
}

static void WakeSpriteCallback(struct Sprite *sprite)
{
    struct Sprite *ship = GetShipSprite();

    if (ship == NULL)
    {
        DestroySprite(sprite);
        return;
    }
    sprite->x = ship->x + ship->x2 + 80;
    if (sprite->data[0] / 6 < 22)
        sprite->data[0]++;
    sprite->x2 = sprite->data[0] / 6;
    if (sprite->x + sprite->x2 < -18)
        DestroySprite(sprite);
}

static void CreateSmokeSprite(void)
{
    struct Sprite *ship = GetShipSprite();
    s16 x;
    u8 spriteId;

    if (ship == NULL)
        return;
    x = ship->x + ship->x2 + 49;
    if (x >= -32)
    {
        spriteId = CreateSprite(&sSmokeSpriteTemplate, x, 78, 8);
        if (spriteId != MAX_SPRITES)
            gSprites[spriteId].oam.paletteNum = ship->oam.paletteNum;
    }
}

static void SmokeSpriteCallback(struct Sprite *sprite)
{
    sprite->data[0]++;
    sprite->x2 = sprite->data[0] / 4;
    if (sprite->animEnded)
        DestroySprite(sprite);
}
