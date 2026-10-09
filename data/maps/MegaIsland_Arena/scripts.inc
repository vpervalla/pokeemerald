MegaIsland_Arena_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_Arena_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, MegaIsland_Arena_OnFrame
	.byte 0

@ MR. M.'s tournament. The player registers with the receptionist in the hall
@ (VAR_MEGA_STORY_STATE 3), then each time they come into the court with a match to play, they
@ walk to the south trainer box and battle the opponent in the north one, with MR. M. watching
@ from his box. Bracket:
@   quarterfinals: PLAYER - ALDER, RIVAL - CYNTHIA, LANCE - DIANTHA, STEVEN - LEON
@   semifinals:    PLAYER - RIVAL, DIANTHA - LEON
@   final:         PLAYER - LEON
@ Losing heals the team and sends the player back to the hall to try again.
MegaIsland_Arena_OnTransition:
	setvar VAR_TEMP_0, 0
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	goto_if_lt VAR_MEGA_STORY_STATE, 3, MegaIsland_Arena_OnTransitionEnd
	goto_if_gt VAR_MEGA_STORY_STATE, 5, MegaIsland_Arena_OnTransitionEnd
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	setvar VAR_TEMP_0, 1
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_CHAMPION_ALDER
	call_if_eq VAR_MEGA_STORY_STATE, 4, MegaIsland_Arena_SetOpponentRival
	call_if_eq VAR_MEGA_STORY_STATE, 5, MegaIsland_Arena_SetOpponentLeon
MegaIsland_Arena_OnTransitionEnd:
	end

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
	call_if_eq VAR_0x8004, 15, MegaIsland_Arena_EventScript_PlayerToMiddle
	applymovement LOCALID_PLAYER, MegaIsland_Arena_Movement_PlayerToTrainerBox
	waitmovement 0
	special SpawnCameraObject
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraToMrM
	waitmovement 0
	switch VAR_MEGA_STORY_STATE
	case 3, MegaIsland_Arena_EventScript_Quarterfinal
	case 4, MegaIsland_Arena_EventScript_Semifinal
	case 5, MegaIsland_Arena_EventScript_Final
	releaseall
	end

MegaIsland_Arena_EventScript_PlayerToMiddle::
	applymovement LOCALID_PLAYER, MegaIsland_Arena_Movement_StepRight
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

MegaIsland_Arena_EventScript_Lost::
	call MegaIsland_Arena_EventScript_CameraOnField
	applymovement LOCALID_CAMERA, MegaIsland_Arena_Movement_CameraFieldToMrM
	waitmovement 0
	msgbox MegaIsland_Arena_Text_Lost
	closemessage
	special RemoveCameraObject
MegaIsland_Arena_EventScript_BackToHall::
	warp MAP_MEGA_ISLAND_CASTLE_1F, 12, 2
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
	step_end

MegaIsland_Arena_Movement_CameraToField:
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
	step_end

MegaIsland_Arena_Movement_StepRight:
	walk_right
	step_end

MegaIsland_Arena_Movement_PlayerToTrainerBox:
	walk_up
	walk_up
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

MegaIsland_Arena_Text_Lost:
	.string "MR. M: A fine battle.\p"
	.string "Rest your POKéMON, and return when\n"
	.string "you are ready to try again.$"
