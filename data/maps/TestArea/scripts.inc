TestArea_MapScripts::
	.byte 0

TestArea_EventScript_HouseSign::
	msgbox TestArea_Text_HouseSign, MSGBOX_SIGN
	end

TestArea_Text_HouseSign:
	.string "TEST HOUSE$"

@ Lets the player switch the map's weather on the spot. Menu item i is weather i
@ (WEATHER_NONE to WEATHER_UNDERWATER_BUBBLES), followed by EXIT. The weather lasts
@ until the player leaves the map, which reloads the map's own weather: the dynamic
@ weather leaves a weather set by a script alone.
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
@ followed by EXIT. The choice is saved, and applies to every outdoor map. It also moves the
@ dynamic weather to that phase: the Test Area is in the VIRIDIAN zone, so its weather fades
@ to the new phase's within a second (unless the weather changer set it).
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

@ Reports a zone's dynamic weather today (src/dynamic_weather.c) and shows that zone's weather
@ here. Menu item i is zone i (DYN_ZONE_*), followed by EXIT. The Test Area is in the VIRIDIAN
@ zone, so picking VIRIDIAN gives it back its own, live weather, undoing the weather changer;
@ another zone's weather is shown as it is now and stays until the player leaves. It can step
@ to the next day; that offset applies to every zone until the game is reset. Pick the phase
@ with the time changer.
TestArea_EventScript_DynamicWeather::
	lock
	faceplayer
	message TestArea_Text_PickZone
	waitmessage
	setvar VAR_0x8004, SCROLL_MULTI_TEST_AREA_ZONE
	special ShowScrollableMultichoice
	closemessage
	goto_if_ge VAR_RESULT, DYN_ZONE_COUNT, TestArea_EventScript_DynamicWeatherDone
	copyvar VAR_0x8005, VAR_RESULT
	copyvar VAR_0x8006, VAR_RESULT
	addvar VAR_0x8006, WEATHER_DYNAMIC_START
TestArea_EventScript_DynamicWeatherReport::
	special DynamicWeather_DebugReport
	msgbox TestArea_Text_DynamicWeatherReport, MSGBOX_DEFAULT
	setweather VAR_0x8006
	doweather
	msgbox TestArea_Text_DynamicWeatherNextDay, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_DynamicWeatherDone
	special DynamicWeather_DebugNextDay
	goto TestArea_EventScript_DynamicWeatherReport
TestArea_EventScript_DynamicWeatherDone::
	release
	end

TestArea_Text_PickZone:
	.string "Which zone's weather should I check?$"

TestArea_Text_DynamicWeatherReport:
	.string "{STR_VAR_1} zone, day {STR_VAR_2}.\n{STR_VAR_3}$"

TestArea_Text_DynamicWeatherNextDay:
	.string "Here's how it looks now.\p"
	.string "Look at the next day? It changes\n"
	.string "every zone, until a reset.$"

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

@ Debug shortcuts for the way to Hoenn (see data/scripts/hoenn_story.inc): a ride to SLATEPORT, or the
@ MANSION raid without a Hall of Fame.
TestArea_EventScript_HoennSailor::
	lock
	faceplayer
	msgbox TestArea_Text_SailToSlateport, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_HoennSailorRaid
	call_if_lt VAR_HOENN_STORY_STATE, 3, TestArea_EventScript_SkipToHoennShip  @ BIRCH's welcome at SLATEPORT
	setvar VAR_0x8004, SEAGALLOP_VERMILION_CITY
	setvar VAR_0x8006, SEAGALLOP_SLATEPORT
	goto Kanto_EventScript_SetSail
TestArea_EventScript_HoennSailorRaid::
	msgbox TestArea_Text_StartMansionRaid, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_HoennSailorDone
	setvar VAR_HOENN_STORY_STATE, 2
	msgbox TestArea_Text_MansionRaidStarted
TestArea_EventScript_HoennSailorDone::
	release
	end

TestArea_EventScript_SkipToHoennShip::
	setvar VAR_HOENN_STORY_STATE, 3
	setflag FLAG_HOENN_OPEN
	return

TestArea_Text_SailToSlateport:
	.string "Want a ride to SLATEPORT, HOENN?$"

TestArea_Text_StartMansionRaid:
	.string "Put TEAM AQUA and TEAM MAGMA in\n"
	.string "the POKéMON MANSION?$"

TestArea_Text_MansionRaidStarted:
	.string "They're on CINNABAR ISLAND now.$"

@ Debug card player: first the vintage card test if wanted; otherwise the CARD CASE and the
@ starter deck if the player has none, a card pack on every talk, then a duel against
@ CARD_OPPONENT_TEST_AREA_TESTER.
TestArea_EventScript_CardTester::
	lock
	faceplayer
	msgbox TestArea_Text_CardTesterVintage, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, TestArea_EventScript_CardTesterVintage
	checkitem ITEM_CARD_CASE
	call_if_eq VAR_RESULT, FALSE, TestArea_EventScript_CardTesterGiveCase
	msgbox TestArea_Text_CardTesterHereIsAPack
	closemessage
	special OpenCardPack
	cardduel CARD_OPPONENT_TEST_AREA_TESTER
	release
	end

TestArea_EventScript_CardTesterVintage::
	closemessage
	special OpenVintageCardTest
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

TestArea_Text_CardTesterVintage:
	.string "Want to see a card drawn like\n"
	.string "the old TCG cards?$"

TestArea_Text_CardTesterHereIsAPack:
	.string "Here's a card pack to test!$"

@ Takes the player to NEW BARK TOWN, Johto's starting town (docs/new_bark_town.md), by the west road.
TestArea_EventScript_JohtoGuide::
	lock
	faceplayer
	msgbox TestArea_Text_GoToNewBark, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, TestArea_EventScript_JohtoGuideDone
	closemessage
	warp MAP_NEW_BARK_TOWN, 1, 10
	waitstate
TestArea_EventScript_JohtoGuideDone::
	release
	end

TestArea_Text_GoToNewBark:
	.string "Want to see NEW BARK TOWN,\n"
	.string "in JOHTO?$"
