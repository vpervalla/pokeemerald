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
    const u8 *name;     // For the Test Area
    const u8 *areaName; // For the weather forecasters
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

const u8 gText_DynamicWeatherZone_Viridian[] = _("VIRIDIAN");
const u8 gText_DynamicWeatherZone_Pewter[] = _("PEWTER");
const u8 gText_DynamicWeatherZone_Cerulean[] = _("CERULEAN");
const u8 gText_DynamicWeatherZone_Vermilion[] = _("VERMILION");
const u8 gText_DynamicWeatherZone_Lavender[] = _("LAVENDER");
const u8 gText_DynamicWeatherZone_Celadon[] = _("CELADON");
const u8 gText_DynamicWeatherZone_Fuchsia[] = _("FUCHSIA");
const u8 gText_DynamicWeatherZone_Sea[] = _("SEA");
const u8 gText_DynamicWeatherZone_Indigo[] = _("INDIGO");
const u8 gText_DynamicWeatherZone_SeviiNorth[] = _("SEVII NORTH");
const u8 gText_DynamicWeatherZone_FourIsland[] = _("FOUR ISLAND");
const u8 gText_DynamicWeatherZone_SeviiFourFive[] = _("FIVE ISLAND");
const u8 gText_DynamicWeatherZone_SeviiSouth[] = _("SEVII SOUTH");
const u8 gText_DynamicWeatherZone_MegaIsland[] = _("MEGA ISLAND");

static const u8 sAreaName_Viridian[] = _("PALLET and VIRIDIAN");
static const u8 sAreaName_Pewter[] = _("PEWTER CITY");
static const u8 sAreaName_Cerulean[] = _("CERULEAN CITY");
static const u8 sAreaName_Vermilion[] = _("VERMILION CITY");
static const u8 sAreaName_Lavender[] = _("LAVENDER TOWN");
static const u8 sAreaName_Celadon[] = _("CELADON and SAFFRON");
static const u8 sAreaName_Fuchsia[] = _("FUCHSIA CITY");
static const u8 sAreaName_Sea[] = _("CINNABAR and the sea");
static const u8 sAreaName_Indigo[] = _("INDIGO PLATEAU");
static const u8 sAreaName_SeviiNorth[] = _("ONE, TWO and THREE ISLAND");
static const u8 sAreaName_FourIsland[] = _("FOUR ISLAND");
static const u8 sAreaName_SeviiFourFive[] = _("FIVE ISLAND");
static const u8 sAreaName_SeviiSouth[] = _("SIX and SEVEN ISLAND");

static const u8 sForecast_Clear[] = _("clear skies");
static const u8 sForecast_Cloudy[] = _("cloudy, then clear");
static const u8 sForecast_Overcast[] = _("overcast all day");
static const u8 sForecast_MorningFog[] = _("early and late fog");
static const u8 sForecast_Showers[] = _("a midday shower");
static const u8 sForecast_RainyDay[] = _("rain until evening");
static const u8 sForecast_NightRain[] = _("rain tonight");
static const u8 sForecast_Storm[] = _("storms by day");
static const u8 sForecast_Downpour[] = _("heavy rain all day");
static const u8 sForecast_HeatWave[] = _("harsh sun at midday");
static const u8 sForecast_Dust[] = _("sandstorms by day");
static const u8 sForecast_Snowfall[] = _("snow on and off");
static const u8 sForecast_Flurries[] = _("snow tonight");

static const u8 sWeatherWord_Clear[] = _("clear skies");
static const u8 sWeatherWord_Cloudy[] = _("cloudy");
static const u8 sWeatherWord_Overcast[] = _("overcast");
static const u8 sWeatherWord_Fog[] = _("fog");
static const u8 sWeatherWord_Rain[] = _("rain");
static const u8 sWeatherWord_Storm[] = _("a thunderstorm");
static const u8 sWeatherWord_Downpour[] = _("heavy rain");
static const u8 sWeatherWord_HarshSun[] = _("harsh sun");
static const u8 sWeatherWord_Sandstorm[] = _("a sandstorm");
static const u8 sWeatherWord_Snow[] = _("snow");

// What the weather forecasters say of each pattern weather, for "now" and "later today"
const u8 *DynamicWeather_GetWeatherWord(u8 weather)
{
    switch (weather)
    {
    case WEATHER_SUNNY_CLOUDS:      return sWeatherWord_Cloudy;
    case WEATHER_SHADE:             return sWeatherWord_Overcast;
    case WEATHER_FOG_HORIZONTAL:    return sWeatherWord_Fog;
    case WEATHER_RAIN:              return sWeatherWord_Rain;
    case WEATHER_RAIN_THUNDERSTORM: return sWeatherWord_Storm;
    case WEATHER_DOWNPOUR:          return sWeatherWord_Downpour;
    case WEATHER_DROUGHT:           return sWeatherWord_HarshSun;
    case WEATHER_SANDSTORM:         return sWeatherWord_Sandstorm;
    case WEATHER_SNOW:              return sWeatherWord_Snow;
    default:                        return sWeatherWord_Clear;
    }
}

