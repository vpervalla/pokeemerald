#!/usr/bin/env python3
"""Render a map layout to PNG. usage: render.py MAPNAME OUT.png [--grid] [--coll] [--objs]"""
import json, re, struct, sys, os
import numpy as np
from PIL import Image, ImageDraw
R = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def tileset_dir(sym):
    src = open(R + "/src/data/tilesets/graphics.h").read()
    name = sym.replace("gTileset_", "")
    m = re.search(r"gTilesetTiles_" + name + r"\[\] = INCBIN_U32\(\"([^\"]+)/tiles\.4bpp", src)
    return R + "/" + m.group(1)

def load_ts(d):
    tiles = np.frombuffer(open(d + "/tiles.4bpp", "rb").read(), dtype=np.uint8)
    n = len(tiles) // 32
    t = np.zeros((n, 8, 8), np.uint8)
    b = tiles.reshape(n, 8, 4)
    t[:, :, 0::2] = b & 15
    t[:, :, 1::2] = b >> 4
    pals = []
    for i in range(16):
        p = d + f"/palettes/{i:02d}.gbapal"
        raw = open(p, "rb").read() if os.path.exists(p) else bytes(32)
        cols = []
        for j in range(16):
            c = struct.unpack("<H", raw[j*2:j*2+2])[0]
            cols.append(((c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3))
        pals.append(cols)
    mt = np.frombuffer(open(d + "/metatiles.bin", "rb").read(), dtype="<u2").reshape(-1, 8)
    return t, pals, mt

def render(mapname, grid=False, coll=False, objs=False, scale=1):
    mj = json.load(open(f"{R}/data/maps/{mapname}/map.json"))
    layouts = {l["id"]: l for l in json.load(open(R + "/data/layouts/layouts.json"))["layouts"] if l}
    L = layouts[mj["layout"]]
    kanto = "Kanto" in L["primary_tileset"]
    NT = 640 if kanto else 512
    NM = 640 if kanto else 512
    NP = 7 if kanto else 6
    pt, ppal, pmt = load_ts(tileset_dir(L["primary_tileset"]))
    st, spal, smt = load_ts(tileset_dir(L["secondary_tileset"]))
    pals = ppal[:NP] + spal[NP:13] + [ppal[0]] * 3
    pals = np.array(pals, np.uint8)
    w, h = L["width"], L["height"]
    blocks = np.frombuffer(open(R + "/" + L["blockdata_filepath"], "rb").read(), dtype="<u2").reshape(h, w)
    img = np.zeros((h * 16, w * 16, 3), np.uint8)
    cache = {}
    def tile(entry):
        idx = entry & 0x3FF
        t = pt[idx] if idx < NT else (st[idx - NT] if idx - NT < len(st) else np.zeros((8, 8), np.uint8))
        if entry & 0x400: t = t[:, ::-1]
        if entry & 0x800: t = t[::-1, :]
        return t, (entry >> 12) & 15
    def meta(m):
        if m in cache: return cache[m]
        e = pmt[m] if m < NM else (smt[m - NM] if m - NM < len(smt) else np.zeros(8, np.uint16))
        out = np.zeros((16, 16, 3), np.uint8)
        out[:] = pals[0][0]
        for layer in range(2):
            for k in range(4):
                t, p = tile(int(e[layer * 4 + k]))
                y, x = (k // 2) * 8, (k % 2) * 8
                sub = out[y:y+8, x:x+8]
                mask = t != 0
                sub[mask] = pals[p][t[mask]]
        cache[m] = out
        return out
    for y in range(h):
        for x in range(w):
            b = int(blocks[y, x])
            img[y*16:y*16+16, x*16:x*16+16] = meta(b & 0x3FF)
            if coll and (b >> 10) & 3:
                img[y*16:y*16+16, x*16:x*16+16] = img[y*16:y*16+16, x*16:x*16+16] // 2 + np.array([100, 0, 0], np.uint8)
    im = Image.fromarray(img)
    if scale != 1:
        im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    s = 16 * scale
    dr = ImageDraw.Draw(im)
    if grid:
        for x in range(w):
            for y in range(h):
                if x % 5 == 0 and y % 5 == 0:
                    dr.text((x*s+1, y*s+1), f"{x},{y}", fill=(255, 255, 0))
        for x in range(0, w, 5): dr.line([(x*s, 0), (x*s, h*s)], fill=(255, 255, 255))
        for y in range(0, h, 5): dr.line([(0, y*s), (w*s, y*s)], fill=(255, 255, 255))
    if objs:
        for o in mj.get("object_events", []):
            dr.rectangle([o["x"]*s+2, o["y"]*s+2, o["x"]*s+s-3, o["y"]*s+s-3], outline=(255, 0, 255), width=2)
        for wp in mj.get("warp_events", []):
            dr.rectangle([wp["x"]*s+4, wp["y"]*s+4, wp["x"]*s+s-5, wp["y"]*s+s-5], outline=(0, 255, 255), width=2)
    return im

if __name__ == "__main__":
    a = sys.argv
    render(a[1], "--grid" in a, "--coll" in a, "--objs" in a, 2 if "--x2" in a else 1).save(a[2])
