@ The double-exchange traders in the Pokémon Centers, so that POKéMON that evolve through trade can
@ evolve without trading with another player. The player gives a POKéMON, receives the trader's,
@ and the two are traded straight back, so the player's POKéMON evolves on its way back
@ (with its held item, if its evolution needs one). See TestArea_House for the original test.
@ Each trader exchanges only once: they first check that the chosen POKéMON would evolve.
@
@ Each trader's script sets VAR_0x8008 to their INGAME_TRADE_EXCHANGE_* and checks their own
@ FLAG_DID_DOUBLE_EXCHANGE_*, then calls EventScript_DoubleExchange, which returns TRUE in
@ VAR_RESULT once the exchange is done.

EventScript_DoubleExchange::
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	msgbox Text_DoubleExchangeExplain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DoubleExchangeDeclined
	msgbox Text_DoubleExchangeWhichMon
	closemessage
	special ChoosePartyMon
	lock
	faceplayer
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_ge VAR_0x8004, PARTY_SIZE, EventScript_DoubleExchangeDeclined
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	goto_if_eq VAR_RESULT, SPECIES_NONE, EventScript_DoubleExchangeEgg
	bufferpartymonnick STR_VAR_1, VAR_0x800A
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, WouldMonEvolveThroughTrade
	goto_if_eq VAR_RESULT, TRADE_EVO_NEEDS_ITEM, EventScript_DoubleExchangeNeedsItem
	goto_if_ne VAR_RESULT, TRADE_EVO_YES, EventScript_DoubleExchangeWontEvolve
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	bufferpartymonnick STR_VAR_1, VAR_0x800A
	msgbox Text_DoubleExchangeConfirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_DoubleExchangeDeclined
	@ First half: the player's POKéMON for the trader's.
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateDoubleExchangeTradePokemon
	special DoInGameTradeScene
	lock
	faceplayer
	msgbox Text_DoubleExchangeTradeBack
	@ Second half: the trader's POKéMON back for the player's, which evolves on its way back.
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateDoubleExchangeReturnPokemon
	special DoInGameTradeScene
	lock
	faceplayer
	copyvar VAR_0x8005, VAR_0x800A
	special RestoreDoubleExchangeFriendship
	msgbox Text_DoubleExchangeDone
	setvar VAR_RESULT, TRUE
	return

EventScript_DoubleExchangeDeclined::
	msgbox Text_DoubleExchangeDeclined
	setvar VAR_RESULT, FALSE
	return

EventScript_DoubleExchangeEgg::
	msgbox Text_DoubleExchangeEgg
	setvar VAR_RESULT, FALSE
	return

EventScript_DoubleExchangeNeedsItem::
	msgbox Text_DoubleExchangeNeedsItem
	setvar VAR_RESULT, FALSE
	return

EventScript_DoubleExchangeWontEvolve::
	msgbox Text_DoubleExchangeWontEvolve
	setvar VAR_RESULT, FALSE
	return

EventScript_DoubleExchangeAlreadyDone::
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	msgbox Text_DoubleExchangeAlreadyDone
EventScript_DoubleExchangeEnd::
	release
	end

Text_DoubleExchangeExplain:
	.string "Have you heard of DOUBLE\n"
	.string "EXCHANGES?\p"
	.string "You give me a POKéMON, I give you\n"
	.string "my {STR_VAR_2}, and then we trade\l"
	.string "them straight back.\p"
	.string "Some POKéMON evolve when they're\n"
	.string "traded, and some only while holding\l"
	.string "a special item.\p"
	.string "I can only do it once, though.\n"
	.string "My {STR_VAR_2} hates being away\l"
	.string "from me!\p"
	.string "Want to do a DOUBLE EXCHANGE?$"

Text_DoubleExchangeWhichMon:
	.string "Which POKéMON will you exchange?$"

Text_DoubleExchangeConfirm:
	.string "Oh, your {STR_VAR_1}!\n"
	.string "It'll evolve on its way back!\p"
	.string "Remember, I can only do this once.\n"
	.string "Ready to exchange?$"

Text_DoubleExchangeTradeBack:
	.string "That was the first exchange.\n"
	.string "Now let's trade them back!$"

Text_DoubleExchangeDone:
	.string "There! Your POKéMON is back.\n"
	.string "Take good care of it!$"

Text_DoubleExchangeNeedsItem:
	.string "Your {STR_VAR_1} would evolve if it\n"
	.string "were holding a {STR_VAR_3}.\p"
	.string "Come back once it's holding one!$"

Text_DoubleExchangeWontEvolve:
	.string "Hmm… Your {STR_VAR_1} wouldn't\n"
	.string "change if we exchanged it.\p"
	.string "I can only do this once, so bring\n"
	.string "a POKéMON that evolves through\l"
	.string "trade!$"

Text_DoubleExchangeEgg:
	.string "An EGG can't be exchanged.$"

Text_DoubleExchangeDeclined:
	.string "Okay. Come back if you change\n"
	.string "your mind!$"

Text_DoubleExchangeAlreadyDone:
	.string "My {STR_VAR_2} is staying with me\n"
	.string "now. Thanks for the exchange!$"
