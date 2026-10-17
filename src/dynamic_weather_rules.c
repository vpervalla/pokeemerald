#include "global.h"
#include "dynamic_weather.h"
#include "constants/day_night.h"
#include "constants/weather.h"

// The dynamic weather's tables and its draw. Nothing here reads the save, the clock or the
// map, so tools/dynamic_weather/run_tests.sh can build it on the host.
//
// Each day, each zone draws one day pattern from its weights (percent, summing to 100).
// The draw hashes the save's trainer ID, the day and the zone, so it gives the same pattern
// every time it is made, needs no save space, and differs between saves.

struct DynamicWeatherZone
{
    const u8 *name;
    u8 fallback; // Before the story turns the dynamic weather on
    u8 weights[DYN_PATTERN_COUNT];
};

#define FOG WEATHER_FOG_HORIZONTAL

static const u8 sPatternWeather[DYN_PATTERN_COUNT][DAY_NIGHT_PHASE_COUNT] =
{                                 // MORNING                 DAY                        EVENING               NIGHT
    [DYN_PATTERN_CLEAR]       = {WEATHER_SUNNY,            WEATHER_SUNNY,             WEATHER_SUNNY,        WEATHER_SUNNY},
    [DYN_PATTERN_CLOUDY]      = {WEATHER_SUNNY_CLOUDS,     WEATHER_SUNNY_CLOUDS,      WEATHER_SUNNY,        WEATHER_SUNNY},
    [DYN_PATTERN_OVERCAST]    = {WEATHER_SHADE,            WEATHER_SHADE,             WEATHER_SHADE,        WEATHER_SHADE},
    [DYN_PATTERN_MORNING_FOG] = {FOG,                      WEATHER_SUNNY,             WEATHER_SUNNY,        FOG},
    [DYN_PATTERN_SHOWERS]     = {WEATHER_SUNNY,            WEATHER_RAIN,              WEATHER_SUNNY_CLOUDS, WEATHER_SUNNY},
    [DYN_PATTERN_RAINY_DAY]   = {WEATHER_RAIN,             WEATHER_RAIN,              WEATHER_SHADE,        WEATHER_SHADE},
    [DYN_PATTERN_NIGHT_RAIN]  = {WEATHER_SUNNY,            WEATHER_SUNNY,             WEATHER_SHADE,        WEATHER_RAIN},
    [DYN_PATTERN_STORM]       = {WEATHER_RAIN,             WEATHER_RAIN_THUNDERSTORM, WEATHER_RAIN,         WEATHER_SHADE},
    [DYN_PATTERN_DOWNPOUR]    = {WEATHER_RAIN_THUNDERSTORM, WEATHER_DOWNPOUR,         WEATHER_RAIN,         WEATHER_RAIN},
    [DYN_PATTERN_HEAT_WAVE]   = {WEATHER_SUNNY,            WEATHER_DROUGHT,           WEATHER_SUNNY,        WEATHER_SUNNY},
    [DYN_PATTERN_DUST]        = {WEATHER_SANDSTORM,        WEATHER_SANDSTORM,         WEATHER_SUNNY,        WEATHER_SUNNY},
    [DYN_PATTERN_SNOWFALL]    = {WEATHER_SNOW,             WEATHER_SHADE,             WEATHER_SNOW,         WEATHER_SNOW},
    [DYN_PATTERN_FLURRIES]    = {WEATHER_SUNNY,            WEATHER_SUNNY,             WEATHER_SHADE,        WEATHER_SNOW},
};

#undef FOG

static const u8 sPatternName_Clear[] = _("CLEAR");
static const u8 sPatternName_Cloudy[] = _("CLOUDY");
static const u8 sPatternName_Overcast[] = _("OVERCAST");
static const u8 sPatternName_MorningFog[] = _("MORNING FOG");
static const u8 sPatternName_Showers[] = _("SHOWERS");
static const u8 sPatternName_RainyDay[] = _("RAINY DAY");
static const u8 sPatternName_NightRain[] = _("NIGHT RAIN");
static const u8 sPatternName_Storm[] = _("STORM");
static const u8 sPatternName_Downpour[] = _("DOWNPOUR");
static const u8 sPatternName_HeatWave[] = _("HEAT WAVE");
static const u8 sPatternName_Dust[] = _("DUST");
static const u8 sPatternName_Snowfall[] = _("SNOWFALL");
static const u8 sPatternName_Flurries[] = _("FLURRIES");

