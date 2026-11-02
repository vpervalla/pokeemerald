@ Interact with cuttable tree
EventScript_CutTree::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_CUT
	specialvar VAR_RESULT, CheckBadgeForFieldMove
	goto_if_eq VAR_RESULT, FALSE, EventScript_CheckTreeCantCut
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CheckTreeCantCut
	setfieldeffectargument 0, VAR_RESULT
	buffermovename STR_VAR_2, MOVE_CUT
	msgbox Text_WantToCut, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_CancelCut
	call EventScript_ShowFieldMoveUser
	closemessage
	dofieldeffect FLDEFF_USE_CUT_ON_TREE
	waitstate
	goto EventScript_CutTreeDown
	end

@ Use cut from party menu
EventScript_UseCut::
	lockall
	dofieldeffect FLDEFF_USE_CUT_ON_TREE
	waitstate
	goto EventScript_CutTreeDown
	end

EventScript_CutTreeDown::
	applywaitmovement VAR_LAST_TALKED, Movement_CutTreeDown
	removeobject VAR_LAST_TALKED
	releaseall
	end

Movement_CutTreeDown:
	cut_tree
	step_end

EventScript_CheckTreeCantCut::
	msgbox Text_CantCut, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CancelCut::
	closemessage
	releaseall
	end

Text_WantToCut:
	.string "This tree looks like it can be\n"
	.string "CUT down!\p"
	.string "Would you like to CUT it?$"

Text_MonUsedFieldMove:
	.string "{STR_VAR_1} used {STR_VAR_2}!$"

Text_CantCut:
	.string "This tree looks like it can be\n"
	.string "CUT down!$"

@ Interact with smashable rock
EventScript_RockSmash::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_ROCK_SMASH
	specialvar VAR_RESULT, CheckBadgeForFieldMove
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantSmashRock
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CantSmashRock
	setfieldeffectargument 0, VAR_RESULT
	buffermovename STR_VAR_2, MOVE_ROCK_SMASH
	msgbox Text_WantToSmash, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_CancelSmash
	call EventScript_ShowFieldMoveUser
	closemessage
	dofieldeffect FLDEFF_USE_ROCK_SMASH
	waitstate
	goto EventScript_SmashRock
	end

@ Use rock smash from party menu
EventScript_UseRockSmash::
	lockall
	dofieldeffect FLDEFF_USE_ROCK_SMASH
	waitstate
	goto EventScript_SmashRock
	end

EventScript_SmashRock::
	applywaitmovement VAR_LAST_TALKED, Movement_SmashRock
	removeobject VAR_LAST_TALKED
	specialvar VAR_RESULT, TryUpdateRusturfTunnelState
	goto_if_eq VAR_RESULT, TRUE, EventScript_EndSmash
	special RockSmashWildEncounter
	goto_if_eq VAR_RESULT, FALSE, EventScript_EndSmash
	waitstate
	releaseall
	end

EventScript_EndSmash::
	releaseall
	end

Movement_SmashRock:
	rock_smash_break
	step_end

EventScript_CantSmashRock::
	msgbox Text_CantSmash, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CancelSmash::
	closemessage
	releaseall
	end

Text_WantToSmash:
	.string "This rock appears to be breakable.\n"
	.string "Would you like to use ROCK SMASH?$"

Text_CantSmash:
	.string "It's a rugged rock, but a POKéMON\n"
	.string "may be able to smash it.$"

EventScript_StrengthBoulder::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_STRENGTH
	specialvar VAR_RESULT, CheckBadgeForFieldMove
	goto_if_eq VAR_RESULT, FALSE, EventScript_CantStrength
	goto_if_set FLAG_SYS_USE_STRENGTH, EventScript_CheckActivatedBoulder
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CantStrength
	setfieldeffectargument 0, VAR_RESULT
	msgbox Text_WantToStrength, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_CancelStrength
	call EventScript_BorrowedMonAppears
	closemessage
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	goto EventScript_ActivateStrength
	end

EventScript_UseStrength::
	lockall
	dofieldeffect FLDEFF_USE_STRENGTH
	waitstate
	goto EventScript_ActivateStrength
	end

EventScript_ActivateStrength::
	setflag FLAG_SYS_USE_STRENGTH
	goto_if_eq VAR_0x8005, FIELD_MOVE_MON_BORROWED, EventScript_ActivateStrengthBorrowed
	msgbox Text_MonUsedStrength, MSGBOX_DEFAULT
	releaseall
	end

EventScript_ActivateStrengthBorrowed::
	msgbox Text_WildMonUsedStrength, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CantStrength::
	msgbox Text_CantStrength, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CheckActivatedBoulder::
	msgbox Text_StrengthActivated, MSGBOX_DEFAULT
	releaseall
	end

EventScript_CancelStrength::
	closemessage
	releaseall
	end

Text_WantToStrength:
	.string "It's a big boulder, but a POKéMON\n"
	.string "may be able to push it aside.\p"
	.string "Would you like to use STRENGTH?$"

