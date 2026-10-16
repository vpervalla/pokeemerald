@ The card game (docs/card_game_plan.md): the card players' shared script, PROF. OAK's AIDE who
@ brings the CARD CASE and the daily pack, the other ways to get packs, Mega Island's card
@ tournament, and a debug script.

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
	call CardGame_EventScript_Duel
	return

@ The duel with VAR_0x8004, then their line and the rewards. VAR_0x8006: CARD_GAME_RESULT_*.
CardGame_EventScript_Duel::
	special PlayCardGame
	copyvar VAR_0x8006, VAR_RESULT
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
	goto_if_eq VAR_RESULT, 0, CardGame_EventScript_Return
	copyvar VAR_0x8007, VAR_RESULT
	msgbox CardGame_Text_TakeThisCard
	playfanfare MUS_OBTAIN_ITEM
	message CardGame_Text_ReceivedCard
	waitmessage
	waitfanfare
	waitbuttonpress
	call_if_eq VAR_0x8007, 2, CardGame_EventScript_LeadersBonus
	closemessage
	special OpenCardRewardScreen
	return

@ The last of the 8 GYM LEADERS beaten at cards: CARD_LEADERS_BONUS too
CardGame_EventScript_LeadersBonus::
	msgbox CardGame_Text_AllLeadersBeaten
	playfanfare MUS_OBTAIN_ITEM
	message CardGame_Text_ReceivedCard2
	waitmessage
	waitfanfare
	waitbuttonpress
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

@ The card fan of the Two Island Game Corner: another pack a day, late in the game
CardGame_EventScript_DailyPack2::
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_Return
	goto_if_set FLAG_DAILY_CARD_PACK_2, CardGame_EventScript_Return
	msgbox CardGame_Text_FanHereIsAPack
	closemessage
	setflag FLAG_DAILY_CARD_PACK_2
	special OpenCardPack
	return

@ The CELADON DEPT. STORE's 4F clerk sells packs, before the usual shop
CardGame_EventScript_PackShop::
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_Return
	msgbox CardGame_Text_SellPacks, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CardGame_EventScript_Return
CardGame_EventScript_BuyPack::
	checkmoney CARD_PACK_PRICE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_NotEnoughMoney
	removemoney CARD_PACK_PRICE
	playse SE_SHOP
	msgbox CardGame_Text_HereIsYourPack
	closemessage
	special OpenCardPack
	msgbox CardGame_Text_AnotherPack, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, CardGame_EventScript_BuyPack
	return

CardGame_EventScript_NotEnoughMoney::
	msgbox CardGame_Text_NotEnoughMoney
	return

@ The veteran in the INDIGO PLATEAU's POKéMON CENTER plays cards after the HALL OF FAME
CardGame_EventScript_IndigoVeteran::
	cardduel CARD_OPPONENT_INDIGO_VETERAN
	return

@ Mega Island's card tournament, in the castle's entrance hall, once a day: 2 of the castle's
@ guests at random, then MR. M. Every duel counts as one with that card player (met, beaten and
@ their reward); the champion gets a pack.
MegaIsland_Castle_1F_EventScript_CardDealer::
	lock
	faceplayer
	checkitem ITEM_CARD_CASE
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_TournamentNoCardCase
	goto_if_set FLAG_DAILY_CARD_TOURNAMENT, CardGame_EventScript_TournamentTomorrow
	msgbox CardGame_Text_TournamentWelcome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, CardGame_EventScript_TournamentDeclined
	special CardGame_CheckActiveDeck
	goto_if_eq VAR_RESULT, FALSE, CardGame_EventScript_TournamentDeckNotReady
	setflag FLAG_DAILY_CARD_TOURNAMENT
	special CardGame_StartCardTournament
	copyvar VAR_0x8004, VAR_0x8008
	special CardGame_BufferOpponentName
	msgbox CardGame_Text_TournamentFirstRound
	call CardGame_EventScript_TournamentDuel
	goto_if_ne VAR_0x8006, CARD_GAME_RESULT_WON, CardGame_EventScript_TournamentOut
	copyvar VAR_0x8004, VAR_0x8009
	special CardGame_BufferOpponentName
	msgbox CardGame_Text_TournamentSecondRound
	call CardGame_EventScript_TournamentDuel
	goto_if_ne VAR_0x8006, CARD_GAME_RESULT_WON, CardGame_EventScript_TournamentOut
	setvar VAR_0x8004, CARD_OPPONENT_MR_M
	msgbox CardGame_Text_TournamentFinal
	call CardGame_EventScript_TournamentDuel
	goto_if_ne VAR_0x8006, CARD_GAME_RESULT_WON, CardGame_EventScript_TournamentOut
	msgbox CardGame_Text_TournamentChampion
	closemessage
	special OpenCardPack
	msgbox CardGame_Text_TournamentComeBack
	release
	end

