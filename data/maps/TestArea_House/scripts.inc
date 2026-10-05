TestArea_House_MapScripts::
	.byte 0

@ Test case for double exchanges of Pokémon that evolve through trade: the player
@ gives any Pokémon, receives the trader's KADABRA, and the two are traded straight
@ back. Each side is traded once, so a trade evolver on either side evolves
@ (e.g. MACHOKE → MACHAMP for the player, KADABRA → ALAKAZAM before going back).
@ The exchange has no completion flag so it can be repeated.
TestArea_House_EventScript_Trader::
	lock
	faceplayer
	setvar VAR_0x8008, INGAME_TRADE_KADABRA
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	msgbox TestArea_House_Text_ExplainDoubleExchange, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_House_EventScript_DeclineTrade
	special ChoosePartyMon
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, TestArea_House_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	goto_if_eq VAR_RESULT, SPECIES_NONE, TestArea_House_EventScript_CantTradeEgg
	@ First half: the player's Pokémon for the trader's KADABRA.
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateDoubleExchangeTradePokemon
	special DoInGameTradeScene
	msgbox TestArea_House_Text_NowTradeBack, MSGBOX_DEFAULT
	@ Second half: the KADABRA (or what it evolved into) for the player's Pokémon.
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateDoubleExchangeReturnPokemon
	special DoInGameTradeScene
	copyvar VAR_0x8005, VAR_0x800A
	special RestoreDoubleExchangeFriendship
	msgbox TestArea_House_Text_DoubleExchangeDone, MSGBOX_DEFAULT
	release
	end

TestArea_House_EventScript_DeclineTrade::
	msgbox TestArea_House_Text_Declined, MSGBOX_DEFAULT
	release
	end

TestArea_House_EventScript_CantTradeEgg::
	msgbox TestArea_House_Text_CantTradeEgg, MSGBOX_DEFAULT
	release
	end

TestArea_House_Text_ExplainDoubleExchange:
	.string "I'm testing a DOUBLE EXCHANGE!\p"
	.string "You give me a POKéMON, and I'll\n"
	.string "give you my {STR_VAR_2} for it.\p"
	.string "Then we exchange them back right\n"
	.string "away, so you get yours back.\p"
	.string "POKéMON that evolve through trade\n"
	.string "will evolve along the way.\p"
	.string "Try it with a {STR_VAR_1}!\n"
	.string "Want to do a DOUBLE EXCHANGE?$"

TestArea_House_Text_NowTradeBack:
	.string "That was the first exchange.\n"
	.string "Now let's trade them back!$"

TestArea_House_Text_DoubleExchangeDone:
	.string "The DOUBLE EXCHANGE is complete!\n"
	.string "Your POKéMON is back with you.\p"
	.string "Come back to run it again.$"

TestArea_House_Text_CantTradeEgg:
	.string "Sorry, an EGG can't be exchanged.$"

TestArea_House_Text_Declined:
	.string "No problem. Come back whenever\n"
	.string "you want to run the test.$"
