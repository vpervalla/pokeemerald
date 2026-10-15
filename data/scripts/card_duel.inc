@ The card game (docs/card_game_plan.md). For now only debug scripts, from the START menu when
@ CARD_GAME_DEBUG is defined in include/config.h: the first time, the CARD CASE and the starter
@ deck; then a duel against one of the NPC decks (at random), a pack, or the deck builder.
Debug_EventScript_CardDuel::
	lockall
	checkitem ITEM_CARD_CASE
	call_if_eq VAR_RESULT, FALSE, Debug_EventScript_CardGiveCase
	msgbox Debug_Text_CardDuelAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Debug_EventScript_CardPlayDuel
	msgbox Debug_Text_CardPackAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Debug_EventScript_CardOpenPack
	msgbox Debug_Text_CardDecksAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Debug_EventScript_CardDecks
	releaseall
	end

Debug_EventScript_CardGiveCase::
	giveitem ITEM_CARD_CASE
	special CardGame_GiveStarterDeck
	msgbox Debug_Text_CardGotStarter, MSGBOX_DEFAULT
	return

Debug_EventScript_CardOpenPack::
	special OpenCardPack
	releaseall
	end

Debug_EventScript_CardDecks::
	special OpenCardDeckBuilderFromScript
	releaseall
	end

Debug_EventScript_CardPlayDuel::
	special CardGame_CheckActiveDeck
	goto_if_eq VAR_RESULT, FALSE, Debug_EventScript_CardDeckNotReady
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

Debug_EventScript_CardDeckNotReady::
	msgbox Debug_Text_CardDeckNotReady, MSGBOX_DEFAULT
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

Debug_Text_CardGotStarter:
	.string "Your CARD CASE has a starter deck.$"

Debug_Text_CardDuelAsk:
	.string "Play a card duel?$"

Debug_Text_CardPackAsk:
	.string "Open a card pack?$"

Debug_Text_CardDecksAsk:
	.string "Build your decks?$"

Debug_Text_CardDeckNotReady:
	.string "Your deck can't be played yet.\n"
	.string "Fix it with the CARD CASE.$"

Debug_Text_CardDuelWon:
	.string "Card duel: you won!$"

Debug_Text_CardDuelLost:
	.string "Card duel: you lost.$"

Debug_Text_CardDuelDraw:
	.string "Card duel: a draw.$"
