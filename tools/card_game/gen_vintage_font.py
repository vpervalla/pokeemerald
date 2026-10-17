#!/usr/bin/env python3
"""Writes the vintage card test's tiny font (src/card_game_vintage.c) as a C table.

    python3 tools/card_game/gen_vintage_font.py

    src/data/card_game/vintage_font.h   per game character: its width and 8 rows of pixels

Capitals and digits are 6 pixels tall, lowercase 4 (with ascenders and descenders), a pixel
smaller than FONT_SMALL; the cards' small print. Rows are 0-7: capitals on rows 0-5,
descenders on rows 6-7. Characters not here are drawn as a space.
"""

import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
OUT = os.path.join(ROOT, 'src', 'data', 'card_game', 'vintage_font.h')
SPACE_WIDTH = 2

# Each glyph: its rows from the top (row 0), '#' drawn; a glyph's width is its rows' width.
# Lowercase start at row 2 unless they have an ascender (given from row 0).
UPPER = {
    'A': [".##.", "#..#", "#..#", "####", "#..#", "#..#"],
    'B': ["###.", "#..#", "###.", "#..#", "#..#", "###."],
    'C': [".##.", "#..#", "#...", "#...", "#..#", ".##."],
    'D': ["###.", "#..#", "#..#", "#..#", "#..#", "###."],
    'E': ["####", "#...", "###.", "#...", "#...", "####"],
    'F': ["####", "#...", "###.", "#...", "#...", "#..."],
    'G': [".##.", "#...", "#.##", "#..#", "#..#", ".###"],
    'H': ["#..#", "#..#", "####", "#..#", "#..#", "#..#"],
    'I': ["###", ".#.", ".#.", ".#.", ".#.", "###"],
    'J': ["..##", "...#", "...#", "...#", "#..#", ".##."],
    'K': ["#..#", "#.#.", "##..", "#.#.", "#..#", "#..#"],
    'L': ["#...", "#...", "#...", "#...", "#...", "####"],
    'M': ["#...#", "##.##", "#.#.#", "#...#", "#...#", "#...#"],
    'N': ["#..#", "##.#", "#.##", "#..#", "#..#", "#..#"],
    'O': [".##.", "#..#", "#..#", "#..#", "#..#", ".##."],
    'P': ["###.", "#..#", "#..#", "###.", "#...", "#..."],
    'Q': [".##.", "#..#", "#..#", "#..#", "#.#.", ".#.#"],
    'R': ["###.", "#..#", "#..#", "###.", "#.#.", "#..#"],
    'S': [".###", "#...", ".##.", "...#", "...#", "###."],
    'T': ["###", ".#.", ".#.", ".#.", ".#.", ".#."],
    'U': ["#..#", "#..#", "#..#", "#..#", "#..#", ".##."],
    'V': ["#...#", "#...#", "#...#", ".#.#.", ".#.#.", "..#.."],
    'W': ["#...#", "#...#", "#.#.#", "#.#.#", "##.##", "#...#"],
    'X': ["#..#", "#..#", ".##.", ".##.", "#..#", "#..#"],
    'Y': ["#.#", "#.#", "#.#", ".#.", ".#.", ".#."],
    'Z': ["####", "...#", "..#.", ".#..", "#...", "####"],
    '0': [".##.", "#..#", "#.##", "##.#", "#..#", ".##."],
    '1': [".#.", "##.", ".#.", ".#.", ".#.", "###"],
    '2': [".##.", "#..#", "..#.", ".#..", "#...", "####"],
    '3': ["###.", "...#", ".##.", "...#", "...#", "###."],
    '4': ["#..#", "#..#", "####", "...#", "...#", "...#"],
    '5': ["####", "#...", "###.", "...#", "...#", "###."],
    '6': [".##.", "#...", "###.", "#..#", "#..#", ".##."],
    '7': ["####", "...#", "..#.", ".#..", ".#..", ".#.."],
    '8': [".##.", "#..#", ".##.", "#..#", "#..#", ".##."],
    '9': [".##.", "#..#", "#..#", ".###", "...#", ".##."],
    '.': [".", ".", ".", ".", ".", "#"],
    ',': ["..", "..", "..", "..", "..", ".#", "#."],
    ':': [".", "#", ".", ".", "#", "."],
    '’': ["#", "#"],
    '”': ["#.#", "#.#"],
    '(': [".#", "#.", "#.", "#.", "#.", ".#"],
    ')': ["#.", ".#", ".#", ".#", ".#", "#."],
    '-': ["...", "...", "...", "###"],
    '+': ["...", "...", ".#.", "###", ".#."],
    '!': ["#", "#", "#", "#", ".", "#"],
    '?': [".##.", "#..#", "..#.", ".#..", "....", ".#.."],
    '/': ["..#", "..#", ".#.", ".#.", "#..", "#.."],
    'é': ["..#.", ".#..", ".##.", "####", "#...", ".###"],
}
LOWER = {
    'a': [".###", "#..#", "#..#", ".###"],
    'b': ["#...", "#...", "###.", "#..#", "#..#", "###."],
    'c': [".##", "#..", "#..", ".##"],
    'd': ["...#", "...#", ".###", "#..#", "#..#", ".###"],
    'e': [".##.", "####", "#...", ".###"],
    'f': [".##", "#..", "###", "#..", "#..", "#.."],
    'g': [".###", "#..#", "#..#", ".###", "...#", ".##."],
    'h': ["#...", "#...", "###.", "#..#", "#..#", "#..#"],
    'i': ["#", ".", "#", "#", "#", "#"],
    'j': [".#", "..", ".#", ".#", ".#", ".#", ".#", "#."],
    'k': ["#...", "#...", "#..#", "###.", "#.#.", "#..#"],
    'l': ["#", "#", "#", "#", "#", "#"],
    'm': ["####.", "#.#.#", "#.#.#", "#.#.#"],
    'n': ["###.", "#..#", "#..#", "#..#"],
    'o': [".##.", "#..#", "#..#", ".##."],
    'p': ["###.", "#..#", "#..#", "###.", "#...", "#..."],
    'q': [".###", "#..#", "#..#", ".###", "...#", "...#"],
    'r': ["#.#", "##.", "#..", "#.."],
    's': [".###", "##..", "..##", "###."],
    't': ["...", ".#.", "###", ".#.", ".#.", "..#"],
    'u': ["#..#", "#..#", "#..#", ".###"],
    'v': ["#.#", "#.#", "#.#", ".#."],
    'w': ["#...#", "#.#.#", "#.#.#", ".#.#."],
    'x': ["#..#", ".##.", ".##.", "#..#"],
    'y': ["#..#", "#..#", "#..#", ".###", "...#", ".##."],
    'z': ["####", "..#.", ".#..", "####"],
}
# Lowercase with an ascender (or a dot) start at row 0, the others at row 2
ASCENDERS = set('bdfhijklt')


