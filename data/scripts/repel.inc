EventScript_RepelWoreOff::
	lockall
	specialvar VAR_RESULT, BufferLastRepelCount
	goto_if_eq VAR_RESULT, 0, EventScript_RepelWoreOffNoneLeft
	msgbox Text_RepelWoreOffUseAnother, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RepelWoreOffEnd
	special UseLastRepel
	playse SE_REPEL
	waitse
	msgbox Text_UsedAnotherRepel, MSGBOX_DEFAULT
	releaseall
	end

EventScript_RepelWoreOffNoneLeft:
	msgbox Text_RepelWoreOff, MSGBOX_DEFAULT
EventScript_RepelWoreOffEnd:
	releaseall
	end

Text_RepelWoreOff:
	.string "REPEL's effect wore off…$"

Text_RepelWoreOffUseAnother:
	.string "REPEL's effect wore off…\p"
	.string "Use another {STR_VAR_1}?\n"
	.string "You have {STR_VAR_2} left.$"

Text_UsedAnotherRepel:
	.string "{PLAYER} used the\n"
	.string "{STR_VAR_1}.$"
