#!/usr/bin/env python3
"""Import Mega Evolutions: sprites, species data, Mega Stones.

Copies the Gen 3 style Mega sprites and Mega Stone icons from a checkout of
pokeemerald-expansion (https://github.com/rh-hideout/pokeemerald-expansion; see
docs/mega_evolution_credits.md), and writes every table entry the Megas in MEGAS need:
species constants and data, sprite tables, the Mega Stone items, and the
sMegaEvolutions table. Generated code sits between "<mega-evolutions>" markers, so
running it again regenerates it instead of adding to it.

To add a Mega, add a row to MEGAS and run this from the pokeemerald root:

    git clone --depth 1 --filter=blob:none --sparse https://github.com/rh-hideout/pokeemerald-expansion.git exp
    git -C exp sparse-checkout set --no-cone '/graphics/pokemon/*/mega*/' '/graphics/items/icons/' \
        '/graphics/items/icon_palettes/' '/src/data/pokemon/species_info/'
    python3 tools/mega_evolutions/import_megas.py exp

Needs Pillow. Sprite offsets and elevations come from the expansion's species data.
"""
import glob, os, re, sys, shutil
from PIL import Image

EXP = sys.argv[1]

# Rows are only ever appended, so existing species and item ids (items are saved) don't change.
# (mega const suffix, base species const, graphics dir, C symbol, stone const, stone name, stone file,
#  stats HP/Atk/Def/Spe/SpA/SpD, types, ability)
MEGAS = [
 ('BLAZIKEN_MEGA',   'BLAZIKEN',  'blaziken/mega',   'BlazikenMega',  'BLAZIKENITE',   'BLAZIKENITE',   'blazikenite',    (80,160,80,100,130,80),   ('FIRE','FIGHTING'),  'SPEED_BOOST'),
 ('VENUSAUR_MEGA',   'VENUSAUR',  'venusaur/mega',   'VenusaurMega',  'VENUSAURITE',   'VENUSAURITE',   'venusaurite',    (80,100,123,80,122,120),  ('GRASS','POISON'),   'THICK_FAT'),
 ('CHARIZARD_MEGA_Y','CHARIZARD', 'charizard/mega_y','CharizardMegaY','CHARIZARDITE_Y','CHARZARDITE Y', 'charizardite_y', (78,104,78,100,159,115),  ('FIRE','FLYING'),    'DROUGHT'),
 ('ALAKAZAM_MEGA',   'ALAKAZAM',  'alakazam/mega',   'AlakazamMega',  'ALAKAZITE',     'ALAKAZITE',     'alakazite',      (55,50,65,150,175,105),   ('PSYCHIC','PSYCHIC'),'TRACE'),
 ('SLOWBRO_MEGA',    'SLOWBRO',   'slowbro/mega',    'SlowbroMega',   'SLOWBRONITE',   'SLOWBRONITE',   'slowbronite',    (95,75,180,30,130,80),    ('WATER','PSYCHIC'),  'SHELL_ARMOR'),
 ('GENGAR_MEGA',     'GENGAR',    'gengar/mega',     'GengarMega',    'GENGARITE',     'GENGARITE',     'gengarite',      (60,65,80,130,170,95),    ('GHOST','POISON'),   'SHADOW_TAG'),
 ('MEWTWO_MEGA_Y',   'MEWTWO',    'mewtwo/mega_y',   'MewtwoMegaY',   'MEWTWONITE_Y',  'MEWTWONITE Y',  'mewtwonite_y',   (106,150,70,140,194,120), ('PSYCHIC','PSYCHIC'),'INSOMNIA'),
 ('TYRANITAR_MEGA',  'TYRANITAR', 'tyranitar/mega',  'TyranitarMega', 'TYRANITARITE',  'TYRANITARITE',  'tyranitarite',   (100,164,150,71,95,120),  ('ROCK','DARK'),      'SAND_STREAM'),
 ('SCEPTILE_MEGA',   'SCEPTILE',  'sceptile/mega',   'SceptileMega',  'SCEPTILITE',    'SCEPTILITE',    'sceptilite',     (70,110,75,145,145,85),   ('GRASS','DRAGON'),   'LIGHTNING_ROD'),
 ('SWAMPERT_MEGA',   'SWAMPERT',  'swampert/mega',   'SwampertMega',  'SWAMPERTITE',   'SWAMPERTITE',   'swampertite',    (100,150,110,70,95,110),  ('WATER','GROUND'),   'SWIFT_SWIM'),
 ('MAWILE_MEGA',     'MAWILE',    'mawile/mega',     'MawileMega',    'MAWILITE',      'MAWILITE',      'mawilite',       (50,105,125,50,55,95),    ('STEEL','FAIRY'),    'HUGE_POWER'),
 ('MEDICHAM_MEGA',   'MEDICHAM',  'medicham/mega',   'MedichamMega',  'MEDICHAMITE',   'MEDICHAMITE',   'medichamite',    (60,100,85,100,80,85),    ('FIGHTING','PSYCHIC'),'PURE_POWER'),
 ('MANECTRIC_MEGA',  'MANECTRIC', 'manectric/mega',  'ManectricMega', 'MANECTITE',     'MANECTITE',     'manectite',      (70,75,80,135,135,80),    ('ELECTRIC','ELECTRIC'),'INTIMIDATE'),
 ('LATIAS_MEGA',     'LATIAS',    'latias/mega',     'LatiasMega',    'LATIASITE',     'LATIASITE',     'latiasite',      (80,100,120,110,140,150), ('DRAGON','PSYCHIC'), 'LEVITATE'),
 ('LATIOS_MEGA',     'LATIOS',    'latios/mega',     'LatiosMega',    'LATIOSITE',     'LATIOSITE',     'latiosite',      (80,130,100,110,160,120), ('DRAGON','PSYCHIC'), 'LEVITATE'),
 ('CHARIZARD_MEGA_X','CHARIZARD', 'charizard/mega_x','CharizardMegaX','CHARIZARDITE_X','CHARZARDITE X', 'charizardite_x', (78,130,111,100,130,85),  ('FIRE','DRAGON'),    'TOUGH_CLAWS'),
 ('BLASTOISE_MEGA',  'BLASTOISE', 'blastoise/mega',  'BlastoiseMega', 'BLASTOISINITE', 'BLASTOISINITE', 'blastoisinite',  (79,103,120,78,135,115),  ('WATER','WATER'),    'MEGA_LAUNCHER'),
 ('PIDGEOT_MEGA',    'PIDGEOT',   'pidgeot/mega',    'PidgeotMega',   'PIDGEOTITE',    'PIDGEOTITE',    'pidgeotite',     (83,80,80,121,135,80),    ('NORMAL','FLYING'),  'NO_GUARD'),
 ('GYARADOS_MEGA',   'GYARADOS',  'gyarados/mega',   'GyaradosMega',  'GYARADOSITE',   'GYARADOSITE',   'gyaradosite',    (95,155,109,81,70,130),   ('WATER','DARK'),     'MOLD_BREAKER'),
 # No Mega Stone: Rayquaza Mega Evolves by knowing Dragon Ascent (see MOVE_MEGAS)
 ('RAYQUAZA_MEGA',   'RAYQUAZA',  'rayquaza/mega',   'RayquazaMega',  None,            None,            None,             (105,180,100,115,180,100),('DRAGON','FLYING'),  'DELTA_STREAM'),
 ('MEWTWO_MEGA_X',   'MEWTWO',    'mewtwo/mega_x',   'MewtwoMegaX',   'MEWTWONITE_X',  'MEWTWONITE X',  'mewtwonite_x',   (106,190,100,130,154,100),('PSYCHIC','FIGHTING'),'STEADFAST'),
 ('PINSIR_MEGA',     'PINSIR',    'pinsir/mega',     'PinsirMega',    'PINSIRITE',     'PINSIRITE',     'pinsirite',      (65,155,120,105,65,90),   ('BUG','FLYING'),     'AERILATE'),
 ('AERODACTYL_MEGA', 'AERODACTYL','aerodactyl/mega', 'AerodactylMega','AERODACTYLITE', 'AERODACTYLITE', 'aerodactylite',  (80,135,85,150,70,95),    ('ROCK','FLYING'),    'TOUGH_CLAWS'),
 ('KANGASKHAN_MEGA', 'KANGASKHAN','kangaskhan/mega', 'KangaskhanMega','KANGASKHANITE', 'KANGASKHANITE', 'kangaskhanite',  (105,125,100,100,60,100), ('NORMAL','NORMAL'),  'PARENTAL_BOND'),
 ('SALAMENCE_MEGA',  'SALAMENCE', 'salamence/mega',  'SalamenceMega', 'SALAMENCITE',   'SALAMENCITE',   'salamencite',    (95,145,130,120,120,90),  ('DRAGON','FLYING'),  'AERILATE'),
 ('METAGROSS_MEGA',  'METAGROSS', 'metagross/mega',  'MetagrossMega', 'METAGROSSITE',  'METAGROSSITE',  'metagrossite',   (80,145,150,110,105,110), ('STEEL','PSYCHIC'),  'TOUGH_CLAWS'),
 ('AMPHAROS_MEGA',   'AMPHAROS',  'ampharos/mega',   'AmpharosMega',  'AMPHAROSITE',   'AMPHAROSITE',   'ampharosite',    (90,95,105,45,165,110),   ('ELECTRIC','DRAGON'),'MOLD_BREAKER'),
 ('BEEDRILL_MEGA',   'BEEDRILL',  'beedrill/mega',   'BeedrillMega',  'BEEDRILLITE',   'BEEDRILLITE',   'beedrillite',    (65,150,40,145,15,80),    ('BUG','POISON'),     'ADAPTABILITY'),
 ('GLALIE_MEGA', 'GLALIE', 'glalie/mega', 'GlalieMega', 'GLALITITE', 'GLALITITE', 'glalitite', (80,120,80,100,120,80), ('ICE','ICE'), 'REFRIGERATE'),
 ('STEELIX_MEGA', 'STEELIX', 'steelix/mega', 'SteelixMega', 'STEELIXITE', 'STEELIXITE', 'steelixite', (75,125,230,30,55,95), ('STEEL','GROUND'), 'SAND_FORCE'),
 ('SCIZOR_MEGA', 'SCIZOR', 'scizor/mega', 'ScizorMega', 'SCIZORITE', 'SCIZORITE', 'scizorite', (70,150,140,75,65,100), ('BUG','STEEL'), 'TECHNICIAN'),
 ('HERACROSS_MEGA', 'HERACROSS', 'heracross/mega', 'HeracrossMega', 'HERACRONITE', 'HERACRONITE', 'heracronite', (80,185,115,75,40,105), ('BUG','FIGHTING'), 'SKILL_LINK'),
 ('HOUNDOOM_MEGA', 'HOUNDOOM', 'houndoom/mega', 'HoundoomMega', 'HOUNDOOMINITE', 'HOUNDOOMINITE', 'houndoominite', (75,90,90,115,140,90), ('DARK','FIRE'), 'SOLAR_POWER'),
 ('AGGRON_MEGA', 'AGGRON', 'aggron/mega', 'AggronMega', 'AGGRONITE', 'AGGRONITE', 'aggronite', (70,140,230,50,60,80), ('STEEL','STEEL'), 'FILTER'),
 ('CAMERUPT_MEGA', 'CAMERUPT', 'camerupt/mega', 'CameruptMega', 'CAMERUPTITE', 'CAMERUPTITE', 'cameruptite', (70,120,100,20,145,105), ('FIRE','GROUND'), 'SHEER_FORCE'),
 ('BANETTE_MEGA', 'BANETTE', 'banette/mega', 'BanetteMega', 'BANETTITE', 'BANETTITE', 'banettite', (64,165,75,75,93,83), ('GHOST','GHOST'), 'PRANKSTER'),
 ('SHARPEDO_MEGA', 'SHARPEDO', 'sharpedo/mega', 'SharpedoMega', 'SHARPEDONITE', 'SHARPEDONITE', 'sharpedonite', (70,140,70,105,110,65), ('WATER','DARK'), 'STRONG_JAW'),
 ('SABLEYE_MEGA', 'SABLEYE', 'sableye/mega', 'SableyeMega', 'SABLENITE', 'SABLENITE', 'sablenite', (50,85,125,20,85,115), ('DARK','GHOST'), 'MAGIC_BOUNCE'),
 ('ABSOL_MEGA', 'ABSOL', 'absol/mega', 'AbsolMega', 'ABSOLITE', 'ABSOLITE', 'absolite', (65,150,60,115,115,60), ('DARK','DARK'), 'MAGIC_BOUNCE'),
 ('GARDEVOIR_MEGA', 'GARDEVOIR', 'gardevoir/mega', 'GardevoirMega', 'GARDEVOIRITE', 'GARDEVOIRITE', 'gardevoirite', (68,85,65,100,165,135), ('PSYCHIC','FAIRY'), 'PIXILATE'),
 ('ALTARIA_MEGA', 'ALTARIA', 'altaria/mega', 'AltariaMega', 'ALTARIANITE', 'ALTARIANITE', 'altarianite', (75,110,110,80,110,105), ('DRAGON','FAIRY'), 'PIXILATE'),
]

