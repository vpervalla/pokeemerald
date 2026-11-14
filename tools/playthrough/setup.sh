#!/bin/bash
# setup.sh [SAVE.sav] : get a session ready to play.
#   - checks the toolchain and libmgba, builds pokeemerald_modern.gba if it is missing
#   - compiles the headless emulator driver into $PLAY_DIR (default ~/.playthrough)
#   - with SAVE.sav: copies it into $PLAY_DIR/game.sav and boots to the overworld through CONTINUE
#     (without one, the next `g` boots the ROM from the title screen with whatever game.sav holds)
# Then: export PATH="$PWD/tools/playthrough/bin:$PATH" PLAY_DIR=...
set -e
HERE=$(cd "$(dirname "$0")" && pwd); ROOT=$(cd "$HERE/../.." && pwd)
PLAY_DIR=${PLAY_DIR:-$HOME/.playthrough}; export PLAY_DIR
mkdir -p "$PLAY_DIR"
missing=""
command -v arm-none-eabi-gcc >/dev/null || missing="$missing gcc-arm-none-eabi"
[ -e /usr/include/mgba/core/core.h ] || missing="$missing libmgba-dev"
[ -e /usr/include/png.h ] || missing="$missing libpng-dev"
python3 -c "import PIL, numpy" 2>/dev/null || missing="$missing python3-pil python3-numpy"
if [ -n "$missing" ]; then
  echo "installing:$missing"; apt-get update -qq && apt-get install -y -qq $missing >/dev/null
fi
[ -e "$ROOT/pokeemerald_modern.gba" ] || make -C "$ROOT" modern -j"$(nproc)"
cc -O2 -o "$PLAY_DIR/mgba_driver" "$ROOT/tools/test_saves/mgba_driver.c" -lmgba
if [ -n "$1" ]; then
  cp "$1" "$PLAY_DIR/game.sav"; rm -f "$PLAY_DIR/cur.ss"
  # title screen -> CONTINUE -> overworld: press A until the overworld callback runs
  (cd "$HERE" && python3 -c "
from emu import *
e = Emu(); e.wait(300)
for _ in range(20):
    if e.cb2() == CB2_OVERWORLD: break
    e.keys(BIT['A'], 6); e.wait(120)
e.wait(60); e.commit()")
  "$HERE/bin/st"
fi
echo "ready: PLAY_DIR=$PLAY_DIR  (add $HERE/bin to PATH)"
