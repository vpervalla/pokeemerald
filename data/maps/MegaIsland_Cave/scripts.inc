MegaIsland_Cave_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Cave_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, MegaIsland_Cave_OnResume
	.byte 0

@ After the tournament MEWTWO (MR. M.) waits here for the player (VAR_MEGA_STORY_STATE 9), and the
@ hiker keeps away. It's a legendary battle at lv 80: MEWTWO holds MEWTWONITE X and Mega Evolves.
@ Only catching it ends the story (state 10, and the player gets the stone with it); if it's
@ beaten or the player runs, it waits here again next time.
MegaIsland_Cave_OnTransition:
	call_if_eq VAR_MEGA_STORY_STATE, 9, MegaIsland_Cave_EventScript_MewtwoWaits
	call_if_ne VAR_MEGA_STORY_STATE, 9, MegaIsland_Cave_EventScript_HideMewtwo
	end

MegaIsland_Cave_EventScript_MewtwoWaits::
	setflag FLAG_TEMP_2
	return

MegaIsland_Cave_EventScript_HideMewtwo::
	setflag FLAG_TEMP_1
	return

MegaIsland_Cave_OnResume:
	call_if_set FLAG_SYS_SPECIAL_WILD_BATTLE, MegaIsland_Cave_EventScript_TryRemoveMewtwo
	end

MegaIsland_Cave_EventScript_TryRemoveMewtwo::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, MegaIsland_Cave_EventScript_Return
	removeobject LOCALID_MEGA_CAVE_MEWTWO
MegaIsland_Cave_EventScript_Return::
	return

MegaIsland_Cave_EventScript_Mewtwo::
	lock
	faceplayer
	msgbox MegaIsland_Cave_Text_MewtwoChallenge
	closemessage
	waitse
	playmoncry SPECIES_MEWTWO, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	setwildbattle SPECIES_MEWTWO, 80, ITEM_MEWTWONITE_X
	setflag FLAG_SYS_SPECIAL_WILD_BATTLE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_SPECIAL_WILD_BATTLE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, MegaIsland_Cave_EventScript_CaughtMewtwo
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, MegaIsland_Cave_EventScript_MewtwoBeaten
	msgbox MegaIsland_Cave_Text_MewtwoStillWaits
	release
	end

MegaIsland_Cave_EventScript_MewtwoBeaten::
	msgbox MegaIsland_Cave_Text_MewtwoNotYet
	closemessage
	playse SE_M_TELEPORT
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_MEGA_CAVE_MEWTWO
	fadescreen FADE_FROM_WHITE
	release
	end

MegaIsland_Cave_EventScript_CaughtMewtwo::
	setvar VAR_MEGA_STORY_STATE, 10
	release
	end

MegaIsland_Cave_EventScript_ItemMaxRevive::
	finditem ITEM_MAX_REVIVE
	end

MegaIsland_Cave_EventScript_Hiker::
	msgbox MegaIsland_Cave_Text_Hiker, MSGBOX_NPC
	end

MegaIsland_Cave_Text_Hiker:
	.string "This cave doesn't go any deeper.\p"
	.string "But sometimes the walls glow, like\n"
	.string "the stones the champions carry.\p"
	.string "There must be more of them buried\n"
	.string "somewhere on this island…$"

MegaIsland_Cave_Text_MewtwoChallenge:
	.string "MEWTWO: So you came, alone.\p"
	.string "No crowd. No rules. No dream to\n"
	.string "wake from.\p"
	.string "Show me the bond between you and\n"
	.string "your POKéMON… and I will show you\l"
	.string "mine!$"

MegaIsland_Cave_Text_MewtwoNotYet:
	.string "MEWTWO: …You are strong.\p"
	.string "But strength alone will not\n"
	.string "bind me.\p"
	.string "I will be waiting here.$"

MegaIsland_Cave_Text_MewtwoStillWaits:
	.string "MEWTWO is still waiting, its eyes\n"
	.string "fixed on {PLAYER}…$"