# Megas that need a move instead of a Mega Stone
MOVE_MEGAS = {'RAYQUAZA_MEGA': 'DRAGON_ASCENT'}
STONE_MEGAS = [m for m in MEGAS if m[4] is not None]

def rd(p): return open(p, encoding='utf-8').read()

def read_expansion_coords():
    src = ''.join(rd(p) for p in sorted(glob.glob(os.path.join(EXP, 'src/data/pokemon/species_info/gen_*_families.h'))))
    data = {}
    for m in MEGAS:
        i = src.index('[SPECIES_%s] =' % m[0]); blk = src[i:src.index('\n    },', i)]
        def field(k):
            f = re.search(r'\.%s\s*=\s*(MON_COORDS_SIZE\(\d+,\s*\d+\)|[^,\n]+)' % k, blk)
            return f.group(1).strip() if f else None
        data[m[0]] = {k: field(k) for k in ('frontPicSize', 'frontPicYOffset', 'backPicSize', 'backPicYOffset', 'enemyMonElevation')}
    return data
def wr(p, s): open(p, 'w', encoding='utf-8').write(s)
def coords(s):
    w, h = re.match(r'MON_COORDS_SIZE\((\d+),\s*(\d+)\)', s).groups(); return int(w), int(h)

def to4bit(src, dst, stack_to_128=False):
    im = Image.open(src)
    assert im.mode == 'P'
    assert max(im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata()) < 16, src
    if stack_to_128 and im.size == (64, 64):
        two = Image.new('P', (64, 128)); two.putpalette(im.getpalette()); two.paste(im, (0, 0)); two.paste(im, (0, 64)); im = two
    pal = im.getpalette()[:48]
    out = Image.new('P', im.size); out.putpalette(pal); out.putdata(list(im.get_flattened_data() if hasattr(im, 'get_flattened_data') else im.getdata()))
    out.save(dst, bits=4)

