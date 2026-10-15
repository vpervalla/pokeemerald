MegaIsland_Arena_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Arena_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, MegaIsland_Arena_OnFrame
	.byte 0

@ MR. M.'s tournament. The player registers with the receptionist in the hall
@ (VAR_MEGA_STORY_STATE 3) and sleeps in their guest room (4); then each time they come up to the
@ arena, on the castle's top floor, with a match to play, they walk from the entrance to the south
@ end of the battlefield and battle the opponent at the north end, with MR. M. watching from his red
@ mat between the statues.
@ Bracket:
@   quarterfinals: PLAYER - STEVEN, CYNTHIA - RIVAL, LANCE - DIANTHA, ALDER - LEON
@   semifinals:    PLAYER - CYNTHIA, DIANTHA - LEON
@   final:         PLAYER - LEON
@ Losing: the player wakes up in bed, it was a dream, and the tournament starts again.
@ After the story, the tournament is replayed each day with random opponents (see the end of this file).
MegaIsland_Arena_OnTransition:
	setvar VAR_TEMP_0, 0
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	call_if_eq VAR_MEGA_STORY_STATE, 7, MegaIsland_Arena_EventScript_ShowMrM
	goto_if_ge VAR_MEGA_STORY_STATE, 9, MegaIsland_Arena_OnTransitionReplay
	goto_if_lt VAR_MEGA_STORY_STATE, 4, MegaIsland_Arena_OnTransitionEnd
	goto_if_gt VAR_MEGA_STORY_STATE, 6, MegaIsland_Arena_OnTransitionEnd
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	setvar VAR_TEMP_0, 1
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_STEVEN
	call_if_eq VAR_MEGA_STORY_STATE, 5, MegaIsland_Arena_SetOpponentCynthia
	call_if_eq VAR_MEGA_STORY_STATE, 6, MegaIsland_Arena_SetOpponentLeon
MegaIsland_Arena_OnTransitionEnd:
	end

@ After the final MR. M. waits on his dais for the champion (state 7).
MegaIsland_Arena_EventScript_ShowMrM:
	clearflag FLAG_TEMP_2
	return

MegaIsland_Arena_SetOpponentCynthia:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_CYNTHIA
	return

MegaIsland_Arena_SetOpponentLeon:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_LEON
	return

MegaIsland_Arena_OnFrame:
	map_script_2 VAR_TEMP_0, 1, MegaIsland_Arena_EventScript_Match
	.2byte 0

MegaIsland_Arena_EventScript_Match::
	lockall
	setvar VAR_TEMP_0, 2
	getplayerxy VAR_0x8004, VAR_0x8005
	call_if_eq VAR_0x8004, 10, MegaIsland_Arena_EventScript_StepRight
	call_if_eq VAR_0x8004, 12, MegaIsland_Arena_EventScript_StepLeft
	applymovement LOCALID_PLAYER, MegaIsland_Arena_Movement_PlayerToTrainerBox
	waitmovement 0
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToMrM
	waitmovement 0
	goto_if_ge VAR_MEGA_STORY_STATE, 9, MegaIsland_Arena_EventScript_ReplayMatch
	switch VAR_MEGA_STORY_STATE
	case 4, MegaIsland_Arena_EventScript_Quarterfinal
	case 5, MegaIsland_Arena_EventScript_Semifinal
	case 6, MegaIsland_Arena_EventScript_Final
	releaseall
	end

MegaIsland_Arena_EventScript_StepRight::
	applymovement LOCALID_PLAYER, MegaIsland_Arena_Movement_StepRight
	waitmovement 0
	return

MegaIsland_Arena_EventScript_StepLeft::
	applymovement LOCALID_PLAYER, MegaIsland_Arena_Movement_StepLeft
	waitmovement 0
	return

MegaIsland_Arena_EventScript_Quarterfinal::
	msgbox MegaIsland_Arena_Text_QuarterfinalIntro
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToField
	waitmovement 0
	msgbox MegaIsland_Arena_Text_StevenIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_STORY_STEVEN, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_StevenDefeat, MegaIsland_Arena_Text_StevenVictory
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_Lost
	call MegaIsland_Arena_EventScript_CameraOnField
	msgbox MegaIsland_Arena_Text_StevenAfter
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraFieldToMrM
	waitmovement 0
	msgbox MegaIsland_Arena_Text_QuarterfinalResults
	goto MegaIsland_Arena_EventScript_Won
	end

