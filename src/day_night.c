#include "global.h"
#include "day_night.h"
#include "event_data.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "sprite.h"
#include "tilesets.h"
#include "constants/field_weather.h"
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
    const struct Tileset *tileset;
    u8 paletteNum;
    u8 firstColor;
    u8 numColors;
    const u16 *colors;
};

static const struct TintMultipliers sPhaseTints[DAY_NIGHT_PHASE_COUNT] =
{
    [DAY_NIGHT_PHASE_MORNING] = {248, 236, 228},
    [DAY_NIGHT_PHASE_DAY]     = {TINT_FULL, TINT_FULL, TINT_FULL},
    [DAY_NIGHT_PHASE_EVENING] = {256, 208, 176},
    [DAY_NIGHT_PHASE_NIGHT]   = {104, 120, 168},
};

// The houses' window glass: colours 9-12 of palette 7 (see split_pallet_window_palette.py),
// from the bright bottom row of the glass to its dark top row.
static const u16 sPalletTownHouseWindowLight[] =
{
    RGB(31, 30, 20),
    RGB(31, 27, 14),
    RGB(30, 24, 10),
    RGB(28, 20, 8),
};

// Oak's lab's window and door glass: colours 8-10 of palette 9, light to dark.
static const u16 sPalletTownLabWindowLight[] =
{
    RGB(31, 29, 18),
    RGB(31, 25, 12),
    RGB(29, 21, 9),
};

static const struct LitPalette sLitPalettes[] =
{
    {&gTileset_KantoPalletTown, 7, 9, ARRAY_COUNT(sPalletTownHouseWindowLight), sPalletTownHouseWindowLight},
    {&gTileset_KantoPalletTown, 9, 8, ARRAY_COUNT(sPalletTownLabWindowLight), sPalletTownLabWindowLight},
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

        if (lp->tileset != layout->primaryTileset && lp->tileset != layout->secondaryTileset)
            continue;

        for (j = 0; j < lp->numColors; j++)
        {
            u16 index = BG_PLTT_ID(lp->paletteNum) + lp->firstColor + j;
            u16 color;

            if (lit)
            {
                color = lp->colors[j];
                sUntintedColors[lp->paletteNum] |= 1 << (lp->firstColor + j);
            }
            else
            {
                color = lp->tileset->palettes[lp->paletteNum][lp->firstColor + j];
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
                for (j = 0; j < lp->numColors; j++)
                {
                    u16 index = BG_PLTT_ID(lp->paletteNum) + lp->firstColor + j;
                    gPlttBufferFaded[index] = gPlttBufferUnfaded[index];
                }
            }
        }
    }
}

// Called after the map's tileset palettes are (re)loaded.
void DayNight_OnTilesetPalettesLoaded(void)
{
    sClockPhase = ReadClockPhase();
    sClockTimer = 0;
    SetWindowsLit(ShouldLightWindows(), TRUE);
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

// Bit n of a palette mask is BG palette n (0-15); bit 16 + n is OBJ palette n.
#define BG_PAL_BIT(n)  (1 << (n))
#define OBJ_PAL_BIT(n) (1 << (16 + (n)))

// The field tints the map's BG palettes (the rest are UI: text boxes, menus, the map name popup)
// and every sprite.
#define FIELD_TINTED_PALS (((1 << NUM_PALS_TOTAL) - 1) | 0xFFFF0000)

// Battles tint the scene but not the UI or the move animations: the battle environment
// (BG 2-4), the battlers' palettes (OBJ 0-3, which hold the Pokemon and the player's back
// pic, plus their BG 8-11 copies used by some animations), and the opposing trainers' pics.
#define BATTLE_ENVIRONMENT_PALS (BG_PAL_BIT(2) | BG_PAL_BIT(3) | BG_PAL_BIT(4))
#define BATTLE_MON_PALS         (BG_PAL_BIT(8) | BG_PAL_BIT(9) | BG_PAL_BIT(10) | BG_PAL_BIT(11) \
                                 | OBJ_PAL_BIT(0) | OBJ_PAL_BIT(1) | OBJ_PAL_BIT(2) | OBJ_PAL_BIT(3))

static EWRAM_DATA u32 sTintedPals = 0;    // Palette mask that sTintedPltt was made with
static EWRAM_DATA bool8 sLitColorsUsed = FALSE;

static void TintPalettes(bool8 all, u32 tintedPals, bool8 useLitColors)
{
    u32 i;

    for (i = 0; i < PLTT_BUFFER_SIZE; i++)
    {
        u16 color = gPlttBufferFaded[i];
        u32 pal = i / 16;

        if (!all && color == sTintSource[i])
            continue;
        sTintSource[i] = color;

        if (!(tintedPals & (1 << pal))
         || (useLitColors && pal < NUM_PALS_TOTAL && (sUntintedColors[pal] & (1 << (i % 16)))))
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

static void UpdateTint(bool8 reentered, u32 tintedPals, bool8 useLitColors)
{
    const struct TintMultipliers *target;
    struct TintMultipliers prevTint = sTint;
    u8 phase = IsMapTinted() ? DayNight_GetPhase() : DAY_NIGHT_PHASE_DAY;

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

    // A new tint or a change to which colours are tinted redoes every colour; otherwise only
    // the colours that changed in gPlttBufferFaded since the last frame are tinted again.
    TintPalettes(!sTintActive || reentered || sRetintAll
                 || tintedPals != sTintedPals || useLitColors != sLitColorsUsed
                 || sTint.r != prevTint.r || sTint.g != prevTint.g || sTint.b != prevTint.b,
                 tintedPals, useLitColors);
    sTintedPals = tintedPals;
    sLitColorsUsed = useLitColors;
    sRetintAll = FALSE;
    sTintActive = TRUE;
}

// Runs once per overworld frame, after the palette fade has been updated.
void DayNight_UpdateField(void)
{
    bool8 reentered = StartTintUpdate();
    bool8 lit = ShouldLightWindows();

    if (lit != sWindowsLit)
        SetWindowsLit(lit, FALSE);
    UpdateTint(reentered, FIELD_TINTED_PALS, TRUE);
}

// Runs once per battle frame. The battle takes the tint of the map it was started on.
void DayNight_UpdateBattle(void)
{
    bool8 reentered = StartTintUpdate();
    u32 tintedPals = BATTLE_ENVIRONMENT_PALS | BATTLE_MON_PALS;
    u32 i;

    // Trainer front pics get a free sprite palette tagged with their TRAINER_PIC_* id, and the
    // player's (or partner's) back pic moves to one tagged 0xD6F8 (0xD6F9) while throwing the
    // first Poke Ball, so the Pokemon can take over the battler's palette.
    for (i = MAX_BATTLERS_COUNT; i < 16; i++)
    {
        u16 tag = GetSpritePaletteTagByPaletteNum(i);

        if (tag < TRAINER_PIC_COUNT || tag == 0xD6F8 || tag == 0xD6F9)
            tintedPals |= OBJ_PAL_BIT(i);
    }
    UpdateTint(reentered, tintedPals, FALSE);
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
