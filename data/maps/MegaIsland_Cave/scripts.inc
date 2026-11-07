MegaIsland_Cave_MapScripts::
	.byte 0

@ The ladder leads down to MegaIsland_LedgeRoom, below its ledge: a dead end from here, and the way
@ back up from MEWTWO's sanctum once the passage behind MEWTWO has opened.
MegaIsland_Cave_EventScript_ItemMaxRevive::
	finditem ITEM_MAX_REVIVE
	end

MegaIsland_Cave_EventScript_Hiker::
	goto_if_set FLAG_OPENED_MEWTWO_PASSAGE, MegaIsland_Cave_EventScript_HikerLadder
	msgbox MegaIsland_Cave_Text_Hiker, MSGBOX_NPC
	end

MegaIsland_Cave_EventScript_HikerLadder::
	msgbox MegaIsland_Cave_Text_HikerLadder, MSGBOX_NPC
	end

MegaIsland_Cave_Text_Hiker:
	.string "This cave doesn't go anywhere.\p"
	.string "There's a ladder, but the room down\n"
	.string "there is a dead end. A ledge too\l"
	.string "high to climb blocks the way.\p"
	.string "But sometimes the walls glow, like\n"
	.string "the stones the champions carry.\p"
	.string "There must be more of them buried\n"
	.string "somewhere on this island…$"

MegaIsland_Cave_Text_HikerLadder:
	.string "Whoa! Where did you come from?\p"
	.string "I've been here all along, and I\n"
	.string "never saw you go down that ladder!\p"
	.string "The room down there is a dead end.\n"
	.string "How did you get in?$"