MegaIsland_Arena_EventScript_Semifinal::
	msgbox MegaIsland_Arena_Text_SemifinalIntro
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToField
	waitmovement 0
	msgbox MegaIsland_Arena_Text_CynthiaIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_STORY_CYNTHIA, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_CynthiaDefeat, MegaIsland_Arena_Text_CynthiaVictory
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_Lost
	call MegaIsland_Arena_EventScript_CameraOnField
	msgbox MegaIsland_Arena_Text_CynthiaAfter
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraFieldToMrM
	waitmovement 0
	msgbox MegaIsland_Arena_Text_SemifinalResults
	goto MegaIsland_Arena_EventScript_Won
	end

MegaIsland_Arena_EventScript_Final::
	msgbox MegaIsland_Arena_Text_FinalIntro
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToField
	waitmovement 0
	msgbox MegaIsland_Arena_Text_LeonIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_STORY_LEON, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_LeonDefeat, MegaIsland_Arena_Text_LeonVictory
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_Lost
	call MegaIsland_Arena_EventScript_CameraOnField
	msgbox MegaIsland_Arena_Text_LeonAfter
	call MegaIsland_Arena_EventScript_LeonGiveStone
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraFieldToMrM
	waitmovement 0
	msgbox MegaIsland_Arena_Text_TournamentWon
	goto MegaIsland_Arena_EventScript_Won
	end

MegaIsland_Arena_EventScript_Won::
	addvar VAR_MEGA_STORY_STATE, 1
	closemessage
	special RemoveCameraObject
	goto MegaIsland_Arena_EventScript_BackToHall
	end

@ Losing: the player wakes up in their guest room. It was all a dream, and the tournament starts
@ again from the quarterfinals (see MegaIsland_Castle_Room1).
MegaIsland_Arena_EventScript_Lost::
	special RemoveCameraObject
	setvar VAR_MEGA_STORY_STATE, 8
	fadescreen FADE_TO_WHITE
	warpsilent MAP_MEGA_ISLAND_CASTLE_ROOM1, 2, 5
	waitstate
	releaseall
	end

MegaIsland_Arena_EventScript_BackToHall::
	warp MAP_MEGA_ISLAND_CASTLE_1F, 11, 7
	waitstate
	releaseall
	end

@ After a battle the camera is back on the player: move it up to show both trainers again
MegaIsland_Arena_EventScript_CameraOnField::
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToField2
	waitmovement 0
	return

MegaIsland_Arena_Movement_CameraToMrM:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

MegaIsland_Arena_Movement_CameraToField:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

MegaIsland_Arena_Movement_CameraToField2:
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

MegaIsland_Arena_Movement_CameraFieldToMrM:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

MegaIsland_Arena_Movement_StepRight:
	walk_right
	step_end

MegaIsland_Arena_Movement_StepLeft:
	walk_left
	step_end

MegaIsland_Arena_Movement_PlayerToTrainerBox:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

MegaIsland_Arena_Text_QuarterfinalIntro:
	.string "MR. M: Welcome, champions, to my\n"
	.string "island.\p"
	.string "Eight trainers. Eight regions.\n"
	.string "Each of you holds a MEGA STONE.\p"
	.string "Let us see whose bond with their\n"
	.string "POKéMON is the strongest.\p"
	.string "The first quarterfinal:\n"
	.string "{PLAYER} of KANTO against\l"
	.string "STEVEN of HOENN!$"

MegaIsland_Arena_Text_StevenIntro:
	.string "STEVEN: So we meet on the field,\n"
	.string "{PLAYER}.\p"
	.string "My METAGROSS and I are curious\n"
	.string "about this new power. Show us!$"

MegaIsland_Arena_Text_StevenDefeat:
	.string "Your bond is stronger than\n"
	.string "any stone.$"

MegaIsland_Arena_Text_StevenVictory:
	.string "A good battle.\n"
	.string "Come back stronger!$"

