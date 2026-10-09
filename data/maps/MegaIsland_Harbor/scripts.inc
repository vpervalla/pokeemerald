MegaIsland_Harbor_MapScripts::
	.byte 0

MegaIsland_Harbor_EventScript_Sailor::
	lock
	faceplayer
	message Kanto_Text_WhereDoYouWantToSail
	waitmessage
	setvar VAR_0x8004, SEAGALLOP_MEGA_ISLAND
	multichoice 20, 8, MULTI_KANTO_SEAGALLOP_VERMILION, FALSE
	switch VAR_RESULT
	case 0, Kanto_EventScript_SailToVermilionCity
	case 1, Kanto_EventScript_CancelSail
	case 127, Kanto_EventScript_CancelSail
	end
