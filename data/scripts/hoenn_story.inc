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
