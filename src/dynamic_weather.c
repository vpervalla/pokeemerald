#include "global.h"
#include "day_night.h"
#include "dynamic_weather.h"
#include "event_data.h"
#include "field_weather.h"
#include "palette.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "constants/day_night.h"
#include "constants/weather.h"

// Dynamic weather: maps whose header weather is WEATHER_DYNAMIC_<zone> get the weather of
// their zone's day pattern (src/dynamic_weather_rules.c) in the current day/night phase.
// TranslateWeatherNum() asks for it whenever such a map's weather is set, which is on map
// load and on resetweather.
//
// While the player stays on such a map, DynamicWeather_UpdateField() fades to the next weather
// when the phase changes (or the day, or the story gate), unless a script or a coord event
// picked the map's weather. It waits while a script, a menu or a fade has the screen.
//
// The weather day starts with the morning at 06:00, so a night that runs past midnight keeps
// one weather. The dynamic weather stays off (each zone's fallback, clear) until the player
// has the POKéDEX, and also when the clock can't be read.

#define WEATHER_DAY_START_HOUR 6
#define UPDATE_FRAMES          60   // How often the field checks whether the weather is due to change

// Days added by the Test Area's debug entry, to look at other days without changing the clock.
// Not saved.
static EWRAM_DATA u16 sDebugDayOffset = 0;

// The current map's weather was set by a script or a coord event, not by its zone.
static EWRAM_DATA bool8 sOverridden = FALSE;
static EWRAM_DATA u8 sUpdateTimer = 0;

bool8 IsDynamicWeather(u32 weather)
{
    return weather >= WEATHER_DYNAMIC_START && weather < WEATHER_DYNAMIC_END;
}

static bool8 IsClockReadable(void)
{
    return FlagGet(FLAG_SYS_CLOCK_SET) && !(RtcGetErrorStatus() & RTC_ERR_FLAG_MASK);
}

static bool8 IsDynamicWeatherOn(void)
{
    return FlagGet(FLAG_SYS_POKEDEX_GET) && IsClockReadable();
}

static u32 GetSeed(void)
{
    return gSaveBlock2Ptr->playerTrainerId[0]
         | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
         | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
         | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
}

static u16 GetWeatherDay(void)
{
    u16 day;

    RtcCalcLocalTime();
    day = gLocalTime.days;
    if (gLocalTime.hours < WEATHER_DAY_START_HOUR && day != 0)
        day--;
    return day + sDebugDayOffset;
}

static u8 GetTodaysPattern(u8 zone)
{
    return DynamicWeather_PickPattern(zone, GetSeed(), GetWeatherDay());
}

u8 GetDynamicWeather(u8 zone)
{
    if (zone >= DYN_ZONE_COUNT)
        return WEATHER_NONE;
    if (!IsDynamicWeatherOn())
        return DynamicWeather_GetFallback(zone);
    return DynamicWeather_GetPatternWeather(GetTodaysPattern(zone), DayNight_GetPhaseNow());
}

void DynamicWeather_SetOverridden(bool8 overridden)
{
    sOverridden = overridden;
}

// Called each overworld frame.
void DynamicWeather_UpdateField(void)
{
    if (++sUpdateTimer < UPDATE_FRAMES)
        return;
    sUpdateTimer = 0;

    // Try again next time rather than change the weather under a script, a menu or a fade
    if (ArePlayerFieldControlsLocked() || gPaletteFade.active)
        return;
    UpdateWeatherForTimeOfDay();
    if (!IsDynamicWeather(gMapHeader.weather) || sOverridden)
        return;
    if (GetDynamicWeather(gMapHeader.weather - WEATHER_DYNAMIC_START) == GetSavedWeather())
        return;

    SetSavedWeatherFromCurrMapHeader();
    DoCurrentWeather();
}

// ---------------------------------------------------------------------------------------------
// Weather forecasters (data/scripts/weather_forecaster.inc)

static const u8 sText_LaterToday[] = _(".\lLater today, ");

// Buffers the forecast for zone VAR_0x8005 of forecaster VAR_0x8004's list (FORECAST_*):
// gStringVar1: the area, gStringVar2: the weather now, and the next change later today if any,
// gStringVar3: tomorrow. VAR_RESULT is FALSE past the end of the list. Today's is told from the
// weather the zone's maps have now, then the phases left in the day, so it is what the player
// sees outside. While the dynamic weather is off the maps stay clear, and so does the forecast.
void DynamicWeather_BufferForecast(void)
{
    u8 zone = DynamicWeather_GetForecastZone(gSpecialVar_0x8004, gSpecialVar_0x8005);
    u8 now, phase, today, tomorrow = DYN_PATTERN_CLEAR;
    u8 *str;

    if (zone >= DYN_ZONE_COUNT)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    now = GetDynamicWeather(zone);
    str = StringCopy(gStringVar2, DynamicWeather_GetWeatherWord(now));
    if (IsDynamicWeatherOn())
    {
        u16 day = GetWeatherDay();

        today = DynamicWeather_PickPattern(zone, GetSeed(), day);
        tomorrow = DynamicWeather_PickPattern(zone, GetSeed(), day + 1);
        // The night runs to 06:00, the end of the weather day
        for (phase = DayNight_GetPhaseNow() + 1; phase < DAY_NIGHT_PHASE_COUNT; phase++)
        {
            u8 later = DynamicWeather_GetPatternWeather(today, phase);

            if (DynamicWeather_GetWeatherWord(later) != DynamicWeather_GetWeatherWord(now))
            {
                str = StringCopy(str, sText_LaterToday);
                StringCopy(str, DynamicWeather_GetWeatherWord(later));
                break;
            }
        }
    }
    StringCopy(gStringVar1, DynamicWeather_GetAreaName(zone));
    StringCopy(gStringVar3, DynamicWeather_GetForecastPhrase(tomorrow));
    gSpecialVar_Result = TRUE;
}

