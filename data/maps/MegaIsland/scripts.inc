MegaIsland_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, MegaIsland_OnFrame
	.byte 0

@ After MEWTWO (the passage behind it open, FLAG_OPENED_MEWTWO_PASSAGE: won or caught), LEON and CYNTHIA
@ meet the player on the plateau in front of the island's cave: they followed MR. M. and the player to
@ the arena's hidden door, but it was locked for them. Told about MEWTWO, they explain that the
@ tournament goes on, a new one each day (the receptionist gives its rules); the daily tournament opens
@ (FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, see MegaIsland_Castle_1F).
@ Coming out of the cave (the cave mouth's warp, 3,14), the player sees them come up the plateau's
@ stairs from below the screen. It's the only way out after MEWTWO: the arena's door shuts behind the
@ player too (MegaIsland_HiddenRoom), and there's no escaping from the caves. LEON uses the special NPC
@ palette slot and CYNTHIA its reflection's (OBJ_EVENT_GFX_CHAMPION_CYNTHIA_2), so both fit on the map.
MegaIsland_OnTransition:
	setflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	setvar VAR_TEMP_1, 0
	goto_if_unset FLAG_OPENED_MEWTWO_PASSAGE, MegaIsland_OnTransitionEnd
	goto_if_set FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT, MegaIsland_OnTransitionEnd
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_ne VAR_0x8004, 3, MegaIsland_OnTransitionEnd
	goto_if_ne VAR_0x8005, 14, MegaIsland_OnTransitionEnd
	@ Out of the cave: they start below the screen, at (3,21) and (4,22)
	clearflag FLAG_TEMP_1
	clearflag FLAG_TEMP_2
	setvar VAR_TEMP_1, 1
MegaIsland_OnTransitionEnd:
	end

MegaIsland_OnFrame:
	map_script_2 VAR_TEMP_1, 1, MegaIsland_EventScript_ChampionsApproach
	.2byte 0

MegaIsland_EventScript_ChampionsApproach::
	lockall
	setvar VAR_TEMP_1, 2
	applymovement LOCALID_PLAYER, MegaIsland_Movement_PlayerFaceDown
	waitmovement 0
	delay 16
	msgbox MegaIsland_Text_LeonCalls
	closemessage
	applymovement LOCALID_ROUTE_LEON, MegaIsland_Movement_LeonApproach
	applymovement LOCALID_ROUTE_CYNTHIA, MegaIsland_Movement_CynthiaApproach
	waitmovement 0
	call MegaIsland_EventScript_ChampionsTalk
	@ Back down the stairs, and away
	applymovement LOCALID_ROUTE_LEON, MegaIsland_Movement_LeonLeave
	applymovement LOCALID_ROUTE_CYNTHIA, MegaIsland_Movement_CynthiaLeave
	waitmovement 0
	removeobject LOCALID_ROUTE_LEON
	removeobject LOCALID_ROUTE_CYNTHIA
	releaseall
	end

@ The talk itself: the locked door, MEWTWO, and the tournament that goes on.
MegaIsland_EventScript_ChampionsTalk::
	msgbox MegaIsland_Text_LockedDoor
	msgbox MegaIsland_Text_WhatHappened
	msgbox MegaIsland_Text_PlayerExplains
	msgbox MegaIsland_Text_TheyRealize
	call_if_eq VAR_MEGA_STORY_STATE, 10, MegaIsland_EventScript_MewtwoCaught
	call_if_ne VAR_MEGA_STORY_STATE, 10, MegaIsland_EventScript_MewtwoWaits
	msgbox MegaIsland_Text_TournamentGoesOn
	msgbox MegaIsland_Text_SeeYouThere
	closemessage
	setflag FLAG_CHAMPIONS_EXPLAINED_TOURNAMENT
	return

MegaIsland_EventScript_MewtwoCaught::
	msgbox MegaIsland_Text_MewtwoCaught
	return

MegaIsland_EventScript_MewtwoWaits::
	msgbox MegaIsland_Text_MewtwoWaits
	return

MegaIsland_Movement_PlayerFaceDown:
	face_down
	step_end

@ LEON from (3,21) up the stairs to the player's left, (2,16); CYNTHIA from (4,22) to the right, (4,16).
MegaIsland_Movement_LeonApproach:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_up
	face_up
	step_end

MegaIsland_Movement_CynthiaApproach:
	delay_16
	walk_up
	walk_up
	walk_up
	walk_left
	walk_up
	walk_up
	walk_right
	walk_up
	face_up
	step_end

@ From (2,16) and (4,16), down the stairs (3,18) and below the screen.
MegaIsland_Movement_LeonLeave:
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

MegaIsland_Movement_CynthiaLeave:
	delay_16
	delay_16
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

MegaIsland_Text_LeonCalls:
	.string "LEON: There you are, {PLAYER}!$"

MegaIsland_Text_LockedDoor:
	.string "LEON: We saw MR. M. lead you through\n"
	.string "that door behind his dais, so we\l"
	.string "went after you…\p"
	.string "CYNTHIA: But the door wouldn't\n"
	.string "open for us. It was locked tight,\l"
	.string "as if something held it shut.\p"
	.string "LEON: We searched the whole island\n"
	.string "for another way in!\p"
	.string "…I only got lost twice.\p"
	.string "CYNTHIA: And then you walk out of\n"
	.string "this little cave.$"

MegaIsland_Text_WhatHappened:
	.string "CYNTHIA: {PLAYER}, what happened\n"
	.string "after you went through that door?$"

MegaIsland_Text_PlayerExplains:
	.string "{PLAYER} told LEON and CYNTHIA about\n"
	.string "the cave under the castle, and the\l"
	.string "POKéMON waiting there…$"

MegaIsland_Text_TheyRealize:
	.string "CYNTHIA: MEWTWO…\p"
	.string "Then MR. M. was MEWTWO all along.\p"
	.string "It explains the MEGA STONES, and\n"
	.string "how it knew every one of us.\p"
	.string "LEON: The letters, the castle, the\n"
	.string "whole tournament…\p"
	.string "It wanted to see what trainers and\n"
	.string "POKéMON can do together.\p"
	.string "And then it wanted to test the best\n"
	.string "of us itself!$"

MegaIsland_Text_MewtwoCaught:
	.string "CYNTHIA: And now it's come with you.\p"
	.string "It chose its trainer well.$"

MegaIsland_Text_MewtwoWaits:
	.string "LEON: And it's still waiting for you\n"
	.string "down there?\p"
	.string "Ha! Then you'll just have to go back\n"
	.string "and win it over!$"

MegaIsland_Text_TournamentGoesOn:
	.string "LEON: Well! We champions have been\n"
	.string "talking.\p"
	.string "None of us wants to go home just yet.\p"
	.string "So the tournament goes on! A new one\n"
	.string "every day, here in the castle.$"

MegaIsland_Text_SeeYouThere:
	.string "CYNTHIA: Register at the desk in\n"
	.string "the castle hall when you're ready.\p"
	.string "LEON: We'll be waiting.\n"
	.string "Don't keep us waiting too long!$"
