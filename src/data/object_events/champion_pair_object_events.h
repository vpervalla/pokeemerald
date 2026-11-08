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

static const struct ObjectEventGraphicsInfo *const sChampionPairObjectEventGraphicsInfoPointers[NUM_CHAMPION_PAIR_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_CHAMPION_CYNTHIA_2 - OBJ_EVENT_GFX_CHAMPION_PAIR_START] = &gObjectEventGraphicsInfo_ChampionCynthia2,
};
