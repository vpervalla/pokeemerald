// Host tests for the dynamic weather's tables and draw (src/dynamic_weather_rules.c).
// Build and run with tools/dynamic_weather/run_tests.sh.

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "dynamic_weather.h"
#include "constants/day_night.h"
#include "constants/weather.h"

#define DAYS 20000

static int sFailures;

#define CHECK(cond, ...)                         \
    do                                           \
    {                                            \
        if (!(cond))                             \
        {                                        \
            printf("FAIL %s:%d: ", __FILE__, __LINE__); \
            printf(__VA_ARGS__);                 \
            printf("\n");                        \
            sFailures++;                         \
        }                                        \
    } while (0)

static const u32 sSeeds[] = {0, 1, 0x12345678, 0xFFFFFFFF, 0x0000D1A7};

static void TestWeightsSumTo100(void)
{
    for (u8 zone = 0; zone < DYN_ZONE_COUNT; zone++)
    {
        int total = 0;
        for (u8 pattern = 0; pattern < DYN_PATTERN_COUNT; pattern++)
            total += DynamicWeather_GetPatternWeight(zone, pattern);
        CHECK(total == 100, "zone %d: weights sum to %d", zone, total);
    }
}

static void TestTablesHoldRealWeathers(void)
{
    for (u8 pattern = 0; pattern < DYN_PATTERN_COUNT; pattern++)
    {
        CHECK(DynamicWeather_GetPatternName(pattern) != NULL, "pattern %d has no name", pattern);
        for (u8 phase = 0; phase < DAY_NIGHT_PHASE_COUNT; phase++)
        {
            u8 weather = DynamicWeather_GetPatternWeather(pattern, phase);
            CHECK(weather != WEATHER_NONE && weather <= WEATHER_DOWNPOUR,
                  "pattern %d phase %d: weather %d", pattern, phase, weather);
        }
    }
    for (u8 zone = 0; zone < DYN_ZONE_COUNT; zone++)
    {
        u8 fallback = DynamicWeather_GetFallback(zone);
        CHECK(fallback < WEATHER_DYNAMIC_START, "zone %d: fallback %d is itself dynamic", zone, fallback);
        CHECK(DynamicWeather_GetZoneName(zone) != NULL, "zone %d has no name", zone);
    }
}

static void TestSameInputsSamePattern(void)
{
    for (unsigned s = 0; s < ARRAY_COUNT(sSeeds); s++)
        for (u8 zone = 0; zone < DYN_ZONE_COUNT; zone++)
            for (u32 day = 0; day < 1000; day++)
                CHECK(DynamicWeather_PickPattern(zone, sSeeds[s], day) == DynamicWeather_PickPattern(zone, sSeeds[s], day),
                      "seed %u zone %d day %u: two draws differ", sSeeds[s], zone, day);
}

// Over many days, each pattern comes up about as often as its weight says, and a pattern
// with no weight never does.
static void TestFrequenciesMatchWeights(void)
{
    for (unsigned s = 0; s < ARRAY_COUNT(sSeeds); s++)
    {
        for (u8 zone = 0; zone < DYN_ZONE_COUNT; zone++)
        {
            int counts[DYN_PATTERN_COUNT] = {0};
            int repeats = 0;
            u8 prev = 0xFF;
            double expectedRepeats = 0;

            for (u32 day = 0; day < DAYS; day++)
            {
                u8 pattern = DynamicWeather_PickPattern(zone, sSeeds[s], day);
                CHECK(pattern < DYN_PATTERN_COUNT, "pattern %d out of range", pattern);
                counts[pattern]++;
                if (pattern == prev)
                    repeats++;
                prev = pattern;
            }
            for (u8 pattern = 0; pattern < DYN_PATTERN_COUNT; pattern++)
            {
                double weight = DynamicWeather_GetPatternWeight(zone, pattern) / 100.0;
                double freq = counts[pattern] / (double)DAYS;
                if (weight == 0)
                    CHECK(counts[pattern] == 0, "seed %u zone %d: pattern %d came up with no weight", sSeeds[s], zone, pattern);
                else
                    CHECK(freq > weight - 0.015 && freq < weight + 0.015,
                          "seed %u zone %d pattern %d: %.3f, weight %.2f", sSeeds[s], zone, pattern, freq, weight);
                expectedRepeats += weight * weight;
            }
            // Neighbouring days are drawn independently: the same pattern two days running
            // happens about as often as chance says.
            CHECK(abs(repeats - (int)(expectedRepeats * (DAYS - 1))) < DAYS / 50,
                  "seed %u zone %d: %d repeats, about %.0f expected", sSeeds[s], zone, repeats, expectedRepeats * (DAYS - 1));
        }
    }
}

// Two saves with different trainer IDs don't get the same weather.
static void TestSeedsDiffer(void)
{
    for (u8 zone = 0; zone < DYN_ZONE_COUNT; zone++)
    {
        int same = 0;
        for (u32 day = 0; day < 1000; day++)
            if (DynamicWeather_PickPattern(zone, sSeeds[1], day) == DynamicWeather_PickPattern(zone, sSeeds[2], day))
                same++;
        CHECK(same < 500, "zone %d: two seeds agree on %d of 1000 days", zone, same);
    }
}

static void TestOutOfRange(void)
{
    CHECK(DynamicWeather_PickPattern(DYN_ZONE_COUNT, 1, 1) == DYN_PATTERN_CLEAR, "bad zone");
    CHECK(DynamicWeather_GetPatternWeather(DYN_PATTERN_COUNT, 0) == WEATHER_SUNNY, "bad pattern");
    CHECK(DynamicWeather_GetPatternWeather(0, DAY_NIGHT_PHASE_COUNT) == WEATHER_SUNNY, "bad phase");
    CHECK(DynamicWeather_GetFallback(DYN_ZONE_COUNT) == WEATHER_NONE, "bad zone fallback");
}

int main(void)
{
    TestWeightsSumTo100();
    TestTablesHoldRealWeathers();
    TestSameInputsSamePattern();
    TestFrequenciesMatchWeights();
    TestSeedsDiffer();
    TestOutOfRange();
    if (sFailures)
    {
        printf("%d failure(s)\n", sFailures);
        return 1;
    }
    printf("All dynamic weather tests passed (%d zone(s), %d patterns)\n", DYN_ZONE_COUNT, DYN_PATTERN_COUNT);
    return 0;
}
