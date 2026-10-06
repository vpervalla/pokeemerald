#!/usr/bin/env python3
"""Headless play-testing driver for the ROM, built on libmgba (apt install libmgba-dev).

Each run loads a savestate, runs a command script, saves the state again and prints the
player's map, position, party and (in battle) both battlers. Work files (the shim library,
symbols, states, screenshots and the cartridge save game.sav) live in build/playtest.

    python3 tools/playtest/emu.py --new w600 ST w200        # boot the ROM from scratch
    python3 tools/playtest/emu.py G13,-6                    # walk toward (13,-6), fighting wild battles
    python3 tools/playtest/emu.py --state pewter xa shot    # load states/pewter.ss, advance text, screenshot

Options: --new (power on instead of loading a state), --state NAME (load NAME.ss instead of
cur.ss), --nosave (don't overwrite cur.ss), --sav PATH (cartridge save to use).

Commands (space separated):
  A B ST SE U D L R Lb Rb   tap a button (A*5 taps it 5 times)
  hU3                       walk 3 tiles up (U/D/L/R)
  w60                       wait 60 frames
  m20 / bm20                mash A / B 20 times
  xa / xb                   advance the open message with A / B (prints the text)
  gX,Y / tX,Y               path-find to (X,Y) / next to (X,Y) facing it (map coords)
  GX,Y                      like g, but fights wild and trainer battles on the way
  mv=N / tmv=N              move slot G uses in wild / trainer battles (mv=9: run from wild ones)
  grind:N:LR                pace left/right in grass until N battles are won
  fN / faN                  one battle turn with move slot N / repeat until the battle ends
  run / bwait               run from battle / wait for the battle's action menu
  map / bag                 print the walkable grid around the player / the bag
  shot / shot:PATH          screenshot (several in one run also make montage.png)
  save:NAME                 save the state as NAME.ss
"""
import ctypes, json, os, re, struct, subprocess, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
WORK = os.path.join(ROOT, "build", "playtest")
ROM = f"{ROOT}/pokeemerald_modern.gba" if os.path.exists(f"{ROOT}/pokeemerald_modern.gba") else f"{ROOT}/pokeemerald.gba"
SAV = f"{WORK}/game.sav"
STATES = f"{WORK}/states"
SHOTS = f"{WORK}/shots"
for d in (WORK, STATES, SHOTS): os.makedirs(d, exist_ok=True)
if "--sav" in sys.argv:
    i = sys.argv.index("--sav"); SAV = os.path.abspath(sys.argv[i + 1]); del sys.argv[i:i + 2]

KEYS = dict(A=1, B=2, SE=4, ST=8, R=16, L=32, U=64, D=128, Rb=256, Lb=512)
DIRS = dict(U=64, D=128, L=32, R=16)

if not os.path.exists(f"{WORK}/libshim.so") or os.path.getmtime(f"{WORK}/libshim.so") < os.path.getmtime(f"{HERE}/shim.c"):
    subprocess.run(["gcc", "-O2", "-shared", "-fPIC", f"{HERE}/shim.c", "-o", f"{WORK}/libshim.so", "-lmgba"], check=True)
elf = os.path.splitext(ROM)[0] + ".elf"
if not os.path.exists(f"{WORK}/syms.txt") or os.path.getmtime(f"{WORK}/syms.txt") < os.path.getmtime(elf):
    with open(f"{WORK}/syms.txt", "w") as f:
        subprocess.run(["arm-none-eabi-nm", elf], stdout=f, check=True)
lib = ctypes.CDLL(f"{WORK}/libshim.so")
lib.emu_frame.restype = ctypes.POINTER(ctypes.c_uint32)

syms, allsyms = {}, {}
for line in open(f"{WORK}/syms.txt"):
    p = line.split()
    if len(p) == 3: syms[p[2]] = int(p[0], 16); allsyms.setdefault(p[2], []).append(int(p[0], 16))

def rd(addr, n): b = (ctypes.c_ubyte * n)(); lib.emu_read(addr, b, n); return bytes(b)
def u8(a): return rd(a, 1)[0]
def u16(a): return struct.unpack("<H", rd(a, 2))[0]
def u32(a): return struct.unpack("<I", rd(a, 4))[0]
def s16(a): return struct.unpack("<h", rd(a, 2))[0]

charmap = {}
for m in re.finditer(r"^'(.)'\s*=\s*([0-9A-F]{2})\s*$", open(f"{ROOT}/charmap.txt", encoding="utf-8").read(), re.M):
    charmap.setdefault(int(m.group(2), 16), m.group(1))
