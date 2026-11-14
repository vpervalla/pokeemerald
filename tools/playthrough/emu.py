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
