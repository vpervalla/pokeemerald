@ The way to Hoenn (docs/hoenn_plan.md, Act 0). After the first Hall of Fame, MOM passes on a call
@ from BLAINE (PalletTown_PlayersHouse_1F); Team Aqua and Team Magma are in the POKéMON MANSION for
@ the lab's last file, the PRIMAL NOTES; when the player beats SHELLY and TABITHA on B1F, they flee to
@ Hoenn with half of it each (PokemonMansion_B1F) and the SEAGALLOP at VERMILION sails to SLATEPORT
@ (VermilionCity). VAR_HOENN_STORY_STATE follows it.

@ The raiders are in the MANSION only while VAR_HOENN_STORY_STATE is 2. Called on transition by its floors.
HoennStory_EventScript_UpdateMansion::
	setflag FLAG_HIDE_MANSION_BLAINE
	setflag FLAG_HIDE_MANSION_RAIDERS
	goto_if_ne VAR_HOENN_STORY_STATE, 2, Common_EventScript_NopReturn
	clearflag FLAG_HIDE_MANSION_RAIDERS
	return

@ The sailor in SLATEPORT's and LILYCOVE's harbors, who sails back to VERMILION
HoennStory_EventScript_SeagallopSailor::
	lock
	faceplayer
	message HoennStory_Text_SailBackToKanto
	waitmessage
	setvar VAR_0x8004, SEAGALLOP_SLATEPORT
	multichoice 20, 8, MULTI_KANTO_SEAGALLOP_VERMILION, FALSE
	switch VAR_RESULT
	case 0, Kanto_EventScript_SailToVermilionCity
	case 1, Kanto_EventScript_CancelSail
	case 127, Kanto_EventScript_CancelSail
	end

HoennStory_Text_SailBackToKanto:
	.string "Ahoy! This SEAGALLOP sails back\n"
	.string "to VERMILION, in KANTO.\p"
	.string "Where do you want to sail?$"

@ Before BIRCH's lab (Act 1): the state that Emerald's truck, house, clock and Route 101 scenes would
@ have left, so its story goes on from the rival on Route 103. The player isn't from LITTLEROOT: the
@ house of their gender is a guest room BIRCH lends them, with no MOM, and they wake up there after
@ a whiteout.
HoennStory_EventScript_ReadyHoennOpening::
	setvar VAR_HOENN_STORY_STATE, 4
	setflag FLAG_ARRIVED_IN_HOENN
	setflag FLAG_HIDE_SLATEPORT_HARBOR_BIRCH
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
	setvar VAR_LITTLEROOT_RIVAL_STATE, 3
	setvar VAR_LITTLEROOT_TOWN_STATE, 2
	setflag FLAG_RESCUED_BIRCH
	setvar VAR_ROUTE101_STATE, 3
	setflag FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE
	setflag FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_BEDROOM_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN
	setflag FLAG_MET_RIVAL_MOM
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, HoennStory_EventScript_ReadyHoennOpeningFemale
	@ The guest room is BRENDAN's house, the rival MAY's (as InsideOfTruck and MAY's bedroom would set)
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MAY
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F
	return

HoennStory_EventScript_ReadyHoennOpeningFemale::
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_BRENDAN
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F
	return