CardGame_EventScript_TournamentDuel::
	special CardGame_MeetOpponent
	closemessage
	call CardGame_EventScript_Duel
	return

CardGame_EventScript_TournamentOut::
	msgbox CardGame_Text_TournamentOut
	release
	end

CardGame_EventScript_TournamentNoCardCase::
	msgbox CardGame_Text_TournamentNoCardCase
	release
	end

CardGame_EventScript_TournamentTomorrow::
	msgbox CardGame_Text_TournamentTomorrow
	release
	end

CardGame_EventScript_TournamentDeclined::
	msgbox CardGame_Text_TournamentComeBack
	release
	end

CardGame_EventScript_TournamentDeckNotReady::
	msgbox CardGame_Text_DeckNotReady
	release
	end

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

CardGame_Text_ReceivedCard2:
	.string "{PLAYER} received the\n"
	.string "{STR_VAR_2} card!$"

CardGame_Text_AllLeadersBeaten:
	.string "And you've beaten all 8 GYM\n"
	.string "LEADERS at cards!\p"
	.string "That deserves this card, too!$"

CardGame_Text_FanHereIsAPack:
	.string "You play POKéMON cards? Me too!\n"
	.string "Here, a card pack. Have fun!$"

CardGame_Text_SellPacks:
	.string "We also sell POKéMON card packs,\n"
	.string "¥500 each. Would you like one?$"

CardGame_Text_HereIsYourPack:
	.string "Here you are! Thank you!$"

CardGame_Text_AnotherPack:
	.string "Would you like another pack?$"

CardGame_Text_NotEnoughMoney:
	.string "You don't have enough money.$"

CardGame_Text_TournamentNoCardCase:
	.string "This is the castle's card table.\n"
	.string "Come back with a deck!$"

CardGame_Text_TournamentWelcome:
	.string "Welcome to MR. M.'s card table!\p"
	.string "Every day, a card tournament:\n"
	.string "three duels in a row.\p"
	.string "Two of the castle's guests, then\n"
	.string "MR. M. himself, if you get there.\p"
	.string "Will you enter today?$"

CardGame_Text_TournamentFirstRound:
	.string "First round!\n"
	.string "Your opponent: {STR_VAR_1}!$"

CardGame_Text_TournamentSecondRound:
	.string "On to the second round!\n"
	.string "Your opponent: {STR_VAR_1}!$"

CardGame_Text_TournamentFinal:
	.string "The final! Your opponent is our\n"
	.string "host, MR. M.!$"

CardGame_Text_TournamentChampion:
	.string "Congratulations! You are today's\n"
	.string "card champion!\p"
	.string "Your prize: a card pack!$"

CardGame_Text_TournamentOut:
	.string "That's the end of the tournament\n"
	.string "for you today. Come back tomorrow!$"

CardGame_Text_TournamentTomorrow:
	.string "Today's tournament is over.\n"
	.string "Come back tomorrow!$"

CardGame_Text_TournamentComeBack:
	.string "The card table is open every day.\n"
	.string "Come back anytime!$"

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
