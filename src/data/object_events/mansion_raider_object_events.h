// Team Aqua and Team Magma in the POKéMON MANSION (docs/hoenn_plan.md, Act 0). Emerald's grunts use the
// NPC 2 and NPC 4 slots, which hold FRLG's palettes on Kanto maps, so there they come out in the wrong
// colors. These copies load Emerald's palettes in the special NPC slot (Aqua) and its reflection's (Magma),
// as the Primal KYOGRE and GROUDON do: the mansion has no other special NPC and no water to reflect in.
// Included by event_object_movement.c after the Hoenn legend object events.

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KantoAquaMemberM = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_NPC_4,
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
    .images = sPicTable_AquaMemberM,
    .affineAnims = gDummySpriteAffineAnimTable,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KantoAquaMemberF = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_NPC_4,
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
    .images = sPicTable_AquaMemberF,
    .affineAnims = gDummySpriteAffineAnimTable,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KantoMagmaMemberM = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_NPC_2,
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
    .images = sPicTable_MagmaMemberM,
    .affineAnims = gDummySpriteAffineAnimTable,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KantoMagmaMemberF = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_NPC_2,
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
    .images = sPicTable_MagmaMemberF,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sMansionRaiderObjectEventGraphicsInfoPointers[NUM_MANSION_RAIDER_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_KANTO_AQUA_MEMBER_M - OBJ_EVENT_GFX_MANSION_RAIDER_START] = &gObjectEventGraphicsInfo_KantoAquaMemberM,
    [OBJ_EVENT_GFX_KANTO_AQUA_MEMBER_F - OBJ_EVENT_GFX_MANSION_RAIDER_START] = &gObjectEventGraphicsInfo_KantoAquaMemberF,
    [OBJ_EVENT_GFX_KANTO_MAGMA_MEMBER_M - OBJ_EVENT_GFX_MANSION_RAIDER_START] = &gObjectEventGraphicsInfo_KantoMagmaMemberM,
    [OBJ_EVENT_GFX_KANTO_MAGMA_MEMBER_F - OBJ_EVENT_GFX_MANSION_RAIDER_START] = &gObjectEventGraphicsInfo_KantoMagmaMemberF,
};