// What the weather forecasters say of a day with each pattern
static const u8 *const sForecastPhrases[DYN_PATTERN_COUNT] =
{
    [DYN_PATTERN_CLEAR]       = sForecast_Clear,
    [DYN_PATTERN_CLOUDY]      = sForecast_Cloudy,
    [DYN_PATTERN_OVERCAST]    = sForecast_Overcast,
    [DYN_PATTERN_MORNING_FOG] = sForecast_MorningFog,
    [DYN_PATTERN_SHOWERS]     = sForecast_Showers,
    [DYN_PATTERN_RAINY_DAY]   = sForecast_RainyDay,
    [DYN_PATTERN_NIGHT_RAIN]  = sForecast_NightRain,
    [DYN_PATTERN_STORM]       = sForecast_Storm,
    [DYN_PATTERN_DOWNPOUR]    = sForecast_Downpour,
    [DYN_PATTERN_HEAT_WAVE]   = sForecast_HeatWave,
    [DYN_PATTERN_DUST]        = sForecast_Dust,
    [DYN_PATTERN_SNOWFALL]    = sForecast_Snowfall,
    [DYN_PATTERN_FLURRIES]    = sForecast_Flurries,
};

#define FORECAST_END DYN_ZONE_COUNT

// The zones each weather forecaster covers, nearest first. MEGA ISLAND is a secret: no one forecasts it.
static const u8 sForecastZones[FORECAST_COUNT][5] =
{
    [FORECAST_VIRIDIAN]   = {DYN_ZONE_VIRIDIAN, DYN_ZONE_PEWTER, DYN_ZONE_INDIGO, FORECAST_END},
    [FORECAST_CERULEAN]   = {DYN_ZONE_CERULEAN, DYN_ZONE_VERMILION, DYN_ZONE_LAVENDER, FORECAST_END},
    [FORECAST_CELADON]    = {DYN_ZONE_CELADON, DYN_ZONE_FUCHSIA, DYN_ZONE_SEA, FORECAST_END},
    [FORECAST_ONE_ISLAND] = {DYN_ZONE_SEVII_NORTH, DYN_ZONE_FOUR_ISLAND, DYN_ZONE_SEVII_FOUR_FIVE, DYN_ZONE_SEVII_SOUTH, FORECAST_END},
};

