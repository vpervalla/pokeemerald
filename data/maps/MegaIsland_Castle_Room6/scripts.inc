MegaIsland_Castle_Room6_MapScripts::
	.byte 0

MegaIsland_Castle_Room6_EventScript_Diantha::
	lock
	faceplayer
	msgbox MegaIsland_Castle_Room6_Text_Diantha
	cardduel CARD_OPPONENT_DIANTHA
	release
	end

MegaIsland_Castle_Room6_Text_Diantha:
	.string "DIANTHA: Oh! You must be KANTO's\n"
	.string "new CHAMPION. I'm DIANTHA.\p"
	.string "I'm the CHAMPION of KALOS, and an\n"
	.string "actress, too.\p"
	.string "My GARDEVOIR and I have always\n"
	.string "wanted to shine a little brighter.\p"
	.string "Perhaps this stone is the answer.$"