// ---------------------------------------------------------------------------------------------
// Test Area debug entry (TestArea_EventScript_DynamicWeather)

static const u8 sText_Morning[] = _("MORNING");
static const u8 sText_Day[]     = _("DAY");
static const u8 sText_Evening[] = _("EVENING");
static const u8 sText_Night[]   = _("NIGHT");

static const u8 *const sPhaseNames[DAY_NIGHT_PHASE_COUNT] =
{
    [DAY_NIGHT_PHASE_MORNING] = sText_Morning,
    [DAY_NIGHT_PHASE_DAY]     = sText_Day,
    [DAY_NIGHT_PHASE_EVENING] = sText_Evening,
    [DAY_NIGHT_PHASE_NIGHT]   = sText_Night,
};

static const u8 sText_WeatherOther[]    = _("?");
static const u8 sText_WeatherNone[]     = _("NONE");
static const u8 sText_WeatherClouds[]   = _("CLOUDS");
static const u8 sText_WeatherClear[]    = _("CLEAR");
static const u8 sText_WeatherRain[]     = _("RAIN");
static const u8 sText_WeatherSnow[]     = _("SNOW");
static const u8 sText_WeatherStorm[]    = _("THUNDERSTORM");
static const u8 sText_WeatherFog[]      = _("FOG");
static const u8 sText_WeatherSand[]     = _("SANDSTORM");
static const u8 sText_WeatherShade[]    = _("OVERCAST");
static const u8 sText_WeatherDrought[]  = _("HARSH SUN");
static const u8 sText_WeatherDownpour[] = _("DOWNPOUR");

static const u8 *GetWeatherName(u8 weather)
{
    switch (weather)
    {
    case WEATHER_NONE:              return sText_WeatherNone;
    case WEATHER_SUNNY_CLOUDS:      return sText_WeatherClouds;
    case WEATHER_SUNNY:             return sText_WeatherClear;
    case WEATHER_RAIN:              return sText_WeatherRain;
    case WEATHER_SNOW:              return sText_WeatherSnow;
    case WEATHER_RAIN_THUNDERSTORM: return sText_WeatherStorm;
    case WEATHER_FOG_HORIZONTAL:    return sText_WeatherFog;
    case WEATHER_SANDSTORM:         return sText_WeatherSand;
    case WEATHER_SHADE:             return sText_WeatherShade;
    case WEATHER_DROUGHT:           return sText_WeatherDrought;
    case WEATHER_DOWNPOUR:          return sText_WeatherDownpour;
    default:                        return sText_WeatherOther;
    }
}

static const u8 sText_PatternIs[]   = _("Pattern: ");
static const u8 sText_Colon[]       = _(": ");
static const u8 sText_NewLine[]     = _("\n");
static const u8 sText_NewPage[]     = _("\p");
static const u8 sText_NowIs[]       = _("Now it's ");
static const u8 sText_Period[]      = _(".");
static const u8 sText_Gated[]       = _("\pIt's off until the player has the\nPOKéDEX: the maps stay CLEAR.");
static const u8 sText_NoClock[]     = _("\pThe clock can't be read, so the\nmaps stay CLEAR.");

// Shows the weather today of zone VAR_0x8005, ignoring the story gate so it can be checked
// from a new game. gStringVar1: zone, gStringVar2: day, gStringVar3: the pattern and its weathers.
void DynamicWeather_DebugReport(void)
{
    u8 zone = gSpecialVar_0x8005 < DYN_ZONE_COUNT ? gSpecialVar_0x8005 : DYN_ZONE_VIRIDIAN;
    u8 pattern = GetTodaysPattern(zone);
    u8 now = DayNight_GetPhaseNow();
    u8 phase;
    u8 *str;

    StringCopy(gStringVar1, DynamicWeather_GetZoneName(zone));
    ConvertIntToDecimalStringN(gStringVar2, GetWeatherDay(), STR_CONV_MODE_LEFT_ALIGN, 5);

    str = StringCopy(gStringVar3, sText_PatternIs);
    str = StringAppend(str, DynamicWeather_GetPatternName(pattern));
    for (phase = 0; phase < DAY_NIGHT_PHASE_COUNT; phase++)
    {
        // Two phases a page
        str = StringAppend(str, (phase % 2 == 0) ? sText_NewPage : sText_NewLine);
        str = StringAppend(str, sPhaseNames[phase]);
        str = StringAppend(str, sText_Colon);
        str = StringAppend(str, GetWeatherName(DynamicWeather_GetPatternWeather(pattern, phase)));
    }
    str = StringAppend(str, sText_NewPage);
    str = StringAppend(str, sText_NowIs);
    str = StringAppend(str, sPhaseNames[now]);
    str = StringAppend(str, sText_Period);
    if (!IsClockReadable())
        StringAppend(str, sText_NoClock);
    else if (!FlagGet(FLAG_SYS_POKEDEX_GET))
        StringAppend(str, sText_Gated);
}

void DynamicWeather_DebugNextDay(void)
{
    sDebugDayOffset++;
}
