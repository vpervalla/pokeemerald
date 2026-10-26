---
name: kanto-playthrough
description: Play this ROM (the kanto-hoenn build) headless in an emulator and record the run. Use when asked to play the game, continue a playthrough, reach a badge or a place, catch or train Pokémon, take screenshots of events, or write a walkthrough artifact of a run.
---

# Playing the ROM

1. Read `docs/playthrough_guide.md` in full before the first input. It has the setup, the tools,
   the input quirks that cost earlier sessions time, battle tactics, and route notes for this build
   (which differs from FireRed: Pikachu start, starter side quests, day/night, a card game).
2. Set up and load the latest save:
   ```sh
   tools/playthrough/setup.sh saves/<latest>.sav
   export PATH="$PWD/tools/playthrough/bin:$PATH" PLAY_DIR=~/.playthrough
   st
   ```
3. Play in small steps: one `g`/`goto`/`fight` call, then look at `$PLAY_DIR/last.png` or `st`.
   Plan routes with `render MAP out.png --grid --coll --objs --x2` and `goto X Y`.
   `g undo` steps back one call when something goes wrong.
4. If the user wants the run recorded, `shot:NAME` every event and follow the guide's section 7.
5. At the end of a leg: save in the game, copy `$PLAY_DIR/game.sav` to `saves/<name>.sav`,
   `git add -f` it (`.gitignore` matches `*.sa*`), and add the row to the guide's Saves table.
   Add anything new you learned about the game or the tools to the guide.