def stillfront(anim_png, dst):
    im = Image.open(anim_png); top = im.crop((0, 0, 64, 64))
    out = Image.new('P', (64, 64)); out.putpalette(im.getpalette()[:48]); out.putdata(list(top.get_flattened_data() if hasattr(top, 'get_flattened_data') else top.getdata())); out.save(dst, bits=4)

DATA = read_expansion_coords()

# ---------- assets ----------
for m in MEGAS:
    const, base, gdir, sym, stone, sname, sfile = m[:7]
    src = os.path.join(EXP, 'graphics/pokemon', gdir); dst = os.path.join('graphics/pokemon', gdir)
    if os.path.isdir(dst): shutil.rmtree(dst)
    os.makedirs(dst)
    front = os.path.join(src, 'anim_front.png') if os.path.exists(os.path.join(src, 'anim_front.png')) else os.path.join(src, 'front.png')
    to4bit(front, os.path.join(dst, 'anim_front.png'), stack_to_128=True)
    stillfront(os.path.join(dst, 'anim_front.png'), os.path.join(dst, 'front.png'))
    to4bit(os.path.join(src, 'back.png'), os.path.join(dst, 'back.png'))
    for p in ('normal.pal', 'shiny.pal'): shutil.copy(os.path.join(src, p), os.path.join(dst, p))
    if sfile is None:
        continue
    to4bit(os.path.join(EXP, 'graphics/items/icons', sfile + '.png'), os.path.join('graphics/items/icons', sfile + '.png'))
    shutil.copy(os.path.join(EXP, 'graphics/items/icon_palettes', sfile + '.pal'), os.path.join('graphics/items/icon_palettes', sfile + '.pal'))
