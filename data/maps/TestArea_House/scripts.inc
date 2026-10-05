TestArea_House_MapScripts::
	.byte 0

@ Test case for exchanging Pokémon that evolve through trade: the trader asks for
@ a MACHOKE and gives a KADABRA, so both sides of the exchange are trade evolvers.
@ The trade is repeatable (no completion flag) so it can be run again and again.
TestArea_House_EventScript_Trader::
	lock
	faceplayer
	setvar VAR_0x8008, INGAME_TRADE_KADABRA
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	msgbox TestArea_House_Text_IllTradeIfYouWant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_House_EventScript_DeclineTrade
	special ChoosePartyMon
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, TestArea_House_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	goto_if_ne VAR_RESULT, VAR_0x8009, TestArea_House_EventScript_NotRequestedMon
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	msgbox TestArea_House_Text_TradeDone, MSGBOX_DEFAULT
	release
	end

TestArea_House_EventScript_DeclineTrade::
	msgbox TestArea_House_Text_Declined, MSGBOX_DEFAULT
	release
	end

TestArea_House_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox TestArea_House_Text_NotRequestedMon, MSGBOX_DEFAULT
	release
	end

TestArea_House_Text_IllTradeIfYouWant:
	.string "I'm testing trades between POKéMON\n"
	.string "that evolve when traded.\p"
	.string "I'll trade you my {STR_VAR_2} for\n"
	.string "a {STR_VAR_1}. How about it?$"

TestArea_House_Text_TradeDone:
	.string "Great, the test trade worked!\n"
	.string "Come back to run it again.$"

TestArea_House_Text_NotRequestedMon:
	.string "Hm? That isn't a {STR_VAR_1}.\n"
	.string "The test needs a {STR_VAR_1}.$"

TestArea_House_Text_Declined:
	.string "No problem. Come back whenever\n"
	.string "you want to run the test.$"