MegaIsland_Arena_Text_StevenAfter:
	.string "STEVEN: I came for a rare stone,\n"
	.string "and found a rare trainer.\p"
	.string "Go on. Win this for both of us!$"

MegaIsland_Arena_Text_QuarterfinalResults:
	.string "MR. M: {PLAYER} advances to the\n"
	.string "semifinals.\p"
	.string "In the other quarterfinals,\n"
	.string "CYNTHIA defeated {RIVAL},\p"
	.string "DIANTHA defeated LANCE,\n"
	.string "and LEON defeated ALDER.\p"
	.string "Rest your POKéMON, {PLAYER}.\n"
	.string "The semifinals will follow.$"

MegaIsland_Arena_Text_SemifinalIntro:
	.string "MR. M: The first semifinal:\n"
	.string "{PLAYER} of KANTO against\l"
	.string "CYNTHIA of SINNOH!$"

MegaIsland_Arena_Text_CynthiaIntro:
	.string "CYNTHIA: The old myths spoke of\n"
	.string "this power…\p"
	.string "Now let's see whose bond can\n"
	.string "truly master it!$"

MegaIsland_Arena_Text_CynthiaDefeat:
	.string "That was wonderful…\n"
	.string "You've earned the final.$"

MegaIsland_Arena_Text_CynthiaVictory:
	.string "You'll be even stronger\n"
	.string "next time.$"

MegaIsland_Arena_Text_CynthiaAfter:
	.string "CYNTHIA: Your POKéMON trust you\n"
	.string "completely. I could feel it.\p"
	.string "Leon is waiting. Show him what\n"
	.string "you showed me!$"

MegaIsland_Arena_Text_SemifinalResults:
	.string "MR. M: {PLAYER} advances to the\n"
	.string "final.\p"
	.string "In the other semifinal,\n"
	.string "LEON defeated DIANTHA.\p"
	.string "Rest well, {PLAYER}.\n"
	.string "The final awaits.$"

MegaIsland_Arena_Text_FinalIntro:
	.string "MR. M: The final.\p"
	.string "{PLAYER} of KANTO against LEON\n"
	.string "of GALAR, the unbeaten CHAMPION!$"

MegaIsland_Arena_Text_LeonIntro:
	.string "LEON: So it's you and me!\p"
	.string "My CHARIZARD and I have never lost\n"
	.string "an official match.\p"
	.string "Let's make this a champion time!$"

MegaIsland_Arena_Text_LeonDefeat:
	.string "That was the best battle of my\n"
	.string "life!$"

MegaIsland_Arena_Text_LeonVictory:
	.string "What a champion time!\n"
	.string "Come back for a rematch!$"

MegaIsland_Arena_Text_LeonAfter:
	.string "LEON: My first loss…\n"
	.string "and I couldn't be happier!\p"
	.string "You're the real deal, {PLAYER}!$"

MegaIsland_Arena_Text_TournamentWon:
	.string "MR. M: …Magnificent.\p"
	.string "The tournament is yours, {PLAYER}.\n"
	.string "The champion of champions.\p"
	.string "You have earned more than a\n"
	.string "trophy.\p"
	.string "Rest now. When you are ready,\n"
	.string "there is something I must show you.$"


@ After the tournament (state 7) MR. M. reveals that he is MEWTWO, and doesn't battle the player here:
@ he leaves for the cave on the island's route and waits for them there (state 9, see MegaIsland_Cave).
MegaIsland_Arena_EventScript_MrM::
	lock
	faceplayer
	msgbox MegaIsland_Arena_Text_MrMReveal
	closemessage
	playse SE_M_TELEPORT
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_ARENA_MR_M
	clearflag FLAG_TEMP_3
	addobject LOCALID_ARENA_MEWTWO
	fadescreen FADE_FROM_WHITE
	playmoncry SPECIES_MEWTWO, CRY_MODE_ENCOUNTER
	waitmoncry
	msgbox MegaIsland_Arena_Text_MewtwoInvitesToCave
	closemessage
	playse SE_M_TELEPORT
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_ARENA_MEWTWO
	fadescreen FADE_FROM_WHITE
	setvar VAR_MEGA_STORY_STATE, 9
	msgbox MegaIsland_Arena_Text_MewtwoVanished
	release
	end

