// The Hoenn chapter's Sootopolis (docs/hoenn_plan.md, Act 3): PRIMAL KYOGRE and PRIMAL GROUDON clash, and
// MEGA RAYQUAZA calms them. Emerald's pics with the palettes of tools/hoenn/legend_palettes.py. KYOGRE
// and GROUDON are on screen together, so each has a slot of its own: the special NPC's and its reflection's
// (Sootopolis has no special NPC, and these have no reflection). RAYQUAZA only comes once they're gone,
// and takes the first again.
// Included by event_object_movement.c after the lab trainer object events.

#define OBJ_EVENT_PAL_TAG_KYOGRE_PRIMAL  0x1156
#define OBJ_EVENT_PAL_TAG_GROUDON_PRIMAL 0x1157
#define OBJ_EVENT_PAL_TAG_RAYQUAZA_MEGA  0x1158

// Entries for sObjectEventSpritePalettes
#define HOENN_LEGEND_OBJECT_EVENT_SPRITE_PALETTES \
    {gObjectEventPal_KyogrePrimal, OBJ_EVENT_PAL_TAG_KYOGRE_PRIMAL}, \
    {gObjectEventPal_GroudonPrimal, OBJ_EVENT_PAL_TAG_GROUDON_PRIMAL}, \
    {gObjectEventPal_RayquazaMega, OBJ_EVENT_PAL_TAG_RAYQUAZA_MEGA},

const u16 gObjectEventPal_KyogrePrimal[] = INCGFX_U16("graphics/object_events/palettes/hoenn/kyogre_primal.pal", ".gbapal");
const u16 gObjectEventPal_GroudonPrimal[] = INCGFX_U16("graphics/object_events/palettes/hoenn/groudon_primal.pal", ".gbapal");
const u16 gObjectEventPal_RayquazaMega[] = INCGFX_U16("graphics/object_events/palettes/hoenn/rayquaza_mega.pal", ".gbapal");

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_KyogrePrimal = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_KYOGRE_PRIMAL,
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
    .images = sPicTable_KyogreSide,
    .affineAnims = sAffineAnimTable_KyogreGroudon,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_GroudonPrimal = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_GROUDON_PRIMAL,
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
    .anims = sAnimTable_GroudonSide,
    .images = sPicTable_GroudonSide,
    .affineAnims = sAffineAnimTable_KyogreGroudon,
};

const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_RayquazaMega = {
    .tileTag = TAG_NONE,
    .paletteTag = OBJ_EVENT_PAL_TAG_RAYQUAZA_MEGA,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 2048,
    .width = 64,
    .height = 64,
    .paletteSlot = PALSLOT_NPC_SPECIAL,
    .shadowSize = SHADOW_SIZE_M,
    .inanimate = FALSE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_FOOT,
    .oam = &gObjectEventBaseOam_64x64,
    .subspriteTables = sOamTables_64x64,
    .anims = sAnimTable_Rayquaza,
    .images = sPicTable_Rayquaza,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct ObjectEventGraphicsInfo *const sHoennLegendObjectEventGraphicsInfoPointers[NUM_HOENN_LEGEND_OBJ_EVENT_GFX] = {
    [OBJ_EVENT_GFX_KYOGRE_PRIMAL - OBJ_EVENT_GFX_HOENN_LEGEND_START] = &gObjectEventGraphicsInfo_KyogrePrimal,
    [OBJ_EVENT_GFX_GROUDON_PRIMAL - OBJ_EVENT_GFX_HOENN_LEGEND_START] = &gObjectEventGraphicsInfo_GroudonPrimal,
    [OBJ_EVENT_GFX_RAYQUAZA_MEGA - OBJ_EVENT_GFX_HOENN_LEGEND_START] = &gObjectEventGraphicsInfo_RayquazaMega,
};