to4bit(os.path.join(EXP, 'graphics/items/icons/mega_ring.png'), 'graphics/items/icons/mega_ring.png')
shutil.copy(os.path.join(EXP, 'graphics/items/icon_palettes/mega_ring.pal'), 'graphics/items/icon_palettes/mega_ring.pal')

# ---------- helpers for idempotent generated blocks ----------
BEGIN, END = '// <mega-evolutions>', '// </mega-evolutions>'
def put_block(path, anchor, body, after=True):
    s = rd(path)
    s = re.sub(r'[ \t]*' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '', s, flags=re.S)
    i = s.index(anchor)
    i = i + len(anchor) if after else i
    ind = re.match(r'[ \t]*', body).group(0)
    s = s[:i] + ind + BEGIN + '\n' + body + ind + END + '\n' + s[i:]
    wr(path, s)

def drop_lines(path, pattern):
    s = rd(path); s = '\n'.join(l for l in s.split('\n') if not re.search(pattern, l)); wr(path, s)

# ---------- constants ----------
s = rd('include/constants/species.h')
s = re.sub(r'#define FORMS_MEGA_START \(SPECIES_UNOWN_QMARK \+ 1\)\n.*?#define FORMS_MEGA_END [A-Z_]+\n',
           '#define FORMS_MEGA_START (SPECIES_UNOWN_QMARK + 1)\n' +
           ''.join('#define SPECIES_%s (FORMS_MEGA_START + %d)\n' % (m[0], i) for i, m in enumerate(MEGAS)) +
           '#define FORMS_MEGA_END SPECIES_%s\n' % MEGAS[-1][0], s, flags=re.S)
