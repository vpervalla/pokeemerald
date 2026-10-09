MegaIsland_Castle_1F_MapScripts::
	.byte 0

@ The receptionist registers the player for MR. M.'s tournament, and heals their team between
@ rounds. VAR_MEGA_STORY_STATE: 2 invited, 3 registered (to sleep in guest room 1), 4 quarterfinal
@ next, 5 semifinal next, 6 final next, 7 tournament won, 8 waking up from a lost match. The matches are in MegaIsland_Arena.
MegaIsland_Castle_1F_EventScript_Receptionist::
	lock
	faceplayer
	goto_if_eq VAR_MEGA_STORY_STATE, 7, MegaIsland_Castle_1F_EventScript_ReceptionistChampion
	goto_if_eq VAR_MEGA_STORY_STATE, 3, MegaIsland_Castle_1F_EventScript_ReceptionistGoToSleep
	goto_if_ge VAR_MEGA_STORY_STATE, 4, MegaIsland_Castle_1F_EventScript_ReceptionistNextRound
	msgbox MegaIsland_Castle_1F_Text_Register, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MegaIsland_Castle_1F_EventScript_ReceptionistNotYet
	setvar VAR_MEGA_STORY_STATE, 3
	msgbox MegaIsland_Castle_1F_Text_Registered
	release
	end

MegaIsland_Castle_1F_EventScript_ReceptionistGoToSleep::
	msgbox MegaIsland_Castle_1F_Text_Registered
	release
	end

MegaIsland_Castle_1F_EventScript_ReceptionistNotYet::
	msgbox MegaIsland_Castle_1F_Text_NotYet
	release
	end

MegaIsland_Castle_1F_EventScript_ReceptionistNextRound::
	bufferstring STR_VAR_1, MegaIsland_Castle_1F_Text_Quarterfinal
	call_if_eq VAR_MEGA_STORY_STATE, 5, MegaIsland_Castle_1F_EventScript_BufferSemifinal
	call_if_eq VAR_MEGA_STORY_STATE, 6, MegaIsland_Castle_1F_EventScript_BufferFinal
	msgbox MegaIsland_Castle_1F_Text_RestoreBeforeMatch
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	msgbox MegaIsland_Castle_1F_Text_GoToTheCourt
	release
	end

MegaIsland_Castle_1F_EventScript_BufferSemifinal::
	bufferstring STR_VAR_1, MegaIsland_Castle_1F_Text_Semifinal
	return

MegaIsland_Castle_1F_EventScript_BufferFinal::
	bufferstring STR_VAR_1, MegaIsland_Castle_1F_Text_Final
	return

MegaIsland_Castle_1F_EventScript_ReceptionistChampion::
	msgbox MegaIsland_Castle_1F_Text_Champion
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	release
	end

MegaIsland_Castle_1F_EventScript_ReceptionistHeal::
	closemessage
	call Kanto_EventScript_OutOfCenterPartyHeal
	return

MegaIsland_Castle_1F_Text_Register:
	.string "Welcome to the castle, {PLAYER}.\p"
	.string "You are the last of our guests.\n"
	.string "MR. M.'s tournament can begin.\p"
	.string "Eight trainers from eight regions\n"
	.string "will battle until one remains.\p"
	.string "The quarterfinals will be:\n"
	.string "{PLAYER} against ALDER,\p"
	.string "{RIVAL} against CYNTHIA,\n"
	.string "LANCE against DIANTHA,\p"
	.string "and STEVEN against LEON.\p"
	.string "Shall I register you?$"

MegaIsland_Castle_1F_Text_Registered:
	.string "You are registered, {PLAYER}.\p"
	.string "The tournament begins tomorrow\n"
	.string "morning.\p"
	.string "Your room is upstairs: the first\n"
	.string "door on the left.\p"
	.string "Rest well tonight!$"

MegaIsland_Castle_1F_Text_RestoreBeforeMatch:
	.string "Your next match is the\n"
	.string "{STR_VAR_1}.\p"
	.string "Allow me to restore your POKéMON.$"

MegaIsland_Castle_1F_Text_GoToTheCourt:
	.string "Please go through to the court\n"
	.string "when you are ready.\p"
	.string "MR. M. is waiting.$"

MegaIsland_Castle_1F_Text_NotYet:
	.string "Take your time. The tournament\n"
	.string "will not begin without you.$"

MegaIsland_Castle_1F_Text_Champion:
	.string "Congratulations, {PLAYER}!\n"
	.string "The champion of champions!\p"
	.string "Allow me to restore your POKéMON.$"

MegaIsland_Castle_1F_Text_Quarterfinal:
	.string "QUARTERFINAL$"

MegaIsland_Castle_1F_Text_Semifinal:
	.string "SEMIFINAL$"

MegaIsland_Castle_1F_Text_Final:
	.string "FINAL$"

MegaIsland_Castle_1F_EventScript_Blue::
	msgbox MegaIsland_Castle_1F_Text_Blue, MSGBOX_NPC
	end

MegaIsland_Castle_1F_Text_Blue:
	.string "{RIVAL}: Hey, {PLAYER}!\n"
	.string "Took you long enough.\p"
	.string "So you got a letter from this\n"
	.string "MR. M. too, huh?\p"
	.string "There are CHAMPIONS here from\n"
	.string "regions I've never even heard of!\p"
	.string "Doesn't matter. Their stones,\n"
	.string "their fancy titles…\p"
	.string "I'm the one who's walking out of\n"
	.string "here with the trophy!\p"
	.string "Smell ya later!$"