def decode(b):
    s = ""
    for c in b:
        if c == 0xFF: break
        if c in (0xFE,): s += "\n"; continue
        if c in (0xFA, 0xFB): s += "\n"; continue
        if c == 0xFC or c == 0xFD: s += "~"; continue
        s += charmap.get(c, "?")
    return s

groups = json.load(open(f"{ROOT}/data/maps/map_groups.json"))
def mapname(g, n):
    try: return groups[groups["group_order"][g]][n]
    except Exception: return f"?{g}.{n}"

species = {}
for m in re.finditer(r"#define SPECIES_(\w+)\s+(\d+)", open(f"{ROOT}/include/constants/species.h").read()):
    species.setdefault(int(m.group(2)), m.group(1))
moves = {}
for m in re.finditer(r"#define MOVE_(\w+)\s+(\d+)", open(f"{ROOT}/include/constants/moves.h").read()):
    moves.setdefault(int(m.group(2)), m.group(1))
ORDER = [(0,1,2,3),(0,1,3,2),(0,2,1,3),(0,3,1,2),(0,2,3,1),(0,3,2,1),(1,0,2,3),(1,0,3,2),
         (2,0,1,3),(3,0,1,2),(2,0,3,1),(3,0,2,1),(1,2,0,3),(1,3,0,2),(2,1,0,3),(3,1,0,2),
         (2,3,0,1),(3,2,0,1),(1,2,3,0),(1,3,2,0),(2,1,3,0),(3,1,2,0),(2,3,1,0),(3,2,1,0)]

def mon(addr):
    pers, otid = struct.unpack("<II", rd(addr, 8))
    if pers == 0 and otid == 0: return None
    sec = rd(addr + 0x20, 48); key = pers ^ otid
    dec = b"".join(struct.pack("<I", struct.unpack("<I", sec[i:i+4])[0] ^ key) for i in range(0, 48, 4))
    order = ORDER[pers % 24]
    g = dec[order.index(0)*12:][:12]; a = dec[order.index(1)*12:][:12]
    sp, item, exp = struct.unpack("<HHI", g[:8])
    mv = struct.unpack("<4H", a[:8]); pp = a[8:12]
    lvl, _, hp, mhp = struct.unpack("<BBHH", rd(addr + 0x54, 6))
    nick = decode(rd(addr + 8, 10))
    return dict(species=species.get(sp, sp), nick=nick, lvl=lvl, hp=hp, maxhp=mhp, exp=exp,
                moves=[f"{moves.get(x,x)}({pp[i]})" for i, x in enumerate(mv) if x])

def player_pos():
    sb1 = u32(syms["gSaveBlock1Ptr"])
    x, y = s16(sb1), s16(sb1 + 2)
    g, n = u8(sb1 + 4), u8(sb1 + 5)
    return g, n, x, y

def status():
    g, n, x, y = player_pos()
    out = [f"MAP {mapname(g,n)} ({g}.{n}) pos=({x},{y})"]
    pidx = u8(syms["gPlayerAvatar"] + 5)
    facing = u8(syms["gObjectEvents"] + pidx * 0x24 + 0x18) & 0xF
    money = u32(u32(syms["gSaveBlock1Ptr"]) + 0x490) ^ u32(u32(syms["gSaveBlock2Ptr"]) + 0xAC)
    out.append(f"money={money}")
    out.append(f"facing={ {1:'D',2:'U',3:'L',4:'R'}.get(facing, facing)}")
    bt = u32(syms["gBattleTypeFlags"])
    cb2 = u32(syms["gMain"] + 4)
    out.append(f"battleflags={bt:#x} cb2={cb2:#x} inBattle={in_battle()}")
    cnt = u8(syms["gPlayerPartyCount"])
    for i in range(min(cnt, 6)):
        m = mon(syms["gPlayerParty"] + 100 * i)
        if m: out.append(f"  P{i}: {m['species']} '{m['nick']}' L{m['lvl']} {m['hp']}/{m['maxhp']} xp{m['exp']} {' '.join(m['moves'])}")
    if in_battle():
        for i in range(2):
            b = syms["gBattleMons"] + 0x58 * i
            sp = u16(b); hp = u16(b + 0x28); lvl = u8(b + 0x2A); mhp = u16(b + 0x2C)
            out.append(f"  B{i}: {species.get(sp, sp)} L{lvl} {hp}/{mhp}")
        out.append("  battletext: " + decode(rd(syms["gDisplayedStringBattle"], 200)).replace("\n", " / "))
    return "\n".join(out)


