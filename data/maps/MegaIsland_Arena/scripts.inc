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
@   quarterfinals: PLAYER - ALDER, RIVAL - CYNTHIA, LANCE - DIANTHA, STEVEN - LEON
@   semifinals:    PLAYER - RIVAL, DIANTHA - LEON
@   final:         PLAYER - LEON
@ Losing: the player wakes up in bed, it was a dream, and the tournament starts again.
MegaIsland_Arena_OnTransition:
	setvar VAR_TEMP_0, 0
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setflag FLAG_TEMP_3
	call_if_eq VAR_MEGA_STORY_STATE, 7, MegaIsland_Arena_EventScript_ShowMrM
	goto_if_lt VAR_MEGA_STORY_STATE, 4, MegaIsland_Arena_OnTransitionEnd
	goto_if_gt VAR_MEGA_STORY_STATE, 6, MegaIsland_Arena_OnTransitionEnd
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	setvar VAR_TEMP_0, 1
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_ALDER
	call_if_eq VAR_MEGA_STORY_STATE, 5, MegaIsland_Arena_SetOpponentRival
	call_if_eq VAR_MEGA_STORY_STATE, 6, MegaIsland_Arena_SetOpponentLeon
MegaIsland_Arena_OnTransitionEnd:
	end

@ After the final MR. M. waits on his dais for the champion (state 7).
MegaIsland_Arena_EventScript_ShowMrM:
	clearflag FLAG_TEMP_2
	return

MegaIsland_Arena_SetOpponentRival:
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_KANTO_BLUE
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
	msgbox MegaIsland_Arena_Text_AlderIntro
	closemessage
	special RemoveCameraObject
	trainerbattle_earlyrival TRAINER_TOURNAMENT_ALDER, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_AlderDefeat, MegaIsland_Arena_Text_AlderVictory
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_Lost
	call MegaIsland_Arena_EventScript_CameraOnField
	msgbox MegaIsland_Arena_Text_AlderAfter
	call MegaIsland_Arena_EventScript_AlderGiveStone
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
	msgbox MegaIsland_Arena_Text_RivalIntro
	closemessage
	special RemoveCameraObject
	switch VAR_STARTER_MON
	case 0, MegaIsland_Arena_EventScript_BattleRivalCharmander
	case 1, MegaIsland_Arena_EventScript_BattleRivalBulbasaur
	case 2, MegaIsland_Arena_EventScript_BattleRivalSquirtle
	end

@ The rival has the starter that beats the player's, as in FRLG
MegaIsland_Arena_EventScript_BattleRivalCharmander::
	trainerbattle_earlyrival TRAINER_TOURNAMENT_BLUE_CHARMANDER, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_RivalDefeat, MegaIsland_Arena_Text_RivalVictory
	goto MegaIsland_Arena_EventScript_AfterRival
	end

MegaIsland_Arena_EventScript_BattleRivalBulbasaur::
	trainerbattle_earlyrival TRAINER_TOURNAMENT_BLUE_BULBASAUR, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_RivalDefeat, MegaIsland_Arena_Text_RivalVictory
	goto MegaIsland_Arena_EventScript_AfterRival
	end

MegaIsland_Arena_EventScript_BattleRivalSquirtle::
	trainerbattle_earlyrival TRAINER_TOURNAMENT_BLUE_SQUIRTLE, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_RivalDefeat, MegaIsland_Arena_Text_RivalVictory
	goto MegaIsland_Arena_EventScript_AfterRival
	end

MegaIsland_Arena_EventScript_AfterRival::
	goto_if_eq VAR_RESULT, TRUE, MegaIsland_Arena_EventScript_Lost
	call MegaIsland_Arena_EventScript_CameraOnField
	msgbox MegaIsland_Arena_Text_RivalAfter
	call MegaIsland_Arena_EventScript_RivalGiveStone
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
	trainerbattle_earlyrival TRAINER_TOURNAMENT_LEON, RIVAL_BATTLE_HEAL_AFTER, MegaIsland_Arena_Text_LeonDefeat, MegaIsland_Arena_Text_LeonVictory
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
	.string "ALDER of UNOVA!$"

MegaIsland_Arena_Text_AlderIntro:
	.string "ALDER: Ha ha! Let's see what you've\n"
	.string "got, youngster!\p"
	.string "Don't hold anything back!$"

MegaIsland_Arena_Text_AlderDefeat:
	.string "Splendid! Your bond is the real\n"
	.string "thing!$"

MegaIsland_Arena_Text_AlderVictory:
	.string "A fine battle!\n"
	.string "Come back stronger, youngster.$"

MegaIsland_Arena_Text_AlderAfter:
	.string "ALDER: Ha ha ha! I haven't had a\n"
	.string "battle like that in years!\p"
	.string "Go on and win the whole thing!$"

MegaIsland_Arena_Text_QuarterfinalResults:
	.string "MR. M: {PLAYER} advances to the\n"
	.string "semifinals.\p"
	.string "In the other quarterfinals,\n"
	.string "{RIVAL} defeated CYNTHIA,\p"
	.string "DIANTHA defeated LANCE,\n"
	.string "and LEON defeated STEVEN.\p"
	.string "Rest your POKéMON, {PLAYER}.\n"
	.string "The semifinals will follow.$"

MegaIsland_Arena_Text_SemifinalIntro:
	.string "MR. M: The first semifinal:\n"
	.string "{PLAYER} against {RIVAL},\l"
	.string "both of KANTO!$"

MegaIsland_Arena_Text_RivalIntro:
	.string "{RIVAL}: Heh! Of course it comes\n"
	.string "down to us again, {PLAYER}!\p"
	.string "This time, I'm taking it all!$"

MegaIsland_Arena_Text_RivalDefeat:
	.string "What?! Not again!$"

MegaIsland_Arena_Text_RivalVictory:
	.string "Hah! Smell ya later!$"

MegaIsland_Arena_Text_RivalAfter:
	.string "{RIVAL}: …Fine. You win this one.\p"
	.string "But you'd better win the final,\n"
	.string "or I'll never let you hear the end\l"
	.string "of it!$"

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

@ Added: each opponent the player beats gives them the MEGA STONE of a POKéMON in their party, once
@ (a lost tournament starts again from the quarterfinal). With a full BAG it goes to the PC, as
@ the tournament moves on.
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
