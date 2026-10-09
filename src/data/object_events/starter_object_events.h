// BULBASAUR, CHARMANDER and SQUIRTLE as overworld objects, for the side quests that give the player the
// two Kanto starters they didn't choose. They use the POKéMON's PC box icon (two 32x32 frames, facing
// left) and the icon's palette, in the special NPC palette slot. Included by event_object_movement.c
// after the champions' object events.

#define OBJ_EVENT_PAL_TAG_STARTER_ICON_0  0x1160
#define OBJ_EVENT_PAL_TAG_STARTER_ICON_1  0x1161

// Entries for sObjectEventSpritePalettes
#define STARTER_OBJECT_EVENT_SPRITE_PALETTES \
    {gMonIconPalettes[0], OBJ_EVENT_PAL_TAG_STARTER_ICON_0}, \
    {gMonIconPalettes[1], OBJ_EVENT_PAL_TAG_STARTER_ICON_1},

// Standing frames are the icon's first frame, walking frames alternate between its two frames.
#define STARTER_PIC_TABLE(icon)                  \
{                                                \
    overworld_frame(icon, 4, 4, 0),              \
    overworld_frame(icon, 4, 4, 0),              \
    overworld_frame(icon, 4, 4, 0),              \
    overworld_frame(icon, 4, 4, 1),              \
    overworld_frame(icon, 4, 4, 0),              \
    overworld_frame(icon, 4, 4, 1),              \
    overworld_frame(icon, 4, 4, 0),              \
    overworld_frame(icon, 4, 4, 1),              \
    overworld_frame(icon, 4, 4, 0),              \
}

static const struct SpriteFrameImage sPicTable_StarterBulbasaur[] = STARTER_PIC_TABLE(gMonIcon_Bulbasaur);
static const struct SpriteFrameImage sPicTable_StarterCharmander[] = STARTER_PIC_TABLE(gMonIcon_Charmander);
static const struct SpriteFrameImage sPicTable_StarterSquirtle[] = STARTER_PIC_TABLE(gMonIcon_Squirtle);

#define STARTER_GRAPHICS_INFO(pal, pics)         \
{                                                \
    .tileTag = TAG_NONE,                         \
    .paletteTag = pal,                           \
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE, \
    .size = 512,                                 \
    .width = 32,                                 \
    .height = 32,                                \
    .paletteSlot = PALSLOT_NPC_SPECIAL,          \
    .shadowSize = SHADOW_SIZE_S,                 \
    .inanimate = FALSE,                          \
    .disableReflectionPaletteLoad = TRUE,        \
    .tracks = TRACKS_FOOT,                       \
    .oam = &gObjectEventBaseOam_32x32,           \
    .subspriteTables = sOamTables_32x32,         \
    .anims = sAnimTable_Standard,                \
    .images = pics,                              \
    .affineAnims = gDummySpriteAffineAnimTable,  \
}

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterBulbasaur = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_1, sPicTable_StarterBulbasaur);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterCharmander = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_0, sPicTable_StarterCharmander);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterSquirtle = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_0, sPicTable_StarterSquirtle);

static const struct ObjectEventGraphicsInfo *const sStarterObjectEventGraphicsInfoPointers[NUM_STARTER_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_STARTER_BULBASAUR - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterBulbasaur,
    [OBJ_EVENT_GFX_STARTER_CHARMANDER - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterCharmander,
    [OBJ_EVENT_GFX_STARTER_SQUIRTLE - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterSquirtle,
};
