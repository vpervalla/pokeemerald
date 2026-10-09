#include "global.h"
#include "day_night.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_door.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "sprite.h"
#include "tilesets.h"
#include "constants/field_weather.h"
#include "constants/layouts.h"
#include "constants/weather.h"
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
// palettes is listed where nothing but windows uses its colours. Buildings you can't enter keep their
// windows dark: tools/kanto_port/night_windows.py gives them unlit copies of their window metatiles.
static const struct LitPalette sLitPalettes[] =
{
    {&gTileset_KantoGeneral,        7,  3,       COLORS(9, 13)},
    {&gTileset_KantoPalletTown,     9,  NO_COPY, COLORS(8, 10)},                // Oak's lab
    {&gTileset_KantoPewterCity,     11, NO_COPY, (1 << 5) | (1 << 6) | (1 << 12)}, // Museum windows
    {&gTileset_KantoPewterCity,     12, NO_COPY, COLORS(2, 4)},                 // Museum glass front (night_windows.py)
    {&gTileset_KantoCeladonCity,    10, NO_COPY, COLORS(10, 12)},               // Department Store
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

// The Poke Ball emblems on Pokemon Centers, Marts and Gyms, their "P.C", "MART" and "GYM" signs, and the
// Gyms' billboards (ball, text and dots) glow while the windows are lit. So does the Fighting Dojo's open
// doorway, though not its Poke Ball sign or billboard. Their colours are shared with walls and roofs, so instead of lighting palette colours, sprites of the emblems (cut out
// of the tileset by tools/kanto_port/make_sign_sprites.py) are laid over them, with untinted palettes.
// Their glass doors are lit like the windows, by sprites of the glass that follow the door's opening
// and closing: each is a strip of the closed door's glass and the glass of each animation frame.
// The overworld leaves only a couple of sprite palette slots free, so the sprites share one palette,
// made by the script.
#define TAG_SIGN_POKEMON_CENTER 0x2E00
#define TAG_SIGN_MART           0x2E01
#define TAG_SIGN_GYM            0x2E02
#define TAG_DOOR_SLIDING        0x2E03
#define TAG_DOOR_GYM            0x2E04
#define TAG_GLOW_PAL            0x2E05
#define TAG_SIGN_POKEMON_CENTER_TEXT 0x2E06
#define TAG_DOOR_DEPT_STORE     0x2E07
#define TAG_SIGN_MART_TEXT      0x2E08
#define TAG_SIGN_GYM_TEXT       0x2E09
#define TAG_GYM_BILLBOARD_TOP   0x2E0A
#define TAG_GYM_BILLBOARD_BOTTOM 0x2E0B
#define TAG_DOJO_DOORWAY        0x2E0C
#define DOOR_GLOW_FRAMES        4 // Closed, then the 3 frames of the Kanto doors' animations
#define MAX_SIGN_SPRITES        16
#define SIGN_MAGIC              0x5167 // In data[7], to recognise the sprites after a sprite reset
#define METATILE_KANTO_POKEMON_CENTER_EMBLEM 0x05A
#define METATILE_KANTO_MART_EMBLEM_LEFT      0x039
#define METATILE_KANTO_POKEMON_CENTER_TEXT   0x061 // Right half of the "P.C" sign (Saffron has its own left half)
#define METATILE_KANTO_MART_TEXT             0x041 // Right half of the "MART" sign (Saffron has its own left half)
#define METATILE_KANTO_GYM_TEXT              0x151 // Top half of the "GYM" sign (Saffron has its own bottom half)
#define METATILE_KANTO_GYM_EMBLEM            0x153
#define METATILE_KANTO_GYM_BILLBOARD_TOP     0x160
#define METATILE_KANTO_GYM_BILLBOARD_BOTTOM  0x168
#define METATILE_SAFFRON_GYM_BILLBOARD_TOP   0x304 // Saffron's and Cinnabar's own copies
#define METATILE_SAFFRON_GYM_BILLBOARD_BOTTOM 0x30C
#define METATILE_CINNABAR_GYM_BILLBOARD_TOP  0x2BF
#define METATILE_SAFFRON_DOJO_DOORWAY        0x333
#define METATILE_KANTO_SLIDING_DOOR          0x062 // Pokemon Centers and Marts
#define METATILE_KANTO_GYM_DOOR              0x15B
#define METATILE_CELADON_DEPT_STORE_DOOR     0x294

#define sDoorX data[0] // A door sprite's door, in map coordinates with MAP_OFFSET
#define sDoorY data[1]
#define sIsDoor data[2]
#define sBaseTile data[3] // A door sprite's first tile, its closed frame

static const u32 sPokemonCenterSign_Gfx[] = INCGFX_U32("graphics/day_night/pokemon_center_sign.png", ".4bpp");
static const u16 sGlow_Pal[] = INCGFX_U16("graphics/day_night/pokemon_center_sign.png", ".gbapal");
static const u32 sPokemonCenterText_Gfx[] = INCGFX_U32("graphics/day_night/pokemon_center_text.png", ".4bpp");
static const u32 sGymBillboardTop_Gfx[] = INCGFX_U32("graphics/day_night/gym_billboard_top.png", ".4bpp");
static const u32 sGymBillboardBottom_Gfx[] = INCGFX_U32("graphics/day_night/gym_billboard_bottom.png", ".4bpp");
static const u32 sDojoDoorway_Gfx[] = INCGFX_U32("graphics/day_night/dojo_doorway.png", ".4bpp");
static const u32 sMartText_Gfx[] = INCGFX_U32("graphics/day_night/mart_text.png", ".4bpp");
static const u32 sGymText_Gfx[] = INCGFX_U32("graphics/day_night/gym_text.png", ".4bpp");
static const u32 sMartSign_Gfx[] = INCGFX_U32("graphics/day_night/mart_sign.png", ".4bpp");
static const u32 sGymSign_Gfx[] = INCGFX_U32("graphics/day_night/gym_sign.png", ".4bpp");
static const u32 sSlidingDoor_Gfx[] = INCGFX_U32("graphics/day_night/sliding_door.png", ".4bpp");
static const u32 sGymDoor_Gfx[] = INCGFX_U32("graphics/day_night/gym_door.png", ".4bpp");
static const u32 sDeptStoreDoor_Gfx[] = INCGFX_U32("graphics/day_night/dept_store_door.png", ".4bpp");

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

// The billboard's top metatile draws it on the top BG layer too.
static const struct OamData sOam_SmallTopLayer =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static void SpriteCB_Sign(struct Sprite *sprite);

static const struct SpriteTemplate sSpriteTemplate_PokemonCenterSign =
{
    .tileTag = TAG_SIGN_POKEMON_CENTER,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_PokemonCenterText =
{
    .tileTag = TAG_SIGN_POKEMON_CENTER_TEXT,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymBillboardTop =
{
    .tileTag = TAG_GYM_BILLBOARD_TOP,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_SmallTopLayer,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymBillboardBottom =
{
    .tileTag = TAG_GYM_BILLBOARD_BOTTOM,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_DojoDoorway =
{
    .tileTag = TAG_DOJO_DOORWAY,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_MartText =
{
    .tileTag = TAG_SIGN_MART_TEXT,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymText =
{
    .tileTag = TAG_SIGN_GYM_TEXT,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_MartSign =
{
    .tileTag = TAG_SIGN_MART,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Sign,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymSign =
{
    .tileTag = TAG_SIGN_GYM,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_SignTopLayer,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_SlidingDoor =
{
    .tileTag = TAG_DOOR_SLIDING,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_GymDoor =
{
    .tileTag = TAG_DOOR_GYM,
    .paletteTag = TAG_GLOW_PAL,
    .oam = &sOam_Door,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Sign,
};

static const struct SpriteTemplate sSpriteTemplate_DeptStoreDoor =
{
    .tileTag = TAG_DOOR_DEPT_STORE,
    .paletteTag = TAG_GLOW_PAL,
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
    const struct Tileset *secondaryTileset; // For a secondary metatile id; NULL for kanto_general's
};

static const struct GlowingSign sGlowingSigns[] =
{
    {METATILE_KANTO_POKEMON_CENTER_EMBLEM, -8, -16, 32, FALSE, &sSpriteTemplate_PokemonCenterSign,
     {sPokemonCenterSign_Gfx, 32 * 32 / 2, TAG_SIGN_POKEMON_CENTER}, {sGlow_Pal, TAG_GLOW_PAL}},
    // The letters are in the sprite's top half.
    {METATILE_KANTO_POKEMON_CENTER_TEXT, -16, 0, 32, FALSE, &sSpriteTemplate_PokemonCenterText,
     {sPokemonCenterText_Gfx, 32 * 32 / 2, TAG_SIGN_POKEMON_CENTER_TEXT}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_KANTO_MART_EMBLEM_LEFT, 0, -16, 32, FALSE, &sSpriteTemplate_MartSign,
     {sMartSign_Gfx, 32 * 32 / 2, TAG_SIGN_MART}, {sGlow_Pal, TAG_GLOW_PAL}},
    // The letters are in the sprite's top half.
    {METATILE_KANTO_MART_TEXT, -16, 0, 32, FALSE, &sSpriteTemplate_MartText,
     {sMartText_Gfx, 32 * 32 / 2, TAG_SIGN_MART_TEXT}, {sGlow_Pal, TAG_GLOW_PAL}},
    // The letters are in the sprite's left half, across the sign's two metatiles.
    {METATILE_KANTO_GYM_TEXT, 0, 0, 32, FALSE, &sSpriteTemplate_GymText,
     {sGymText_Gfx, 32 * 32 / 2, TAG_SIGN_GYM_TEXT}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_KANTO_GYM_BILLBOARD_TOP, 0, 0, 16, FALSE, &sSpriteTemplate_GymBillboardTop,
     {sGymBillboardTop_Gfx, 16 * 16 / 2, TAG_GYM_BILLBOARD_TOP}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_KANTO_GYM_BILLBOARD_BOTTOM, 0, 0, 16, FALSE, &sSpriteTemplate_GymBillboardBottom,
     {sGymBillboardBottom_Gfx, 16 * 16 / 2, TAG_GYM_BILLBOARD_BOTTOM}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_SAFFRON_GYM_BILLBOARD_TOP, 0, 0, 16, FALSE, &sSpriteTemplate_GymBillboardTop,
     {sGymBillboardTop_Gfx, 16 * 16 / 2, TAG_GYM_BILLBOARD_TOP}, {sGlow_Pal, TAG_GLOW_PAL}, &gTileset_KantoSaffronCity},
    {METATILE_SAFFRON_GYM_BILLBOARD_BOTTOM, 0, 0, 16, FALSE, &sSpriteTemplate_GymBillboardBottom,
     {sGymBillboardBottom_Gfx, 16 * 16 / 2, TAG_GYM_BILLBOARD_BOTTOM}, {sGlow_Pal, TAG_GLOW_PAL}, &gTileset_KantoSaffronCity},
    {METATILE_CINNABAR_GYM_BILLBOARD_TOP, 0, 0, 16, FALSE, &sSpriteTemplate_GymBillboardTop,
     {sGymBillboardTop_Gfx, 16 * 16 / 2, TAG_GYM_BILLBOARD_TOP}, {sGlow_Pal, TAG_GLOW_PAL}, &gTileset_KantoCinnabarIsland},
    {METATILE_SAFFRON_DOJO_DOORWAY, 0, 0, 16, FALSE, &sSpriteTemplate_DojoDoorway,
     {sDojoDoorway_Gfx, 16 * 16 / 2, TAG_DOJO_DOORWAY}, {sGlow_Pal, TAG_GLOW_PAL}, &gTileset_KantoSaffronCity},
    // The ball on the gold sign above the door is in the sprite's top half.
    {METATILE_KANTO_GYM_EMBLEM, -8, 0, 32, FALSE, &sSpriteTemplate_GymSign,
     {sGymSign_Gfx, 32 * 32 / 2, TAG_SIGN_GYM}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_KANTO_SLIDING_DOOR, 0, 0, 16, TRUE, &sSpriteTemplate_SlidingDoor,
     {sSlidingDoor_Gfx, 16 * 16 / 2 * DOOR_GLOW_FRAMES, TAG_DOOR_SLIDING}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_KANTO_GYM_DOOR, 0, 0, 16, TRUE, &sSpriteTemplate_GymDoor,
     {sGymDoor_Gfx, 16 * 16 / 2 * DOOR_GLOW_FRAMES, TAG_DOOR_GYM}, {sGlow_Pal, TAG_GLOW_PAL}},
    {METATILE_CELADON_DEPT_STORE_DOOR, 0, 0, 16, TRUE, &sSpriteTemplate_DeptStoreDoor,
     {sDeptStoreDoor_Gfx, 16 * 16 / 2 * DOOR_GLOW_FRAMES, TAG_DOOR_DEPT_STORE}, {sGlow_Pal, TAG_GLOW_PAL},
     &gTileset_KantoCeladonCity},
};

static EWRAM_DATA u8 sSignSpriteIds[MAX_SIGN_SPRITES] = {0};
static EWRAM_DATA u8 sNumSignSprites = 0;
static EWRAM_DATA bool8 sSignsShown = FALSE;
static EWRAM_DATA bool8 sSignsDirty = FALSE;

// Hides the sprite while it's off screen, so its coordinates can't wrap around into view,
// and shows a door's glass for the frame the door is drawn with.
static void SpriteCB_Sign(struct Sprite *sprite)
{
    s16 x = sprite->x + sprite->x2 + gSpriteCoordOffsetX;
    s16 y = sprite->y + sprite->y2 + gSpriteCoordOffsetY;

    sprite->invisible = (x < -32 || x > DISPLAY_WIDTH + 32 || y < -32 || y > DISPLAY_HEIGHT + 32);
    if (sprite->sIsDoor)
    {
        s32 frame = FieldGetDoorAnimFrame(sprite->sDoorX, sprite->sDoorY) + 1;

        if (frame >= DOOR_GLOW_FRAMES)
            sprite->invisible = TRUE;
        else
            sprite->oam.tileNum = sprite->sBaseTile + frame * 4;
    }
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
    gSprites[spriteId].sBaseTile = gSprites[spriteId].oam.tileNum;
    gSprites[spriteId].data[7] = SIGN_MAGIC;
    SpriteCB_Sign(&gSprites[spriteId]);
    sSignSpriteIds[sNumSignSprites++] = spriteId;
}

// Signs that stay dark: the Fighting Dojo's billboard, the same metatiles as the Saffron Gym's.
static const struct
{
    u16 layoutId;
    u8 x, y; // The billboard's top metatile
} sDarkSigns[] =
{
    {LAYOUT_SAFFRON_CITY,            42, 13},
    {LAYOUT_SAFFRON_CITY_CONNECTION, 32, 6},
};

static bool8 IsDarkSign(s32 x, s32 y)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDarkSigns); i++)
    {
        if (sDarkSigns[i].layoutId == gMapHeader.mapLayoutId && sDarkSigns[i].x == x
         && (sDarkSigns[i].y == y || sDarkSigns[i].y + 1 == y))
            return TRUE;
    }
    return FALSE;
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
                if (metatileId == sGlowingSigns[i].metatileId && sNumSignSprites < MAX_SIGN_SPRITES
                 && (sGlowingSigns[i].secondaryTileset == NULL || sGlowingSigns[i].secondaryTileset == layout->secondaryTileset)
                 && !IsDarkSign(x, y))
                    CreateSignSprite(&sGlowingSigns[i], x, y);
            }
        }
    }
}

// Street lamps (tools/kanto_port/make_lamp_sprites.py): a lantern on an iron post, standing on a tile its
// map gives collision, with its head on the tile above. It's a sprite sorted like an object standing on
// that tile. In the evening and at night its glass is lit, and it casts a pool of light: a sprite blended
// additively onto the ground (and only the ground, as sprites don't blend with sprites). Weathers that
// use the blend registers themselves (clouds, fog, ash, sandstorm, bubbles) go without the pools.
#define TAG_LAMP             0x2E10
#define TAG_LAMP_POOL        0x2E11
#define TAG_LAMP_PAL         0x2E12
#define LAMP_IRON_COLORS     COLORS(1, 4) // Tinted; the glass and the pools' colours aren't, once lit
#define LAMP_POOL_ALPHA      8            // The pools add 8/16 of their colour to the ground
#define LAMP_ELEVATION       3            // The ground's
#define MAX_LAMPS            12 // On a map
#define FIELD_BLDALPHA       BLDALPHA_BLEND(13, 7) // What the field sets up (InitOverworldGraphicsRegisters)

struct Lamp
{
    u16 layoutId;
    u8 x, y; // The tile it stands on
};

#include "data/day_night_lamps.h"

static const u32 sLamp_Gfx[] = INCGFX_U32("graphics/day_night/lamp.png", ".4bpp");
static const u16 sLampDay_Pal[] = INCGFX_U16("graphics/day_night/lamp.png", ".gbapal");
static const u16 sLampNight_Pal[] = INCGFX_U16("graphics/day_night/lamp_night.pal", ".gbapal");
static const u32 sLampPool_Gfx[] = INCGFX_U32("graphics/day_night/lamp_pool.png", ".4bpp");

static const struct OamData sOam_Lamp =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
};

static const struct OamData sOam_LampPool =
{
    .objMode = ST_OAM_OBJ_BLEND,
    .shape = SPRITE_SHAPE(64x32),
    .size = SPRITE_SIZE(64x32),
    .priority = 2, // Under the top BG layer (roofs, treetops), which stays dark
};

static void SpriteCB_Lamp(struct Sprite *sprite);
static void SpriteCB_LampPool(struct Sprite *sprite);

static const struct SpriteTemplate sSpriteTemplate_Lamp =
{
    .tileTag = TAG_LAMP,
    .paletteTag = TAG_LAMP_PAL,
    .oam = &sOam_Lamp,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Lamp,
};

static const struct SpriteTemplate sSpriteTemplate_LampPool =
{
    .tileTag = TAG_LAMP_POOL,
    .paletteTag = TAG_LAMP_PAL,
    .oam = &sOam_LampPool,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_LampPool,
};

static EWRAM_DATA u8 sLampSpriteIds[MAX_LAMPS * 2] = {0};
static EWRAM_DATA u8 sNumLampSprites = 0;
static EWRAM_DATA bool8 sLampPoolsShown = FALSE;
static EWRAM_DATA bool8 sLampBlendSet = FALSE;

static void SpriteCB_Lamp(struct Sprite *sprite)
{
    SpriteCB_Sign(sprite);
    sprite->oam.priority = ElevationToPriority(LAMP_ELEVATION);
    SetObjectSubpriorityByElevation(LAMP_ELEVATION, sprite, 1);
}

static void SpriteCB_LampPool(struct Sprite *sprite)
{
    SpriteCB_Sign(sprite);
    if (!sLampPoolsShown)
        sprite->invisible = TRUE;
}

static void DestroyLampSprites(void)
{
    u32 i;

    for (i = 0; i < sNumLampSprites; i++)
    {
        struct Sprite *sprite = &gSprites[sLampSpriteIds[i]];

        if (sprite->inUse && (sprite->callback == SpriteCB_Lamp || sprite->callback == SpriteCB_LampPool)
         && sprite->data[7] == SIGN_MAGIC)
            DestroySprite(sprite);
    }
    sNumLampSprites = 0;
    FreeSpriteTilesByTag(TAG_LAMP);
    FreeSpriteTilesByTag(TAG_LAMP_POOL);
    FreeSpritePaletteByTag(TAG_LAMP_PAL);
}

// (cx, cy): the sprite's centre, in pixels from the top-left corner of the lamp's tile
static void CreateLampSprite(const struct SpriteTemplate *template, const struct Lamp *lamp, s16 cx, s16 cy, u8 subpriority)
{
    s16 x, y;
    u8 spriteId;

    SetSpritePosToMapCoords(lamp->x + MAP_OFFSET, lamp->y + MAP_OFFSET, &x, &y);
    spriteId = CreateSprite(template, x + cx, y + cy, subpriority);
    if (spriteId == MAX_SPRITES)
        return;
    gSprites[spriteId].coordOffsetEnabled = TRUE;
    gSprites[spriteId].data[7] = SIGN_MAGIC;
    gSprites[spriteId].callback(&gSprites[spriteId]);
    sLampSpriteIds[sNumLampSprites++] = spriteId;
}

static void CreateLampSprites(bool8 lit)
{
    static const struct SpriteSheet lampSheet = {sLamp_Gfx, 16 * 32 / 2, TAG_LAMP};
    static const struct SpriteSheet poolSheet = {sLampPool_Gfx, 64 * 32 / 2, TAG_LAMP_POOL};
    struct SpritePalette palette = {lit ? sLampNight_Pal : sLampDay_Pal, TAG_LAMP_PAL};
    bool8 loaded = FALSE;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLamps) && sNumLampSprites + 2 <= ARRAY_COUNT(sLampSpriteIds); i++)
    {
        if (sLamps[i].layoutId != gMapHeader.mapLayoutId)
            continue;
        if (!loaded)
        {
            u8 paletteNum = LoadSpritePalette(&palette);

            if (paletteNum == 0xFF)
                return;
            UpdateSpritePaletteWithWeather(paletteNum);
            LoadSpriteSheet(&lampSheet);
            if (lit)
                LoadSpriteSheet(&poolSheet);
            loaded = TRUE;
        }
        // Positioned like an object's sprite on that tile: the post's foot on it, the head above.
        CreateLampSprite(&sSpriteTemplate_Lamp, &sLamps[i], 8, 0, 0);
        if (lit)
            CreateLampSprite(&sSpriteTemplate_LampPool, &sLamps[i], 8, 12, 0xFF);
    }
}

static bool8 IsWeatherUsingBlend(void)
{
    switch (gWeatherPtr->currWeather)
    {
    case WEATHER_SUNNY_CLOUDS:
    case WEATHER_FOG_HORIZONTAL:
    case WEATHER_FOG_DIAGONAL:
    case WEATHER_VOLCANIC_ASH:
    case WEATHER_SANDSTORM:
    case WEATHER_UNDERWATER_BUBBLES:
        return TRUE;
    }
    return FALSE;
}

// The pools add their colour to the ground: the ground's full colour (16/16) plus LAMP_POOL_ALPHA/16 of theirs.
static void UpdateLampPools(bool8 lit)
{
    sLampPoolsShown = lit && sNumLampSprites != 0 && !IsWeatherUsingBlend();
    if (sLampPoolsShown)
    {
        SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(LAMP_POOL_ALPHA, 16));
        sLampBlendSet = TRUE;
    }
    else if (sLampBlendSet)
    {
        if (!IsWeatherUsingBlend())
            SetGpuReg(REG_OFFSET_BLDALPHA, FIELD_BLDALPHA);
        sLampBlendSet = FALSE;
    }
}

static void UpdateSigns(bool8 lit)
{
    if (sSignsDirty || lit != sSignsShown)
    {
        DestroySignSprites();
        DestroyLampSprites();
        CreateLampSprites(lit);
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
static EWRAM_DATA bool8 sLitColorsUsed = FALSE;

static void TintPalettes(bool8 all, const u16 *tintedColors, bool8 useLitColors)
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

static void UpdateTint(bool8 reentered, const u16 *tintedColors, bool8 useLitColors)
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

    // A new tint or a change to which colours are tinted redoes every colour; otherwise only
    // the colours that changed in gPlttBufferFaded since the last frame are tinted again.
    TintPalettes(!sTintActive || reentered || sRetintAll || masksChanged || useLitColors != sLitColorsUsed
                 || sTint.r != prevTint.r || sTint.g != prevTint.g || sTint.b != prevTint.b,
                 tintedColors, useLitColors);
    sLitColorsUsed = useLitColors;
    sRetintAll = FALSE;
    sTintActive = TRUE;
}

// Runs once per overworld frame, after the palette fade has been updated. The field tints the
// map's BG palettes (the others are UI: text boxes, menus, the map name popup) and every sprite
// but the glowing emblems and door glass.
void DayNight_UpdateField(void)
{
    bool8 reentered = StartTintUpdate();
    bool8 lit = ShouldLightWindows();
    u16 tintedColors[NUM_TINT_PALETTES];
    u32 i;

    if (lit != sWindowsLit)
        SetWindowsLit(lit, FALSE);
    UpdateSigns(lit);
    UpdateLampPools(lit);

    for (i = 0; i < 16; i++)
    {
        u16 tag = GetSpritePaletteTagByPaletteNum(i);

        tintedColors[i] = (i < NUM_PALS_TOTAL) ? ALL_COLORS : 0;
        tintedColors[OBJ_PAL(i)] = (tag == TAG_GLOW_PAL) ? 0 : ALL_COLORS;
        if (tag == TAG_LAMP_PAL && lit)
            tintedColors[OBJ_PAL(i)] = LAMP_IRON_COLORS;
    }
    UpdateTint(reentered, tintedColors, TRUE);
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
    UpdateTint(reentered, tintedColors, FALSE);
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

// The palettes the VBlank would show, for copies to palette RAM outside it (BeginNormalPaletteFade):
// gPlttBufferFaded, tinted first while the tint is in use, so a fade doesn't flash the untinted colours.
const u16 *DayNight_GetPlttBufferToShow(void)
{
    if (sTintActive && gMain.vblankCounter1 - sLastTintFrame <= 2)
    {
        TintPalettes(FALSE, sTintedColors, sLitColorsUsed);
        return sTintedPltt;
    }
    return gPlttBufferFaded;
}
