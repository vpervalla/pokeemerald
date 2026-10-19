// The Hoenn chapter's theft (docs/hoenn_plan.md, section 3b): on the first crossing to Hoenn, Team Aqua and
// Team Magma take the player's party, BAG and money, and cut the link with Kanto. Until ARCHIE and MAXIE
// give everything back after Hoenn's Hall of Fame, the PC shows only boxes 8-14 (HOENN 1-7); Kanto's
// POKéMON wait in boxes 1-7. What was taken is kept in gPokemonStoragePtr->hoennStolenTeam.
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "save.h"
#include "string_util.h"
#include "constants/items.h"

STATIC_ASSERT(sizeof(struct PokemonStorage) <= NUM_PKMN_STORAGE_SECTORS * SECTOR_DATA_SIZE, PokemonStorageFitsItsSectors);
STATIC_ASSERT(NUM_HOENN_BOXES * IN_BOX_COUNT <= HOENN_FIRST_BOX * IN_BOX_COUNT, KantoBoxesHoldTheHoennOnes);

#define HOENN_STARTING_MONEY 3000

static const u8 sText_HoennBox[] = _("HOENN ");

bool8 IsKantoLinkCut(void)
{
    return FlagGet(FLAG_HOENN_TEAM_STOLEN);
}

static struct HoennStolenTeam *GetStolenTeam(void)
{
    return &gPokemonStoragePtr->hoennStolenTeam;
}

static u32 CountBoxedMons(void)
{
    u32 boxId, pos, count = 0;

    for (boxId = 0; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            if (GetBoxMonDataAt(boxId, pos, MON_DATA_SPECIES) != SPECIES_NONE)
                count++;
        }
    }
    return count;
}

// The Vermilion sailor, before the first crossing: Kanto's POKéMON must fit in Kanto's 7 boxes
void CheckKantoBoxesFit(void)
{
    gSpecialVar_Result = (CountBoxedMons() <= HOENN_FIRST_BOX * IN_BOX_COUNT);
}

// Moves every POKéMON in the Hoenn boxes to the first free places of the Kanto boxes, in order
static void EmptyHoennBoxes(void)
{
    u32 boxId, pos, freeBox = 0, freePos = 0;
    struct BoxPokemon mon;

    for (boxId = HOENN_FIRST_BOX; boxId < TOTAL_BOXES_COUNT; boxId++)
    {
        for (pos = 0; pos < IN_BOX_COUNT; pos++)
        {
            if (GetBoxMonDataAt(boxId, pos, MON_DATA_SPECIES) == SPECIES_NONE)
                continue;
            while (freeBox < HOENN_FIRST_BOX && GetBoxMonDataAt(freeBox, freePos, MON_DATA_SPECIES) != SPECIES_NONE)
            {
                if (++freePos == IN_BOX_COUNT)
                {
                    freePos = 0;
                    freeBox++;
                }
            }
            if (freeBox == HOENN_FIRST_BOX)
                return; // CheckKantoBoxesFit makes sure this doesn't happen
            CopyBoxMonAt(boxId, pos, &mon);
            SetBoxMonAt(freeBox, freePos, &mon);
            ZeroBoxMonAt(boxId, pos);
        }
    }
}

