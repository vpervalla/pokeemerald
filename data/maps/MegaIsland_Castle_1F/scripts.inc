MegaIsland_Castle_1F_MapScripts::
	.byte 0

MegaIsland_Castle_1F_EventScript_Receptionist::
	msgbox MegaIsland_Castle_1F_Text_Welcome, MSGBOX_NPC
	end

MegaIsland_Castle_1F_Text_Welcome:
	.string "Welcome to the castle.\p"
	.string "The court lies beyond these doors.\n"
	.string "The guests' rooms are upstairs.$"

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
