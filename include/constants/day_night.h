#ifndef GUARD_CONSTANTS_DAY_NIGHT_H
#define GUARD_CONSTANTS_DAY_NIGHT_H

// Phases of the day, taken from the RTC's local time.
#define DAY_NIGHT_PHASE_MORNING  0  // 06:00 - 09:59
#define DAY_NIGHT_PHASE_DAY      1  // 10:00 - 16:59
#define DAY_NIGHT_PHASE_EVENING  2  // 17:00 - 19:59
#define DAY_NIGHT_PHASE_NIGHT    3  // 20:00 - 05:59
#define DAY_NIGHT_PHASE_COUNT    4

// Times of day of the wild encounter tables (src/data/wild_encounters.json). A map's own table
// is used at any time; one for DAY or NIGHT, right after it, replaces the fields it has.
#define WILD_TIME_ANY    0
#define WILD_TIME_DAY    1  // Morning and day
#define WILD_TIME_NIGHT  2  // Evening and night

// Values of VAR_DAY_NIGHT_OVERRIDE. Anything but NONE pins the phase, ignoring the clock.
#define DAY_NIGHT_OVERRIDE_NONE     0
#define DAY_NIGHT_OVERRIDE_MORNING  (DAY_NIGHT_PHASE_MORNING + 1)
#define DAY_NIGHT_OVERRIDE_DAY      (DAY_NIGHT_PHASE_DAY + 1)
#define DAY_NIGHT_OVERRIDE_EVENING  (DAY_NIGHT_PHASE_EVENING + 1)
#define DAY_NIGHT_OVERRIDE_NIGHT    (DAY_NIGHT_PHASE_NIGHT + 1)

#endif // GUARD_CONSTANTS_DAY_NIGHT_H
