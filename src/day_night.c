#include "global.h"
#include "day_night.h"
#include "event_data.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "tilesets.h"
#include "constants/field_weather.h"
#include "constants/rgb.h"

// Day/night tinting for outdoor maps.
//
// The tint is applied to the final palettes, after weather and fades have
// written gPlttBufferFaded: each overworld frame DayNight_UpdateField tints
// that buffer into sTintedPltt, and the field VBlank copies sTintedPltt to
// palette RAM instead. The UI palettes (BG palettes past the map's) are
// left alone so text boxes and popups keep their colours.
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

static void TintPalettes(bool8 all)
{
    u32 i;

    for (i = 0; i < PLTT_BUFFER_SIZE; i++)
    {
        u16 color = gPlttBufferFaded[i];

        if (!all && color == sTintSource[i])
            continue;
        sTintSource[i] = color;

        if (i < BG_PLTT_ID(NUM_PALS_TOTAL))
        {
            if (sUntintedColors[i / 16] & (1 << (i % 16)))
                sTintedPltt[i] = color;
            else
                sTintedPltt[i] = TintColor(color);
        }
        else if (i < OBJ_PLTT_OFFSET)
        {
            // UI palettes: text boxes, menus, the map name popup
            sTintedPltt[i] = color;
        }
        else
        {
            sTintedPltt[i] = TintColor(color);
        }
    }
}

// Runs once per overworld frame, after the palette fade has been updated.
void DayNight_UpdateField(void)
{
    const struct TintMultipliers *target;
    struct TintMultipliers prevTint = sTint;
    bool8 reentered = (gMain.vblankCounter1 - sLastTintFrame > 2);
    bool8 lit;
    u8 phase;

    sLastTintFrame = gMain.vblankCounter1;
    if (reentered || ++sClockTimer >= CLOCK_CHECK_FRAMES)
    {
        sClockTimer = 0;
        sClockPhase = ReadClockPhase();
    }

    lit = ShouldLightWindows();
    if (lit != sWindowsLit)
        SetWindowsLit(lit, FALSE);

    phase = IsMapTinted() ? DayNight_GetPhase() : DAY_NIGHT_PHASE_DAY;
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

    // A new tint or a change to which colours are lit redoes every colour; otherwise only
    // the colours that changed in gPlttBufferFaded since the last frame are tinted again.
    TintPalettes(!sTintActive || reentered || sRetintAll
                 || sTint.r != prevTint.r || sTint.g != prevTint.g || sTint.b != prevTint.b);
    sRetintAll = FALSE;
    sTintActive = TRUE;
}

// Replaces TransferPlttBuffer in the field VBlank. The tinted buffer is only used while
// the overworld keeps it up to date, so a screen that borrows the field VBlank while the
// overworld isn't running (e.g. during a map load) gets the plain palettes.
void DayNight_TransferPlttBuffer(void)
{
    if (sTintActive && gMain.vblankCounter1 - sLastTintFrame <= 2)
        TransferPlttBufferFrom(sTintedPltt);
    else
        TransferPlttBuffer();
}
