MegaIsland_Cave_MapScripts::
	.byte 0

MegaIsland_Cave_EventScript_ItemMaxRevive::
	finditem ITEM_MAX_REVIVE
	end

MegaIsland_Cave_EventScript_Hiker::
	msgbox MegaIsland_Cave_Text_Hiker, MSGBOX_NPC
	end

MegaIsland_Cave_Text_Hiker:
	.string "This cave doesn't go any deeper.\p"
	.string "But sometimes the walls glow, like\n"
	.string "the stones the champions carry.\p"
	.string "There must be more of them buried\n"
	.string "somewhere on this island…$"