static void NameHoennBoxes(void)
{
    struct HoennStolenTeam *stolen = GetStolenTeam();
    u32 i;

    for (i = 0; i < NUM_HOENN_BOXES; i++)
    {
        u8 *name = GetBoxNamePtr(HOENN_FIRST_BOX + i);
        StringCopy(stolen->boxNames[i], name);
        name = StringCopy(name, sText_HoennBox);
        ConvertIntToDecimalStringN(name, i + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    }
}

// The SEAGALLOP's first arrival at Slateport (VAR_HOENN_STORY_STATE 3)
void StealPlayerTeamAndBag(void)
{
    struct HoennStolenTeam *stolen = GetStolenTeam();
    u32 i, j, slot = 0;

    if (IsKantoLinkCut())
        return;

    CalculatePlayerPartyCount();
    stolen->partyCount = gPlayerPartyCount;
    for (i = 0; i < PARTY_SIZE; i++)
        stolen->party[i] = gPlayerParty[i];
    ZeroPlayerPartyMons();
    CalculatePlayerPartyCount();

    for (i = 0; i < POCKETS_COUNT; i++)
    {
        for (j = 0; j < gBagPockets[i].capacity; j++, slot++)
        {
            stolen->bag[slot].itemId = gBagPockets[i].itemSlots[j].itemId;
            stolen->bag[slot].quantity = GetBagItemSlotQuantity(&gBagPockets[i].itemSlots[j]);
        }
    }
    ClearBag();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;

    stolen->money = GetMoney(&gSaveBlock1Ptr->money);
    SetMoney(&gSaveBlock1Ptr->money, HOENN_STARTING_MONEY);

    EmptyHoennBoxes();
    NameHoennBoxes();
    gPokemonStoragePtr->currentBox = HOENN_FIRST_BOX;
    VarSet(VAR_PC_BOX_TO_SEND_MON, HOENN_FIRST_BOX);
    FlagSet(FLAG_HOENN_TEAM_STOLEN);
}

// The Magma Hideout, after MAXIE: the grunts kept the MEGA RING apart. The script gives it.
void TakeStolenMegaRing(void)
{
    struct HoennStolenTeam *stolen = GetStolenTeam();
    u32 i;

    gSpecialVar_Result = FALSE;
    for (i = 0; i < NUM_STOLEN_BAG_SLOTS; i++)
    {
        if (stolen->bag[i].itemId == ITEM_MEGA_RING && stolen->bag[i].quantity != 0)
        {
            stolen->bag[i].itemId = ITEM_NONE;
            stolen->bag[i].quantity = 0;
            gSpecialVar_Result = TRUE;
        }
    }
}

// Gives back whatever of the stash fits: the POKéMON to the PC's first free places, the items to the BAG
// or else the item storage, the money. What doesn't fit stays, for the player's PC to hand over later.
// Returns TRUE if nothing is left.
bool8 TryReturnStolenTeam(void)
{
    struct HoennStolenTeam *stolen = GetStolenTeam();
    bool8 allBack = TRUE;
    u32 i, kept = 0;

    for (i = 0; i < stolen->partyCount; i++)
    {
        if (GiveMonToPC(&stolen->party[i]) == MON_CANT_GIVE)
            stolen->party[kept++] = stolen->party[i];
    }
    for (i = kept; i < PARTY_SIZE; i++)
        ZeroMonData(&stolen->party[i]);
    stolen->partyCount = kept;
    if (kept != 0)
        allBack = FALSE;

    for (i = 0; i < NUM_STOLEN_BAG_SLOTS; i++)
    {
        struct ItemSlot *slot = &stolen->bag[i];

        if (slot->itemId == ITEM_NONE || slot->quantity == 0)
            continue;
        if (AddBagItem(slot->itemId, slot->quantity) || AddPCItem(slot->itemId, slot->quantity))
        {
            slot->itemId = ITEM_NONE;
            slot->quantity = 0;
        }
        else
        {
            allBack = FALSE;
        }
    }

    AddMoney(&gSaveBlock1Ptr->money, stolen->money);
    stolen->money = 0;
    return allBack;
}

// ARCHIE and MAXIE in LITTLEROOT, after Hoenn's Hall of Fame: the cable to Kanto is mended
void RestoreKantoLink(void)
{
    struct HoennStolenTeam *stolen = GetStolenTeam();
    u32 i;

    if (!IsKantoLinkCut())
        return;

    FlagSet(FLAG_HOENN_KANTO_LINK);
    FlagClear(FLAG_HOENN_TEAM_STOLEN);
    for (i = 0; i < NUM_HOENN_BOXES; i++)
        StringCopy(GetBoxNamePtr(HOENN_FIRST_BOX + i), stolen->boxNames[i]);
    gPokemonStoragePtr->currentBox = 0;
    TryReturnStolenTeam();
}

// The player's PC, after the link: hands over what didn't fit when it came back
void TryReturnStolenLeftovers(void)
{
    if (FlagGet(FLAG_HOENN_KANTO_LINK))
        TryReturnStolenTeam();
}

// For scripts (the Route 117 day care, the sailors)
void ScriptIsKantoLinkCut(void)
{
    gSpecialVar_Result = IsKantoLinkCut();
}
