#!/bin/sh
# Builds the card game's rules engine on the host and runs its tests:
#   test_rules: the rules, with the test's own cards
#   test_data:  the game's real cards and decks (src/card_game_data.c)
# Usage: tools/card_game/run_tests.sh   (from anywhere in the repository)
set -e
cd "$(dirname "$0")/../.."
out="${TMPDIR:-/tmp}/card_game_tests"
mkdir -p "$out"
CFLAGS="-std=c99 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-sign -g -fsanitize=address,undefined"
ENGINE="src/card_game_rules.c src/card_game_effects.c"
${CC:-cc} $CFLAGS -I tools/card_game/stub -I include -I src $ENGINE tools/card_game/test_rules.c -o "$out/test_rules"
${CC:-cc} $CFLAGS -I tools/card_game/stub -I include -I src $ENGINE src/card_game_data.c tools/card_game/test_data.c -o "$out/test_data"
echo "test_rules:"
"$out/test_rules"
echo "test_data:"
"$out/test_data"