static const struct DynamicWeatherZone sZones[DYN_ZONE_COUNT] =
{
    [DYN_ZONE_VIRIDIAN] =
    {
        .name = gText_DynamicWeatherZone_Viridian,
        .areaName = sAreaName_Viridian,
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
    [DYN_ZONE_PEWTER] =
    {
        .name = gText_DynamicWeatherZone_Pewter,
        .areaName = sAreaName_Pewter,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]     = 45,
            [DYN_PATTERN_CLOUDY]    = 15,
            [DYN_PATTERN_OVERCAST]  = 5,
            [DYN_PATTERN_SHOWERS]   = 10,
            [DYN_PATTERN_STORM]     = 5,
            [DYN_PATTERN_HEAT_WAVE] = 5,
            [DYN_PATTERN_DUST]      = 15,
        },
    },
    [DYN_ZONE_CERULEAN] =
    {
        .name = gText_DynamicWeatherZone_Cerulean,
        .areaName = sAreaName_Cerulean,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 25,
            [DYN_PATTERN_CLOUDY]      = 15,
            [DYN_PATTERN_OVERCAST]    = 5,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SHOWERS]     = 20,
            [DYN_PATTERN_RAINY_DAY]   = 15,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 10,
        },
    },
    [DYN_ZONE_VERMILION] =
    {
        .name = gText_DynamicWeatherZone_Vermilion,
        .areaName = sAreaName_Vermilion,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 35,
            [DYN_PATTERN_CLOUDY]      = 15,
            [DYN_PATTERN_MORNING_FOG] = 20,
            [DYN_PATTERN_SHOWERS]     = 15,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 5,
        },
    },
    [DYN_ZONE_LAVENDER] =
    {
        .name = gText_DynamicWeatherZone_Lavender,
        .areaName = sAreaName_Lavender,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 20,
            [DYN_PATTERN_CLOUDY]      = 10,
            [DYN_PATTERN_OVERCAST]    = 25,
            [DYN_PATTERN_MORNING_FOG] = 15,
            [DYN_PATTERN_SHOWERS]     = 5,
            [DYN_PATTERN_RAINY_DAY]   = 10,
            [DYN_PATTERN_NIGHT_RAIN]  = 10,
            [DYN_PATTERN_STORM]       = 5,
        },
    },
    [DYN_ZONE_CELADON] =
    {
        .name = gText_DynamicWeatherZone_Celadon,
        .areaName = sAreaName_Celadon,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]      = 40,
            [DYN_PATTERN_CLOUDY]     = 25,
            [DYN_PATTERN_OVERCAST]   = 5,
            [DYN_PATTERN_SHOWERS]    = 15,
            [DYN_PATTERN_RAINY_DAY]  = 5,
            [DYN_PATTERN_NIGHT_RAIN] = 5,
            [DYN_PATTERN_STORM]      = 5,
        },
    },
    [DYN_ZONE_FUCHSIA] =
    {
        .name = gText_DynamicWeatherZone_Fuchsia,
        .areaName = sAreaName_Fuchsia,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 35,
            [DYN_PATTERN_CLOUDY]      = 10,
            [DYN_PATTERN_MORNING_FOG] = 20,
            [DYN_PATTERN_SHOWERS]     = 15,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 5,
            [DYN_PATTERN_HEAT_WAVE]   = 5,
        },
    },
    [DYN_ZONE_SEA] =
    {
        .name = gText_DynamicWeatherZone_Sea,
        .areaName = sAreaName_Sea,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 30,
            [DYN_PATTERN_CLOUDY]      = 10,
            [DYN_PATTERN_MORNING_FOG] = 15,
            [DYN_PATTERN_SHOWERS]     = 10,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 10,
            [DYN_PATTERN_DOWNPOUR]    = 10,
            [DYN_PATTERN_HEAT_WAVE]   = 10,
        },
    },
    [DYN_ZONE_INDIGO] =
    {
        .name = gText_DynamicWeatherZone_Indigo,
        .areaName = sAreaName_Indigo,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 25,
            [DYN_PATTERN_CLOUDY]      = 20,
            [DYN_PATTERN_OVERCAST]    = 15,
            [DYN_PATTERN_MORNING_FOG] = 15,
            [DYN_PATTERN_SHOWERS]     = 5,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_STORM]       = 15,
        },
    },
    [DYN_ZONE_SEVII_NORTH] =
    {
        .name = gText_DynamicWeatherZone_SeviiNorth,
        .areaName = sAreaName_SeviiNorth,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 40,
            [DYN_PATTERN_CLOUDY]      = 15,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SHOWERS]     = 10,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 5,
            [DYN_PATTERN_HEAT_WAVE]   = 20,
        },
    },
    [DYN_ZONE_FOUR_ISLAND] =
    {
        .name = gText_DynamicWeatherZone_FourIsland,
        .areaName = sAreaName_FourIsland,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 20,
            [DYN_PATTERN_CLOUDY]      = 10,
            [DYN_PATTERN_OVERCAST]    = 15,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SNOWFALL]    = 30,
            [DYN_PATTERN_FLURRIES]    = 20,
        },
    },
    [DYN_ZONE_SEVII_FOUR_FIVE] =
    {
        .name = gText_DynamicWeatherZone_SeviiFourFive,
        .areaName = sAreaName_SeviiFourFive,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 30,
            [DYN_PATTERN_CLOUDY]      = 20,
            [DYN_PATTERN_OVERCAST]    = 10,
            [DYN_PATTERN_MORNING_FOG] = 20,
            [DYN_PATTERN_SHOWERS]     = 10,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
        },
    },
    [DYN_ZONE_SEVII_SOUTH] =
    {
        .name = gText_DynamicWeatherZone_SeviiSouth,
        .areaName = sAreaName_SeviiSouth,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 30,
            [DYN_PATTERN_CLOUDY]      = 15,
            [DYN_PATTERN_OVERCAST]    = 5,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SHOWERS]     = 10,
            [DYN_PATTERN_RAINY_DAY]   = 5,
            [DYN_PATTERN_NIGHT_RAIN]  = 5,
            [DYN_PATTERN_STORM]       = 15,
            [DYN_PATTERN_DUST]        = 10,
        },
    },
    [DYN_ZONE_MEGA_ISLAND] =
    {
        .name = gText_DynamicWeatherZone_MegaIsland,
        .fallback = WEATHER_SUNNY,
        .weights =
        {
            [DYN_PATTERN_CLEAR]       = 55,
            [DYN_PATTERN_CLOUDY]      = 20,
            [DYN_PATTERN_MORNING_FOG] = 5,
            [DYN_PATTERN_SHOWERS]     = 10,
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

const u8 *DynamicWeather_GetAreaName(u8 zone)
{
    if (zone >= DYN_ZONE_COUNT)
        return NULL;
    return sZones[zone].areaName;
}

const u8 *DynamicWeather_GetForecastPhrase(u8 pattern)
{
    if (pattern >= DYN_PATTERN_COUNT)
        return NULL;
    return sForecastPhrases[pattern];
}

// Zone i of a forecaster's list, or DYN_ZONE_COUNT past its end
u8 DynamicWeather_GetForecastZone(u8 forecaster, u8 i)
{
    if (forecaster >= FORECAST_COUNT || i >= ARRAY_COUNT(sForecastZones[0]))
        return DYN_ZONE_COUNT;
    return sForecastZones[forecaster][i];
}
