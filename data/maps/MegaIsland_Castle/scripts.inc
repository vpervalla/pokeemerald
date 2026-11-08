MegaIsland_Castle_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Castle_OnTransition
	.byte 0

@ ALAIN, who came for the daily tournament, and WALLY, who came from HOENN to watch it, wait on the
@ lawns either side of the approach once LEON and CYNTHIA have told the player that the tournament goes
@ on (FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, see MegaIsland). ALAIN uses the special NPC palette slot and
@ WALLY its reflection's (OBJ_EVENT_GFX_KANTO_WALLY, Emerald's WALLY in his own palette).
MegaIsland_Castle_OnTransition:
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	call_if_set FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, MegaIsland_Castle_EventScript_ShowAlain
	end

MegaIsland_Castle_EventScript_ShowAlain::
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	return

MegaIsland_Castle_EventScript_Wally::
	msgbox MegaIsland_Castle_Text_Wally, MSGBOX_NPC
	end

MegaIsland_Castle_Text_Wally:
	.string "WALLY: {PLAYER}! I knew I'd find you\n"
	.string "here!\p"
	.string "Everyone in HOENN was talking\n"
	.string "about this tournament, so I came\l"
	.string "to watch.\p"
	.string "All those Mega Evolutions…\n"
	.string "GARDEVOIR and I want to try it\l"
	.string "too, someday.\p"
	.string "I'll keep training. Next time,\n"
	.string "I want to be one of the trainers\l"
	.string "out there!$"

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
