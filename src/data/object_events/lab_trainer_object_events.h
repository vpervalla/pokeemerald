// RED, LEAF, ETHAN and KRIS as NPCs: the trainers who took the Kanto starters from OAK's lab. They use
// the players' walking sprites, in the special NPC palette slot. OBJ_EVENT_GFX_RED_OR_ETHAN shows ETHAN
// when the player is RED, and OBJ_EVENT_GFX_LEAF_OR_KRIS shows KRIS when the player is LEAF.
// Included by event_object_movement.c after the starter object events.

#define LAB_TRAINER_NPC(name, palTag, pics)                 \
const struct ObjectEventGraphicsInfo name = {               \
    .tileTag = TAG_NONE,                                    \
    .paletteTag = palTag,                                   \
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,         \
    .size = 512,                                            \
    .width = 16,                                            \
    .height = 32,                                           \
    .paletteSlot = PALSLOT_NPC_SPECIAL,                     \
    .shadowSize = SHADOW_SIZE_M,                            \
    .inanimate = FALSE,                                     \
    .disableReflectionPaletteLoad = TRUE,                   \
    .tracks = TRACKS_FOOT,                                  \
    .oam = &gObjectEventBaseOam_16x32,                      \
    .subspriteTables = sOamTables_16x32,                    \
    .anims = sAnimTable_BrendanMayNormal,                   \
    .images = pics,                                         \
    .affineAnims = gDummySpriteAffineAnimTable,             \
}

LAB_TRAINER_NPC(gObjectEventGraphicsInfo_RedNpc, OBJ_EVENT_PAL_TAG_PLAYER_RED_LEAF, sPicTable_RedPlayerNormal);
LAB_TRAINER_NPC(gObjectEventGraphicsInfo_LeafNpc, OBJ_EVENT_PAL_TAG_PLAYER_RED_LEAF, sPicTable_LeafPlayerNormal);
LAB_TRAINER_NPC(gObjectEventGraphicsInfo_EthanNpc, OBJ_EVENT_PAL_TAG_PLAYER_ETHAN, sPicTable_EthanPlayerNormal);
LAB_TRAINER_NPC(gObjectEventGraphicsInfo_KrisNpc, OBJ_EVENT_PAL_TAG_PLAYER_KRIS, sPicTable_KrisPlayerNormal);

static const struct ObjectEventGraphicsInfo *GetLabTrainerGraphicsInfo(u16 graphicsId)
{
    bool8 playerIsFRLG = (gSaveBlock2Ptr->playerCostume == PLAYER_COSTUME_FRLG);

    if (graphicsId == OBJ_EVENT_GFX_RED_OR_ETHAN)
        return (playerIsFRLG && gSaveBlock2Ptr->playerGender == MALE) ? &gObjectEventGraphicsInfo_EthanNpc : &gObjectEventGraphicsInfo_RedNpc;
    else
        return (playerIsFRLG && gSaveBlock2Ptr->playerGender == FEMALE) ? &gObjectEventGraphicsInfo_KrisNpc : &gObjectEventGraphicsInfo_LeafNpc;
}