MegaIsland_Arena_Text_MrMReveal:
	.string "MR. M: You came.\p"
	.string "I have watched every one of your\n"
	.string "battles, {PLAYER}.\p"
	.string "You fight as one with your\n"
	.string "POKéMON. I did not think that\l"
	.string "possible for a human.\p"
	.string "It is time you saw who invited\n"
	.string "you here.$"

MegaIsland_Arena_Text_MewtwoInvitesToCave:
	.string "MEWTWO: …Yes. I am MEWTWO.\p"
	.string "Humans made me. Humans caged me.\n"
	.string "I broke free, and I took the\l"
	.string "stones they hoarded.\p"
	.string "I gathered the strongest trainers\n"
	.string "to learn what they could do\l"
	.string "with this power.\p"
	.string "You surpassed them all.\p"
	.string "Now I will see if you can surpass\n"
	.string "me.\p"
	.string "Not here, before an audience.\p"
	.string "Come to the cave on this island's\n"
	.string "road. Come alone.\p"
	.string "There, I will hold nothing back.$"

MegaIsland_Arena_Text_MewtwoVanished:
	.string "MEWTWO vanished!$"

@ Added: the opponents whose POKéMON can be caught in KANTO give the MEGA STONE of one of them, once:
@ LEON after the story's final, ALDER and the rival the first time they're beaten in the daily
@ tournament. With a full BAG it goes to the PC, as the tournament moves on.
MegaIsland_Arena_EventScript_AlderGiveStone::
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_ALDER, MegaIsland_Arena_EventScript_Return
	msgbox MegaIsland_Arena_Text_AlderGiveStone
	setflag FLAG_GOT_MEGA_STONE_FROM_ALDER
	bufferitemname STR_VAR_2, ITEM_PINSIRITE
	giveitem ITEM_PINSIRITE
	goto_if_eq VAR_RESULT, FALSE, MegaIsland_Arena_EventScript_AlderStoneToPC
	bufferspeciesname STR_VAR_1, SPECIES_PINSIR
	msgbox GymRematch_Text_ExplainStone
	return

MegaIsland_Arena_EventScript_AlderStoneToPC::
	addpcitem ITEM_PINSIRITE
	msgbox MegaIsland_Arena_Text_StoneToPC
	return

MegaIsland_Arena_EventScript_RivalGiveStone::
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_RIVAL, MegaIsland_Arena_EventScript_Return
	msgbox MegaIsland_Arena_Text_RivalGiveStone
	setflag FLAG_GOT_MEGA_STONE_FROM_RIVAL
	bufferitemname STR_VAR_2, ITEM_PIDGEOTITE
	giveitem ITEM_PIDGEOTITE
	goto_if_eq VAR_RESULT, FALSE, MegaIsland_Arena_EventScript_RivalStoneToPC
	bufferspeciesname STR_VAR_1, SPECIES_PIDGEOT
	msgbox GymRematch_Text_ExplainStone
	return

MegaIsland_Arena_EventScript_RivalStoneToPC::
	addpcitem ITEM_PIDGEOTITE
	msgbox MegaIsland_Arena_Text_StoneToPC
	return

MegaIsland_Arena_EventScript_LeonGiveStone::
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_LEON, MegaIsland_Arena_EventScript_Return
	msgbox MegaIsland_Arena_Text_LeonGiveStone
	setflag FLAG_GOT_MEGA_STONE_FROM_LEON
	bufferitemname STR_VAR_2, ITEM_CHARIZARDITE_Y
	giveitem ITEM_CHARIZARDITE_Y
	goto_if_eq VAR_RESULT, FALSE, MegaIsland_Arena_EventScript_LeonStoneToPC
	bufferspeciesname STR_VAR_1, SPECIES_CHARIZARD
	msgbox GymRematch_Text_ExplainStone
	return

MegaIsland_Arena_EventScript_LeonStoneToPC::
	addpcitem ITEM_CHARIZARDITE_Y
	msgbox MegaIsland_Arena_Text_StoneToPC
	return

MegaIsland_Arena_EventScript_Return::
	return

MegaIsland_Arena_Text_AlderGiveStone:
	.string "ALDER: Here, take this.\n"
	.string "The PINSIRITE!\p"
	.string "My PINSIR and I never quite\n"
	.string "mastered it.\p"
	.string "Perhaps you and yours will!$"

