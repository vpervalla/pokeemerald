// The lost BULBASAUR of Viridian Forest and the BEEDRILL that chased it. Their sprites are pokeemerald-expansion's
// follower sprites, converted by tools/viridian_bulbasaur/sprites.py to one shared palette, so both fit in the
// special NPC palette slot together. Included by event_object_movement.c after the champion object events.

#define OBJ_EVENT_PAL_TAG_VIRIDIAN_BULBASAUR 0x1155

// Entries for sObjectEventSpritePalettes
#define STORY_MON_OBJECT_EVENT_SPRITE_PALETTES \
    {gObjectEventPal_ViridianBulbasaur, OBJ_EVENT_PAL_TAG_VIRIDIAN_BULBASAUR},

const u16 gObjectEventPal_ViridianBulbasaur[] = INCGFX_U16("graphics/object_events/palettes/kanto/viridian_bulbasaur.pal", ".gbapal");
const u32 gObjectEventPic_Bulbasaur[] = INCGFX_U32("graphics/object_events/pics/kanto/pokemon/bulbasaur.png", ".4bpp", "-mwidth 4 -mheight 4");
const u32 gObjectEventPic_Beedrill[] = INCGFX_U32("graphics/object_events/pics/kanto/pokemon/beedrill.png", ".4bpp", "-mwidth 4 -mheight 4");

// The source frames are: down, down step, up, up step, left, left step
static const struct SpriteFrameImage sPicTable_Bulbasaur[] = {
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 0),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 2),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 4),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 1),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 1),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 3),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 3),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 5),
    overworld_frame(gObjectEventPic_Bulbasaur, 4, 4, 5),
};

static const struct SpriteFrameImage sPicTable_Beedrill[] = {
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 0),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 2),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 4),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 1),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 1),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 3),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 3),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 5),
    overworld_frame(gObjectEventPic_Beedrill, 4, 4, 5),
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_Bulbasaur = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_VIRIDIAN_BULBASAUR,
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
    .images = sPicTable_Bulbasaur,
    .affineAnims = gDummySpriteAffineAnimTable,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_Beedrill = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_VIRIDIAN_BULBASAUR,
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
    .images = sPicTable_Beedrill,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sStoryMonObjectEventGraphicsInfoPointers[NUM_STORY_MON_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_BULBASAUR - OBJ_EVENT_GFX_STORY_MON_START] = &gObjectEventGraphicsInfo_Bulbasaur,
    [OBJ_EVENT_GFX_BEEDRILL - OBJ_EVENT_GFX_STORY_MON_START] = &gObjectEventGraphicsInfo_Beedrill,
};
