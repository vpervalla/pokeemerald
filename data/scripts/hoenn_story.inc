@ The way to Hoenn (docs/hoenn_plan.md, Act 0). After the first Hall of Fame, MOM passes on a call
@ from BLAINE (PalletTown_PlayersHouse_1F); Team Aqua and Team Magma are in the POKéMON MANSION for
@ the lab's last file, the PRIMAL NOTES; when the player beats SHELLY and TABITHA on B1F, they flee to
@ Hoenn with half of it each (PokemonMansion_B1F) and the SEAGALLOP at VERMILION sails to SLATEPORT
@ (VermilionCity). VAR_HOENN_STORY_STATE follows it.

@ The raiders are in the MANSION only while VAR_HOENN_STORY_STATE is 2. Called on transition by its floors.
HoennStory_EventScript_UpdateMansion::
	setflag FLAG_HIDE_MANSION_BLAINE
	setflag FLAG_HIDE_MANSION_RAIDERS
	goto_if_ne VAR_HOENN_STORY_STATE, 2, Common_EventScript_NopReturn
	clearflag FLAG_HIDE_MANSION_RAIDERS
	return

@ The sailor in SLATEPORT's and LILYCOVE's harbors, who sails back to VERMILION
HoennStory_EventScript_SeagallopSailor::
	lock
	faceplayer
	goto_if_set FLAG_HOENN_TEAM_STOLEN, HoennStory_EventScript_SeagallopCantSail
	message HoennStory_Text_SailBackToKanto
	waitmessage
	setvar VAR_0x8004, SEAGALLOP_SLATEPORT
	multichoice 20, 8, MULTI_KANTO_SEAGALLOP_VERMILION, FALSE
	switch VAR_RESULT
	case 0, Kanto_EventScript_SailToVermilionCity
	case 1, Kanto_EventScript_CancelSail
	case 127, Kanto_EventScript_CancelSail
	end

@ From the crossing to ARCHIE and MAXIE's visit after Hoenn's Hall of Fame, the link with Kanto is cut
HoennStory_EventScript_SeagallopCantSail::
	msgbox HoennStory_Text_SeagallopCantSail
	release
	end

HoennStory_Text_SeagallopCantSail:
	.string "Ahoy… The SEAGALLOP can't sail.\p"
	.string "With the radio smashed and the cable\n"
	.string "to KANTO cut, there's no word from\l"
	.string "the other side. It's not safe.\p"
	.string "I'll wait here until the line is\n"
	.string "mended.$"

HoennStory_Text_SailBackToKanto:
	.string "Ahoy! This SEAGALLOP sails back\n"
	.string "to VERMILION, in KANTO.\p"
	.string "Where do you want to sail?$"