MegaIsland_Arena_Text_RivalGiveStone:
	.string "{RIVAL}: And take this. My\n"
	.string "PIDGEOTITE.\p"
	.string "Don't get the wrong idea!\n"
	.string "I'll just win it back later!$"

MegaIsland_Arena_Text_LeonGiveStone:
	.string "LEON: Here, my CHARIZARDITE Y.\p"
	.string "Let a CHARIZARD of yours burn as\n"
	.string "bright as mine did today!$"

MegaIsland_Arena_Text_StoneToPC:
	.string "Your BAG is full, so the\n"
	.string "{STR_VAR_2} was sent to your PC.$"

@ Added: from VAR_MEGA_STORY_STATE 9 on, a new tournament is held each day (see MegaIsland_Castle_1F):
@ three rounds against opponents drawn at random among the eight other champions, ALAIN included
@ (RandomizeTournamentOpponents, GetTournamentOpponent), all at lv 85-90 (the story's tournament uses
@ TRAINER_TOURNAMENT_STORY_* copies of STEVEN, CYNTHIA and LEON at lower levels). The rival (always
@ with SQUIRTLE) and ALDER give their MEGA STONE the first time they're beaten. There is no MR. M. any more, nor any MEGA
@ STONE to win (the tournament ones are given once), and a lost match just ends the day's tournament.
MegaIsland_Arena_OnTransitionReplay:
	goto_if_eq VAR_TOURNAMENT_ROUND, 0, MegaIsland_Arena_OnTransitionEnd
	clearflag FLAG_TEMP_1
	setvar VAR_TEMP_0, 1
	specialvar VAR_RESULT, GetTournamentOpponent
	call_if_eq VAR_RESULT, 0, MegaIsland_Arena_SetReplayOpponentRival
	call_if_eq VAR_RESULT, 1, MegaIsland_Arena_SetReplayOpponentLance
	call_if_eq VAR_RESULT, 2, MegaIsland_Arena_SetReplayOpponentSteven
	call_if_eq VAR_RESULT, 3, MegaIsland_Arena_SetReplayOpponentCynthia
	call_if_eq VAR_RESULT, 4, MegaIsland_Arena_SetReplayOpponentAlder
	call_if_eq VAR_RESULT, 5, MegaIsland_Arena_SetReplayOpponentDiantha
	call_if_eq VAR_RESULT, 6, MegaIsland_Arena_SetReplayOpponentLeon
	call_if_eq VAR_RESULT, 7, MegaIsland_Arena_SetReplayOpponentAlain
	end

MegaIsland_Arena_SetReplayOpponentRival:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_KANTO_BLUE
	return

MegaIsland_Arena_SetReplayOpponentLance:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_KANTO_LANCE
	return

MegaIsland_Arena_SetReplayOpponentSteven:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_STEVEN
	return

MegaIsland_Arena_SetReplayOpponentCynthia:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_CYNTHIA
	return

MegaIsland_Arena_SetReplayOpponentAlder:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_ALDER
	return

MegaIsland_Arena_SetReplayOpponentDiantha:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_DIANTHA
	return

MegaIsland_Arena_SetReplayOpponentLeon:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_LEON
	return

MegaIsland_Arena_SetReplayOpponentAlain:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_ALAIN
	return

MegaIsland_Arena_EventScript_ReplayMatch::
	bufferstring STR_VAR_2, MegaIsland_Arena_Text_ReplayQuarterfinal
	call_if_eq VAR_TOURNAMENT_ROUND, 2, MegaIsland_Arena_EventScript_BufferReplaySemifinal
	call_if_eq VAR_TOURNAMENT_ROUND, 3, MegaIsland_Arena_EventScript_BufferReplayFinal
	specialvar VAR_RESULT, GetTournamentOpponent
	switch VAR_RESULT
	case 0, MegaIsland_Arena_EventScript_ReplayRival
	case 1, MegaIsland_Arena_EventScript_ReplayLance
	case 2, MegaIsland_Arena_EventScript_ReplaySteven
	case 3, MegaIsland_Arena_EventScript_ReplayCynthia
	case 4, MegaIsland_Arena_EventScript_ReplayAlder
	case 5, MegaIsland_Arena_EventScript_ReplayDiantha
	case 6, MegaIsland_Arena_EventScript_ReplayLeon
	case 7, MegaIsland_Arena_EventScript_ReplayAlain
	end

