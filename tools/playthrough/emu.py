"""A driver process over the current savestate, shared by g.py, st.py and fight.py."""
import glob, os, shutil, subprocess
from PIL import Image
from syms import *

BIT = {"A": 1, "B": 2, "SELECT": 4, "START": 8, "RIGHT": 16, "LEFT": 32, "UP": 64, "DOWN": 128, "R": 256, "L": 512}


class Emu:
    def __init__(self, sav=SAV):
        for d in (HIST, SHOTS):
            os.makedirs(d, exist_ok=True)
        self.p = subprocess.Popen([DRIVER, ROM, sav], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        if os.path.exists(CUR):
            self.send(f"loadstate {CUR}")

    def send(self, c):
        self.p.stdin.write(c + "\n"); self.p.stdin.flush()

    def keys(self, mask, n):
        self.send(f"keys {mask} {n}")

    def wait(self, n):
        self.send(f"wait {n}")

    def peek(self, a, n):
        self.send(f"peek {a:x} {n}"); return bytes.fromhex(self.p.stdout.readline().strip())

    def u32(self, a):
        return int.from_bytes(self.peek(a, 4), "little")

    def cb2(self):
        return self.u32(GMAIN_CB2) | 1

    def pos(self):
        return self.peek(self.u32(S["gSaveBlock1Ptr"]), 4)

    def obj_pos(self):
        """The player's ObjectEvent tile (collision truth). Spinner slides can leave the save-block position behind it."""
        import re, struct
        if not hasattr(self, "_objaddr"):
            m = open(os.path.join(os.path.dirname(__file__), "..", "..", "pokeemerald_modern.map")).read()
            self._og = int(re.search(r"(0x[0-9a-f]+)\s+gObjectEvents\n", m).group(1), 16)
            self._pa = int(re.search(r"(0x[0-9a-f]+)\s+gPlayerAvatar\n", m).group(1), 16)
            self._objaddr = True
        base = self._og + self.peek(self._pa, 8)[5] * 0x24
        x, y = struct.unpack("<hh", self.peek(base + 0x10, 4))
        return x - 7, y - 7

    def fix_pos(self):
        """Copy the object's tile into the save-block position when a spinner slide left them apart."""
        import struct
        ox, oy = self.obj_pos()
        sb = self.u32(S["gSaveBlock1Ptr"])
        if struct.unpack("<hh", self.peek(sb, 4)) != (ox, oy):
            self.send(f"poke {sb:x}:" + struct.pack("<hh", ox, oy).hex())
            return True
        return False

    def shot(self, path):
        self.send(f"shot {path}")

    def commit(self, keep=True):
        """Save the new state as cur.ss (the old one goes to hist/) and write last.png. keep=False discards."""
        tmp_ss, tmp_ppm = os.path.join(PLAY_DIR, "new.ss"), os.path.join(PLAY_DIR, "last.ppm")
        self.wait(1); self.shot(tmp_ppm); self.send(f"savestate {tmp_ss}"); self.send("sync"); self.p.stdout.readline()
        self.p.stdin.close(); self.p.wait()
        if not keep:
            return
        if os.path.exists(CUR):
            n = len(glob.glob(os.path.join(HIST, "*.ss")))
            shutil.copy(CUR, os.path.join(HIST, f"{n:05d}.ss"))
        os.replace(tmp_ss, CUR)
        Image.open(tmp_ppm).resize((480, 320), Image.NEAREST).save(os.path.join(PLAY_DIR, "last.png"))
