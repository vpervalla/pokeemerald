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

To keep playing while you change the source, copy the ROM and ELF to a stable place and point the tools at them:
`cp pokeemerald_modern.gba pokeemerald_modern.elf $PLAY_DIR/rom/; export PLAY_ROM=$PLAY_DIR/rom/pokeemerald_modern.gba`
(a rebuild invalidates `cur.ss`; in-game saves from `saves/` still load).

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
| `gotf X Y` | `goto`, and when a trainer or script stops the walk it presses A until the battle starts, fights with move `${SLOT:-0}` and carries on (`ROUNDS`, default 4, interruptions). The workhorse for long walks. |
| `lead A B` | Swap party slots A and B in RAM (overworld only). Faster and safer than START > POKéMON > SWITCH. |
| `sendout K` | On the forced "Choose a POKéMON" screen after a faint, send out party slot K. |
| `xpathf MAP X Y GOALMAP[:X,Y]` | Cross-map route: which warps to take to get from here to another map, with the walk inside each map planned by `pathf`. Use it for caves with several floors (Rock Tunnel, Mt. Moon) before walking. |
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
- **Stairs, ladders and some doors are arrow warps.** `goto` can land on the warp tile without anything
  happening. Stand on the tile and press the direction the stairs lead (look at the screenshot): `U1` then `L1`
  on the Underground Path stairs, `R1` in the S.S. Anne, `D1` at the ship's gangway. A plain door warp triggers by
  just stepping onto it.
- **Messages come in parts.** "This tree looks like it can be CUT down!" is only the first box; press A once more
  for the YES/NO. The Captain's, Bill's and most NPC scenes are the same: keep pressing A with a screenshot in
  between, and stop pressing A once you are facing an NPC that only offers a card duel ("Maybe next time" loops).
