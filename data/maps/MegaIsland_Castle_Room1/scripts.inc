MegaIsland_Castle_Room1_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, MegaIsland_Castle_Room1_OnFrame
	.byte 0

@ The player's guest room. After registering (VAR_MEGA_STORY_STATE 3) the player sleeps here and
@ the tournament begins the next morning (4). After losing a match they wake up here (8): it was
@ all a dream, and the tournament begins again.
MegaIsland_Castle_Room1_OnFrame:
	map_script_2 VAR_MEGA_STORY_STATE, 8, MegaIsland_Castle_Room1_EventScript_WakeFromDream
	.2byte 0

MegaIsland_Castle_Room1_EventScript_Bed::
	lockall
	goto_if_ne VAR_MEGA_STORY_STATE, 3, MegaIsland_Castle_Room1_EventScript_NotSleepy
	msgbox MegaIsland_Castle_Room1_Text_GoToSleep, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, MegaIsland_Castle_Room1_EventScript_BedEnd
	closemessage
	call MegaIsland_Castle_Room1_EventScript_Sleep
	setvar VAR_MEGA_STORY_STATE, 4
	msgbox MegaIsland_Castle_Room1_Text_Morning
	releaseall
	end

MegaIsland_Castle_Room1_EventScript_NotSleepy::
	msgbox MegaIsland_Castle_Room1_Text_SoftBed
MegaIsland_Castle_Room1_EventScript_BedEnd::
	releaseall
	end

MegaIsland_Castle_Room1_EventScript_Sleep::
	fadescreen FADE_TO_BLACK
	playfanfare MUS_RG_HEAL
	waitfanfare
	special HealPlayerParty
	delay 30
	fadescreen FADE_FROM_BLACK
	return

MegaIsland_Castle_Room1_EventScript_WakeFromDream::
	lockall
	setvar VAR_MEGA_STORY_STATE, 4
	special HealPlayerParty
	msgbox MegaIsland_Castle_Room1_Text_WasADream
	releaseall
	end

MegaIsland_Castle_Room1_Text_GoToSleep:
	.string "A soft, warm bed.\n"
	.string "Go to sleep for the night?$"

MegaIsland_Castle_Room1_Text_Morning:
	.string "{PLAYER} slept soundly…\p"
	.string "It's morning! Fanfares are ringing\n"
	.string "from the court.\p"
	.string "The tournament is about to begin!$"

MegaIsland_Castle_Room1_Text_SoftBed:
	.string "A soft, warm bed.$"

MegaIsland_Castle_Room1_Text_WasADream:
	.string "{PLAYER} woke up with a start!\p"
	.string "…It was all a dream.\p"
	.string "It's morning, and the tournament is\n"
	.string "about to begin!$"
