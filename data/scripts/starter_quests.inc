@ The side quests that give the player the Kanto starters, so that every POKéMON can be caught
@ without trading (see docs/catch_them_all.md). Each starter always has its own quest, whichever
@ one the player chose in PROF. OAK's lab:
@ - VIRIDIAN FOREST: two BEEDRILL corner a lost BULBASAUR. Its trainer in VERMILION CITY gives the
@   player one of their BULBASAUR (ViridianForest and VermilionCity_House2 scripts).
@ - MT. MOON: LUNA got lost and a wild CHARMANDER corners her. Her mom waits in the ROUTE 4
@   POKéMON CENTER.
@ - CERULEAN CITY: a woman looks after an abandoned SQUIRTLE that only trusts a TRAINER whose lead
@   POKéMON is very friendly.

@ Puts the species of the MT. MOON starter in VAR_0x8008 and its name in STR_VAR_1.
EventScript_GetMtMoonStarter::
	setvar VAR_0x8008, SPECIES_CHARMANDER
	bufferspeciesname STR_VAR_1, VAR_0x8008
	return

@ Puts the species of the CERULEAN CITY starter in VAR_0x8008 and its name in STR_VAR_1.
EventScript_GetCeruleanStarter::
	setvar VAR_0x8008, SPECIES_SQUIRTLE
	bufferspeciesname STR_VAR_1, VAR_0x8008
	return
