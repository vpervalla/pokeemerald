#include "global.h"
#include "day_night.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_door.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "sprite.h"
#include "tilesets.h"
#include "constants/field_weather.h"
#include "battle.h"
#include "battle_bg.h"
#include "battle_interface.h"
#include "decompress.h"
#include "constants/battle.h"
#include "constants/rgb.h"
#include "constants/trainers.h"

// Day/night tinting for outdoor maps, and for battles started on them.
//
// The tint is applied to the final palettes, after weather and fades have
// written gPlttBufferFaded: each overworld (or battle) frame tints that buffer
// into sTintedPltt, and the field (or battle) VBlank copies sTintedPltt to
// palette RAM instead. UI palettes are left alone so text boxes, menus and
// popups keep their colours.
//
// Lit windows: in the evening and at night, some tileset palette colours that
// only window glass uses are swapped for warm light and left untinted, so the
// windows glow against the darkened map.

#define CLOCK_CHECK_FRAMES 60   // How often the RTC is read while in the field
#define TINT_EASE_STEP     4    // Change per frame of each channel's multiplier when the phase changes
#define TINT_FULL          256  // Multiplier that leaves a channel unchanged

struct TintMultipliers
{
    u16 r, g, b;
};

struct LitPalette
{
    const struct Tileset *tileset; // Applies to maps using this tileset
    u8 paletteNum;
    u8 copyOf;                     // If not NO_COPY, only when paletteNum holds a copy of this palette's lit colours
    u16 colors;                    // Bit n: colour n is window glass
};

#define NO_COPY 0xFF
#define COLORS(first, last) (((1 << ((last) + 1)) - 1) & ~((1 << (first)) - 1))

static const struct TintMultipliers sPhaseTints[DAY_NIGHT_PHASE_COUNT] =
{
    [DAY_NIGHT_PHASE_MORNING] = {248, 236, 228},
    [DAY_NIGHT_PHASE_DAY]     = {TINT_FULL, TINT_FULL, TINT_FULL},
    [DAY_NIGHT_PHASE_EVENING] = {256, 208, 176},
    [DAY_NIGHT_PHASE_NIGHT]   = {144, 156, 200},
};

// Window glass lit in the evening and at night. Most Kanto windows use colours 9-13 of kanto_general's
// palette 3, which roofs and water share, so tools/kanto_port/split_window_palettes.py moved the window
// tiles to a copy of that palette in each Kanto secondary tileset's slot 7. Glass in a town's own
// palettes is listed where nothing but windows uses its colours.
static const struct LitPalette sLitPalettes[] =
{
    {&gTileset_KantoGeneral,        7,  3,       COLORS(9, 13)},
    {&gTileset_KantoPalletTown,     9,  NO_COPY, COLORS(8, 10)},                // Oak's lab
    {&gTileset_KantoPewterCity,     11, NO_COPY, (1 << 5) | (1 << 6) | (1 << 12)}, // Museum
    {&gTileset_KantoVermilionCity,  9,  NO_COPY, COLORS(8, 9) | COLORS(14, 15)},
    {&gTileset_KantoSaffronCity,    9,  NO_COPY, COLORS(13, 14)},               // Silph Co.
    {&gTileset_KantoSaffronCity,    12, NO_COPY, (1 << 13) | (1 << 15)},
    {&gTileset_KantoCinnabarIsland, 8,  NO_COPY, 1 << 5},
    {&gTileset_KantoIndigoPlateau,  10, NO_COPY, COLORS(12, 14)},               // Pokemon League
    {&gTileset_KantoSeviiIslands123, 11, NO_COPY, COLORS(8, 9)},
    {&gTileset_KantoSeviiIslands45, 9,  NO_COPY, (1 << 1) | COLORS(14, 15)},    // Purple houses
};

static EWRAM_DATA u16 sTintedPltt[PLTT_BUFFER_SIZE] = {0};
static EWRAM_DATA u16 sTintSource[PLTT_BUFFER_SIZE] = {0}; // The gPlttBufferFaded colours sTintedPltt was made from
static EWRAM_DATA u16 sUntintedColors[NUM_PALS_TOTAL] = {0}; // Per map BG palette, a bit for each lit colour
static EWRAM_DATA struct TintMultipliers sTint = {0};
static EWRAM_DATA u32 sLastTintFrame = 0;
static EWRAM_DATA u8 sClockPhase = 0;
static EWRAM_DATA u8 sClockTimer = 0;
static EWRAM_DATA bool8 sTintActive = FALSE;
static EWRAM_DATA bool8 sWindowsLit = FALSE;
static EWRAM_DATA bool8 sRetintAll = FALSE;

