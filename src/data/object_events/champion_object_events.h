// The champions of other regions, for the Mega Evolution tournament. Their sprites are made by
// tools/mega_island/champions/overworld.py and use the Kanto NPC palettes. Included by
// event_object_movement.c after the Kanto object events.

const u32 gObjectEventPic_ChampionCynthia[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_cynthia.png", ".4bpp", "-mwidth 2 -mheight 4");
const u32 gObjectEventPic_ChampionAlder[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_alder.png", ".4bpp", "-mwidth 2 -mheight 4");
const u32 gObjectEventPic_ChampionDiantha[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_diantha.png", ".4bpp", "-mwidth 2 -mheight 4");
const u32 gObjectEventPic_ChampionLeon[] = INCGFX_U32("graphics/object_events/pics/kanto/people/champion_leon.png", ".4bpp", "-mwidth 2 -mheight 4");

static const struct SpriteFrameImage sPicTable_ChampionCynthia[] = {
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionCynthia, 2, 4, 2),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionCynthia = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_KANTO_NPC_WHITE,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_4,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = FALSE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionCynthia,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionAlder[] = {
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionAlder, 2, 4, 2),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionAlder = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_KANTO_NPC_WHITE,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_4,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = FALSE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionAlder,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionDiantha[] = {
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionDiantha, 2, 4, 2),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionDiantha = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_KANTO_NPC_PINK,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_2,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = FALSE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_ChampionDiantha,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpriteFrameImage sPicTable_ChampionLeon[] = {
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 0),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 1),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 2),
    overworld_frame(gObjectEventPic_ChampionLeon, 2, 4, 2),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionLeon = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_KANTO_NPC_BLUE,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_1,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = FALSE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
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
