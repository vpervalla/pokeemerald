@ The side quests that give the player the two Kanto starters they didn't choose, so that every
@ POKéMON can be caught without trading (see docs/catch_them_all.md):
@ - MT. MOON: LUNA got lost and a wild CHARMANDER corners her. It's a SQUIRTLE if the player chose
@   CHARMANDER. Her mom waits in the ROUTE 4 POKéMON CENTER.
@ - CERULEAN CITY: a woman looks after an abandoned BULBASAUR that only trusts a TRAINER whose lead
@   POKéMON is very friendly. It's a SQUIRTLE if the player chose BULBASAUR.
@ VAR_STARTER_MON is 0 for BULBASAUR, 1 for SQUIRTLE and 2 for CHARMANDER.

@ Puts the species of the MT. MOON starter in VAR_0x8008 and its name in STR_VAR_1.
EventScript_GetMtMoonStarter::
	setvar VAR_0x8008, SPECIES_CHARMANDER
	call_if_eq VAR_STARTER_MON, 2, EventScript_StarterQuestSquirtle
	bufferspeciesname STR_VAR_1, VAR_0x8008
	return

@ Puts the species of the CERULEAN CITY starter in VAR_0x8008 and its name in STR_VAR_1.
EventScript_GetCeruleanStarter::
	setvar VAR_0x8008, SPECIES_BULBASAUR
	call_if_eq VAR_STARTER_MON, 0, EventScript_StarterQuestSquirtle
	bufferspeciesname STR_VAR_1, VAR_0x8008
	return

EventScript_StarterQuestSquirtle::
	setvar VAR_0x8008, SPECIES_SQUIRTLE
	return

@ The graphics of the starters, in VAR_OBJ_GFX_ID_0, for the maps' ON_TRANSITION scripts.
EventScript_SetMtMoonStarterGfx::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_STARTER_CHARMANDER
	call_if_eq VAR_STARTER_MON, 2, EventScript_SetSquirtleGfx
	return

EventScript_SetCeruleanStarterGfx::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_STARTER_BULBASAUR
	call_if_eq VAR_STARTER_MON, 0, EventScript_SetSquirtleGfx
	return

EventScript_SetSquirtleGfx::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_STARTER_SQUIRTLE
	return
