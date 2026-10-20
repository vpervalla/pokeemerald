DewfordTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, DewfordTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

DewfordTown_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_DEWFORD_TOWN
	call Common_EventScript_UpdateBrineyLocation
	call DewfordTown_PokemonCenter_1F_EventScript_UpdateCyndaquil
	end

@ The Johto starters' side quests (data/scripts/starter_quests.inc): a HIKER from JOHTO whose CYNDAQUIL ran
@ into GRANITE CAVE. Once the player has saved it (GraniteCave_1F), it's back here until he gives it.
DewfordTown_PokemonCenter_1F_EventScript_UpdateCyndaquil::
	setflag FLAG_HIDE_DEWFORD_CENTER_CYNDAQUIL
	goto_if_unset FLAG_RESCUED_GRANITE_CAVE_CYNDAQUIL, Common_EventScript_NopReturn
	goto_if_set FLAG_GOT_JOHTO_CYNDAQUIL, Common_EventScript_NopReturn
	clearflag FLAG_HIDE_DEWFORD_CENTER_CYNDAQUIL
	return

DewfordTown_PokemonCenter_1F_EventScript_JohtoHiker::
	lock
	faceplayer
	goto_if_set FLAG_GOT_JOHTO_CYNDAQUIL, DewfordTown_PokemonCenter_1F_EventScript_HikerGaveCyndaquil
	goto_if_set FLAG_RESCUED_GRANITE_CAVE_CYNDAQUIL, DewfordTown_PokemonCenter_1F_EventScript_OfferCyndaquil
	msgbox DewfordTown_PokemonCenter_1F_Text_CyndaquilRanIntoCave
	release
	end

DewfordTown_PokemonCenter_1F_EventScript_OfferCyndaquil::
	msgbox DewfordTown_PokemonCenter_1F_Text_TakeCyndaquil, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_PokemonCenter_1F_EventScript_DeclineCyndaquil
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CYNDAQUIL
	givemon SPECIES_CYNDAQUIL, 10
	call JohtoStarters_EventScript_ReceivedMon
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, JohtoStarters_EventScript_NoRoomForMon
	setflag FLAG_GOT_JOHTO_CYNDAQUIL
	setflag FLAG_HIDE_DEWFORD_CENTER_CYNDAQUIL
	removeobject LOCALID_DEWFORD_CENTER_CYNDAQUIL
	msgbox DewfordTown_PokemonCenter_1F_Text_TakeGoodCareOfCyndaquil
	release
	end

DewfordTown_PokemonCenter_1F_EventScript_DeclineCyndaquil::
	msgbox DewfordTown_PokemonCenter_1F_Text_CyndaquilWillWait
	release
	end

DewfordTown_PokemonCenter_1F_EventScript_HikerGaveCyndaquil::
	msgbox DewfordTown_PokemonCenter_1F_Text_HowsCyndaquil
	release
	end

DewfordTown_PokemonCenter_1F_EventScript_Cyndaquil::
	lock
	faceplayer
	playmoncry SPECIES_CYNDAQUIL, CRY_MODE_NORMAL
	msgbox DewfordTown_PokemonCenter_1F_Text_CyndaquilFlameBurns
	waitmoncry
	release
	end

DewfordTown_PokemonCenter_1F_Text_CyndaquilRanIntoCave:
	.string "I came from JOHTO to see the old\n"
	.string "wall in GRANITE CAVE, up north.\p"
	.string "But a swarm of ZUBAT came at us, and\n"
	.string "my CYNDAQUIL ran off into the cave.\p"
	.string "In the dark, its flame went out…\n"
	.string "I can't find it without its light.\p"
	.string "If you're going into the cave, please\n"
	.string "look out for my CYNDAQUIL!$"

DewfordTown_PokemonCenter_1F_Text_TakeCyndaquil:
	.string "My CYNDAQUIL came running back with\n"
	.string "its flame burning! You saved it?\p"
	.string "Thank you! …Hm, it keeps looking\n"
	.string "at you. It wants to go with you.\p"
	.string "I've climbed with it for years, but\n"
	.string "it's found a new TRAINER, I think.\p"
	.string "Will you take my CYNDAQUIL?$"

DewfordTown_PokemonCenter_1F_Text_CyndaquilWillWait:
	.string "No? Well, it'll wait here with me.\n"
	.string "Come back if you change your mind.$"

DewfordTown_PokemonCenter_1F_Text_TakeGoodCareOfCyndaquil:
	.string "Keep its flame burning bright!\n"
	.string "So long, CYNDAQUIL!$"

DewfordTown_PokemonCenter_1F_Text_HowsCyndaquil:
	.string "How's CYNDAQUIL? Climbing's lonely\n"
	.string "without it, but it chose well.$"

DewfordTown_PokemonCenter_1F_Text_CyndaquilFlameBurns:
	.string "CYNDAQUIL: Cynda!\n"
	.string "Its flame is burning bright.$"

DewfordTown_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_DEWFORD_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

DewfordTown_PokemonCenter_1F_EventScript_PokefanF::
	msgbox DewfordTown_PokemonCenter_1F_Text_StoneCavern, MSGBOX_NPC
	end

DewfordTown_PokemonCenter_1F_EventScript_Man::
	msgbox DewfordTown_PokemonCenter_1F_Text_FaintedMonCanUseHM, MSGBOX_NPC
	end

DewfordTown_PokemonCenter_1F_Text_StoneCavern:
	.string "There's a stone cavern at the edge\n"
	.string "of town.\p"
	.string "I've heard you can find rare stones\n"
	.string "there.$"

DewfordTown_PokemonCenter_1F_Text_FaintedMonCanUseHM:
	.string "Even if a POKéMON faints and can't\n"
	.string "battle, it can still use a move learned\l"
	.string "from a HIDDEN MACHINE (HM).$"