static u8 ReadClockPhase(void)
{
    if (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK)
        return DAY_NIGHT_PHASE_DAY;

    RtcCalcLocalTime();
    if (gLocalTime.hours >= 20 || gLocalTime.hours < 6)
        return DAY_NIGHT_PHASE_NIGHT;
    if (gLocalTime.hours < 10)
        return DAY_NIGHT_PHASE_MORNING;
    if (gLocalTime.hours < 17)
        return DAY_NIGHT_PHASE_DAY;
    return DAY_NIGHT_PHASE_EVENING;
}

u8 DayNight_GetPhase(void)
{
    u16 override = VarGet(VAR_DAY_NIGHT_OVERRIDE);

    if (override != DAY_NIGHT_OVERRIDE_NONE && override <= DAY_NIGHT_OVERRIDE_NIGHT)
        return override - 1;
    return sClockPhase;
}

static bool8 IsMapTinted(void)
{
    return IsMapTypeOutdoors(gMapHeader.mapType);
}

static bool8 ShouldLightWindows(void)
{
    u8 phase = DayNight_GetPhase();

    return IsMapTinted() && (phase == DAY_NIGHT_PHASE_EVENING || phase == DAY_NIGHT_PHASE_NIGHT);
}

// Warm light for a glass colour: brighter glass gives paler light.
static u16 LitGlassColor(u16 glass)
{
    u32 v = ((glass & 0x1F) + ((glass >> 5) & 0x1F) + ((glass >> 10) & 0x1F)) / 3;
    u32 r = 24 + v / 4;

    if (r > 31)
        r = 31;
    return RGB(r, 12 + v * 18 / 31, 2 + v * 16 / 31);
}

static const struct Tileset *GetPaletteOwner(const struct MapLayout *layout, u8 paletteNum)
{
    return paletteNum < NUM_PALS_IN_PRIMARY ? layout->primaryTileset : layout->secondaryTileset;
}

static bool8 LitPaletteApplies(const struct MapLayout *layout, const struct LitPalette *lp)
{
    const struct Tileset *owner;
    const struct Tileset *source;
    u32 j;

    if (lp->tileset != layout->primaryTileset && lp->tileset != layout->secondaryTileset)
        return FALSE;
    owner = GetPaletteOwner(layout, lp->paletteNum);
    // Tilesets whose isSecondary is neither TRUE nor FALSE have compressed palettes (see LoadTilesetPalette).
    if (owner == NULL || (owner->isSecondary != FALSE && owner->isSecondary != TRUE))
        return FALSE;
    if (lp->copyOf == NO_COPY)
        return TRUE;

    source = GetPaletteOwner(layout, lp->copyOf);
    if (source == NULL)
        return FALSE;
    for (j = 0; j < 16; j++)
    {
        if ((lp->colors & (1 << j)) && owner->palettes[lp->paletteNum][j] != source->palettes[lp->copyOf][j])
            return FALSE;
    }
    return TRUE;
}

// Swaps the lit colours of the current map's tilesets in or out. On a palette load the colours
// go straight into both buffers, like LoadPalette does; otherwise they go into
// gPlttBufferUnfaded and reach gPlttBufferFaded through the weather (or a running fade).
static void SetWindowsLit(bool8 lit, bool8 onLoad)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    u32 i, j;

    for (i = 0; i < NUM_PALS_TOTAL; i++)
        sUntintedColors[i] = 0;
    sWindowsLit = lit;
    sRetintAll = TRUE;
    if (layout == NULL)
        return;

    for (i = 0; i < ARRAY_COUNT(sLitPalettes); i++)
    {
        const struct LitPalette *lp = &sLitPalettes[i];
        const u16 *original;

        if (!LitPaletteApplies(layout, lp))
            continue;

        original = GetPaletteOwner(layout, lp->paletteNum)->palettes[lp->paletteNum];
        for (j = 0; j < 16; j++)
        {
            u16 index = BG_PLTT_ID(lp->paletteNum) + j;
            u16 color;

            if (!(lp->colors & (1 << j)))
                continue;
            if (lit)
            {
                color = LitGlassColor(original[j]);
                sUntintedColors[lp->paletteNum] |= 1 << j;
            }
            else
            {
                color = original[j];
            }
            gPlttBufferUnfaded[index] = color;
            if (onLoad)
                gPlttBufferFaded[index] = color;
        }

        if (!onLoad && !gPaletteFade.active)
        {
            if (gWeatherPtr->palProcessingState == WEATHER_PAL_STATE_IDLE)
            {
                ApplyWeatherColorMapToPal(lp->paletteNum);
            }
            else
            {
                for (j = 0; j < 16; j++)
                {
                    u16 index = BG_PLTT_ID(lp->paletteNum) + j;
                    if (lp->colors & (1 << j))
                        gPlttBufferFaded[index] = gPlttBufferUnfaded[index];
                }
            }
        }
    }
}