- **Move-learning prompts stop `fight`.** "Stop learning X?" (YES = don't learn), "Delete a move to make room for
  X?" (YES then pick the move). Take a screenshot, answer by hand, run `fight` again.
- **Disable / Encore / Sing stalls.** `fight` bans a move after two turns in which neither side's HP changed and
  tries the next one, but look at the screen when a fight takes more than 10 turns.
- **The planner knows more now:** one-way ledges (jumps them in their direction, so a `goto` may drop you where you
  can't climb back; check `render --coll` first), directional cliff walls, and surfable water (walls without Surf).
  It still treats removed objects (cut trees, taken fossils) as blocking: step through by hand.
- **Soft-lock on leaving the S.S. Anne** (fixed in `data/maps/SSAnne_Exterior/scripts.inc`; a ROM built before the
  fix needs `poke 3000d90:00` style rescue: the script stops at `waitstate` and `sGlobalScriptContextStatus`
  (nm it) has to be set back to 0 for it to resume).
- **Weather and time.** Rain starts on Route 10; the clock is the host's, so day/night tables change under you.
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


## 4b. Lessons from the run to Celadon

- **Trainers are about 5 levels above their vanilla teams** (Caterpie L20, Geodude L25, Slowpoke L25 in Rock
  Tunnel) and carry full parties. Expect to lose a Pokémon per trainer until the lead is L25+.
- **A blackout is a cheap heal.** Whiting out returns you to the last Pokémon Center you healed at with the whole
  party restored, keeps all exp and every trainer you already beat, and costs half your money. Heal at a Center
  right before a dangerous stretch (Route 10 before Rock Tunnel) and use wipes as free healing.
- **Status first, then hit.** Butterfree's Sleep Powder (75%) stopped every big trainer Pokémon for 2-5 turns, but
  Starmie/Staryu out-heal Confusion with Recover and resist it. Misty was won with Sleep Powder, Poison Powder and a
  poisoned Starmie dying to the poison tick (L22 Butterfree against a L21 Starmie). Electric or Grass would have
  been far easier: bring them.
- **Type answers that worked:** Pidgeotto's Aerial Ace (TM40) and Gust against Fighting and Bug; Mankey's Low Kick
  against Geodude, Onix, Cubone (weight based, Fighting is 2x on Rock/Ground); Pikachu against Water/Flying;
  Nidoking (Nidorino + Moon Stone at L16) hits hard with Double Kick and the Rock Tomb TM.
- **Raise a weak Pokémon:** lead it, `SWITCH=1 RESHARE=1` to a strong one on turn 1 (see section 4). Both get exp.
  This took Nidoran from L13 to L16 inside Rock Tunnel and evolved Pidgey and Nidorino at the same time.
- **Level-up evolutions are cancelled by B:** answer the post-battle screens with A.
- **Cut trees may need cutting again** after you leave and re-enter a map (Route 9's did); keep a Cut user in the
  party (Nidoran learned it from HM01 without issue; Cut needs the Cascade Badge here).

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

**Mt. Moon.** The floors are islands joined only by ladders (check with `xpathf`, but it treats the fossils as
walls, so finish the fossil step by hand). The route that works, 1F entry (18,36) from Route 4:
1F (31,16) -> B1F (43,21) -> walk to (26,36) -> B2F (17,31) is the Luna/Charmander corridor (a dead end except for
the ladder and an Escape Rope). For the way on: 1F (5,6) -> B1F (3,3) -> walk to (22,18) -> B2F (25,21), the big
cave floor with Rockets at (12,20) and (37,21). Go to the scientist at (13,11) (he stops you: fight his Grimer;
Disable-ing Drowzee and Machop live nearby), take a fossil at (14,7) (Helix) and keep walking up the now free stairs
through (14,7) to the west room, then the ladder (5,10) -> B1F (39,4) -> exit (45,4) onto Route 4 east. The ladder
(17,5) on B1F leads to a sealed pocket (B2F (31,11)): don't take it. Zubat everywhere (Supersonic). 1F also has a
Moon Stone item at (3,2), a Parlyz Heal at (2,22), and five trainers.

**The Charmander quest.** On B2F a Rocket Grunt above the corridor battles you, then Luna is
cornered by a wild **Charmander L10**. Catch it (Sleep Powder, then a weak move to red HP,
Thunder Wave, balls; bring at least 5). Luna thanks you and runs back to her mother; the
**Star Piece** sits up the stairs behind where she stood. Back at the Route 4 Center (an Escape
Rope is the quick way out) her mother gives a **Moon Stone**.


**Cerulean City.** The city is three pockets joined only by doors. From the Route 4 side you can reach the plaza
(Center (22,19), Gym (31,21), Mart (29,28), Route 24 bridge at the top). The thief's house (door (30,11)) has a
hole in its back wall: its exit (4,1) puts you at (31,8) in the back yard, which connects east to Route 9 (exit
(47,18)) and, down a long corridor, to the south road and **Route 5 (enter at x=33, y=39)**. The tree at (26,32)
shortcut needs Cut. Rival Blue waits at the Nugget Bridge (triggers (22-24,6)): Pidgeotto, Kadabra... L17-ish,
use Pikachu/Charmander's Ember and potions. A Rocket in the back yard (33,6) has Machop and Drowzee (Drowzee
Disables a move: switch to another).
**Nugget Bridge, Route 24/25.** Five trainers on the bridge and eight on Route 25 (Franklin's Geodude, Wayne's
Onix: Low Kick). The TM43 ball at (26,2) is behind bushes. Bill's cottage is at (51,4); talk to the Clefairy,
let it enter the teleporter, use the PC at (4,5) from below (face up at (4,6)), then talk to Bill: **S.S. Ticket**.
**Cerulean Gym.** Luis (Horsea), Diana, Misty: Staryu L18 and Starmie L21 hit hard and Recover. Reward
**Cascade Badge** + TM03. Misty's intro text looks like a refusal ("You have to face other TRAINERS"); keep pressing A.
**Route 5 / Underground Path / Route 6 / Vermilion.** Route 5 (33,1) down to the house at (31,31); inside, stairs
(7,4) go down to the tunnel (arrive (4,3)); walk to (3,60) and press `L1` on the stairs; the south house exit (5-7,8)
opens on Route 6 (19,14). Route 6 grass at (13-16,30-33) is L10-16. Vermilion: Center (15,6), Mart (29,17), pier to
the S.S. Anne at (24,33) (show the ticket, walk down the left side of the gangway).
**S.S. Anne.** Exterior door (32,14) `D1`; 1F corridor stairs (3,8) `L1` to 2F; walk to (30,6) to trigger Blue
(Pidgeotto, Kadabra, Raticate-line, Wartortle L18-20: Sleep Powder + Confusion, Pikachu for Wartortle);
then step `R1` at (30,2) into the Captain's cabin; talk to him: **HM01 Cut**. Teach it from the bag.
**Route 9, Route 10, Rock Tunnel.** Cut the tree at (2,8). Route 9 is a one-way descent: do not jump the ledge near
(11,17) unless you mean it. Route 10 has a **Pokémon Center at (13,20)**: heal and save there. Rock Tunnel:
`xpathf RockTunnel_1F 17 3 Route10:8,57` gives the four-leg route (1F (45,2) -> B1F (33,3) -> 1F (20,13) ->
B1F (2,3) -> 1F (18,37)); it is dark but the planner does not care. About ten trainers with L19-25 teams.
**Lavender Town -> Celadon.** Route 10 south to Lavender; the way west to Route 8 is at (0,9-11). Route 8 to the
Underground entrance (13,4) (`U1` onto it), stairs (7,4) `L1`, tunnel east end (76,3) to west end (3,4) and `L1`,
exit (5-7,8), Route 7, west edge (0,1-3), Celadon City at (57,13). The Saffron gates ask for Tea (Celadon) so the
Route 6/5/7/8 gate houses are closed until then.

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
| `saves/ethan_cascade_badge.sav` | Cerulean Center after Misty and Bill: 2 badges, S.S. Ticket. Next: Route 5, Vermilion, Cut. |
| `saves/ethan_route10.sav` | Route 10 Center, Cut learned, Route 9 done. Next: Rock Tunnel. |
| `saves/ethan_lavender.sav` | Lavender Center after Rock Tunnel (Nidoking, Pidgeotto). Next: Route 8, Underground Path, Celadon. |
| `saves/ethan_celadon.sav` | Celadon Center, all healed, 2 badges. Next: Celadon (Rocket hideout, Tea, Erika) and Saffron. |

Save in the game (START → SAVE, then A through the prompts) at the end of each leg, check with
`st`, and copy `$PLAY_DIR/game.sav` into `saves/`.
