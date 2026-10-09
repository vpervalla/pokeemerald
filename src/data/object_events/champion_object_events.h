// The champions of other regions, for the Mega Evolution tournament. Their sprites and palettes are
// made by tools/mega_island/champions/sprites.py. Each has its own palette and uses the special NPC
// palette slot, so only one of them can be on a map at a time. Included by event_object_movement.c
// after the Kanto object events.

#define OBJ_EVENT_PAL_TAG_CHAMPION_CYNTHIA  0x1150
#define OBJ_EVENT_PAL_TAG_CHAMPION_ALDER    0x1151
#define OBJ_EVENT_PAL_TAG_CHAMPION_DIANTHA  0x1152
#define OBJ_EVENT_PAL_TAG_CHAMPION_LEON     0x1153

// Entries for sObjectEventSpritePalettes
#define CHAMPION_OBJECT_EVENT_SPRITE_PALETTES \
    {gObjectEventPal_ChampionCynthia, OBJ_EVENT_PAL_TAG_CHAMPION_CYNTHIA}, \
    {gObjectEventPal_ChampionAlder, OBJ_EVENT_PAL_TAG_CHAMPION_ALDER}, \
    {gObjectEventPal_ChampionDiantha, OBJ_EVENT_PAL_TAG_CHAMPION_DIANTHA}, \
    {gObjectEventPal_ChampionLeon, OBJ_EVENT_PAL_TAG_CHAMPION_LEON},

const u16 gObjectEventPal_ChampionCynthia[] = INCGFX_U16("graphics/object_events/palettes/kanto/champion_cynthia.pal", ".gbapal");
const u32 gObjectEventPic_ChampionCynthia[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_cynthia.png", ".4bpp", "-mwidth 4 -mheight 4");
const u16 gObjectEventPal_ChampionAlder[] = INCGFX_U16("graphics/object_events/palettes/kanto/champion_alder.pal", ".gbapal");
const u32 gObjectEventPic_ChampionAlder[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_alder.png", ".4bpp", "-mwidth 2 -mheight 4");
const u16 gObjectEventPal_ChampionDiantha[] = INCGFX_U16("graphics/object_events/palettes/kanto/champion_diantha.pal", ".gbapal");
const u32 gObjectEventPic_ChampionDiantha[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_diantha.png", ".4bpp", "-mwidth 4 -mheight 4");
const u16 gObjectEventPal_ChampionLeon[] = INCGFX_U16("graphics/object_events/palettes/kanto/champion_leon.pal", ".gbapal");
const u32 gObjectEventPic_ChampionLeon[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_leon.png", ".4bpp", "-mwidth 4 -mheight 4");

static const struct SpriteFrameImage sPicTable_ChampionCynthia[] = {
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 0),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 1),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 2),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 3),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 4),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 5),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 6),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 7),
    overworld_frame(gObjectEventPic_ChampionCynthia, 4, 4, 8),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionCynthia = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_CHAMPION_CYNTHIA,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 512,
    .width = 32,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_32x32,
    .subspriteTables = sOamTables_32x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionCynthia,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionAlder[] = {
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 3),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 4),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 5),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 6),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 7),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 8),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionAlder = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_CHAMPION_ALDER,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionAlder,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionDiantha[] = {
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 0),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 1),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 2),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 3),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 4),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 5),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 6),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 7),
    overworld_frame(gObjectEventPic_ChampionDiantha, 4, 4, 8),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionDiantha = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_CHAMPION_DIANTHA,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 512,
    .width = 32,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_32x32,
    .subspriteTables = sOamTables_32x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionDiantha,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionLeon[] = {
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 0),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 1),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 2),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 3),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 4),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 5),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 6),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 7),
    overworld_frame(gObjectEventPic_ChampionLeon, 4, 4, 8),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionLeon = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_CHAMPION_LEON,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 512,
    .width = 32,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_32x32,
    .subspriteTables = sOamTables_32x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionLeon,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sChampionObjectEventGraphicsInfoPointers[NUM_CHAMPION_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_CHAMPION_CYNTHIA - OBJ_EVENT_GFX_CHAMPION_START] = &gObjectEventGraphicsInfo_ChampionCynthia,
    [OBJ_EVENT_GFX_CHAMPION_ALDER - OBJ_EVENT_GFX_CHAMPION_START] = &gObjectEventGraphicsInfo_ChampionAlder,
    [OBJ_EVENT_GFX_CHAMPION_DIANTHA - OBJ_EVENT_GFX_CHAMPION_START] = &gObjectEventGraphicsInfo_ChampionDiantha,
    [OBJ_EVENT_GFX_CHAMPION_LEON - OBJ_EVENT_GFX_CHAMPION_START] = &gObjectEventGraphicsInfo_ChampionLeon,
};
