MegaIsland_Castle_1F_MapScripts::
	.byte 0

@ The receptionist registers the player for MR. M.'s tournament, and heals their team between
@ rounds. VAR_MEGA_STORY_STATE: 2 invited, 3 registered (to sleep in guest room 1), 4 quarterfinal
@ next, 5 semifinal next, 6 final next, 7 tournament won, 8 waking up from a lost match,
@ 9 MEWTWO waiting in the island's cave, 10 MEWTWO caught. The matches are in MegaIsland_Arena.
@ From state 9 on, a new tournament is held each day (FLAG_DAILY_APEX_TOURNAMENT), against three
@ opponents drawn at random among the eight other champions (VAR_TOURNAMENT_ROUND and
@ VAR_TOURNAMENT_OPPONENTS, see RandomizeTournamentOpponents).
MegaIsland_Castle_1F_EventScript_Receptionist::
	lock
	faceplayer
	goto_if_eq VAR_MEGA_STORY_STATE, 7, MegaIsland_Castle_1F_EventScript_ReceptionistChampion
	goto_if_ge VAR_MEGA_STORY_STATE, 9, MegaIsland_Castle_1F_EventScript_ReceptionistReplay
	goto_if_eq VAR_MEGA_STORY_STATE, 3, MegaIsland_Castle_1F_EventScript_ReceptionistGoToSleep
	goto_if_ge VAR_MEGA_STORY_STATE, 4, MegaIsland_Castle_1F_EventScript_ReceptionistNextRound
	specialvar VAR_RESULT, CanPlayerMegaEvolve
	goto_if_eq VAR_RESULT, FALSE, MegaIsland_Castle_1F_EventScript_ReceptionistCantMegaEvolve
	msgbox MegaIsland_Castle_1F_Text_Register, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MegaIsland_Castle_1F_EventScript_ReceptionistNotYet
	setvar VAR_MEGA_STORY_STATE, 3
	msgbox MegaIsland_Castle_1F_Text_Registered
	release
	end

@ Only a trainer who can Mega Evolve a POKéMON in their party may enter (see CanPlayerMegaEvolve).
@ The post-game LEAF and RED, and the GYM LEADERS in their rematches, give MEGA STONES.
MegaIsland_Castle_1F_EventScript_ReceptionistCantMegaEvolve::
	msgbox MegaIsland_Castle_1F_Text_CantMegaEvolve
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
	msgbox MegaIsland_Castle_1F_Text_GoToTheArena
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
	call_if_eq VAR_MEGA_STORY_STATE, 7, MegaIsland_Castle_1F_EventScript_MrMWaits
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	release
	end

MegaIsland_Castle_1F_EventScript_ReceptionistReplay::
	goto_if_ne VAR_TOURNAMENT_ROUND, 0, MegaIsland_Castle_1F_EventScript_ReplayNextRound
	goto_if_set FLAG_DAILY_APEX_TOURNAMENT, MegaIsland_Castle_1F_EventScript_ReplayTomorrow
	msgbox MegaIsland_Castle_1F_Text_ReplayRegister, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MegaIsland_Castle_1F_EventScript_ReceptionistNotYet
	setflag FLAG_DAILY_APEX_TOURNAMENT
	special RandomizeTournamentOpponents
	setvar VAR_TOURNAMENT_ROUND, 1
	msgbox MegaIsland_Castle_1F_Text_ReplayRegistered
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	msgbox MegaIsland_Castle_1F_Text_ReplayGoToTheArena
	release
	end

MegaIsland_Castle_1F_EventScript_ReplayNextRound::
	bufferstring STR_VAR_1, MegaIsland_Castle_1F_Text_Quarterfinal
	call_if_eq VAR_TOURNAMENT_ROUND, 2, MegaIsland_Castle_1F_EventScript_BufferSemifinal
	call_if_eq VAR_TOURNAMENT_ROUND, 3, MegaIsland_Castle_1F_EventScript_BufferFinal
	msgbox MegaIsland_Castle_1F_Text_RestoreBeforeMatch
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	msgbox MegaIsland_Castle_1F_Text_ReplayGoToTheArena
	release
	end

MegaIsland_Castle_1F_EventScript_ReplayTomorrow::
	msgbox MegaIsland_Castle_1F_Text_Champion
	call MegaIsland_Castle_1F_EventScript_ReceptionistHeal
	msgbox MegaIsland_Castle_1F_Text_ReplayTomorrow
	release
	end

MegaIsland_Castle_1F_EventScript_MrMWaits::
	msgbox MegaIsland_Castle_1F_Text_MrMWaits
	return

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
	.string "{PLAYER} against STEVEN,\p"
	.string "CYNTHIA against {RIVAL},\n"
	.string "LANCE against DIANTHA,\p"
	.string "and ALDER against LEON.\p"
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

MegaIsland_Castle_1F_Text_GoToTheArena:
	.string "The arena is on the top floor.\n"
	.string "Go up when you are ready.\p"
	.string "MR. M. is waiting.$"

MegaIsland_Castle_1F_Text_CantMegaEvolve:
	.string "Welcome to the castle, {PLAYER}.\p"
	.string "MR. M.'s tournament is a battle\n"
	.string "of Mega Evolution.\p"
	.string "Only trainers who can Mega Evolve\n"
	.string "one of their POKéMON may enter.\p"
	.string "None of your POKéMON can Mega\n"
	.string "Evolve yet, I'm afraid.\p"
	.string "MR. M. has entrusted MEGA STONES\n"
	.string "to strong trainers in KANTO.\p"
	.string "Defeat them, and they will give\n"
	.string "you their stone.\p"
	.string "Return with a POKéMON and its\n"
	.string "MEGA STONE, and I'll register you.$"

MegaIsland_Castle_1F_Text_NotYet:
	.string "Take your time. The tournament\n"
	.string "will not begin without you.$"

MegaIsland_Castle_1F_Text_ReplayRegister:
	.string "Welcome back, {PLAYER}!\p"
	.string "The champions have stayed on the\n"
	.string "island, and a new tournament is\l"
	.string "held here every day.\p"
	.string "Your three opponents are drawn\n"
	.string "by lot.\p"
	.string "Shall I register you for today's\n"
	.string "tournament?$"

MegaIsland_Castle_1F_Text_ReplayRegistered:
	.string "You are registered, {PLAYER}!\p"
	.string "Allow me to restore your POKéMON.$"

MegaIsland_Castle_1F_Text_ReplayGoToTheArena:
	.string "The arena is on the top floor.\n"
	.string "Your opponent is waiting.$"

MegaIsland_Castle_1F_Text_ReplayTomorrow:
	.string "Today's tournament is over.\n"
	.string "Come back tomorrow for the next!$"

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

MegaIsland_Castle_1F_Text_MrMWaits:
	.string "MR. M. is waiting for you in the\n"
	.string "arena, whenever you are ready.$"
