# Playing this ROM headless: a guide for agents

This is what earlier sessions learned while playing the `kanto-hoenn` build from a fresh game to
Mt. Moon with no screen and no controller: a headless emulator, scripts that read the game's RAM,
and screenshots looked at one at a time. Read it before you play. It covers setup, the tools, the
input quirks that cost time, battle tactics, and what is where on each route.

The run so far: Ethan, Pikachu start, Boulder Badge, Charmander from the Mt. Moon quest. The saves
are in `saves/` (see [Saves](#saves)).

## 1. Setup (about 5 minutes)

```sh
tools/playthrough/setup.sh saves/ethan_route4_charmander.sav   # or no argument for a new game
export PATH="$PWD/tools/playthrough/bin:$PATH"                  # setup.sh prints the PLAY_DIR it used
export PLAY_DIR=~/.playthrough
st                                                              # where am I, who is in the party
```

`setup.sh` installs the missing packages (`gcc-arm-none-eabi`, `libmgba-dev`, `libpng-dev`, PIL,
numpy), builds `pokeemerald_modern.gba` if it is missing (`make modern -j$(nproc)`), compiles
`tools/test_saves/mgba_driver.c` into `$PLAY_DIR`, and with a `.sav` argument boots it through
CONTINUE into the overworld.

Every RAM and ROM address comes from `pokeemerald_modern.elf` through `arm-none-eabi-nm`
(`tools/playthrough/syms.py`). Never hard-code one: they move whenever the code changes. If you
rebuild the ROM, a savestate made with the old ROM may not load. The in-game save (`game.sav`)
always does.

## 2. The model: one savestate, many small steps

`$PLAY_DIR/cur.ss` is "now". Every tool loads it, runs, and writes it back. The old state is kept
in `$PLAY_DIR/hist/NNNNN.ss`, so `g undo` steps back one call, and any past moment can be reloaded
or re-shot. `$PLAY_DIR/game.sav` is the cartridge's flash, written only when the game itself saves.

After each call look at `$PLAY_DIR/last.png` (480x320) with the Read tool, or run `st`. Don't chain
long blind sequences: a text box or a wild battle in the middle eats the rest of the input.

| Tool | What it does |
| --- | --- |
| `g TOKENS` | Buttons `A B ST SE L R`, walk `U3 D2 L1 R4`, raw hold `hU5`, wait `w60`, repeat `A*5`, `shot:NAME`, `peek:ADDR:N`, `undo`. |
| `st` | Map name, (x,y), money, party (species, level, HP, exp, nature, stats, status, moves with PP), and the enemy in a battle. Read-only. |
| `fight [SLOT] [MAXTURNS]` | Auto-battle with move SLOT (0 top-left, 1 top-right, 2 bottom-left, 3 bottom-right) until the battle ends. Env `SWITCH=n`, `RESHARE=1`, `STOPEVO=1`. |
| `catch` | Throw the ball under the BAG cursor every turn until caught. Open the POKé BALLS pocket by hand once first. |
| `gw TOKENS` | `g`, and if a wild battle interrupts, `fight ${SLOT:-0}`. |
| `goto X Y [--adj]` | Path-find on the current map (collision + NPCs; warps blocked unless they are the goal) and walk, fighting and re-planning. Prints `blocked` when it can't move. |
| `pick X Y NAME` | Walk next to (X,Y), face it, press A, save the screen as `shots/NAME.png`. Items, hidden items, signs, people. |
| `trainer X Y NAME` | `pick` plus `fight`, intro saved as `shots/tr_NAME.png`. |
| `train N` | Pace in grass for N battles. Stops on `CATCH=regex` (battle left open, prints CATCHME) or on a party member under `MINHP` % (prints LOWHP). |
| `heal` | From a Pokémon Center's entrance mat: nurse, heal, back to the mat. |
| `pathf MAP X0 Y0 X1 Y1 [--adj]` | Just the planner: prints walk tokens. |
| `render MAP OUT.png --grid --coll --objs --x2` | Draw a map from the repo data, with a coordinate grid, collision in red, objects and warps. Use it to plan; coordinates match `st`. |
| `strip NAMES...` | Tile shots three across into `$PLAY_DIR/strip.png`, to review a scene at once. |

`tools/playthrough/assets.py` turns maps with numbered markers, front sprites and icons into
`data:` URIs for a single-file HTML walkthrough page.

### Reading the game directly

- `gMain.callback2` (`gMain`+4) tells you where you are: `CB2_Overworld` means you can walk,
  `BattleMainCB2` means a battle. Anything else is a menu, a scene or a transition.
- In a battle, `gBattlerControllerFuncs[0] == HandleInputChooseAction` (the player controller's
  copy; there are several static functions with that name) means the action menu is waiting.
  `gActionSelectionCursor` and `gMoveSelectionCursor` hold the 2x2 cursor positions
  (bit 0 right column, bit 1 bottom row). `gBattleOutcome` 7 means caught.
- SaveBlock1: x,y as s16 at +0, map group/num at +4/+5, money at +0x490 XORed with the
  `encryptionKey` at SaveBlock2+0xAC.
- A party Pokémon is 100 bytes. The four 12-byte substructs at +32 are XORed with
  `personality ^ otId` and ordered by `personality % 24`; level, HP and stats sit unencrypted at
  +80. Nature is `personality % 25`. `st.py` has the decoder.

## 3. Input quirks that cost earlier sessions time

- **Walking.** Holding a direction for a fixed number of frames drifts: turning in place takes
  frames, and a bump into a wall takes more. `g`'s walk tokens wait for the position to change,
  one tile at a time, and stop with `STOPPED` when the game leaves the overworld. Use `hU2` to
  turn in place without walking.
- **Start menu and other menus.** Presses sent too soon after opening a menu are dropped. Wait
  about 90 frames after `ST` before moving the cursor, and check `last.png` before pressing A.
- **Screenshots.** `shot:` as the very first token of a call is blank (no frame drawn since the
  load). Put `w1` or anything before it. A missed scene can be recovered: load a `hist/` state with
  the driver and shoot it.
- **B cancels evolution.** In battles B is safe for advancing text. After a battle, during an
  evolution, press A. `fight` does this; in your own sequences, don't spam B after a level-up win.
- **Move learning.** "Learn a new move?" stops `fight` with `stuck`. Look at the screen: YES, pick
  the move to forget, confirm; or NO, then YES to "stop learning".
- **The bag remembers its pocket.** In battle BAG reopens on the last pocket used. If that was
  ITEMS, `catch` throws Potions. Switch to POKé BALLS by hand (left/right in the bag) once.
- **Fainting.** When the lead faints, the "Use next Pokémon?" prompt appears; `fight` answers it
  with B, which tries to run. If the lead might go down, fight by hand or put a sturdier Pokémon
  in front. `train` stops on LOWHP before that usually happens.
- **Empty move slots.** `fight 3` with three moves lands on another move. Check moves with `st`.
- **Ledges are one-way.** Jumping one can cut you off from where you were going. The planner
  treats ledges as walls (it only knows collision), so it won't jump them for you: when it says
  `no path`, look at `render --coll` and route around, or jump with `hD8` deliberately.
- **Doors, ladders and stairs** are warps. The planner never crosses one unless it is the goal;
  walking onto one by accident changes the map and wastes a round trip.
- **Field poison** takes 1 HP every 4 steps and in this build it can knock a Pokémon out. Carry
  Antidotes from Viridian Forest on.
- **Scripted scenes** (Oak, the rival, Luna) take control: alternate `A w60` and look.
- **Time of day** follows the host clock through the RTC: day 6–17, night 17–6. The night tables
  swap in Hoothoot, Spinarak and others.
- **Pipes.** `g ... | head` can raise BrokenPipe; send stderr to /dev/null or read the whole output.

## 4. Battle and training tactics

- **The swap trick.** To raise a weak Pokémon, put it first and switch to a strong one on turn 1:
  both get experience. `SWITCH=3 fight 0` does it (party slot 3 comes in). Against trainers,
  `RESHARE=1` repeats the swap each time the opponent sends out its next Pokémon. A tired Pokémon
  can be knocked out on the switch-in, so heal first.
- **Grind next to a Pokémon Center.** A good loop: heal, walk to the nearest grass, `train N` with
  `MINHP=40`, walk back, `heal`. Places that worked: Route 1 next to Mom (she heals for free),
  Viridian Center ↔ Route 22, Pewter Center ↔ the top of Route 2.
- **Catching.** Sleep Powder or Thunder Wave first, chip the HP to red with a weak move, then
  `catch`. A full-HP L10 Charmander broke out of a ball even asleep.
- **Bugs evolve fast.** Caterpie L4 → Metapod L7 → Butterfree L10 (learns Confusion on evolving).
  Metapod has only Tackle/Harden: let it fight alone in easy grass.
- **Low Kick** gets stronger with the target's weight: Mankey's Low Kick does 120 to Onix.
- **Gym trainers** can be strung out by Defense Curl and Sand-Attack. Watch PP and heal between
  them.

## 5. What this build changes

- Yellow-style start: Oak gives you **Pikachu**. Red took Charmander, Leaf took Bulbasaur, the
  rival is always **BLUE** with Squirtle. No rival battle in the lab.
- The three Kanto starters are side quests: **Bulbasaur** in Viridian Forest (needs Cut; two L20
  Beedrill in a double battle), **Charmander** in Mt. Moon B2F (see below), **Squirtle** in
  Cerulean.
- Six player characters at the start: BRENDAN, MAY, RED, LEAF, ETHAN, KRIS.
- National Pokédex numbers. Fairy type (Jigglypuff knows Disarming Voice). Day/night encounters.
- A trading card game: Oak's aide in Pewter gives the CARD CASE and a first pack; the first Mart
  clerk you talk to each day gives a free pack.
- The old man's catching demo in Viridian plays in full and gives the Teachy TV.

## 6. Route notes (Pallet Town to Mt. Moon)

Coordinates are map tiles, as `st` and `render --grid` show them.

**New game.** OPTION first: text FAST, battle scene OFF, style SET. Pick ETHAN, type the name,
confirm with START. Potion in the bedroom PC (Item Storage → Withdraw).

**Pallet / Route 1.** Oak stops you at the grass and gives Pikachu (L5). Route 1: Hoothoot,
Rattata, Pidgey. The Mart clerk on Route 1 gives a Potion. Ledges face south: going north use the
west gap of the first ledge.

**Viridian City.** The Mart gives Oak's Parcel; deliver it for the Pokédex and 5 Poké Balls.
Daisy gives the Town Map anyway. Mart: Ball ₽200, Potion ₽300, Antidote ₽100, Parlyz Heal ₽200.

**Route 22.** Grass on the upper path, east of Blue's trigger: Rattata, Mankey, Spearow. Don't jump
the long ledge south of the grass (it leads only to the League gate). Blue: Pidgey, Squirtle; ₽144.

**Route 2 (south).** Rattata, Pidgey, rare Caterpie, Weedle and Ledyba (1%, day). The ledge above
the grass is one-way: cross to the grass's east end, go up and back west to the forest gate.

**Viridian Forest.** A loop: east side north, across the top, down the west side to the exit.
Caterpie and Weedle by day; Spinarak at night. Poison everywhere: bring Antidotes. Items: Potion
south-east, hidden Antidote just south of the tree by the entrance, Antidote north of Doug. Bug
Catchers Rick, Doug, Anthony, Charlie (only if talked to), Sammy. The Bulbasaur clearing is behind
three Cut trees at the top.

**Pewter City.** The Gym is in a fenced loop: from the Center go to (22,27), up to (22,11), left
to (7,11), down to (7,18), right to (15,18), up into the Gym. Mart adds Awakening, Burn Heal,
Escape Rope ₽550, Repel. Oak's aide (card game) is here.

**Pewter Gym.** Camper Liam (Geodude L10, Sandshrew L11), Brock (Geodude L12, Onix L14 with Rock
Tomb, which one-shots a L13 Butterfree). Butterfree's Confusion for Geodude, Mankey's Low Kick for
Onix. ₽1400, Boulder Badge, TM39.

**Route 3.** Eight trainers (Lass Janice with two Pidgey, James, Robin, Youngster Calvin and more)
and two grass patches: Nidoran♂, Nidoran♀, Spearow, Pidgey, Mankey, Jigglypuff. The middle is split by ledges: the gaps at
x 16–17 (y 12) and x 14–15 (y 7) lead up to the top corridor; Youngster Calvin (Spearow L14)
sits in a pocket reached only by dropping from that corridor. To the north end: from (60,6) D5,
R10 to (70,9), R2, then up to Route 4.

**Route 4 Pokémon Center** (at the foot of Mt. Moon). Luna's mother asks you to find Luna in
Mt. Moon. Save here.

**Mt. Moon.** Three floors and a maze; use `render --coll` and `goto` rather than exploring.
1F ladder (19,14) → B1F (25,4) → ladder (17,5) → B2F (31,11). On B2F Rockets stand at (12,20),
(35,12) and (37,21); Miguel and the fossils at (13–14,7). Out: ladder (5,10) → B1F (39,4) → exit
(45,4) onto Route 4 east → Cerulean. Zubat everywhere (Supersonic). 1F also has a Moon Stone item
at (3,2), a Parlyz Heal at (2,22), and five trainers.

**The Charmander quest.** On B2F a Rocket Grunt above the corridor battles you, then Luna is
cornered by a wild **Charmander L10**. Catch it (Sleep Powder, then a weak move to red HP,
Thunder Wave, balls; bring at least 5). Luna thanks you and runs back to her mother; the
**Star Piece** sits up the stairs behind where she stood. Back at the Route 4 Center (an Escape
Rope is the quick way out) her mother gives a **Moon Stone**.

## 7. Recording a run as a walkthrough page

The user wanted the run as a shareable HTML artifact, organized in pages (Intro, Route 1, Viridian
City, …), one per area. What worked:

- Take `shot:NAME` at every event worth showing: a new area, each trainer's intro, each item
  message, each catch, evolutions, badges. Names like `tr_janice`, `got_moonstone`.
- Per page: a map with numbered markers (`assets.map_img`), a legend of items/gifts/trainers with
  "got it / later" chips, encounter tables with level ranges and rates for day and night, trainer
  cards with their teams, then the story as short steps, each with its screenshots.
- A team page at the end: sprites, level, nature, ability, moves with PP, and how each was caught.
- Write facts from the run, not from memory of the original games: this build differs.
- One self-contained HTML file: inline images as `data:` URIs, hash routing between pages.

## Saves

The repo's `.gitignore` has `*.sa*`, so commit saves with `git add -f`.

| File | Where |
| --- | --- |
| `saves/ethan_viridian_forest_gate.sav` | Route 2, at the forest gate. Pikachu, Pidgey, Mankey, Ledyba. |
| `saves/ethan_boulder_badge.sav` | Pewter Center after Brock. Butterfree, Mankey, Ledyba, Pikachu, Pidgey. |
| `saves/ethan_route4_charmander.sav` | Route 4 Center, Charmander caught, Moon Stone. Next: Mt. Moon to Cerulean. |

Save in the game (START → SAVE, then A through the prompts) at the end of each leg, check with
`st`, and copy `$PLAY_DIR/game.sav` into `saves/`.