// The Poke Ball emblems on Pokemon Centers, Marts and Gyms glow while the windows are lit. Their colours
// are shared with walls and roofs, so instead of lighting palette colours, sprites of the emblems (cut out
// of the tileset by tools/kanto_port/make_sign_sprites.py) are laid over them, with untinted palettes.
// Their glass doors let a little light through: sprites of the glass, lightened and only half tinted,
// which step aside while the door is open.
#define TAG_SIGN_POKEMON_CENTER 0x2E00
#define TAG_SIGN_MART           0x2E01
#define TAG_SIGN_GYM            0x2E02
#define TAG_DOOR_SLIDING        0x2E03
#define TAG_DOOR_GYM            0x2E04
#define TAG_DOOR_GLASS          0x2E05 // Palette of both doors' glass
#define MAX_SIGN_SPRITES        8
#define SIGN_MAGIC              0x5167 // In data[7], to recognise the sprites after a sprite reset
#define METATILE_KANTO_POKEMON_CENTER_EMBLEM 0x05A
#define METATILE_KANTO_MART_EMBLEM_LEFT      0x039
#define METATILE_KANTO_GYM_EMBLEM            0x153
#define METATILE_KANTO_SLIDING_DOOR          0x062 // Pokemon Centers and Marts
#define METATILE_KANTO_GYM_DOOR              0x15B

#define sDoorX data[0] // A door sprite's door, in map coordinates with MAP_OFFSET
#define sDoorY data[1]
#define sIsDoor data[2]

static const u32 sPokemonCenterSign_Gfx[] = INCGFX_U32("graphics/day_night/pokemon_center_sign.png", ".4bpp");
static const u16 sPokemonCenterSign_Pal[] = INCGFX_U16("graphics/day_night/pokemon_center_sign.png", ".gbapal");
static const u32 sMartSign_Gfx[] = INCGFX_U32("graphics/day_night/mart_sign.png", ".4bpp");
static const u16 sMartSign_Pal[] = INCGFX_U16("graphics/day_night/mart_sign.png", ".gbapal");
static const u32 sGymSign_Gfx[] = INCGFX_U32("graphics/day_night/gym_sign.png", ".4bpp");
static const u16 sGymSign_Pal[] = INCGFX_U16("graphics/day_night/gym_sign.png", ".gbapal");
static const u32 sSlidingDoor_Gfx[] = INCGFX_U32("graphics/day_night/sliding_door.png", ".4bpp");
static const u32 sGymDoor_Gfx[] = INCGFX_U32("graphics/day_night/gym_door.png", ".4bpp");
static const u16 sDoorGlass_Pal[] = INCGFX_U16("graphics/day_night/sliding_door.png", ".gbapal");

static const struct OamData sOam_Sign =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 2, // The emblem metatiles draw below objects, like the bottom/middle BG layers
};

// The Gym emblem metatile draws its sign on the top BG layer, which covers sprites at priority 2.
static const struct OamData sOam_SignTopLayer =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 1,
};

static const struct OamData sOam_Door =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2,
};

static void SpriteCB_Sign(struct Sprite *sprite);

