GraniteCave_1F_MapScripts::
	.byte 0

@ The Johto starters' side quests (data/scripts/starter_quests.inc): the HIKER's CYNDAQUIL, from the DEWFORD
@ TOWN POKéMON CENTER, cornered by two ZUBAT west of the ladder down. Talking to any of the three starts the
@ scene: a wild battle with each ZUBAT still there (beaten or caught, it stays gone), then CYNDAQUIL runs back.
GraniteCave_1F_EventScript_CyndaquilScene::
	lockall
	playmoncry SPECIES_CYNDAQUIL, CRY_MODE_NORMAL
	msgbox GraniteCave_1F_Text_CyndaquilCornered
	waitmoncry
	goto_if_set FLAG_DEFEATED_GRANITE_CAVE_ZUBAT_1, GraniteCave_1F_EventScript_SecondZubat
	applymovement LOCALID_GRANITE_CAVE_ZUBAT_1, GraniteCave_1F_Movement_ZubatNoticePlayer
	waitmovement 0
	msgbox GraniteCave_1F_Text_ZubatAttacks
	closemessage
	setwildbattle SPECIES_ZUBAT, 8
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, GraniteCave_1F_EventScript_DefeatedFirstZubat
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, GraniteCave_1F_EventScript_DefeatedFirstZubat
	goto GraniteCave_1F_EventScript_ZubatStillThere

GraniteCave_1F_EventScript_DefeatedFirstZubat::
	setflag FLAG_DEFEATED_GRANITE_CAVE_ZUBAT_1
	removeobject LOCALID_GRANITE_CAVE_ZUBAT_1
	msgbox GraniteCave_1F_Text_AnotherZubat
GraniteCave_1F_EventScript_SecondZubat::
	applymovement LOCALID_GRANITE_CAVE_ZUBAT_2, GraniteCave_1F_Movement_ZubatNoticePlayer
	waitmovement 0
	msgbox GraniteCave_1F_Text_ZubatAttacks
	closemessage
	setwildbattle SPECIES_ZUBAT, 8
	dowildbattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, GraniteCave_1F_EventScript_DefeatedSecondZubat
	goto_if_eq VAR_RESULT, B_OUTCOME_CAUGHT, GraniteCave_1F_EventScript_DefeatedSecondZubat
	goto GraniteCave_1F_EventScript_ZubatStillThere

GraniteCave_1F_EventScript_DefeatedSecondZubat::
	setflag FLAG_DEFEATED_GRANITE_CAVE_ZUBAT_2
	removeobject LOCALID_GRANITE_CAVE_ZUBAT_2
	delay 20
	playmoncry SPECIES_CYNDAQUIL, CRY_MODE_NORMAL
	applymovement LOCALID_GRANITE_CAVE_CYNDAQUIL, GraniteCave_1F_Movement_CyndaquilHop
	waitmovement 0
	waitmoncry
	msgbox GraniteCave_1F_Text_CyndaquilFlameLit
	closemessage
	fadescreenspeed FADE_TO_BLACK, 4
	removeobject LOCALID_GRANITE_CAVE_CYNDAQUIL
	setflag FLAG_RESCUED_GRANITE_CAVE_CYNDAQUIL
	fadescreenspeed FADE_FROM_BLACK, 4
	msgbox GraniteCave_1F_Text_CyndaquilRanBack
	releaseall
	end

GraniteCave_1F_EventScript_ZubatStillThere::
	msgbox GraniteCave_1F_Text_ZubatStillThere
	releaseall
	end

GraniteCave_1F_Movement_ZubatNoticePlayer:
	emote_exclamation_mark
	step_end

GraniteCave_1F_Movement_CyndaquilHop:
	jump_in_place_down
	jump_in_place_down
	step_end

GraniteCave_1F_Text_CyndaquilCornered:
	.string "A CYNDAQUIL is cornered by ZUBAT!\n"
	.string "The flame on its back has gone out…$"

GraniteCave_1F_Text_ZubatAttacks:
	.string "The ZUBAT turned on {PLAYER}!$"

GraniteCave_1F_Text_AnotherZubat:
	.string "But another ZUBAT is still there!$"

GraniteCave_1F_Text_ZubatStillThere:
	.string "The ZUBAT are still circling\n"
	.string "around CYNDAQUIL…$"

GraniteCave_1F_Text_CyndaquilFlameLit:
	.string "CYNDAQUIL's flame flared up again!\n"
	.string "It lights up the whole cave!$"

GraniteCave_1F_Text_CyndaquilRanBack:
	.string "CYNDAQUIL ran off toward the exit.\n"
	.string "It must be going back to its TRAINER.$"

GraniteCave_1F_EventScript_Hiker::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_HM_FLASH, GraniteCave_1F_EventScript_ReceivedFlash
	msgbox GraniteCave_1F_Text_GetsDarkAheadHereYouGo, MSGBOX_DEFAULT
	giveitem ITEM_HM_FLASH
	setflag FLAG_RECEIVED_HM_FLASH
	msgbox GraniteCave_1F_Text_ExplainFlash, MSGBOX_DEFAULT
	release
	end

GraniteCave_1F_EventScript_ReceivedFlash::
	msgbox GraniteCave_1F_Text_ExplainFlash, MSGBOX_DEFAULT
	release
	end

GraniteCave_1F_Text_GetsDarkAheadHereYouGo:
	.string "Hey, you.\n"
	.string "It gets awfully dark ahead.\l"
	.string "It'll be tough trying to explore.\p"
	.string "That guy who came by earlier…\n"
	.string "STEVEN, I think it was.\p"
	.string "He knew how to use FLASH, so he ought\n"
	.string "to be all right, but…\p"
	.string "Well, for us HIKERS, helping out those\n"
	.string "that we meet is our motto.\p"
	.string "Here you go, I'll pass this on to you.$"

GraniteCave_1F_Text_ExplainFlash:
	.string "Teach that hidden move FLASH to\n"
	.string "a POKéMON and use it.\p"
	.string "It lights up even the inky darkness\n"
	.string "of caves.\p"
	.string "But, to use it, you need the GYM BADGE\n"
	.string "from DEWFORD's POKéMON GYM.$"

