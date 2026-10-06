# Play-testing harness

`emu.py` drives the built ROM headlessly through libmgba, for play-testing the story
without a screen: it walks with path-finding over the live map grid, fights battles,
advances messages (printing their text) and takes screenshots.

Setup (Ubuntu): `apt install gcc-arm-none-eabi libmgba-dev`, `pip install pillow`, then
`make modern`. The shim library and the symbol list are built into `build/playtest` on
first use. See the docstring at the top of `emu.py` for the command language.

To continue from a cartridge save in `saves/`, copy it over `build/playtest/game.sav`,
power on with `--new`, and pick CONTINUE:

    cp saves/red_boulder_badge.sav build/playtest/game.sav
    python3 tools/playtest/emu.py --new w1200 ST w300 ST w300 A w300 A w600 shot
