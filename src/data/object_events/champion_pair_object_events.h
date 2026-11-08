// CYNTHIA, for scenes where she's on a map with LEON (Mega Island's route, after MEWTWO): the champions'
// sprites each load their own palette in the special NPC slot, so only one fits on a map. This copy of
// hers uses the special NPC's reflection slot instead, as the Primal GROUDON does; it has no
// reflection, so keep it off water. Included by event_object_movement.c after the sanctum MEWTWO.

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ChampionCynthia2 = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_CHAMPION_CYNTHIA,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 512,
    .width = 32,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL_REFLECTION,
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

// Emerald's WALLY on a Kanto map: his palette (NPC 1, which holds an FRLG palette on Kanto maps) loaded
// in the special NPC's reflection slot, as the Team Magma grunts in the POKéMON MANSION do. With ALAIN
// on Mega Island's castle grounds; it has no reflection, so keep it off water.
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KantoWally = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_NPC_1,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 256,
    .width = 16,
    .height = 32,
    .paletteSlot = PALSLOT_NPC_SPECIAL_REFLECTION,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_16x32,
    .subspriteTables = sOamTables_16x32,
    .anims = sAnimTable_Standard,
    .images = sPicTable_Wally,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sChampionPairObjectEventGraphicsInfoPointers[NUM_CHAMPION_PAIR_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_CHAMPION_CYNTHIA_2 - OBJ_EVENT_GFX_CHAMPION_PAIR_START] = &gObjectEventGraphicsInfo_ChampionCynthia2,
    [OBJ_EVENT_GFX_KANTO_WALLY - OBJ_EVENT_GFX_CHAMPION_PAIR_START] = &gObjectEventGraphicsInfo_KantoWally,
};
