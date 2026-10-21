// BULBASAUR, CHARMANDER and SQUIRTLE as overworld objects, for the side quests that give the player the
// two Kanto starters they didn't choose. They use the POKéMON's PC box icon (two 32x32 frames, facing
// left) and the icon's palette, in the special NPC palette slot. Included by event_object_movement.c
// after the champions' object events.

#define OBJ_EVENT_PAL_TAG_STARTER_ICON_0  0x1160
#define OBJ_EVENT_PAL_TAG_STARTER_ICON_1  0x1161
#define OBJ_EVENT_PAL_TAG_STARTER_ICON_2  0x1162

// Entries for sObjectEventSpritePalettes
#define STARTER_OBJECT_EVENT_SPRITE_PALETTES \
    {gMonIconPalettes[0], OBJ_EVENT_PAL_TAG_STARTER_ICON_0}, \
    {gMonIconPalettes[1], OBJ_EVENT_PAL_TAG_STARTER_ICON_1}, \
    {gMonIconPalettes[2], OBJ_EVENT_PAL_TAG_STARTER_ICON_2},

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

#define STARTER_GRAPHICS_INFO_IN_SLOT(pal, pics, slot) \
{                                                \
    .tileTag = TAG_NONE,                         \
    .paletteTag = pal,                           \
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE, \
    .size = 512,                                 \
    .width = 32,                                 \
    .height = 32,                                \
    .paletteSlot = slot,                         \
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

#define STARTER_GRAPHICS_INFO(pal, pics) STARTER_GRAPHICS_INFO_IN_SLOT(pal, pics, PALSLOT_NPC_SPECIAL)

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterBulbasaur = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_1, sPicTable_StarterBulbasaur);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterCharmander = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_0, sPicTable_StarterCharmander);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterSquirtle = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_0, sPicTable_StarterSquirtle);

static const struct ObjectEventGraphicsInfo *const sStarterObjectEventGraphicsInfoPointers[NUM_STARTER_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_STARTER_BULBASAUR - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterBulbasaur,
    [OBJ_EVENT_GFX_STARTER_CHARMANDER - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterCharmander,
    [OBJ_EVENT_GFX_STARTER_SQUIRTLE - OBJ_EVENT_GFX_STARTER_START] = &gObjectEventGraphicsInfo_StarterSquirtle,
};

// The Johto starters' side quests in Hoenn (docs/johto_starters_plan.md). The ZUBAT that corner CYNDAQUIL
// in GRANITE CAVE are on screen with it, so they take the special NPC's reflection slot (the cave has no
// reflections), as the Primal legends do in Sootopolis.
static const struct SpriteFrameImage sPicTable_StarterChikorita[] = STARTER_PIC_TABLE(gMonIcon_Chikorita);
static const struct SpriteFrameImage sPicTable_StarterCyndaquil[] = STARTER_PIC_TABLE(gMonIcon_Cyndaquil);
static const struct SpriteFrameImage sPicTable_StarterTotodile[] = STARTER_PIC_TABLE(gMonIcon_Totodile);
static const struct SpriteFrameImage sPicTable_IconZubat[] = STARTER_PIC_TABLE(gMonIcon_Zubat);
static const struct SpriteFrameImage sPicTable_IconCelebi[] = STARTER_PIC_TABLE(gMonIcon_Celebi); // BERRY FOREST (docs/catch_them_all.md)

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterChikorita = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_1, sPicTable_StarterChikorita);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterCyndaquil = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_1, sPicTable_StarterCyndaquil);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_StarterTotodile = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_2, sPicTable_StarterTotodile);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_IconZubat = STARTER_GRAPHICS_INFO_IN_SLOT(OBJ_EVENT_PAL_TAG_STARTER_ICON_2, sPicTable_IconZubat, PALSLOT_NPC_SPECIAL_REFLECTION);
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_IconCelebi = STARTER_GRAPHICS_INFO(OBJ_EVENT_PAL_TAG_STARTER_ICON_1, sPicTable_IconCelebi);

static const struct ObjectEventGraphicsInfo *const sJohtoStarterObjectEventGraphicsInfoPointers[NUM_JOHTO_STARTER_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_STARTER_CHIKORITA - OBJ_EVENT_GFX_JOHTO_STARTER_START] = &gObjectEventGraphicsInfo_StarterChikorita,
    [OBJ_EVENT_GFX_STARTER_CYNDAQUIL - OBJ_EVENT_GFX_JOHTO_STARTER_START] = &gObjectEventGraphicsInfo_StarterCyndaquil,
    [OBJ_EVENT_GFX_STARTER_TOTODILE - OBJ_EVENT_GFX_JOHTO_STARTER_START] = &gObjectEventGraphicsInfo_StarterTotodile,
    [OBJ_EVENT_GFX_ICON_ZUBAT - OBJ_EVENT_GFX_JOHTO_STARTER_START] = &gObjectEventGraphicsInfo_IconZubat,
    [OBJ_EVENT_GFX_ICON_CELEBI - OBJ_EVENT_GFX_JOHTO_STARTER_START] = &gObjectEventGraphicsInfo_IconCelebi,
};
