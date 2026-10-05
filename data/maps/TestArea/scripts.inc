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
