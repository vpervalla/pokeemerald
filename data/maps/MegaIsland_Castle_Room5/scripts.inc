MegaIsland_Castle_Room5_MapScripts::
	.byte 0

MegaIsland_Castle_Room5_EventScript_Alder::
	lock
	faceplayer
	msgbox MegaIsland_Castle_Room5_Text_Alder
	cardduel CARD_OPPONENT_ALDER
	release
	end

MegaIsland_Castle_Room5_Text_Alder:
	.string "ALDER: Ha ha! A young one!\n"
	.string "I'm ALDER, from UNOVA.\p"
	.string "I came for the battles… and the\n"
	.string "food, of course!\p"
	.string "A stone that makes a POKéMON\n"
	.string "stronger, eh?\p"
	.string "Remember: power is only worth the\n"
	.string "bond behind it.$"