wr('include/constants/species.h', s)

s = rd('include/constants/items.h')
s = re.sub(r'    // Mega Evolution\n.*?\n\n    ITEMS_COUNT', '    // Mega Evolution\n    ITEM_MEGA_RING,\n' +
           ''.join('    ITEM_%s,\n' % m[4] for m in STONE_MEGAS) + '\n    ITEMS_COUNT', s, flags=re.S)
s = re.sub(r'#define LAST_MEGA_STONE  ITEM_[A-Z_]+', '#define LAST_MEGA_STONE  ITEM_%s' % STONE_MEGAS[-1][4], s)
wr('include/constants/items.h', s)

# ---------- species data ----------
_SI = rd('src/data/pokemon/species_info.h')
def base_fields(base):
    # every field of the base species except stats, types and abilities
    blk = re.search(r'\n    \[SPECIES_%s\] =\n    \{\n(.*?)\n    \}' % base, _SI, re.S).group(1)
    keep = [l for l in blk.split('\n') if not re.match(r'\s*\.(base\w+|types|abilities)\s*=', l)]
    return '\n'.join(keep) + '\n'
info = ''
for m in MEGAS:
    const, base, gdir, sym, stone, sname, sfile, st, ty, ab = m
    info += '''
    [SPECIES_%s] =
    {
        .baseHP        = %d,
        .baseAttack    = %d,
        .baseDefense   = %d,
        .baseSpeed     = %d,
        .baseSpAttack  = %d,
        .baseSpDefense = %d,
        .types = { TYPE_%s, TYPE_%s },
        .abilities = {ABILITY_%s, ABILITY_%s},
%s    },
''' % ((const,) + st + ty + (ab, ab, base_fields(base)))
s = rd('src/data/pokemon/species_info.h')
s = re.sub(r'\n[ \t]*' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
i = s.rstrip().rfind('};')
s = s[:i].rstrip().rstrip(',') + ',\n\n    ' + BEGIN + info + '    ' + END + '\n};\n'
wr('src/data/pokemon/species_info.h', s)

put_block('src/data/text/species_names.h', '    [SPECIES_CHIMECHO] = _("CHIMECHO"),\n',
          ''.join('    [SPECIES_%s] = _("%s"),\n' % (m[0], m[1]) for m in MEGAS))

# ---------- graphics tables ----------
def frontanim_of(base):
    return re.search(r'\[SPECIES_%s\]\s*= (sAnims_\w+),' % base, rd('src/data/pokemon_graphics/front_pic_anims.h')).group(1)
def unknown_of(base):
    return re.search(r'\[SPECIES_%s\]\s*= (0x[0-9A-Fa-f]+),' % base, rd('src/data/pokemon_graphics/unknown_table.h')).group(1)
G = 'src/data/pokemon_graphics/'
put_block(G+'front_pic_table.h', '    SPECIES_SPRITE(UNOWN_QMARK, gMonFrontPic_UnownQuestionMark),\n',
          ''.join('    SPECIES_SPRITE(%s, gMonFrontPic_%s),\n' % (m[0], m[3]) for m in MEGAS))
put_block(G+'back_pic_table.h', '    SPECIES_SPRITE(UNOWN_QMARK, gMonBackPic_UnownQuestionMark),\n',
          ''.join('    SPECIES_SPRITE(%s, gMonBackPic_%s),\n' % (m[0], m[3]) for m in MEGAS))
put_block(G+'still_front_pic_table.h', '    SPECIES_SPRITE(UNOWN_QMARK,   gMonStillFrontPic_UnownQuestionMark),\n',
          ''.join('    SPECIES_SPRITE(%s, gMonStillFrontPic_%s),\n' % (m[0], m[3]) for m in MEGAS))
put_block(G+'palette_table.h', '    SPECIES_PAL(UNOWN_QMARK, gMonPalette_Unown),\n',
          ''.join('    SPECIES_PAL(%s, gMonPalette_%s),\n' % (m[0], m[3]) for m in MEGAS))
put_block(G+'shiny_palette_table.h', '    SPECIES_SHINY_PAL(UNOWN_QMARK, gMonShinyPalette_Unown),\n',
          ''.join('    SPECIES_SHINY_PAL(%s, gMonShinyPalette_%s),\n' % (m[0], m[3]) for m in MEGAS))
put_block(G+'front_pic_coordinates.h', '    [SPECIES_UNOWN_QMARK] = { .size = MON_COORDS_SIZE(24, 40), .y_offset = 13 },\n',
          ''.join('    [SPECIES_%s] = { .size = MON_COORDS_SIZE(%d, %d), .y_offset = %s },\n' % ((m[0],) + coords(DATA[m[0]]['frontPicSize']) + (DATA[m[0]]['frontPicYOffset'],)) for m in MEGAS))
put_block(G+'back_pic_coordinates.h', '    [SPECIES_UNOWN_QMARK] = { .size = MON_COORDS_SIZE(32, 56), .y_offset =  6 },\n',
          ''.join('    [SPECIES_%s] = { .size = MON_COORDS_SIZE(%d, %d), .y_offset = %s },\n' % ((m[0],) + coords(DATA[m[0]]['backPicSize']) + (DATA[m[0]]['backPicYOffset'],)) for m in MEGAS))
put_block(G+'front_pic_anims.h', '    [SPECIES_UNOWN_QMARK] = sAnims_UnownQMark,\n',
          ''.join('    [SPECIES_%s] = %s,\n' % (m[0], frontanim_of(m[1])) for m in MEGAS))
put_block(G+'unknown_table.h', '    [SPECIES_UNOWN_QMARK] = 0x888,\n',
          ''.join('    [SPECIES_%s] = %s,\n' % (m[0], unknown_of(m[1])) for m in MEGAS))
put_block(G+'enemy_mon_elevation.h', '    [SPECIES_CHIMECHO] = 12,\n',
          ''.join('    [SPECIES_%s] = %s,\n' % (m[0], DATA[m[0]]['enemyMonElevation']) for m in MEGAS if DATA[m[0]]['enemyMonElevation']))
put_block('src/pokemon_icon.c', '    [SPECIES_UNOWN_QMARK] = gMonIcon_UnownQuestionMark,\n',
          ''.join('    [SPECIES_%s] = gMonIcon_%s,\n' % (m[0], m[1].capitalize()) for m in MEGAS))
s = rd('src/pokemon_icon.c')
i = s.index('    [SPECIES_UNOWN_QMARK] = 0,\n') + len('    [SPECIES_UNOWN_QMARK] = 0,\n')
# icon palette indices: same as the base species
def iconpal_of(base):
    return re.search(r'\[SPECIES_%s\] = (\d+),' % base, s).group(1)
body = ''.join('    [SPECIES_%s] = %s,\n' % (m[0], iconpal_of(m[1])) for m in MEGAS)
s = re.sub(r'[ \t]*' + re.escape(BEGIN) + r'\n(    \[SPECIES_\w+\] = \d+,\n)+[ \t]*' + re.escape(END) + r'\n', '', s)
i = s.index('    [SPECIES_UNOWN_QMARK] = 0,\n') + len('    [SPECIES_UNOWN_QMARK] = 0,\n')
s = s[:i] + '    ' + BEGIN + '\n' + body + '    ' + END + '\n' + s[i:]
wr('src/pokemon_icon.c', s)

# ---------- graphics data ----------
gfx = ''
for m in MEGAS:
    d, sym = 'graphics/pokemon/' + m[2], m[3]
    gfx += 'const u32 gMonStillFrontPic_%s[] = INCGFX_U32("%s/front.png", ".4bpp.lz");\n' % (sym, d)
    gfx += 'const u32 gMonBackPic_%s[] = INCGFX_U32("%s/back.png", ".4bpp.lz");\n' % (sym, d)
    gfx += 'const u32 gMonPalette_%s[] = INCGFX_U32("%s/normal.pal", ".gbapal.lz");\n' % (sym, d)
    gfx += 'const u32 gMonShinyPalette_%s[] = INCGFX_U32("%s/shiny.pal", ".gbapal.lz");\n' % (sym, d)
s = rd('src/data/graphics/pokemon.h')
s = re.sub(r'\n' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
s = s.rstrip('\n') + '\n\n' + BEGIN + '\n' + gfx + END + '\n'
wr('src/data/graphics/pokemon.h', s)
s = rd('src/anim_mon_front_pics.c')
s = re.sub(r'\n' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
s = s.rstrip('\n') + '\n\n' + BEGIN + '\n' + ''.join('const u32 gMonFrontPic_%s[] = INCGFX_U32("graphics/pokemon/%s/anim_front.png", ".4bpp.lz");\n' % (m[3], m[2]) for m in MEGAS) + END + '\n'
wr('src/anim_mon_front_pics.c', s)
ext = ''.join('extern const u32 gMonFrontPic_%s[];\nextern const u32 gMonStillFrontPic_%s[];\nextern const u32 gMonBackPic_%s[];\nextern const u32 gMonPalette_%s[];\nextern const u32 gMonShinyPalette_%s[];\n' % ((m[3],) * 5) for m in MEGAS)
ext += 'extern const u32 gItemIcon_MegaRing[];\nextern const u32 gItemIconPalette_MegaRing[];\n'
ext += ''.join('extern const u32 gItemIcon_%s[];\nextern const u32 gItemIconPalette_%s[];\n' % ((m[6].title().replace('_', ''),) * 2) for m in STONE_MEGAS)
put_block('include/graphics.h', 'extern const u8 gMonFootprint_Blaziken[];\n', ext)

# ---------- items ----------
def stonesym(m): return m[6].title().replace('_', '')
icons = 'const u32 gItemIcon_MegaRing[] = INCGFX_U32("graphics/items/icons/mega_ring.png", ".4bpp.lz");\n'
icons += 'const u32 gItemIconPalette_MegaRing[] = INCGFX_U32("graphics/items/icon_palettes/mega_ring.pal", ".gbapal.lz");\n'
for m in STONE_MEGAS:
    icons += 'const u32 gItemIcon_%s[] = INCGFX_U32("graphics/items/icons/%s.png", ".4bpp.lz");\n' % (stonesym(m), m[6])
    icons += 'const u32 gItemIconPalette_%s[] = INCGFX_U32("graphics/items/icon_palettes/%s.pal", ".gbapal.lz");\n' % (stonesym(m), m[6])
s = rd('src/data/graphics/items.h')
s = re.sub(r'\n' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
s = s.rstrip('\n') + '\n\n' + BEGIN + '\n' + icons + END + '\n'
wr('src/data/graphics/items.h', s)
put_block('src/data/item_icon_table.h', '    [ITEM_OLD_SEA_MAP] = {gItemIcon_OldSeaMap, gItemIconPalette_OldSeaMap},\n',
          '    [ITEM_MEGA_RING] = {gItemIcon_MegaRing, gItemIconPalette_MegaRing},\n' +
          ''.join('    [ITEM_%s] = {gItemIcon_%s, gItemIconPalette_%s},\n' % (m[4], stonesym(m), stonesym(m)) for m in STONE_MEGAS))

def pretty(base): return base.replace('_', ' ')
desc = '''static const u8 sMegaRingDesc[] = _(
    "A ring that lets\\n"
    "POKéMON holding a\\n"
    "MEGA STONE evolve.");
'''
for m in STONE_MEGAS:
    form = ' X' if m[0].endswith('_X') else (' Y' if m[0].endswith('_Y') else '')
    third = 'Evolve in battle.' if not form else 'Evolve into form%s.' % form
    desc += '''
static const u8 s%sDesc[] = _(
    "A MEGA STONE that\\n"
    "lets %s Mega\\n"
    "%s");
''' % (stonesym(m), pretty(m[1]), third)
s = rd('src/data/text/item_descriptions.h')
s = re.sub(r'\n' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
s = s.rstrip('\n') + '\n\n' + BEGIN + '\n' + desc + END + '\n'
wr('src/data/text/item_descriptions.h', s)

items = '''
    [ITEM_MEGA_RING] =
    {
        .name = _("MEGA RING"),
        .itemId = ITEM_MEGA_RING,
        .price = 0,
        .description = sMegaRingDesc,
        .importance = 1,
        .pocket = POCKET_KEY_ITEMS,
        .type = ITEM_USE_BAG_MENU,
        .fieldUseFunc = ItemUseOutOfBattle_CannotUse,
    },
'''
for m in STONE_MEGAS:
    assert len(m[5]) <= 13, m[5]
    items += '''
    [ITEM_%s] =
    {
        .name = _("%s"),
        .itemId = ITEM_%s,
        .price = 0,
        .holdEffect = HOLD_EFFECT_MEGA_STONE,
        .description = s%sDesc,
        .pocket = POCKET_ITEMS,
        .type = ITEM_USE_BAG_MENU,
        .fieldUseFunc = ItemUseOutOfBattle_CannotUse,
    },
''' % (m[4], m[5], m[4], stonesym(m))
s = rd('src/data/items.h')
s = re.sub(r'\n    ' + re.escape(BEGIN) + r'.*?' + re.escape(END) + r'\n', '\n', s, flags=re.S)
i = s.rstrip().rfind('};')
s = s[:i].rstrip() + '\n\n    ' + BEGIN + items + '    ' + END + '\n};\n'
wr('src/data/items.h', s)

# ---------- mega table ----------
s = rd('src/data/pokemon/mega_evolutions.h')
s = re.sub(r'static const struct MegaEvolution sMegaEvolutions\[\] =\n\{\n.*?\n\};', 'static const struct MegaEvolution sMegaEvolutions[] =\n{\n' +
           ''.join('    {SPECIES_%s, ITEM_%s, SPECIES_%s, MOVE_%s},\n' % (m[1], m[4] or 'NONE', m[0], MOVE_MEGAS.get(m[0], 'NONE')) for m in MEGAS).rstrip('\n') + '\n};', s, flags=re.S)
wr('src/data/pokemon/mega_evolutions.h', s)
print('ok', len(MEGAS))