MegaIsland_Arena_EventScript_BufferReplaySemifinal::
	bufferstring STR_VAR_2, MegaIsland_Arena_Text_ReplaySemifinal
	return

MegaIsland_Arena_EventScript_BufferReplayFinal::
	bufferstring STR_VAR_2, MegaIsland_Arena_Text_ReplayFinal
	return

@ The round's intro, then the camera moves down to the battlefield and the opponent speaks
MegaIsland_Arena_EventScript_ReplayRoundIntro::
	msgbox MegaIsland_Arena_Text_ReplayRoundIntro
	closemessage
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToField
	waitmovement 0
	return

MegaIsland_Arena_EventScript_ReplayRival::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayRivalName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayRivalIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_BLUE_SQUIRTLE, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayRivalDefeat, MegaIsland_Arena_Text_ReplayRivalVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayLance::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayLanceName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayLanceIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_LANCE, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayLanceDefeat, MegaIsland_Arena_Text_ReplayLanceVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplaySteven::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayStevenName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayStevenIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_STEVEN, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayStevenDefeat, MegaIsland_Arena_Text_ReplayStevenVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayCynthia::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayCynthiaName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayCynthiaIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_CYNTHIA, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayCynthiaDefeat, MegaIsland_Arena_Text_ReplayCynthiaVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayAlder::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayAlderName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayAlderIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_ALDER, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayAlderDefeat, MegaIsland_Arena_Text_ReplayAlderVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayDiantha::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayDianthaName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayDianthaIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_DIANTHA, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayDianthaDefeat, MegaIsland_Arena_Text_ReplayDianthaVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayLeon::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayLeonName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayLeonIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_LEON, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayLeonDefeat, MegaIsland_Arena_Text_ReplayLeonVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayAlain::
	bufferstring STR_VAR_1, MegaIsland_Arena_Text_ReplayAlainName
	call MegaIsland_Arena_EventScript_ReplayRoundIntro
	msgbox MegaIsland_Arena_Text_ReplayAlainIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_ALAIN, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_ReplayAlainDefeat, MegaIsland_Arena_Text_ReplayAlainVictory
	goto MegaIsland_Arena_EventScript_ReplayAfterBattle
	end

MegaIsland_Arena_EventScript_ReplayAfterBattle::
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_ReplayLost
	call MegaIsland_Arena_EventScript_CameraOnField
	specialvar VAR_RESULT, GetTournamentOpponent
	call_if_eq VAR_RESULT, 0, MegaIsland_Arena_EventScript_ReplayAfterRival
	call_if_eq VAR_RESULT, 1, MegaIsland_Arena_EventScript_ReplayAfterLance
	call_if_eq VAR_RESULT, 2, MegaIsland_Arena_EventScript_ReplayAfterSteven
	call_if_eq VAR_RESULT, 3, MegaIsland_Arena_EventScript_ReplayAfterCynthia
	call_if_eq VAR_RESULT, 4, MegaIsland_Arena_EventScript_ReplayAfterAlder
	call_if_eq VAR_RESULT, 5, MegaIsland_Arena_EventScript_ReplayAfterDiantha
	call_if_eq VAR_RESULT, 6, MegaIsland_Arena_EventScript_ReplayAfterLeon
	call_if_eq VAR_RESULT, 7, MegaIsland_Arena_EventScript_ReplayAfterAlain
	closemessage
	goto_if_eq VAR_TOURNAMENT_ROUND, 3, MegaIsland_Arena_EventScript_ReplayWon
	addvar VAR_TOURNAMENT_ROUND, 1
	msgbox MegaIsland_Arena_Text_ReplayAdvances
	closemessage
	special RemoveCameraObject
	goto MegaIsland_Arena_EventScript_BackToHall
	end

MegaIsland_Arena_EventScript_ReplayWon::
	setvar VAR_TOURNAMENT_ROUND, 0
	msgbox MegaIsland_Arena_Text_ReplayWon
	closemessage
	special RemoveCameraObject
	goto MegaIsland_Arena_EventScript_BackToHall
	end

