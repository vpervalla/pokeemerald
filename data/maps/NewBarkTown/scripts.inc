NewBarkTown_MapScripts::
	.byte 0

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

NewBarkTown_Text_GoBack:
	.string "The road west isn't open yet.\n"
	.string "Want to go back to the TEST AREA?$"