static const u8 *const sPatternNames[DYN_PATTERN_COUNT] =
{
    [DYN_PATTERN_CLEAR]       = sPatternName_Clear,
    [DYN_PATTERN_CLOUDY]      = sPatternName_Cloudy,
    [DYN_PATTERN_OVERCAST]    = sPatternName_Overcast,
    [DYN_PATTERN_MORNING_FOG] = sPatternName_MorningFog,
    [DYN_PATTERN_SHOWERS]     = sPatternName_Showers,
    [DYN_PATTERN_RAINY_DAY]   = sPatternName_RainyDay,
    [DYN_PATTERN_NIGHT_RAIN]  = sPatternName_NightRain,
    [DYN_PATTERN_STORM]       = sPatternName_Storm,
    [DYN_PATTERN_DOWNPOUR]    = sPatternName_Downpour,
    [DYN_PATTERN_HEAT_WAVE]   = sPatternName_HeatWave,
    [DYN_PATTERN_DUST]        = sPatternName_Dust,
    [DYN_PATTERN_SNOWFALL]    = sPatternName_Snowfall,
    [DYN_PATTERN_FLURRIES]    = sPatternName_Flurries,
};

static const u8 sZoneName_Viridian[] = _("VIRIDIAN");

static const struct DynamicWeatherZone sZones[DYN_ZONE_COUNT] =
{
    [DYN_ZONE_VIRIDIAN] =
    {
        .name = sZoneName_Viridian,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 45,
            [DYN_PATTERN_CLOUDY]      = 20,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SHOWERS]     = 15,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 5,
        },
    },
};

// A 32-bit integer mixer (MurmurHash3's finalizer): every input bit changes about half the
// output bits, so neighbouring days and zones get unrelated draws.
static u32 Mix(u32 x)
{
    x ^= x >> 16;
    x *= 0x85EBCA6B;
    x ^= x >> 13;
    x *= 0xC2B2AE35;
    x ^= x >> 16;
    return x;
}

u8 DynamicWeather_PickPattern(u8 zone, u32 seed, u16 day)
{
    u32 roll, total;
    u8 pattern;

    if (zone >= DYN_ZONE_COUNT)
        return DYN_PATTERN_CLEAR;

    roll = Mix(Mix(seed ^ day) + zone) % 100;
    total = 0;
    for (pattern = 0; pattern < DYN_PATTERN_COUNT; pattern++)
    {
        total += sZones[zone].weights[pattern];
        if (roll < total)
            return pattern;
    }
    return DYN_PATTERN_CLEAR;
}

u8 DynamicWeather_GetPatternWeather(u8 pattern, u8 phase)
{
    if (pattern >= DYN_PATTERN_COUNT || phase >= DAY_NIGHT_PHASE_COUNT)
        return WEATHER_SUNNY;
    return sPatternWeather[pattern][phase];
}

u8 DynamicWeather_GetFallback(u8 zone)
{
    if (zone >= DYN_ZONE_COUNT)
        return WEATHER_NONE;
    return sZones[zone].fallback;
}

u8 DynamicWeather_GetPatternWeight(u8 zone, u8 pattern)
{
    if (zone >= DYN_ZONE_COUNT || pattern >= DYN_PATTERN_COUNT)
        return 0;
    return sZones[zone].weights[pattern];
}

const u8 *DynamicWeather_GetZoneName(u8 zone)
{
    if (zone >= DYN_ZONE_COUNT)
        return NULL;
    return sZones[zone].name;
}

const u8 *DynamicWeather_GetPatternName(u8 pattern)
{
    if (pattern >= DYN_PATTERN_COUNT)
        return NULL;
    return sPatternNames[pattern];
}
