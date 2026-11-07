# Builds the castle's interiors after the Pokémon Mansion, from pre-existing Kanto art only: the
# entrance hall with the registration desk (1F), the guest floor (2F), the guest room and the arena
# on the top floor. Nothing is drawn: every cell is a metatile of an existing tileset, or a
# metatile whose layers come from two of them (a piece of furniture on the mansion's floor), and
# its tiles and palettes are copied into the new secondary tilesets. The hall, the guest floor and
# the room share kanto_mega_hall; the arena has kanto_mega_arena. PREVIEW=out.png renders the four
# maps after writing them.
#
# Where the pieces come from (all on gTileset_KantoBuilding, so primary tiles and palettes are
# shared as they are):
#   kanto_pokemon_mansion     walls, windows, floor and its shadows, the pink carpet, stairs,
#                             pillars, the Mewtwo statues, plants, tables, chairs, the red mat
#   kanto_restaurant_hotel    the reception counter (Celadon Hotel)
#   kanto_ss_anne             the bed (the S.S. Anne's cabins)
#   kanto_pokemon_league      the battlefield (Agatha's room)
#   kanto_building (primary)  the black edge and the door mats
#
# The arena's tileset also holds the hidden door and the hidden room behind it, which
# build_hidden_room.py appends: run it after this.
import sys, os
sys.path.insert(0,os.path.dirname(os.path.abspath(__file__)))
from interiors_lib import *

# ---- 1F: the entrance hall ----
hall=Room(23,16)
shell(hall,windows=(8,13))
up1=stairs_up(hall,2)                         # up to the guest floor
for x in (19,20): wall_plant(hall,x)
for x in range(8,15):                         # the registration desk: the hotel's counter
    hall.over(x,5,HOTEL,0x2a2 if x==8 else 0x2a4 if x==14 else 0x2a3)   # the receptionist stands on it
    hall.over(x,6,HOTEL,0x2aa if x==8 else 0x2ac if x==14 else 0x2ab,1)
    hall.beh[6][x]=MB_COUNTER
statue(hall,6,3); statue(hall,16,3)
carpet(hall,10,7,14)                          # its end is the red mat
door(hall,10,3)
for x in (7,14): pillar(hall,x,8)
table(hall,3,8,book=True); table(hall,17,8)
plant(hall,1,13); plant(hall,21,13)

# ---- 2F: the guest floor ----
GF=Room(26,9)
shell(GF,windows=(6,12,18))
down2=stairs_down(GF,1)                       # down to the hall
up2=stairs_up(GF,22)                          # up to the arena
DOORS=(5,8,11,14,17,20)
for x in DOORS: door(GF,x,1)
for x in (9,15): wall_plant(GF,x)
statue(GF,10,3); statue(GF,16,3)

# ---- a guest room (one layout for all six) ----
GR=Room(11,9)
shell(GR,windows=(6,))
for (x,y,m) in ((1,3,0x328),(2,3,0x329),(3,3,0x32a),(1,4,0x330),(2,4,0x331),(3,4,0x332)):
    GR.over(x,y,SSANNE,m,1 if x==2 else 0)    # the bed
table(GR,6,3,chairs=(True,False))
wall_plant(GR,9)
plant(GR,9,6)
exit_mat(GR,4)

# ---- 3F: the arena ----
A=Room(23,22)
shell(A,windows=(3,7,14,18))
A.rows(10,3,((0x348,0x349,0x34a),))           # MR. M.'s red mat
statue(A,8,3,red=True); statue(A,14,3,red=True)
# The battlefield: Agatha's (kanto_pokemon_league), lengthened by repeating its plain rows.
FL=0x2f0
LINE_TOP,SIDES,BOTTOM=(0x2fb,0x2fc,0x2fc,0x2fc,0x2fd),(0x2fe,FL,FL,FL,0x2ff),(0x2ce,0x2cf,0x2cf,0x2cf,0x2a1)
EMBLEM=((0x2fe,0x300,0x301,0x302,0x2ff),(0x306,0x303,0x304,0x305,0x307),(0x30e,0x30b,0x30c,0x30d,0x30f),(0x2fe,0x308,0x309,0x30a,0x2ff))
field=[LINE_TOP]+[SIDES]*3+list(EMBLEM)+[SIDES]*3+[BOTTOM]
field=[(FL,)*5]+field+[(FL,)*5]
for j,r in enumerate(field):
    for i,m in enumerate((FL,)+r+(FL,)):
        A.put(8+i,5+j,mt(LEAGUE,m))
for y in range(8,17,2):                       # the spectators' chairs
    for x in (3,5): A.put(x,y,0x2f0)
    for x in (17,19): A.put(x,y,0x2f1)
for x in (2,20): pillar(A,x,3)
plant(A,1,19); plant(A,21,19)
exit_mat(A,10)

MAPS=((hall,'MegaIsland_Castle_1F'),(GF,'MegaIsland_Castle_2F'),(GR,'MegaIsland_Castle_GuestRoom'),(A,'MegaIsland_Arena'))
for maps,ts in ((MAPS[:3],'kanto_mega_hall'),(MAPS[3:],'kanto_mega_arena')):
    T=Tileset()
    grids=[[[T.metatile(R.cell[y][x],R.beh[y][x]) for x in range(R.w)] for y in range(R.h)] for R,_ in maps]
    T.write(REPO+'data/tilesets/secondary/%s/'%ts)
    for (R,name),g in zip(maps,grids):
        tilesetgen.write_layout(REPO+'data/layouts/%s/'%name,g,R.coll,(BLACK|0xc00|0x3000,)*4)
print('written')

if os.environ.get('PREVIEW'):
    from PIL import Image
    ims=[]
    for _,name in MAPS:
        render.render_layout(name,os.environ['PREVIEW']); ims.append(Image.open(os.environ['PREVIEW']).copy())
    out=Image.new('RGB',(sum(i.width for i in ims)+16*len(ims),max(i.height for i in ims)))
    x=0
    for i in ims: out.paste(i,(x,0)); x+=i.width+16
    out.save(os.environ['PREVIEW'])
