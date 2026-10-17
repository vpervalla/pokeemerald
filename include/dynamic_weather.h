#ifndef GUARD_DYNAMIC_WEATHER_H
#define GUARD_DYNAMIC_WEATHER_H

#include "constants/weather.h"

// Day patterns: one weather for each day/night phase (DAY_NIGHT_PHASE_*)
enum
{
    DYN_PATTERN_CLEAR,
    DYN_PATTERN_CLOUDY,
    DYN_PATTERN_OVERCAST,
    DYN_PATTERN_MORNING_FOG,
    DYN_PATTERN_SHOWERS,
    DYN_PATTERN_RAINY_DAY,
    DYN_PATTERN_NIGHT_RAIN,
    DYN_PATTERN_STORM,
    DYN_PATTERN_DOWNPOUR,
    DYN_PATTERN_HEAT_WAVE,
    DYN_PATTERN_DUST,
    DYN_PATTERN_SNOWFALL,
    DYN_PATTERN_FLURRIES,
    DYN_PATTERN_COUNT
};

// src/dynamic_weather_rules.c: the tables and the draw, with no game state (host-tested by
// tools/dynamic_weather/run_tests.sh)
u8 DynamicWeather_PickPattern(u8 zone, u32 seed, u16 day);
u8 DynamicWeather_GetPatternWeather(u8 pattern, u8 phase);
u8 DynamicWeather_GetFallback(u8 zone);
u8 DynamicWeather_GetPatternWeight(u8 zone, u8 pattern);
const u8 *DynamicWeather_GetZoneName(u8 zone);
const u8 *DynamicWeather_GetPatternName(u8 pattern);

// src/dynamic_weather.c
bool8 IsDynamicWeather(u32 weather);
u8 GetDynamicWeather(u8 zone);
void DynamicWeather_DebugReport(void);
void DynamicWeather_DebugNextDay(void);

#endif // GUARD_DYNAMIC_WEATHER_H
