@ The weather forecasters in the Pokemon Centers of Viridian, Cerulean, Celadon and One Island.
@ Each map's script sets VAR_0x8004 to its FORECAST_* (include/constants/weather.h) and goes
@ here. The forecast comes from the dynamic weather's own draw (src/dynamic_weather.c), so
@ it is always right.
EventScript_WeatherForecaster::
	lock
	faceplayer
	msgbox Text_WeatherForecasterIntro, MSGBOX_DEFAULT
	setvar VAR_0x8005, 0
EventScript_WeatherForecasterArea::
	special DynamicWeather_BufferForecast
	goto_if_eq VAR_RESULT, FALSE, EventScript_WeatherForecasterDone
	msgbox Text_WeatherForecasterArea, MSGBOX_DEFAULT
	addvar VAR_0x8005, 1
	goto EventScript_WeatherForecasterArea
EventScript_WeatherForecasterDone::
	msgbox Text_WeatherForecasterOutro, MSGBOX_DEFAULT
	release
	end

Text_WeatherForecasterIntro:
	.string "I study the weather around here.\n"
	.string "Here's my forecast!$"

Text_WeatherForecasterArea:
	.string "{STR_VAR_1}:\n"
	.string "Today, {STR_VAR_2}.\l"
	.string "Tomorrow, {STR_VAR_3}.$"

Text_WeatherForecasterOutro:
	.string "The weather turns with the time\n"
	.string "of day. Do check back later!$"
