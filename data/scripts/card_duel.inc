@ The card game (docs/card_game_plan.md): the card players' shared script, PROF. OAK's AIDE who
@ brings the CARD CASE and the daily pack, and a debug script.

@ `cardduel CARD_OPPONENT_*` (asm/macros/event.inc), after the NPC's usual line. The NPC's lines
@ are in gCardOpponents (src/data/card_game/opponents.h). Returns to the NPC's script.
Common_EventScript_CardDuel::
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_NoCardCase
	special CardGame_MeetOpponent
	setvar VAR_0x8005, CARD_OPPONENT_TEXT_CHALLENGE
	special CardGame_BufferOpponentText
	msgbox gStringVar4, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CardGame_EventScript_Declined
	special CardGame_CheckActiveDeck
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_DeckNotReady
	closemessage
	special PlayCardGame
	switch VAR_RESULT
	case CARD_GAME_RESULT_WON, CardGame_EventScript_PlayerWon
	case CARD_GAME_RESULT_DRAW, CardGame_EventScript_Draw
	setvar VAR_0x8005, CARD_OPPONENT_TEXT_PLAYER_LOST
	special CardGame_BufferOpponentText
	msgbox gStringVar4
	return

CardGame_EventScript_PlayerWon::
	setvar VAR_0x8005, CARD_OPPONENT_TEXT_PLAYER_WON
	special CardGame_BufferOpponentText
	msgbox gStringVar4
	special CardGame_RewardOpponentWin
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_Return
	msgbox CardGame_Text_TakeThisCard
	playfanfare MUS_OBTAIN_ITEM
	message CardGame_Text_ReceivedCard
	waitmessage
	waitfanfare
	waitbuttonpress
	closemessage
	special OpenCardRewardScreen
	return

CardGame_EventScript_Draw::
	msgbox CardGame_Text_Draw
	return

CardGame_EventScript_NoCardCase::
	msgbox CardGame_Text_NoCardCase
	return

CardGame_EventScript_Declined::
	msgbox CardGame_Text_Declined
	return

CardGame_EventScript_DeckNotReady::
	msgbox CardGame_Text_DeckNotReady
	return

CardGame_EventScript_Return::
	return

@ PROF. OAK's AIDE in PEWTER CITY, after the BOULDERBADGE: the CARD CASE, the starter deck and a
@ first pack (in FRLG, the RUNNING SHOES, which are on from the start here)
CardGame_EventScript_AideGiveCardCase::
	additem ITEM_CARD_CASE
	special CardGame_GiveStarterDeck
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	message CardGame_Text_ReceivedCardCase
	waitmessage
	waitfanfare
	msgbox CardGame_Text_AideExplainCardCase
	closemessage
	special OpenCardPack
	msgbox CardGame_Text_AideExplainPlayers
	return

@ The AIDE again, in PROF. OAK's LAB: a pack a day
CardGame_EventScript_DailyPack::
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_Return
	goto_if_set FLAG_DAILY_CARD_PACK, CardGame_EventScript_DailyPackTomorrow
	msgbox CardGame_Text_AideHereIsTodaysPack
	closemessage
	setflag FLAG_DAILY_CARD_PACK
	special OpenCardPack
	return

CardGame_EventScript_DailyPackTomorrow::
	msgbox CardGame_Text_AideComeBackTomorrow
	return

CardGame_Text_NoCardCase:
	.string "I play POKéMON cards, too.\n"
	.string "Come back when you have a deck!$"

CardGame_Text_Declined:
	.string "Oh… Maybe next time, then.$"

CardGame_Text_DeckNotReady:
	.string "Your deck can't be played yet.\n"
	.string "Fix it with the CARD CASE.$"

CardGame_Text_Draw:
	.string "A draw!\n"
	.string "Let's play again sometime.$"

CardGame_Text_TakeThisCard:
	.string "Here, you've earned this card!$"

CardGame_Text_ReceivedCard:
	.string "{PLAYER} received the\n"
	.string "{STR_VAR_1} card!$"

CardGame_Text_ReceivedCardCase:
	.string "{PLAYER} received the\n"
	.string "CARD CASE from the AIDE.$"

CardGame_Text_AideExplainCardCase:
	.string "There's a deck of 20 cards inside,\n"
	.string "ready to play.\p"
	.string "Every POKéMON you catch also gives\n"
	.string "you its card, two copies of it!\p"
	.string "And here's your first card pack!$"

CardGame_Text_AideExplainPlayers:
	.string "Use the CARD CASE in your BAG to\n"
	.string "build your decks.\p"
	.string "Talk to people in every town.\n"
	.string "Many of them play cards!\p"
	.string "Come see me at the LAB, too.\n"
	.string "I'll have a new pack every day.$"

CardGame_Text_AideHereIsTodaysPack:
	.string "Oh, {PLAYER}!\n"
	.string "Here's today's card pack!$"

CardGame_Text_AideComeBackTomorrow:
	.string "I'll have a new card pack for you\n"
	.string "tomorrow!$"

@ The debug START menu entry, when CARD_GAME_DEBUG is defined in include/config.h: the first time,
@ the CARD CASE and the starter deck; then a duel against one of the card players (at random), a
@ pack, or the deck builder.
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
	random CARD_OPPONENTS_COUNT
	copyvar VAR_0x8004, VAR_RESULT
	call Common_EventScript_CardDuel
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