@ Before BIRCH's lab (Act 1): the state that Emerald's truck, house, clock and Route 101 scenes would
@ have left, so its story goes on from the rival on Route 103. The player isn't from LITTLEROOT: the
@ house of their gender is a guest room BIRCH lends them, with no MOM, and they wake up there after
@ a whiteout.
HoennStory_EventScript_ReadyHoennOpening::
	setvar VAR_HOENN_STORY_STATE, 4
	setflag FLAG_ARRIVED_IN_HOENN
	setflag FLAG_HIDE_SLATEPORT_HARBOR_BIRCH
	setvar VAR_LITTLEROOT_INTRO_STATE, 7
	setvar VAR_LITTLEROOT_RIVAL_STATE, 3
	setvar VAR_LITTLEROOT_TOWN_STATE, 2
	setflag FLAG_RESCUED_BIRCH
	setvar VAR_ROUTE101_STATE, 3
	setflag FLAG_HIDE_ROUTE_101_BIRCH_ZIGZAGOON_BATTLE
	setflag FLAG_HIDE_ROUTE_101_BIRCH_STARTERS_BAG
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCHS_LAB_BIRCH
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_BEDROOM_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_1
	setflag FLAG_HIDE_LITTLEROOT_TOWN_PLAYERS_HOUSE_VIGOROTH_2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN
	setflag FLAG_MET_RIVAL_MOM
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, HoennStory_EventScript_ReadyHoennOpeningFemale
	@ The guest room is BRENDAN's house, the rival MAY's (as InsideOfTruck and MAY's bedroom would set)
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MAY
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_BEDROOM
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F
	return

HoennStory_EventScript_ReadyHoennOpeningFemale::
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_BRENDAN
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F
	return

@ The crossing to HOENN (docs/hoenn_plan.md, Act 1), on the black screen of SlateportCity_Harbor's arrival
HoennStory_Text_NightCrossing:
	.string "The night crossing was calm…\n"
	.string "until the SEAGALLOP stopped dead.\p"
	.string "Voices on deck… Red uniforms and\n"
	.string "blue ones, climbing from the sea…\p"
	.string "… … … … … …\p"
	.string "At dawn, the SEAGALLOP docked\n"
	.string "in SLATEPORT.\p"
	.string "{PLAYER}'s BAG was gone!\n"
	.string "And all the POKé BALLS with it!$"

HoennStory_Text_SailorAfterTheft:
	.string "SAILOR: TEAM AQUA and TEAM MAGMA,\n"
	.string "working together! From a submarine!\p"
	.string "They smashed my radio, and laughed\n"
	.string "that they'd cut the cable to KANTO.\p"
	.string "No PC link, no calls… We're cut\n"
	.string "off from home, {PLAYER}!$"

@ The Route 117 day care shares its POKéMON with FOUR ISLAND's, in KANTO (the same save data), so it can't take
@ or give any while the link with Kanto is cut
HoennStory_EventScript_DaycareClosed::
	msgbox HoennStory_Text_DaycareClosed
	release
	end

HoennStory_Text_DaycareClosed:
	.string "Our DAY CARE works together with\n"
	.string "the one on FOUR ISLAND, in KANTO.\p"
	.string "With the link to KANTO cut, we\n"
	.string "can't look after any POKéMON.\p"
	.string "Come back once it's mended, dear.$"

@ The Magma Hideout, once MAXIE has gone after GROUDON: his grunts kept the MEGA RING apart from the rest of the
@ stolen BAG, to study it. The team, the BAG and the money come back after Hoenn's Hall of Fame.
HoennStory_EventScript_FindMegaRing::
	goto_if_unset FLAG_HOENN_TEAM_STOLEN, Common_EventScript_NopReturn
	goto_if_set FLAG_HOENN_MEGA_RING_RETURNED, Common_EventScript_NopReturn
	special TakeStolenMegaRing
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	msgbox HoennStory_Text_CrateFromKanto
	giveitem ITEM_MEGA_RING
	setflag FLAG_HOENN_MEGA_RING_RETURNED
	msgbox HoennStory_Text_RestOfBagNotHere
	return

HoennStory_Text_CrateFromKanto:
	.string "Where MAXIE stood, there's a crate\n"
	.string "with the SEAGALLOP's mark on it…\p"
	.string "Inside, among TEAM MAGMA's notes on\n"
	.string "MEGA EVOLUTION, something shines!$"

HoennStory_Text_RestOfBagNotHere:
	.string "The rest of the BAG isn't here.\n"
	.string "The grunts must still have it…$"

@ Sootopolis, after RAYQUAZA: ARCHIE and MAXIE give up, and promise the player's team and the link back. They
@ come to LITTLEROOT after Hoenn's Hall of Fame (LittlerootTown).
HoennStory_EventScript_ArchieMaxiePromise::
	goto_if_unset FLAG_HOENN_TEAM_STOLEN, Common_EventScript_NopReturn
	msgbox HoennStory_Text_ArchieMaxiePromise
	return

HoennStory_Text_ArchieMaxiePromise:
	.string "ARCHIE: {PLAYER}… The POKé BALLS\n"
	.string "our grunts took on the SEAGALLOP…\p"
	.string "The storm scattered the grunts, and\n"
	.string "your POKéMON with them.\p"
	.string "MAXIE: We'll find every one of them.\n"
	.string "And the cable to KANTO will be mended.\p"
	.string "It will take time… But TEAM MAGMA\n"
	.string "and TEAM AQUA keep their word.$"

@ Hoenn's Hall of Fame, with the team still lost: ARCHIE and MAXIE will be waiting in LITTLEROOT
HoennStory_EventScript_ArchieMaxieComeToLittleroot::
	setvar VAR_HOENN_STORY_STATE, 6
	return

@ LITTLEROOT's transition: outside the guest room, BRENDAN's house or MAY's
HoennStory_EventScript_UpdateLittlerootArchieMaxie::
	setflag FLAG_HIDE_LITTLEROOT_ARCHIE_MAXIE
	goto_if_ne VAR_HOENN_STORY_STATE, 6, Common_EventScript_NopReturn
	clearflag FLAG_HIDE_LITTLEROOT_ARCHIE_MAXIE
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Common_EventScript_NopReturn
	setobjectxyperm LOCALID_LITTLEROOT_ARCHIE, 13, 10
	setobjectxyperm LOCALID_LITTLEROOT_MAXIE, 15, 10
	return

@ The first step outside after Hoenn's Hall of Fame: the team, the BAG and the link come back (RestoreKantoLink)
HoennStory_EventScript_ArchieMaxieLittleroot::
	lockall
	msgbox HoennStory_Text_ArchieMaxieKeptTheirWord
	special RestoreKantoLink
	playfanfare MUS_OBTAIN_ITEM
	message HoennStory_Text_TeamAndBagReturned
	waitmessage
	waitfanfare
	msgbox HoennStory_Text_CableMended
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_LITTLEROOT_ARCHIE
	removeobject LOCALID_LITTLEROOT_MAXIE
	setflag FLAG_HIDE_LITTLEROOT_ARCHIE_MAXIE
	setvar VAR_HOENN_STORY_STATE, 7
	fadescreen FADE_FROM_BLACK
	releaseall
	end

HoennStory_Text_ArchieMaxieKeptTheirWord:
	.string "ARCHIE: There you are! The new\n"
	.string "CHAMPION of HOENN, no less!\p"
	.string "MAXIE: We said we'd keep our word.\n"
	.string "It took every grunt we have…\p"
	.string "ARCHIE: Every POKé BALL we took off\n"
	.string "that SEAGALLOP, and your BAG too.\l"
	.string "Here, they're yours.$"

HoennStory_Text_TeamAndBagReturned:
	.string "{PLAYER} got back the BAG and\n"
	.string "the POKéMON from KANTO!\p"
	.string "The POKéMON were sent to the PC.$"

HoennStory_Text_CableMended:
	.string "MAXIE: And the cable to KANTO is\n"
	.string "mended. Your PC can reach all of its\l"
	.string "boxes again.\p"
	.string "ARCHIE: The SEAGALLOP can sail home\n"
	.string "whenever you like.\p"
	.string "MAXIE: You beat HOENN's LEAGUE with\n"
	.string "POKéMON you met here…\p"
	.string "Perhaps that's what we should have\n"
	.string "learned, too. Farewell, {PLAYER}.$"