MegaIsland_Arena_EventScript_ReplayLost::
	setvar VAR_TOURNAMENT_ROUND, 0
	special RemoveCameraObject
	msgbox MegaIsland_Arena_Text_ReplayLost
	closemessage
	goto MegaIsland_Arena_EventScript_BackToHall
	end

MegaIsland_Arena_EventScript_ReplayAfterRival::
	msgbox MegaIsland_Arena_Text_ReplayRivalAfter
	call MegaIsland_Arena_EventScript_RivalGiveStone
	return

MegaIsland_Arena_EventScript_ReplayAfterLance::
	msgbox MegaIsland_Arena_Text_ReplayLanceAfter
	return

MegaIsland_Arena_EventScript_ReplayAfterSteven::
	msgbox MegaIsland_Arena_Text_ReplayStevenAfter
	return

MegaIsland_Arena_EventScript_ReplayAfterCynthia::
	msgbox MegaIsland_Arena_Text_ReplayCynthiaAfter
	return

MegaIsland_Arena_EventScript_ReplayAfterAlder::
	msgbox MegaIsland_Arena_Text_ReplayAlderAfter
	call MegaIsland_Arena_EventScript_AlderGiveStone
	return

MegaIsland_Arena_EventScript_ReplayAfterDiantha::
	msgbox MegaIsland_Arena_Text_ReplayDianthaAfter
	return

MegaIsland_Arena_EventScript_ReplayAfterLeon::
	msgbox MegaIsland_Arena_Text_ReplayLeonAfter
	return

MegaIsland_Arena_EventScript_ReplayAfterAlain::
	msgbox MegaIsland_Arena_Text_ReplayAlainAfter
	return

MegaIsland_Arena_Text_ReplayRivalName:
	.string "{RIVAL}$"

MegaIsland_Arena_Text_ReplayRivalIntro:
	.string "{RIVAL}: You again, {PLAYER}?\p"
	.string "Today, the title is mine!$"

MegaIsland_Arena_Text_ReplayRivalDefeat:
	.string "What?! Not again!$"

MegaIsland_Arena_Text_ReplayRivalVictory:
	.string "Hah! Smell ya later!$"

MegaIsland_Arena_Text_ReplayRivalAfter:
	.string "{RIVAL}: Tch… Next time, {PLAYER}.\p"
	.string "Count on it!$"

MegaIsland_Arena_Text_ReplayLanceName:
	.string "LANCE$"

MegaIsland_Arena_Text_ReplayLanceIntro:
	.string "LANCE: The lot brings us together.\p"
	.string "My dragons are ready!$"

MegaIsland_Arena_Text_ReplayLanceDefeat:
	.string "Magnificent…\n"
	.string "You are truly worthy.$"

MegaIsland_Arena_Text_ReplayLanceVictory:
	.string "Train harder, and face me\n"
	.string "again.$"

MegaIsland_Arena_Text_ReplayLanceAfter:
	.string "LANCE: Your bond with your POKéMON\n"
	.string "grows stronger every day.$"

MegaIsland_Arena_Text_ReplayStevenName:
	.string "STEVEN$"

MegaIsland_Arena_Text_ReplayStevenIntro:
	.string "STEVEN: Let's see how strong you've\n"
	.string "become. I won't go easy!$"

MegaIsland_Arena_Text_ReplayStevenDefeat:
	.string "Your strength is the real\n"
	.string "deal.$"

MegaIsland_Arena_Text_ReplayStevenVictory:
	.string "A good battle.\n"
	.string "Come back stronger!$"

MegaIsland_Arena_Text_ReplayStevenAfter:
	.string "STEVEN: A battle like that is worth\n"
	.string "more than any rare stone.$"

MegaIsland_Arena_Text_ReplayCynthiaName:
	.string "CYNTHIA$"

MegaIsland_Arena_Text_ReplayCynthiaIntro:
	.string "CYNTHIA: A battle with you,\n"
	.string "{PLAYER}…\p"
	.string "I've been looking forward to this.$"

MegaIsland_Arena_Text_ReplayCynthiaDefeat:
	.string "That was wonderful…$"

