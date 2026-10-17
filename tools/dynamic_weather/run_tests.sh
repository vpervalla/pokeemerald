#!/bin/sh
# Builds the dynamic weather's tables and draw (src/dynamic_weather_rules.c) on the host and
# runs their tests. Uses the card game's stand-in global.h.
# Usage: tools/dynamic_weather/run_tests.sh   (from anywhere in the repository)
set -e
cd "$(dirname "$0")/../.."
out="${TMPDIR:-/tmp}/dynamic_weather_tests"
mkdir -p "$out"
CFLAGS="-std=c99 -Wall -Wextra -Wno-pointer-sign -g -fsanitize=address,undefined"
${CC:-cc} $CFLAGS -I tools/card_game/stub -I include src/dynamic_weather_rules.c tools/dynamic_weather/test_rules.c -o "$out/test_rules"
"$out/test_rules"