MB = {}
_i = 0
for line in open(f"{ROOT}/include/constants/metatile_behaviors.h"):
    m = re.match(r"\s+(MB_\w+)(?:\s*=\s*(\w+))?,", line)
    if m:
        if m.group(2): _i = int(m.group(2), 0)
        MB[_i] = m.group(1); _i += 1
DELTA = dict(U=(0,-1), D=(0,1), L=(-1,0), R=(1,0))
JUMPDIR = dict(MB_JUMP_EAST="R", MB_JUMP_WEST="L", MB_JUMP_NORTH="U", MB_JUMP_SOUTH="D")
IMPASS = {"MB_IMPASSABLE_EAST": "R", "MB_IMPASSABLE_WEST": "L", "MB_IMPASSABLE_NORTH": "U", "MB_IMPASSABLE_SOUTH": "D"}

class Grid:
    def __init__(self, surf=False):
        bml = syms["gBackupMapLayout"]
        self.w, self.h = struct.unpack("<ii", rd(bml, 8)); mp = u32(bml + 8)
        raw = rd(mp, self.w * self.h * 2)
        self.cells = struct.unpack(f"<{self.w*self.h}H", raw)
        lay = u32(syms["gMapHeader"])
        prim, sec = u32(lay + 0x10), u32(lay + 0x14)
        pa = u32(prim + 0x10); sa = u32(sec + 0x10) if sec else 0
        self.pattr = struct.unpack("<640H", rd(pa, 1280))
        self.sattr = struct.unpack("<384H", rd(sa, 768)) if sa else (0,) * 384
        self.surf = surf
        # objects (VMap coords)
        self.objs = {}
        for i in range(16):
            b = syms["gObjectEvents"] + i * 0x24
            if u8(b) & 1 and not (u8(b + 2) & 1):
                if u8(b + 1) & 0x20: continue  # invisible
                x, y = s16(b + 0x10), s16(b + 0x12)
                self.objs[(x - 7, y - 7)] = u8(b + 8)
        ev = u32(syms["gMapHeader"] + 4)
        oc, wc, cc, bc = rd(ev, 4)
        wp = u32(ev + 8); cp = u32(ev + 12); bp = u32(ev + 16)
        self.warps = {}
        for i in range(wc):
            x, y, el, wid, mn, mg = struct.unpack("<hhBBBB", rd(wp + 8 * i, 8))
            self.warps[(x, y)] = mapname(mg, mn)
        self.coords = set()
        for i in range(cc):
            x, y = struct.unpack("<hh", rd(cp + 16 * i, 4)); self.coords.add((x, y))
        self.bgs = set()
        for i in range(bc):
            x, y = struct.unpack("<HH", rd(bp + 12 * i, 4)); self.bgs.add((x, y))
    def cell(self, x, y):
        vx, vy = x + 7, y + 7
        if not (0 <= vx < self.w and 0 <= vy < self.h): return None
        return self.cells[vx + vy * self.w]
    def beh(self, x, y):
        c = self.cell(x, y)
        if c is None: return "OUT"
        mid = c & 0x3FF
        a = self.pattr[mid] if mid < 640 else self.sattr[mid - 640] if mid < 1024 else 0
        return MB.get(a & 0xFF, "?")
    def blocked(self, x, y):
        c = self.cell(x, y)
        if c is None or (c & 0x3FF) == 0x3FF: return True
        if c & 0xC00: return True
        b = self.beh(x, y)
        if "WATER" in b and "SHALLOW" not in b and not self.surf: return True
        if b in ("MB_WATERFALL", "MB_COUNTER"): return True
        if (x, y) in self.objs: return True
        return False
    def neighbors(self, x, y):
        for d, (dx, dy) in DELTA.items():
            nx, ny = x + dx, y + dy
            b = self.beh(nx, ny)
            if b in JUMPDIR:
                if JUMPDIR[b] == d: yield d, (nx + dx, ny + dy)
                continue
            if self.blocked(nx, ny): continue
            yield d, (nx, ny)
    def path(self, start, goal, adjacent=False):
        from collections import deque
        prev = {start: None}; q = deque([start])
        goals = {goal} if adjacent is not True else {(goal[0]+dx, goal[1]+dy) for dx, dy in DELTA.values()}
        while q:
            p = q.popleft()
            if p in goals:
                out = []
                while prev[p]: d, p = prev[p]; out.append(d)
                return out[::-1]
            for d, n in self.neighbors(*p):
                if n not in prev:
                    prev[n] = (d, p); q.append(n)
                    if n in self.warps and n not in goals: q.pop()  # don't path through warps
        if adjacent == "near":
            best = min(prev, key=lambda p: abs(p[0]-goal[0]) + abs(p[1]-goal[1]))
            if best != start: return self.path(start, best)
        return None
    def ascii(self, cx, cy, r=12):
        lines = []
        for y in range(cy - r // 2 - 2, cy + r // 2 + 3):
            row = f"{y:4d} "
            for x in range(cx - r, cx + r + 1):
                b = self.beh(x, y); c = self.cell(x, y)
                if (x, y) == (cx, cy): ch = "@"
                elif (x, y) in self.objs: ch = "O"
                elif (x, y) in self.warps: ch = "W"
                elif (x, y) in self.bgs: ch = "S"
                elif c is None or (c & 0x3FF) == 0x3FF: ch = " "
                elif b in JUMPDIR: ch = {"R": ">", "L": "<", "U": "^", "D": "v"}[JUMPDIR[b]]
                elif "WATER" in b and "SHALLOW" not in b: ch = "~"
                elif c & 0xC00: ch = "#"
                elif (x, y) in self.coords: ch = "t"
                elif "GRASS" in b: ch = '"'
                else: ch = "."
                row += ch
            lines.append(row)
        hdr = "     " + "".join(str(abs(x) % 10) for x in range(cx - r, cx + r + 1))
        info = "warps: " + ", ".join(f"{k}->{v}" for k, v in self.warps.items())
        info += "\nobjs: " + ", ".join(f"{k}#{v}" for k, v in self.objs.items())
        info += "\nsigns: " + ", ".join(map(str, self.bgs)) + "  triggers: " + ", ".join(map(str, sorted(self.coords)))
        return f"x from {cx-r}\n" + hdr + "\n" + "\n".join(lines) + "\n" + info

def in_battle(): return (u32(syms["gMain"] + 4) & ~1) == syms["BattleMainCB2"]

def goto(gx, gy, adjacent=False, face=None):
    for attempt in range(6):
        g, n, x, y = player_pos()
        grid = Grid()
        p = grid.path((x, y), (gx, gy), adjacent if adjacent else "near")
        if p is None: print("NO PATH to", gx, gy); return False
        if not p: break
        ok = True
        for d in p:
            before = player_pos()
            f = 0
            while player_pos() == before and f < 40:
                lib.emu_run(DIRS[d], 1); f += 1
            if player_pos() == before:
                lib.emu_run(0, 30); ok = False; break
            if player_pos()[:2] != before[:2]:   # warped to another map
                lib.emu_run(0, 60); print("WARPED"); return True
            lib.emu_run(DIRS[d], 1)
            if in_battle(): print("BATTLE!"); return False
        lib.emu_run(0, 16)
        if ok: break
    if adjacent:
        g, n, x, y = player_pos()
        dx, dy = gx - x, gy - y
        d = {(1,0):"R",(-1,0):"L",(0,1):"D",(0,-1):"U"}.get((dx, dy))
        if d: lib.emu_run(DIRS[d], 3); lib.emu_run(0, 10)
    return True

def busy():
    return u8(syms["sFieldMessageBoxMode"]) != 0 or u8(syms["sLockFieldControls"]) != 0

lasttext = [None]
def logtext():
    t = decode(rd(syms["gStringVar4"], 400)).replace("\n", " ")
    if t != lasttext[0]:
        lasttext[0] = t; print("TEXT:", t)

def advance(key=1, maxn=80):
    for i in range(maxn):
        if not busy() or in_battle(): break
        logtext(); tap(key, 4, 16)
    lib.emu_run(0, 10)

# battle_controller_player.c's input handlers (other controllers have static functions with the same names)
CHOOSE_MOVE = syms["HandleInputChooseMove"]
CHOOSE_ACTION = min(allsyms["HandleInputChooseAction"], key=lambda a: abs(a - CHOOSE_MOVE))
YESNO = syms["PlayerHandleYesNoInput"]
def ctrl(): return u32(syms["gBattlerControllerFuncs"]) & ~1

def battle_turn(action, move=0, maxf=4000):
    """wait for the action menu, pick action (0 fight,1 bag,2 mon,3 run) / move, then run until next prompt"""
    f = 0
    while in_battle() and ctrl() != CHOOSE_ACTION and f < maxf:
        if ctrl() == YESNO: print("YESNO prompt"); return "yesno"
        tap(1, 2, 8); f += 10
    if not in_battle(): return "over"
    if action is None: return "menu"
    lib.emu_write8(syms["gActionSelectionCursor"], action)
    tap(1, 2, 20)
    if action == 0:
        f = 0
        while ctrl() != CHOOSE_MOVE and f < 200: lib.emu_run(0, 1); f += 1
        lib.emu_write8(syms["gMoveSelectionCursor"], move)
        tap(1, 2, 20)
    if action in (0, 3):
        f = 0
        while in_battle() and f < maxf:
            c = ctrl()
            if c == CHOOSE_ACTION: break
            if c == YESNO: print("YESNO prompt"); return "yesno"
            tap(1, 2, 8); f += 10
    for l in status().splitlines():
        if l.startswith("  B") or "battletext" in l: print(l)
    return "ok" if in_battle() else "over"

def fight_loop():
    while True:
        trainer = u32(syms["gBattleTypeFlags"]) & 8
        mvi = (trainermove[0] if trainer else automove[0])
        r = battle_turn(3) if mvi == 9 else battle_turn(0, mvi)
        if r != "ok": break
        hp = u16(syms["gBattleMons"] + 0x28); mhp = u16(syms["gBattleMons"] + 0x2C)
        if hp * 3 < mhp: print("LOW HP, stopping"); return "low"
    print("battle:", r, "outcome", u8(syms["gBattleOutcome"]))
    return r

items = {}; _i = 0
for line in open(f"{ROOT}/include/constants/items.h"):
    m = re.match(r"\s+ITEM_(\w+)(?:\s*=\s*(\w+))?,", line)
    if m:
        if m.group(2) and m.group(2).isdigit(): _i = int(m.group(2))
        items.setdefault(_i, m.group(1)); _i += 1
def bag():
    sb1 = u32(syms["gSaveBlock1Ptr"]); key = u32(u32(syms["gSaveBlock2Ptr"]) + 0xAC) & 0xFFFF
    out = []
    for name, off, cap in (("items", 0x560, 30), ("key", 0x5D8, 30), ("balls", 0x650, 16), ("tm", 0x690, 64)):
        for i in range(cap):
            it, q = struct.unpack("<HH", rd(sb1 + off + 4 * i, 4))
            if it: out.append(f"{items.get(it, it)}x{q ^ key}")
    return " ".join(out)

def tap(k, hold=4, rel=12):
    lib.emu_run(k, hold); lib.emu_run(0, rel)

def walk(d, n, maxf=600):
    """walk n tiles in direction d holding it; stops early if blocked"""
    for _ in range(n):
        start = player_pos(); f = 0
        while player_pos() == start and f < 64:
            lib.emu_run(DIRS[d], 1); f += 1
        if player_pos() == start:
            lib.emu_run(0, 1); return False
        # finish the step
        lib.emu_run(DIRS[d], 1)
    # let the last step finish
    lib.emu_run(0, 16)
    return True

shotn = [max([int(f[:5]) for f in os.listdir(SHOTS)] + [0]) + 1]
taken = []
automove = [0]
trainermove = [0]
def shot(name=None):
    p = lib.emu_frame()
    data = bytes(ctypes.cast(p, ctypes.POINTER(ctypes.c_ubyte * (240*160*4))).contents)
    img = Image.frombytes("RGBX", (240, 160), data, "raw", "RGBX", 0, 1).convert("RGB")
    path = name or f"{SHOTS}/{shotn[0]:05d}.png"; shotn[0] += 1
    img.resize((480, 320), Image.NEAREST).save(path)
    taken.append(img)
    print("SHOT", path)

def main():
    args = sys.argv[1:]
    state = "cur"; new = False; nosave = False
    while args and args[0].startswith("--"):
        a = args.pop(0)
        if a == "--new": new = True
        elif a == "--state": state = args.pop(0)
        elif a == "--nosave": nosave = True
    r = lib.emu_init(ROM.encode(), SAV.encode())
    assert r > 0, r
    cur = f"{STATES}/cur.ss"
    if not new:
        assert lib.emu_load_state(f"{STATES}/{state}.ss".encode()), "load failed"
    for tok in " ".join(args).split():
        m = re.fullmatch(r"(A|B|ST|SE|U|D|L|R|Lb|Rb)(?:\*(\d+))?", tok)
        if m:
            for _ in range(int(m.group(2) or 1)): tap(KEYS[m.group(1)])
            continue
        m = re.fullmatch(r"h([UDLR])(\d+)", tok)
        if m:
            if not walk(m.group(1), int(m.group(2))): print("BLOCKED at", player_pos())
            continue
        m = re.fullmatch(r"w(\d+)", tok)
        if m: lib.emu_run(0, int(m.group(1))); continue
        m = re.fullmatch(r"m(\d+)", tok)
        if m:
            for _ in range(int(m.group(1))): tap(1, 4, 20)
            continue
        m = re.fullmatch(r"bm(\d+)", tok)
        if m:
            for _ in range(int(m.group(1))): tap(2, 4, 20)
            continue
        if tok == "shot": shot(); continue
        m = re.fullmatch(r"f(a?)(\d)", tok)
        if m:
            while True:
                r = battle_turn(0, int(m.group(2)))
                if not m.group(1) or r != "ok": break
            print("battle:", r, "outcome", u8(syms["gBattleOutcome"])); continue
        m = re.fullmatch(r"G(-?\d+),(-?\d+)", tok)
        if m:
            gx, gy = int(m.group(1)), int(m.group(2)); stop = False
            for _ in range(30):
                start_map = player_pos()[:2]
                goto(gx, gy)
                if busy() and not in_battle():
                    lib.emu_run(0, 30); advance(); lib.emu_run(0, 60)
                    for _ in range(30):
                        if in_battle() or not busy(): break
                        tap(1, 2, 20)
                if in_battle():
                    r = fight_loop()
                    if r == "low": stop = True
                    if r != "over": stop = True; break
                    lib.emu_run(0, 60); advance()
                    continue
                if player_pos()[:2] != start_map: break
                if player_pos()[2:] == (gx, gy): break
                g_, n_, x_, y_ = player_pos(); break
            if stop: break
            continue
        m = re.fullmatch(r"grind:(\d+):([UDLR])([UDLR])", tok)
        if m:
            n = int(m.group(1)); d1, d2 = m.group(2), m.group(3); fights = 0; steps = 0
            while fights < n and steps < 3000:
                walk(d1, 1); walk(d2, 1); steps += 2
                if in_battle():
                    fights += 1
                    r = fight_loop()
                    if r != "over": break
                    lib.emu_run(0, 60); advance()
                    p = mon(syms["gPlayerParty"])
                    if p["hp"] * 5 < p["maxhp"] * 2: print("HP low after battle"); break
            print("fights", fights); continue
        m = re.fullmatch(r"tmv=(\d)", tok)
        if m: trainermove[0] = int(m.group(1)); continue
        m = re.fullmatch(r"mv=(\d)", tok)
        if m: automove[0] = int(m.group(1)); continue
        if tok == "run": print("battle:", battle_turn(3)); continue
        if tok == "bwait": print("battle:", battle_turn(None)); continue
        if tok in ("xa", "xb"): advance(1 if tok == "xa" else 2); continue
        if tok == "bag": print("BAG:", bag()); continue
        if tok == "map":
            g, n, x, y = player_pos(); print(Grid().ascii(x, y)); continue
        m = re.fullmatch(r"(g|t)(-?\d+),(-?\d+)", tok)
        if m:
            goto(int(m.group(2)), int(m.group(3)), adjacent=m.group(1) == "t"); continue
        m = re.fullmatch(r"shot:(\S+)", tok)
        if m: shot(m.group(1)); continue
        m = re.fullmatch(r"save:(\S+)", tok)
        if m: lib.emu_save_state(f"{STATES}/{m.group(1)}.ss".encode()); continue
        raise SystemExit(f"bad token {tok}")
    if not nosave: lib.emu_save_state(cur.encode())
    if len(taken) > 1:
        cols = 2; rows = (len(taken) + 1) // 2
        mont = Image.new("RGB", (cols * 244, rows * 164), "red")
        for i, im in enumerate(taken): mont.paste(im, ((i % cols) * 244 + 2, (i // cols) * 164 + 2))
        mont.save(f"{WORK}/montage.png"); print("MONTAGE", f"{WORK}/montage.png")
    print(status())
    lib.emu_close()

if __name__ == "__main__":
    main()