Text_MonUsedStrength:
	.string "{STR_VAR_1} used STRENGTH!\p"
	.string "{STR_VAR_1}'s STRENGTH made it\n"
	.string "possible to move boulders around!$"

Text_WildMonUsedStrength:
	.string "The wild {STR_VAR_1} used STRENGTH!\p"
	.string "Its STRENGTH made it possible\n"
	.string "to move boulders around!$"

Text_CantStrength:
	.string "It's a big boulder, but a POKéMON\n"
	.string "may be able to push it aside.$"

Text_StrengthActivated:
	.string "STRENGTH made it possible to move\n"
	.string "boulders around.$"

EventScript_UseWaterfall::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_WATERFALL
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CantWaterfall
	setfieldeffectargument 0, VAR_RESULT
	msgbox Text_WantToWaterfall, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndWaterfall
	buffermovename STR_VAR_2, MOVE_WATERFALL
	call EventScript_ShowFieldMoveUser
	dofieldeffect FLDEFF_USE_WATERFALL
	goto EventScript_EndWaterfall

EventScript_CannotUseWaterfall::
	lockall

EventScript_CantWaterfall::
	msgbox Text_CantWaterfall, MSGBOX_DEFAULT

EventScript_EndWaterfall::
	releaseall
	end

Text_CantWaterfall:
	.string "A wall of water is crashing down with\n"
	.string "a mighty roar.$"

Text_WantToWaterfall:
	.string "It's a large waterfall.\n"
	.string "Would you like to use WATERFALL?$"

EventScript_UseDive::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_DIVE
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CantDive
	setfieldeffectargument 0, VAR_RESULT
	setfieldeffectargument 1, 1
	msgbox Text_WantToDive, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndDive
	buffermovename STR_VAR_2, MOVE_DIVE
	call EventScript_ShowFieldMoveUser
	dofieldeffect FLDEFF_USE_DIVE
	goto EventScript_EndDive
	end

EventScript_CantDive::
	msgbox Text_CantDive, MSGBOX_DEFAULT
	releaseall
	end

EventScript_EndDive::
	releaseall
	end

EventScript_UseDiveUnderwater::
	lockall
	setvar VAR_0x8004, FIELD_MOVE_DIVE
	specialvar VAR_RESULT, GetFieldMoveUserForScript
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_CantSurface
	setfieldeffectargument 0, VAR_RESULT
	setfieldeffectargument 1, 1
	msgbox Text_WantToSurface, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_EndSurface
	buffermovename STR_VAR_2, MOVE_DIVE
	call EventScript_ShowFieldMoveUser
	dofieldeffect FLDEFF_USE_DIVE
	goto EventScript_EndSurface
	end

EventScript_CantSurface::
	lockall
	msgbox Text_CantSurface, MSGBOX_DEFAULT
	goto EventScript_EndSurface
	end

EventScript_EndSurface::
	releaseall
	end

Text_CantDive:
	.string "The sea is deep here. A POKéMON\n"
	.string "may be able to go underwater.$"

Text_WantToDive:
	.string "The sea is deep here.\n"
	.string "Would you like to use DIVE?$"

Text_CantSurface:
	.string "Light is filtering down from above.\n"
	.string "A POKéMON may be able to surface.$"

Text_WantToSurface:
	.string "Light is filtering down from above.\n"
	.string "Would you like to use DIVE?$"

EventScript_FailSweetScent::
	msgbox Text_FailSweetScent, MSGBOX_SIGN
	end

Text_FailSweetScent:
	.string "Looks like there's nothing here…$"

@ After the player agrees to use an HM: when no party POKéMON knows it (VAR_0x8005, from
@ GetFieldMoveUserForScript), a wild one named in STR_VAR_1 comes to lend a hand
EventScript_BorrowedMonAppears::
	goto_if_ne VAR_0x8005, FIELD_MOVE_MON_BORROWED, EventScript_BorrowedMonAppearsEnd
	msgbox Text_BorrowedMonAppears, MSGBOX_DEFAULT
EventScript_BorrowedMonAppearsEnd:
	return

@ The same, then who used the HM (STR_VAR_2): "{STR_VAR_1} used CUT!" or "The wild {STR_VAR_1} used CUT!"
EventScript_ShowFieldMoveUser::
	goto_if_eq VAR_0x8005, FIELD_MOVE_MON_BORROWED, EventScript_ShowBorrowedFieldMoveUser
	msgbox Text_MonUsedFieldMove, MSGBOX_DEFAULT
	return

EventScript_ShowBorrowedFieldMoveUser:
	msgbox Text_BorrowedMonAppears, MSGBOX_DEFAULT
	msgbox Text_WildMonUsedFieldMove, MSGBOX_DEFAULT
	return

Text_BorrowedMonAppears:
	.string "A wild {STR_VAR_1} appeared!\n"
	.string "It looks willing to lend a hand!$"

Text_WildMonUsedFieldMove:
	.string "The wild {STR_VAR_1} used {STR_VAR_2}!$"