static const struct SpriteTemplate sSpriteTemplate_PokemonCenterSign =
{
    .tileTag = TAG_SIGN_POKEMON_CENTER,
    .paletteTag = TAG_SIGN_POKEMON_CENTER,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_MartSign =
{
    .tileTag = TAG_SIGN_MART,
    .paletteTag = TAG_SIGN_MART,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymSign =
{
    .tileTag = TAG_SIGN_GYM,
    .paletteTag = TAG_SIGN_GYM,
    .oam = &sOam_SignTopLayer,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_SlidingDoor =
{
    .tileTag = TAG_DOOR_SLIDING,
    .paletteTag = TAG_DOOR_GLASS,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymDoor =
{
    .tileTag = TAG_DOOR_GYM,
    .paletteTag = TAG_DOOR_GLASS,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

struct GlowingSign
{
    u16 metatileId;
    s16 x, y; // Sprite's top-left corner, in pixels from the metatile's top-left corner
    u8 size;  // Sprite's width and height
    bool8 isDoor;
    const struct SpriteTemplate *template;
    struct SpriteSheet sheet;
    struct SpritePalette palette;
};

static const struct GlowingSign sGlowingSigns[] =
{
    {METATILE_KANTO_POKEMON_CENTER_EMBLEM, -8, -16, 32, FALSE, &sSpriteTemplate_PokemonCenterSign,
     {sPokemonCenterSign_Gfx, 32 * 32 / 2, TAG_SIGN_POKEMON_CENTER}, {sPokemonCenterSign_Pal, TAG_SIGN_POKEMON_CENTER}},
    {METATILE_KANTO_MART_EMBLEM_LEFT, 0, -16, 32, FALSE, &sSpriteTemplate_MartSign,
     {sMartSign_Gfx, 32 * 32 / 2, TAG_SIGN_MART}, {sMartSign_Pal, TAG_SIGN_MART}},
    // The ball on the gold sign above the door is in the sprite's top half.
    {METATILE_KANTO_GYM_EMBLEM, -8, 0, 32, FALSE, &sSpriteTemplate_GymSign,
     {sGymSign_Gfx, 32 * 32 / 2, TAG_SIGN_GYM}, {sGymSign_Pal, TAG_SIGN_GYM}},
    {METATILE_KANTO_SLIDING_DOOR, 0, 0, 16, TRUE, &sSpriteTemplate_SlidingDoor,
     {sSlidingDoor_Gfx, 16 * 16 / 2, TAG_DOOR_SLIDING}, {sDoorGlass_Pal, TAG_DOOR_GLASS}},
    {METATILE_KANTO_GYM_DOOR, 0, 0, 16, TRUE, &sSpriteTemplate_GymDoor,
     {sGymDoor_Gfx, 16 * 16 / 2, TAG_DOOR_GYM}, {sDoorGlass_Pal, TAG_DOOR_GLASS}},
};

static EWRAM_DATA u8 sSignSpriteIds[MAX_SIGN_SPRITES] = {0};
static EWRAM_DATA u8 sNumSignSprites = 0;
static EWRAM_DATA bool8 sSignsShown = FALSE;
static EWRAM_DATA bool8 sSignsDirty = FALSE;

// Hides the sprite while it's off screen, so its coordinates can't wrap around into view,
// and a door's glass while the door is opening, open or closing.
static void SpriteCB_Sign(struct Sprite *sprite)
{
    s16 x = sprite->x + sprite->x2 + gSpriteCoordOffsetX;
    s16 y = sprite->y + sprite->y2 + gSpriteCoordOffsetY;

    sprite->invisible = (x < -32 || x > DISPLAY_WIDTH + 32 || y < -32 || y > DISPLAY_HEIGHT + 32)
                     || (sprite->sIsDoor && FieldIsDoorShownOpen(sprite->sDoorX, sprite->sDoorY));
}

static void DestroySignSprites(void)
{
    u32 i;

    for (i = 0; i < sNumSignSprites; i++)
    {
        struct Sprite *sprite = &gSprites[sSignSpriteIds[i]];

        // A warp resets every sprite, after which the ids may belong to other sprites.
        if (sprite->inUse && sprite->callback == SpriteCB_Sign && sprite->data[7] == SIGN_MAGIC)
            DestroySprite(sprite);
    }
    sNumSignSprites = 0;
    for (i = 0; i < ARRAY_COUNT(sGlowingSigns); i++)
    {
        FreeSpriteTilesByTag(sGlowingSigns[i].sheet.tag);
        FreeSpritePaletteByTag(sGlowingSigns[i].palette.tag);
    }
}

static void CreateSignSprite(const struct GlowingSign *sign, s16 mapX, s16 mapY)
{
    s16 x, y;
    u8 spriteId;

    if (GetSpriteTileStartByTag(sign->sheet.tag) == 0xFFFF)
        LoadSpriteSheet(&sign->sheet);
    if (IndexOfSpritePaletteTag(sign->palette.tag) == 0xFF)
    {
        u8 paletteNum = LoadSpritePalette(&sign->palette);

        if (paletteNum == 0xFF)
            return;
        UpdateSpritePaletteWithWeather(paletteNum);
    }

    SetSpritePosToMapCoords(mapX + MAP_OFFSET, mapY + MAP_OFFSET, &x, &y);
    spriteId = CreateSprite(sign->template, x + sign->x + sign->size / 2, y + sign->y + sign->size / 2, 0xFF);
    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].coordOffsetEnabled = TRUE;
    gSprites[spriteId].sDoorX = mapX + MAP_OFFSET;
    gSprites[spriteId].sDoorY = mapY + MAP_OFFSET;
    gSprites[spriteId].sIsDoor = sign->isDoor;
    gSprites[spriteId].data[7] = SIGN_MAGIC;
    SpriteCB_Sign(&gSprites[spriteId]);
    sSignSpriteIds[sNumSignSprites++] = spriteId;
}

static void CreateSignSprites(void)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    s32 x, y;
    u32 i;

    // The metatile ids are kanto_general's.
    if (layout == NULL || layout->primaryTileset != &gTileset_KantoGeneral)
        return;

    for (y = 0; y < layout->height; y++)
    {
        for (x = 0; x < layout->width; x++)
        {
            u16 metatileId = layout->map[x + y * layout->width] & MAPGRID_METATILE_ID_MASK;

            for (i = 0; i < ARRAY_COUNT(sGlowingSigns); i++)
            {
                if (metatileId == sGlowingSigns[i].metatileId && sNumSignSprites < MAX_SIGN_SPRITES)
                    CreateSignSprite(&sGlowingSigns[i], x, y);
            }
        }
    }
}

static void UpdateSigns(bool8 lit)
{
    if (sSignsDirty || lit != sSignsShown)
    {
        DestroySignSprites();
        if (lit)
            CreateSignSprites();
        sSignsShown = lit;
        sSignsDirty = FALSE;
    }
}

// Called after the map's tileset palettes are (re)loaded.
void DayNight_OnTilesetPalettesLoaded(void)
{
    sClockPhase = ReadClockPhase();
    sClockTimer = 0;
    SetWindowsLit(ShouldLightWindows(), TRUE);
    sSignsDirty = TRUE;
}

static u16 TintColor(u16 color)
{
    u32 r = ((color & 0x1F) * sTint.r) >> 8;
    u32 g = (((color >> 5) & 0x1F) * sTint.g) >> 8;
    u32 b = (((color >> 10) & 0x1F) * sTint.b) >> 8;

    if (r > 31)
        r = 31;
    if (g > 31)
        g = 31;
    if (b > 31)
        b = 31;
    return r | (g << 5) | (b << 10);
}

// Halfway between the colour and its tint, for glass that lets a little light out.
static u16 HalfTintColor(u16 color)
{
    u16 tinted = TintColor(color);
    u32 r = ((color & 0x1F) + (tinted & 0x1F)) / 2;
    u32 g = (((color >> 5) & 0x1F) + ((tinted >> 5) & 0x1F)) / 2;
    u32 b = (((color >> 10) & 0x1F) + ((tinted >> 10) & 0x1F)) / 2;

    return r | (g << 5) | (b << 10);
}

static u16 EaseChannel(u16 current, u16 target)
{
    if (current + TINT_EASE_STEP < target)
        return current + TINT_EASE_STEP;
    if (current > target + TINT_EASE_STEP)
        return current - TINT_EASE_STEP;
    return target;
}

// Which colours get tinted: one u16 per palette (BG 0-15, then OBJ 0-15), bit n for colour n.
#define NUM_TINT_PALETTES (PLTT_BUFFER_SIZE / 16)
#define OBJ_PAL(n)        (16 + (n))
#define ALL_COLORS        0xFFFF

// The ground patches the battlers stand on, per battle environment: colours of BG palettes 2 and 3
// that only the patches use (the backdrop uses colours 1 and 15, its stripes, and slot 3 copies of
// those). Taken from graphics/battle_environment by comparing the patches with the rest of the scene.
static const u16 sBattlePlatformColors[][2] =
{
    [BATTLE_ENVIRONMENT_GRASS]      = {0x00FC, 0x00FC},
    [BATTLE_ENVIRONMENT_LONG_GRASS] = {0x01FC, 0x00FC},
    [BATTLE_ENVIRONMENT_SAND]       = {0x01FC, 0x0000},
    [BATTLE_ENVIRONMENT_UNDERWATER] = {0x01FC, 0x01FC},
    [BATTLE_ENVIRONMENT_WATER]      = {0x01FC, 0x01FC},
    [BATTLE_ENVIRONMENT_POND]       = {0x03FC, 0x03FC},
    [BATTLE_ENVIRONMENT_MOUNTAIN]   = {0x03FC, 0x03FC},
    [BATTLE_ENVIRONMENT_CAVE]       = {0x07FC, 0x07FC},
    [BATTLE_ENVIRONMENT_BUILDING]   = {0x01FC, 0x0000},
    [BATTLE_ENVIRONMENT_PLAIN]      = {0x01FC, 0x0000},
};

static EWRAM_DATA u16 sTintedColors[NUM_TINT_PALETTES] = {0}; // Masks that sTintedPltt was made with
static EWRAM_DATA u16 sHalfTintedObjPals = 0;                   // Likewise; bit n for OBJ palette n
static EWRAM_DATA bool8 sLitColorsUsed = FALSE;

static void TintPalettes(bool8 all, const u16 *tintedColors, u16 halfTintedObjPals, bool8 useLitColors)
{
    u32 i;

    for (i = 0; i < PLTT_BUFFER_SIZE; i++)
    {
        u16 color = gPlttBufferFaded[i];
        u32 pal = i / 16;
        u16 bit = 1 << (i % 16);

        if (!all && color == sTintSource[i])
            continue;
        sTintSource[i] = color;

        if (!(tintedColors[pal] & bit)
         || (useLitColors && pal < NUM_PALS_TOTAL && (sUntintedColors[pal] & bit)))
            sTintedPltt[i] = color;
        else if (pal >= OBJ_PAL(0) && (halfTintedObjPals & (1 << (pal - OBJ_PAL(0)))))
            sTintedPltt[i] = HalfTintColor(color);
        else
            sTintedPltt[i] = TintColor(color);
    }
}

// Start of a frame's update; returns whether the tint was out of use (e.g. during a map load
// or a screen without the tint), in which case the clock is read again and the tint snaps.
static bool8 StartTintUpdate(void)
{
    bool8 reentered = (gMain.vblankCounter1 - sLastTintFrame > 2);

    sLastTintFrame = gMain.vblankCounter1;
    if (reentered || ++sClockTimer >= CLOCK_CHECK_FRAMES)
    {
        sClockTimer = 0;
        sClockPhase = ReadClockPhase();
    }
    return reentered;
}

// halfTintedObjPals: OBJ palettes (bit n for palette n) whose tinted colours are only half tinted.
static void UpdateTint(bool8 reentered, const u16 *tintedColors, u16 halfTintedObjPals, bool8 useLitColors)
{
    const struct TintMultipliers *target;
    struct TintMultipliers prevTint = sTint;
    u8 phase = IsMapTinted() ? DayNight_GetPhase() : DAY_NIGHT_PHASE_DAY;
    bool8 masksChanged = FALSE;
    u32 i;

    target = &sPhaseTints[phase];
    if (reentered)
    {
        sTint = *target;
    }
    else
    {
        sTint.r = EaseChannel(sTint.r, target->r);
        sTint.g = EaseChannel(sTint.g, target->g);
        sTint.b = EaseChannel(sTint.b, target->b);
    }

    if (sTint.r == TINT_FULL && sTint.g == TINT_FULL && sTint.b == TINT_FULL)
    {
        sTintActive = FALSE;
        return;
    }

    for (i = 0; i < NUM_TINT_PALETTES; i++)
    {
        if (tintedColors[i] != sTintedColors[i])
        {
            sTintedColors[i] = tintedColors[i];
            masksChanged = TRUE;
        }
    }
    if (halfTintedObjPals != sHalfTintedObjPals)
    {
        sHalfTintedObjPals = halfTintedObjPals;
        masksChanged = TRUE;
    }

    // A new tint or a change to which colours are tinted redoes every colour; otherwise only
    // the colours that changed in gPlttBufferFaded since the last frame are tinted again.
    TintPalettes(!sTintActive || reentered || sRetintAll || masksChanged || useLitColors != sLitColorsUsed
                 || sTint.r != prevTint.r || sTint.g != prevTint.g || sTint.b != prevTint.b,
                 tintedColors, halfTintedObjPals, useLitColors);
    sLitColorsUsed = useLitColors;
    sRetintAll = FALSE;
    sTintActive = TRUE;
}

// Runs once per overworld frame, after the palette fade has been updated. The field tints the
// map's BG palettes (the others are UI: text boxes, menus, the map name popup) and every sprite
// but the glowing emblems, half tinting the glowing door glass.
void DayNight_UpdateField(void)
{
    bool8 reentered = StartTintUpdate();
    bool8 lit = ShouldLightWindows();
    u16 tintedColors[NUM_TINT_PALETTES];
    u16 halfTintedObjPals = 0;
    u32 i;

    if (lit != sWindowsLit)
        SetWindowsLit(lit, FALSE);
    UpdateSigns(lit);

    for (i = 0; i < 16; i++)
    {
        u16 tag = GetSpritePaletteTagByPaletteNum(i);

        tintedColors[i] = (i < NUM_PALS_TOTAL) ? ALL_COLORS : 0;
        tintedColors[OBJ_PAL(i)] = (tag == TAG_SIGN_POKEMON_CENTER || tag == TAG_SIGN_MART || tag == TAG_SIGN_GYM) ? 0 : ALL_COLORS;
        if (tag == TAG_DOOR_GLASS)
            halfTintedObjPals |= 1 << i;
    }
    UpdateTint(reentered, tintedColors, halfTintedObjPals, TRUE);
}

// Runs once per battle frame. The battle takes the tint of the map it was started on, but only
// on what stands in the scene: the Pokemon, the trainers and the ground patches under the
// battlers. The backdrop, the UI and the move animations keep their colours.
void DayNight_UpdateBattle(void)
{
    bool8 reentered = StartTintUpdate();
    u16 tintedColors[NUM_TINT_PALETTES];
    u32 i;

    for (i = 0; i < NUM_TINT_PALETTES; i++)
        tintedColors[i] = 0;

    // The ground patches, while the environment's own background is shown (not a move's).
    if (gBattleBgShowsEnvironment && gBattleEnvironment < ARRAY_COUNT(sBattlePlatformColors))
    {
        tintedColors[2] = sBattlePlatformColors[gBattleEnvironment][0];
        tintedColors[3] = sBattlePlatformColors[gBattleEnvironment][1];
    }

    // The battlers' palettes hold the Pokemon (and the player's back pic during the intro);
    // BG 8-11 are copies of them that some move animations draw the Pokemon with.
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        tintedColors[OBJ_PAL(i)] = ALL_COLORS;
        tintedColors[8 + i] = ALL_COLORS;
    }

    // Trainer front pics get a free sprite palette tagged with their TRAINER_PIC_* id, and the
    // player's (or partner's) back pic moves to one tagged 0xD6F8 (0xD6F9) while throwing the
    // first Poke Ball, so the Pokemon can take over the battler's palette. The opponents' shadows
    // have a palette of their own too.
    for (i = MAX_BATTLERS_COUNT; i < 16; i++)
    {
        u16 tag = GetSpritePaletteTagByPaletteNum(i);

        if (tag < TRAINER_PIC_KANTO_END || tag == 0xD6F8 || tag == 0xD6F9 || tag == TAG_ENEMY_SHADOW_PAL)
            tintedColors[OBJ_PAL(i)] = ALL_COLORS;
    }
    UpdateTint(reentered, tintedColors, 0, FALSE);
}

// Replaces TransferPlttBuffer in the field and battle VBlanks. The tinted buffer is only used
// while the field or battle keeps it up to date, so a screen that borrows their VBlank without
// updating the tint (e.g. during a map load) gets the plain palettes.
void DayNight_TransferPlttBuffer(void)
{
    if (sTintActive && gMain.vblankCounter1 - sLastTintFrame <= 2)
        TransferPlttBufferFrom(sTintedPltt);
    else
        TransferPlttBuffer();
}
