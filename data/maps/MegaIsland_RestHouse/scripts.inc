MegaIsland_RestHouse_MapScripts::
	.byte 0

@ The rest house on the route to the castle: the woman restores the player's team.
MegaIsland_RestHouse_EventScript_Woman::
	lock
	faceplayer
	msgbox MegaIsland_RestHouse_Text_TakeARest
	closemessage
	call Kanto_EventScript_OutOfCenterPartyHeal
	msgbox MegaIsland_RestHouse_Text_AllRested
	release
	end

MegaIsland_RestHouse_EventScript_OldMan::
	msgbox MegaIsland_RestHouse_Text_OldMan, MSGBOX_NPC
	end

MegaIsland_RestHouse_Text_TakeARest:
	.string "Welcome to the rest house.\n"
	.string "You've come a long way, haven't you?\p"
	.string "Please, let your POKéMON rest.$"

MegaIsland_RestHouse_Text_AllRested:
	.string "There! Your POKéMON look full of\n"
	.string "energy again. Good luck!$"

MegaIsland_RestHouse_Text_OldMan:
	.string "This island had no name on any map\n"
	.string "until a few months ago.\p"
	.string "Then the castle appeared, and the\n"
	.string "gentleman who lives in it.\p"
	.string "Nobody saw it being built.\n"
	.string "Odd, isn't it?$"
