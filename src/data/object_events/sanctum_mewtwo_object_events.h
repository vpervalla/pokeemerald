// MEWTWO waiting in its sanctum on Mega Island: its idle frames from a Mystery Dungeon style sheet,
// 32x32, with its own palette in the special NPC slot (the sanctum has no other special NPC). Made by
// tools/mega_island/sanctum_mewtwo.py. Included by event_object_movement.c after the mansion raiders.
// (The Kanto MEWTWO, OBJ_EVENT_GFX_KANTO_MEWTWO, is FRLG's small front view.)

#define OBJ_EVENT_PAL_TAG_SANCTUM_MEWTWO 0x1159

// Entries for sObjectEventSpritePalettes
#define SANCTUM_MEWTWO_OBJECT_EVENT_SPRITE_PALETTES \
    {gObjectEventPal_SanctumMewtwo, OBJ_EVENT_PAL_TAG_SANCTUM_MEWTWO},

const u16 gObjectEventPal_SanctumMewtwo[] = INCGFX_U16("graphics/object_events/palettes/kanto/sanctum_mewtwo.pal", ".gbapal");
const u32 gObjectEventPic_SanctumMewtwo[] = INCGFX_U32("graphics/object_events/pics/kanto/pokemon/sanctum_mewtwo.png", ".4bpp", "-mwidth 4 -mheight 4");

// The source frames are: down, up, left (it stands still, so they serve for walking too)
static const struct SpriteFrameImage sPicTable_SanctumMewtwo[] = {
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 0),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 1),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 2),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 0),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 0),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 1),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 1),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 2),
    overworld_frame(gObjectEventPic_SanctumMewtwo, 4, 4, 2),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_SanctumMewtwo = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_SANCTUM_MEWTWO,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 512,
    .width = 32,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_NONE,
    .oam = &gObjectEventBaseOam_32x32,
    .subspriteTables = sOamTables_32x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_SanctumMewtwo,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sSanctumMewtwoObjectEventGraphicsInfoPointers[NUM_SANCTUM_MEWTWO_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_SANCTUM_MEWTWO - OBJ_EVENT_GFX_SANCTUM_MEWTWO_START] = &gObjectEventGraphicsInfo_SanctumMewtwo,
};