def glyph_rows(ch):
    if ch in UPPER:
        return UPPER[ch]
    rows = LOWER[ch]
    return rows if ch in ASCENDERS else [''] * 2 + rows


def read_charmap():
    codes = {}
    with open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8') as f:
        for line in f:
            # 'A' = BB
            if len(line) >= 7 and line[0] == "'" and line[2] == "'":
                value = line[3:].split('@')[0].split('=')
                if len(value) == 2 and len(value[1].split()) == 1:
                    codes.setdefault(line[1], int(value[1], 16))
    return codes


def main():
    codes = read_charmap()
    widths = [0] * 256
    pixels = [[0] * 8 for _ in range(256)]
    for ch in list(UPPER) + list(LOWER):
        if ch not in codes:
            raise SystemExit(f'{ch!r} is not in charmap.txt')
        rows = glyph_rows(ch)
        code = codes[ch]
        widths[code] = max(len(r) for r in rows)
        for y, row in enumerate(rows):
            for x, p in enumerate(row):
                if p == '#':
                    pixels[code][y] |= 0x80 >> x
    widths[codes[' ']] = SPACE_WIDTH

    with open(OUT, 'w') as f:
        f.write('// Generated by tools/card_game/gen_vintage_font.py. Do not edit by hand: rerun the script.\n\n')
        f.write('// Per game character: its width in pixels (0: not in the font, drawn as a space)\n')
        f.write('static const u8 sTinyGlyphWidths[256] =\n{\n')
        for i in range(0, 256, 16):
            f.write('    ' + ', '.join(str(w) for w in widths[i:i + 16]) + ',\n')
        f.write('};\n\n')
        f.write('// Per game character: 8 rows, the leftmost pixel in the top bit\n')
        f.write('static const u8 sTinyGlyphs[256][8] =\n{\n')
        for code in range(256):
            if widths[code] and any(pixels[code]):
                f.write(f'    [0x{code:02X}] = {{' + ', '.join(f'0x{r:02X}' for r in pixels[code]) + '},\n')
        f.write('};\n')
    print(f'{sum(1 for w in widths if w)} glyphs in {os.path.relpath(OUT, ROOT)}')


if __name__ == '__main__':
    main()
