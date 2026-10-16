MegaIsland_Castle_Room4_MapScripts::
	.byte 0

MegaIsland_Castle_Room4_EventScript_Cynthia::
	lock
	faceplayer
	msgbox MegaIsland_Castle_Room4_Text_Cynthia
	cardduel CARD_OPPONENT_CYNTHIA
	release
	end

MegaIsland_Castle_Room4_Text_Cynthia:
	.string "CYNTHIA: I'm CYNTHIA, the CHAMPION\n"
	.string "of SINNOH.\p"
	.string "Old myths speak of POKéMON that\n"
	.string "could go beyond their limits.\p"
	.string "His letter knew things about my team\n"
	.string "that nobody should know.\p"
	.string "MR. M.… I wonder who he really is.$"
