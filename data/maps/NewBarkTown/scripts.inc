NewBarkTown_MapScripts::
	.byte 0

NewBarkTown_EventScript_TownSign::
	msgbox NewBarkTown_Text_TownSign, MSGBOX_SIGN
	end

NewBarkTown_EventScript_LabSign::
	msgbox NewBarkTown_Text_LabSign, MSGBOX_SIGN
	end

NewBarkTown_EventScript_HouseSign::
	msgbox NewBarkTown_Text_HouseSign, MSGBOX_SIGN
	end

@ The way back for now: the town has no route yet, so the man by the west road takes the
@ player back to the TEST AREA, where the man who brought them here stands.
NewBarkTown_EventScript_Guide::
	lock
	faceplayer
	msgbox NewBarkTown_Text_GoBack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, NewBarkTown_EventScript_GuideDone
	closemessage
	warp MAP_TEST_AREA, 16, 7
	waitstate
NewBarkTown_EventScript_GuideDone::
	release
	end

NewBarkTown_Text_TownSign:
	.string "NEW BARK TOWN\n"
	.string "The town where the winds of a\l"
	.string "new beginning blow$"

NewBarkTown_Text_LabSign:
	.string "ELM POKéMON LAB$"

NewBarkTown_Text_HouseSign:
	.string "ELM'S HOUSE$"

NewBarkTown_Text_GoBack:
	.string "The road west isn't open yet.\n"
	.string "Want to go back to the TEST AREA?$"
