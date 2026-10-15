@ The card game (docs/card_game_plan.md). For now only a debug duel, from the START menu when
@ CARD_GAME_DEBUG is defined in include/config.h: against one of the NPC decks, at random.
Debug_EventScript_CardDuel::
	lockall
	random CARD_DECKS_COUNT - 1
	addvar VAR_RESULT, CARD_DECK_BUG_CATCHER
	copyvar VAR_0x8004, VAR_RESULT
	special PlayCardGame
	switch VAR_RESULT
	case CARD_GAME_RESULT_WON, Debug_EventScript_CardDuelWon
	case CARD_GAME_RESULT_DRAW, Debug_EventScript_CardDuelDraw
	msgbox Debug_Text_CardDuelLost, MSGBOX_DEFAULT
	releaseall
	end

Debug_EventScript_CardDuelWon::
	msgbox Debug_Text_CardDuelWon, MSGBOX_DEFAULT
	releaseall
	end

Debug_EventScript_CardDuelDraw::
	msgbox Debug_Text_CardDuelDraw, MSGBOX_DEFAULT
	releaseall
	end

Debug_Text_CardDuelWon:
	.string "Card duel: you won!$"

Debug_Text_CardDuelLost:
	.string "Card duel: you lost.$"

Debug_Text_CardDuelDraw:
	.string "Card duel: a draw.$"
