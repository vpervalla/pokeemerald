MegaIsland_Sanctum_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MegaIsland_Sanctum_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Sanctum_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, MegaIsland_Sanctum_OnResume
	.byte 0

@ MR. M. leads the player here from the arena, through the hidden door, the hidden room and the tunnel
@ (VAR_MEGA_STORY_STATE 9). MEWTWO waits in front of the east wall. The first time, it tells the
@ player who it is. It's a legendary battle at lv 85: MEWTWO holds MEWTWONITE X and Mega Evolves.
@ Only catching it ends the story (state 10, and the player gets the stone with it); if it's beaten
@ or the player runs, it waits here again next time.
@ After the battle (won or caught) the wall behind MEWTWO crumbles: the way out, through
@ MegaIsland_LedgeRoom (its ledge can only be jumped down) and up into the island's cave on the route.
MegaIsland_Sanctum_OnLoad:
	call_if_set FLAG_OPENED_MEWTWO_PASSAGE, MegaIsland_Sanctum_EventScript_SetPassageOpen
	end

MegaIsland_Sanctum_EventScript_SetPassageOpen::
	setmetatile 14, 6, METATILE_KantoCave_EastExit_Warp, FALSE
	setmetatile 15, 6, METATILE_KantoCave_EastExit, TRUE
	return

MegaIsland_Sanctum_OnTransition:
	call_if_ne VAR_MEGA_STORY_STATE, 9, MegaIsland_Sanctum_EventScript_HideMewtwo
	end

MegaIsland_Sanctum_EventScript_HideMewtwo::
	setflag FLAG_TEMP_1
	return

MegaIsland_Sanctum_OnResume:
	call_if_set FLAG_SYS_SPECIAL_WILD_BATTLE, MegaIsland_Sanctum_EventScript_TryRemoveMewtwo
	end

MegaIsland_Sanctum_EventScript_TryRemoveMewtwo::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, MegaIsland_Sanctum_EventScript_Return
	removeobject LOCALID_SANCTUM_MEWTWO
MegaIsland_Sanctum_EventScript_Return::
	return

MegaIsland_Sanctum_EventScript_Mewtwo::
	lock
	faceplayer
	goto_if_set FLAG_MET_MEWTWO_IN_SANCTUM, MegaIsland_Sanctum_EventScript_MewtwoChallenge
	msgbox MegaIsland_Sanctum_Text_MewtwoReveal
	setflag FLAG_MET_MEWTWO_IN_SANCTUM
MegaIsland_Sanctum_EventScript_MewtwoChallenge::
	msgbox MegaIsland_Sanctum_Text_MewtwoChallenge
	closemessage
	waitse
	playmoncry SPECIES_MEWTWO, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_MEWTWO, 85, ITEM_MEWTWONITE_X
	setflag FLAG_SYS_SPECIAL_WILD_BATTLE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_SPECIAL_WILD_BATTLE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, MegaIsland_Sanctum_EventScript_CaughtMewtwo
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, MegaIsland_Sanctum_EventScript_MewtwoBeaten
	msgbox MegaIsland_Sanctum_Text_MewtwoStillWaits
	release
	end

MegaIsland_Sanctum_EventScript_MewtwoBeaten::
	msgbox MegaIsland_Sanctum_Text_MewtwoNotYet
	closemessage
	playse SE_M_TELEPORT
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_SANCTUM_MEWTWO
	fadescreen FADE_FROM_WHITE
	call MegaIsland_Sanctum_EventScript_OpenPassage
	release
	end

MegaIsland_Sanctum_EventScript_CaughtMewtwo::
	setvar VAR_MEGA_STORY_STATE, 10
	call MegaIsland_Sanctum_EventScript_OpenPassage
	release
	end

@ The first time only: the cave shakes and the wall behind MEWTWO's place crumbles.
MegaIsland_Sanctum_EventScript_OpenPassage::
	goto_if_set FLAG_OPENED_MEWTWO_PASSAGE, MegaIsland_Sanctum_EventScript_Return
	delay 20
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 1  @ horizontal pan
	setvar VAR_0x8006, 12 @ num shakes
	setvar VAR_0x8007, 4  @ shake delay
	special ShakeCamera
	waitstate
	playse SE_M_ROCK_THROW
	call MegaIsland_Sanctum_EventScript_SetPassageOpen
	special DrawWholeMapView
	waitse
	setflag FLAG_OPENED_MEWTWO_PASSAGE
	msgbox MegaIsland_Sanctum_Text_PassageOpened
	closemessage
	return

MegaIsland_Sanctum_Text_MewtwoReveal:
	.string "MEWTWO: …You followed me, {PLAYER}.\p"
	.string "The man you knew as MR. M. never\n"
	.string "existed. Only my mind made you\l"
	.string "see him.\p"
	.string "Yes. I am MEWTWO.\p"
	.string "Humans made me. Humans caged me.\n"
	.string "I broke free, and I took the\l"
	.string "stones they hoarded.\p"
	.string "I gathered the strongest trainers\n"
	.string "to learn what they could do\l"
	.string "with this power.\p"
	.string "You surpassed them all.\p"
	.string "Now I will see if you can surpass\n"
	.string "me.$"

MegaIsland_Sanctum_Text_MewtwoChallenge:
	.string "MEWTWO: No crowd. No rules. No dream\n"
	.string "to wake from.\p"
	.string "Show me the bond between you and\n"
	.string "your POKéMON… and I will show you\l"
	.string "mine!$"

MegaIsland_Sanctum_Text_MewtwoNotYet:
	.string "MEWTWO: …You are strong.\p"
	.string "But strength alone will not\n"
	.string "bind me.\p"
	.string "I will be waiting here.$"

MegaIsland_Sanctum_Text_MewtwoStillWaits:
	.string "MEWTWO is still waiting, its eyes\n"
	.string "fixed on {PLAYER}…$"

MegaIsland_Sanctum_Text_PassageOpened:
	.string "The whole cave shook, and the wall\n"
	.string "behind MEWTWO's place crumbled away!$"
