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

@ The Johto starters' side quests in Hoenn (docs/johto_starters_plan.md), early in the story, before WATTSON:
@ - ROUTE 104, the PRETTY PETAL FLOWER SHOP: a CHIKORITA from JOHTO won't eat until the player brings an
@   ORAN, a PECHA and a CHERI BERRY (level 5).
@ - DEWFORD TOWN: a HIKER's CYNDAQUIL is cornered by two ZUBAT in GRANITE CAVE 1F (level 10).
@ - ROUTE 109's SEASHORE HOUSE: a TOTODILE steals SODA POP and hides on ROUTE 109's, SLATEPORT's and
@   DEWFORD's beaches, one after the other, until it's found the third time (level 15).
@ The maps' own scripts are in Route104_PrettyPetalFlowerShop, DewfordTown_PokemonCenter_1F, GraniteCave_1F
@ and Route109_SeashoreHouse.

@ Call right after givemon, with the species in VAR_TEMP_TRANSFERRED_SPECIES: the fanfare, the nickname and
@ the PC's message. Leaves givemon's result in VAR_0x8008 (MON_CANT_GIVE: nothing was given).
JohtoStarters_EventScript_ReceivedMon::
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, Common_EventScript_NopReturn
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	playfanfare MUS_OBTAIN_ITEM
	message JohtoStarters_Text_ReceivedMon
	waitmessage
	waitfanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoStarters_EventScript_MonNamed
	goto_if_eq VAR_0x8008, MON_GIVEN_TO_PC, JohtoStarters_EventScript_NameMonInPC
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto JohtoStarters_EventScript_MonNamed

JohtoStarters_EventScript_NameMonInPC::
	call Common_EventScript_NameReceivedBoxMon
JohtoStarters_EventScript_MonNamed::
	call_if_eq VAR_0x8008, MON_GIVEN_TO_PC, Common_EventScript_TransferredToPC
	return

@ The TOTODILE thief. One object on each beach, all hidden by FLAG_HIDE_TOTODILE; each map's transition
@ calls this with its place in VAR_0x8004 (0 ROUTE 109, 1 SLATEPORT, 2 DEWFORD).
JohtoStarters_EventScript_UpdateTotodile::
	setflag FLAG_HIDE_TOTODILE
	goto_if_set FLAG_GOT_JOHTO_TOTODILE, Common_EventScript_NopReturn
	goto_if_ne VAR_TOTODILE_TIMES_FOUND, VAR_0x8004, Common_EventScript_NopReturn
	clearflag FLAG_HIDE_TOTODILE
	return

JohtoStarters_EventScript_Totodile::
	lock
	faceplayer
	playmoncry SPECIES_TOTODILE, CRY_MODE_NORMAL
	waitmoncry
	goto_if_ge VAR_TOTODILE_TIMES_FOUND, 2, JohtoStarters_EventScript_TotodileOutOfBreath
	msgbox JohtoStarters_Text_TotodileSpotted
	closemessage
	applymovement VAR_LAST_TALKED, JohtoStarters_Movement_TotodileHop
	waitmovement 0
	playse SE_M_SURF
	removeobject VAR_LAST_TALKED
	waitse
	setflag FLAG_HIDE_TOTODILE
	addvar VAR_TOTODILE_TIMES_FOUND, 1
	call_if_eq VAR_TOTODILE_TIMES_FOUND, 1, JohtoStarters_EventScript_TotodileSwamToSlateport
	call_if_eq VAR_TOTODILE_TIMES_FOUND, 2, JohtoStarters_EventScript_TotodileSwamToDewford
	release
	end

JohtoStarters_EventScript_TotodileSwamToSlateport::
	msgbox JohtoStarters_Text_TotodileSwamToSlateport
	return

JohtoStarters_EventScript_TotodileSwamToDewford::
	msgbox JohtoStarters_Text_TotodileSwamToDewford
	return

JohtoStarters_EventScript_TotodileOutOfBreath::
	msgbox JohtoStarters_Text_TotodileOutOfBreath
	msgbox JohtoStarters_Text_TotodileWantsToCome, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, JohtoStarters_EventScript_TotodileWaits
	checkitem ITEM_SODA_POP
	call_if_eq VAR_RESULT, TRUE, JohtoStarters_EventScript_TotodileDrinksSodaPop
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TOTODILE
	givemon SPECIES_TOTODILE, 15
	call JohtoStarters_EventScript_ReceivedMon
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, JohtoStarters_EventScript_NoRoomForMon
	setflag FLAG_GOT_JOHTO_TOTODILE
	setflag FLAG_HIDE_TOTODILE
	removeobject VAR_LAST_TALKED
	release
	end

JohtoStarters_EventScript_TotodileDrinksSodaPop::
	removeitem ITEM_SODA_POP
	msgbox JohtoStarters_Text_TotodileDrinksSodaPop
	return

JohtoStarters_EventScript_TotodileWaits::
	msgbox JohtoStarters_Text_TotodileWaits
	release
	end

JohtoStarters_EventScript_NoRoomForMon::
	msgbox JohtoStarters_Text_NoRoomForMon
	release
	end

JohtoStarters_Movement_TotodileHop:
	jump_in_place_down
	jump_in_place_down
	step_end

JohtoStarters_Text_ReceivedMon:
	.string "{PLAYER} received the {STR_VAR_1}!$"

JohtoStarters_Text_NoRoomForMon:
	.string "There's no room for it in your party\n"
	.string "or your BOXES. Make some room first!$"

JohtoStarters_Text_TotodileSpotted:
	.string "It's the POKéMON that steals the\n"
	.string "SEASHORE HOUSE's SODA POP!\p"
	.string "It's a TOTODILE! It snapped its jaws\n"
	.string "and dove into the sea!$"

JohtoStarters_Text_TotodileSwamToSlateport:
	.string "TOTODILE swam off to the north,\n"
	.string "toward SLATEPORT's beach.$"

JohtoStarters_Text_TotodileSwamToDewford:
	.string "TOTODILE swam off to the west,\n"
	.string "toward DEWFORD TOWN.$"

JohtoStarters_Text_TotodileOutOfBreath:
	.string "It's the TOTODILE again!\n"
	.string "It's too out of breath to swim away.$"

JohtoStarters_Text_TotodileDrinksSodaPop:
	.string "TOTODILE snatched a SODA POP from\n"
	.string "the BAG and gulped it down!$"

JohtoStarters_Text_TotodileWantsToCome:
	.string "TOTODILE is looking at {PLAYER}.\n"
	.string "It seems to want to come along.\p"
	.string "Will you take TOTODILE with you?$"

JohtoStarters_Text_TotodileWaits:
	.string "TOTODILE flopped down on the sand.\n"
	.string "It's still too tired to swim away.$"
