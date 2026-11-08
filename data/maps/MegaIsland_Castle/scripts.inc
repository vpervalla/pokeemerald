MegaIsland_Castle_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Castle_OnTransition
	.byte 0

@ ALAIN, who came for the daily tournament, waits on the lawn by the approach once LEON and CYNTHIA
@ have told the player that the tournament goes on (FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, see MegaIsland).
MegaIsland_Castle_OnTransition:
	setflag FLAG_TEMP_1
	call_if_set FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, MegaIsland_Castle_EventScript_ShowAlain
	end

MegaIsland_Castle_EventScript_ShowAlain::
	clearflag FLAG_TEMP_1
	return

MegaIsland_Castle_EventScript_Alain::
	msgbox MegaIsland_Castle_Text_Alain, MSGBOX_NPC
	end

MegaIsland_Castle_Text_Alain:
	.string "ALAIN: …You're {PLAYER}.\p"
	.string "I heard about MR. M.'s tournament\n"
	.string "too late to enter it.\p"
	.string "So it was MEWTWO that gathered the\n"
	.string "MEGA STONES, and you who stood up\l"
	.string "to it.\p"
	.string "CHARIZARD and I came here to get\n"
	.string "stronger. If we meet in the\l"
	.string "tournament, hold nothing back.$"