MegaIsland_Arena_Text_ReplayCynthiaVictory:
	.string "You'll be even stronger\n"
	.string "next time.$"

MegaIsland_Arena_Text_ReplayCynthiaAfter:
	.string "CYNTHIA: Let's battle again\n"
	.string "someday, {PLAYER}.$"

MegaIsland_Arena_Text_ReplayAlderName:
	.string "ALDER$"

MegaIsland_Arena_Text_ReplayAlderIntro:
	.string "ALDER: Ha ha! You again, youngster!\p"
	.string "Let's have a battle to remember!$"

MegaIsland_Arena_Text_ReplayAlderDefeat:
	.string "Splendid! Your bond is the real\n"
	.string "thing!$"

MegaIsland_Arena_Text_ReplayAlderVictory:
	.string "A fine battle!\n"
	.string "Come back stronger, youngster.$"

MegaIsland_Arena_Text_ReplayAlderAfter:
	.string "ALDER: Ha ha ha! Every battle with\n"
	.string "you makes me feel young again!$"

MegaIsland_Arena_Text_ReplayDianthaName:
	.string "DIANTHA$"

MegaIsland_Arena_Text_ReplayDianthaIntro:
	.string "DIANTHA: The stage is set!\p"
	.string "Let's give everyone a show to\n"
	.string "remember!$"

MegaIsland_Arena_Text_ReplayDianthaDefeat:
	.string "Bravo!\n"
	.string "What a performance!$"

MegaIsland_Arena_Text_ReplayDianthaVictory:
	.string "What a lovely battle!\n"
	.string "Thank you!$"

MegaIsland_Arena_Text_ReplayDianthaAfter:
	.string "DIANTHA: You shine brighter than\n"
	.string "any star on the screen!$"

MegaIsland_Arena_Text_ReplayLeonName:
	.string "LEON$"

MegaIsland_Arena_Text_ReplayLeonIntro:
	.string "LEON: A rematch!\p"
	.string "Let's make this a champion time!$"

MegaIsland_Arena_Text_ReplayLeonDefeat:
	.string "Brilliant!\n"
	.string "What a battle!$"

MegaIsland_Arena_Text_ReplayLeonVictory:
	.string "What a champion time!\n"
	.string "Come back for a rematch!$"

MegaIsland_Arena_Text_ReplayLeonAfter:
	.string "LEON: You're the real deal,\n"
	.string "{PLAYER}!$"

MegaIsland_Arena_Text_ReplayAlainName:
	.string "ALAIN$"

MegaIsland_Arena_Text_ReplayAlainIntro:
	.string "ALAIN: So you're {PLAYER}.\p"
	.string "My CHARIZARD and I want to test\n"
	.string "our bond against yours!$"

MegaIsland_Arena_Text_ReplayAlainDefeat:
	.string "So this is the strength of a\n"
	.string "true bond…$"

MegaIsland_Arena_Text_ReplayAlainVictory:
	.string "Our bond won today.\n"
	.string "Train hard, and come back.$"

MegaIsland_Arena_Text_ReplayAlainAfter:
	.string "ALAIN: My CHARIZARD and I will keep\n"
	.string "training until we can beat you.$"

MegaIsland_Arena_Text_ReplayQuarterfinal:
	.string "Today's quarterfinal$"

MegaIsland_Arena_Text_ReplaySemifinal:
	.string "Today's semifinal$"

MegaIsland_Arena_Text_ReplayFinal:
	.string "Today's final$"

MegaIsland_Arena_Text_ReplayRoundIntro:
	.string "{STR_VAR_2}!\n"
	.string "{PLAYER} against {STR_VAR_1}!$"

MegaIsland_Arena_Text_ReplayAdvances:
	.string "{PLAYER} advances to the next\n"
	.string "round!\p"
	.string "Rest your POKéMON in the hall.$"

MegaIsland_Arena_Text_ReplayWon:
	.string "{PLAYER} won today's tournament!\p"
	.string "The champion of champions,\n"
	.string "once again!$"

MegaIsland_Arena_Text_ReplayLost:
	.string "{PLAYER} was eliminated…\p"
	.string "There will be another tournament\n"
	.string "tomorrow!$"
