#ifndef GUARD_GLOBAL_H
#define GUARD_GLOBAL_H

// Stand-in for include/global.h, so the card game's rules engine builds on the host
// (tools/card_game/test_rules.c). Only what the engine uses.

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef u8 bool8;
typedef u16 bool16;
typedef u32 bool32;

#define TRUE  1
#define FALSE 0

// Strings stay plain C strings on the host
#define _(x) x

#include "constants/global.h"

#define ARRAY_COUNT(array) (size_t)(sizeof(array) / sizeof((array)[0]))

#endif // GUARD_GLOBAL_H
