TestArea_MapScripts::
	.byte 0

TestArea_EventScript_HouseSign::
	msgbox TestArea_Text_HouseSign, MSGBOX_SIGN
	end

TestArea_Text_HouseSign:
	.string "TEST HOUSE$"

@ Lets the player switch the map's weather on the spot. Menu item i is weather i
@ (WEATHER_NONE to WEATHER_UNDERWATER_BUBBLES), followed by EXIT. The weather lasts
@ until the player leaves the map, which reloads the map's own weather.
TestArea_EventScript_WeatherChanger::
	lock
	faceplayer
	message TestArea_Text_PickWeather
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_TEST_AREA_WEATHER
	special ShowScrollableMultichoice
	closemessage
	goto_if_gt VAR_RESULT, WEATHER_UNDERWATER_BUBBLES, TestArea_EventScript_WeatherChangerDone
	setweather VAR_RESULT
	doweather
TestArea_EventScript_WeatherChangerDone::
	release
	end

TestArea_Text_PickWeather:
	.string "Which weather should I summon?$"

@ Pins the day/night phase (or hands it back to the real clock) so each phase's tint and the
@ lit windows can be checked without waiting. Menu item i is DAY_NIGHT_OVERRIDE_* value i,
@ followed by EXIT. The choice is saved, and applies to every outdoor map.
TestArea_EventScript_TimeChanger::
	lock
	faceplayer
	message TestArea_Text_PickTime
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_TEST_AREA_TIME
	special ShowScrollableMultichoice
	closemessage
	goto_if_gt VAR_RESULT, DAY_NIGHT_OVERRIDE_NIGHT, TestArea_EventScript_TimeChangerDone
	copyvar VAR_DAY_NIGHT_OVERRIDE, VAR_RESULT
TestArea_EventScript_TimeChangerDone::
	release
	end

TestArea_Text_PickTime:
	.string "What time of day should it be?$"

@ Reports the VIRIDIAN zone's dynamic weather today (src/dynamic_weather.c) and shows its weather
@ for the current phase here. It can step to the next day; that offset also applies to the zone's
@ maps (Pallet Town is just east of here) until the game is reset. Pick the phase with the
@ time changer.
TestArea_EventScript_DynamicWeather::
	lock
	faceplayer
TestArea_EventScript_DynamicWeatherReport::
	special DynamicWeather_DebugReport
	copyvar VAR_0x8004, VAR_RESULT
	msgbox TestArea_Text_DynamicWeatherReport, MSGBOX_DEFAULT
	setweather VAR_0x8004
	doweather
	msgbox TestArea_Text_DynamicWeatherNextDay, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_DynamicWeatherDone
	special DynamicWeather_DebugNextDay
	goto TestArea_EventScript_DynamicWeatherReport
TestArea_EventScript_DynamicWeatherDone::
	release
	end

TestArea_Text_DynamicWeatherReport:
	.string "{STR_VAR_1} zone, day {STR_VAR_2}.\n{STR_VAR_3}$"

TestArea_Text_DynamicWeatherNextDay:
	.string "Here's how it looks now.\p"
	.string "Look at the next day? It changes the\n"
	.string "zone's maps too, until a reset.$"

@ Debug shortcut to the tournament island until the story gives the player a way there.
TestArea_EventScript_IslandSailor::
	lock
	faceplayer
	msgbox TestArea_Text_SailToIsland, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_IslandSailorDone
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	setvar VAR_0x8006, SEAGALLOP_MEGA_ISLAND
	goto Kanto_EventScript_SetSail
TestArea_EventScript_IslandSailorDone::
	release
	end

TestArea_Text_SailToIsland:
	.string "Want a ride to the castle island?$"

@ Debug card player: the CARD CASE and the starter deck if the player has none, a card pack on
@ every talk, then a duel against CARD_OPPONENT_TEST_AREA_TESTER.
TestArea_EventScript_CardTester::
	lock
	faceplayer
	checkitem ITEM_CARD_CASE
	call_if_eq VAR_RESULT, FALSE, TestArea_EventScript_CardTesterGiveCase
	msgbox TestArea_Text_CardTesterHereIsAPack
	closemessage
	special OpenCardPack
	cardduel CARD_OPPONENT_TEST_AREA_TESTER
	release
	end

TestArea_EventScript_CardTesterGiveCase::
	msgbox TestArea_Text_CardTesterNoCardCase
	giveitem ITEM_CARD_CASE
	special CardGame_GiveStarterDeck
	msgbox Debug_Text_CardGotStarter
	return

TestArea_Text_CardTesterNoCardCase:
	.string "No CARD CASE? Take this one,\n"
	.string "then we can test the card game!$"

TestArea_Text_CardTesterHereIsAPack:
	.string "Here's a card pack to test!$"
