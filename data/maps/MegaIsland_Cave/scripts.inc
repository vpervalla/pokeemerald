MegaIsland_Cave_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MegaIsland_Cave_OnLoad
	.byte 0

@ The way out from MEWTWO's sanctum: the ladder up from MegaIsland_LedgeRoom comes out here. Until the
@ passage behind MEWTWO has opened, a boulder sits on the ladder.
MegaIsland_Cave_OnLoad:
	call_if_set FLAG_OPENED_MEWTWO_PASSAGE, MegaIsland_Cave_EventScript_UncoverLadder
	end

MegaIsland_Cave_EventScript_UncoverLadder::
	setmetatile 5, 5, METATILE_KantoCave_LadderDown, FALSE
	return

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
	.string "This cave doesn't go any deeper.\p"
	.string "But sometimes the walls glow, like\n"
	.string "the stones the champions carry.\p"
	.string "There must be more of them buried\n"
	.string "somewhere on this island…$"

MegaIsland_Cave_Text_HikerLadder:
	.string "Whoa! You came up out of the ground!\p"
	.string "That boulder sat there for as long\n"
	.string "as I've been coming here.\p"
	.string "And there was a ladder under it\n"
	.string "the whole time…$"
