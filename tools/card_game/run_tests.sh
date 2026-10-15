#!/bin/sh
# Builds the card game's rules engine on the host and runs its tests.
# Usage: tools/card_game/run_tests.sh   (from the repository root)
set -e
cd "$(dirname "$0")/../.."
out="${TMPDIR:-/tmp}/card_game_test_rules"
${CC:-cc} -std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -g -fsanitize=address,undefined \
    -I tools/card_game/stub -I include \
    src/card_game_rules.c src/card_game_effects.c tools/card_game/test_rules.c -o "$out"
"$out"
