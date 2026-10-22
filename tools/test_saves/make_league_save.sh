#!/bin/sh
# Makes a save for testing the Kanto ELITE FOUR and the post-game: all eight Kanto badges, the Kanto
# story done up to the League (see league_setup.c), a level 60 team (VENUSAUR, CHARIZARD, BLASTOISE,
# GENGAR, SNORLAX, DRAGONITE) and the player at the door of the INDIGO PLATEAU.
#
#   tools/test_saves/make_league_save.sh [OUT.sav]     (default: pokeemerald_league.sav)
#
# Needs arm-none-eabi-gcc (the modern build) and libmgba (Debian/Ubuntu: libmgba-dev). It copies the
# tree to a temporary directory, builds a throwaway ROM whose NEW GAME skips OAK's speech and calls
# TestSave_SetupLeague(), plays it headless to the START menu's SAVE, and writes the save. The
# save is for the normal ROM (same save layout): load it with CONTINUE.
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=$(realpath -m "${1:-$ROOT/pokeemerald_league.sav}")
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "copying the tree to $WORK"
cp -a "$ROOT/." "$WORK/"
cp "$ROOT/tools/test_saves/league_setup.c" "$WORK/src/test_save_league.c"
# The hook: NEW GAME goes straight to CB2_NewGame, and NewGameInitData ends with the setup.
sed -i 's/^void StartNewGameScene(void)$/&\n{\n    SetMainCallback2(CB2_NewGame);\n}\nvoid UnusedStartNewGameScene(void)/' "$WORK/src/oak_speech.c"
sed -i 's/^void NewGameInitData(void)$/void TestSave_SetupLeague(void);\n&/; /^    ResetContestLinkResults();$/a\    TestSave_SetupLeague();' "$WORK/src/new_game.c"
grep -q "TestSave_SetupLeague();" "$WORK/src/new_game.c" && grep -q UnusedStartNewGameScene "$WORK/src/oak_speech.c"

echo "building the test ROM"
make -C "$WORK" modern -j"$(nproc)" >"$WORK/build.log" 2>&1 || { tail -30 "$WORK/build.log"; exit 1; }
cc -O2 -o "$WORK/mgba_driver" "$ROOT/tools/test_saves/mgba_driver.c" -lmgba

echo "playing to the save"
rm -f "$OUT"
# Title screen, NEW GAME (the only choice on a blank save; the menu takes a few presses to answer),
# then START > SAVE > YES. Extra A presses at the door do nothing.
"$WORK/mgba_driver" "$WORK/pokeemerald_modern.gba" "$OUT" <<CMDS
wait 600
press START
wait 240
press A
wait 120
press A
wait 120
press A
wait 120
press A
wait 120
press A
wait 400
press START
wait 30
press DOWN
press DOWN
press DOWN
press DOWN
press A
wait 90
press A
wait 120
press A
wait 600
shot $WORK/saved.ppm
CMDS
# mGBA adds 16 bytes of RTC state after the 128 KB flash: keep only the flash, which every emulator reads.
test "$(wc -c <"$OUT")" -ge 131072 || { echo "no save written"; exit 1; }
truncate -s 131072 "$OUT"
echo "wrote $OUT"
