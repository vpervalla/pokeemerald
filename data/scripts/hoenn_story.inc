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
HoennStory_EventScript_SeagallopSailorSkipLock::
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
	msgbox HoennStory_Text_BlaineMessage
	call_if_unset FLAG_SYS_CAN_LINK_WITH_RS, HoennStory_EventScript_BlaineMessageSevii
	msgbox HoennStory_Text_BlaineMessageEnd
	msgbox HoennStory_Text_SeeBirch
	call_if_lt VAR_MT_PYRE_STATE, 3, HoennStory_EventScript_MeetAtMtPyre
	call_if_ge VAR_MT_PYRE_STATE, 3, HoennStory_EventScript_OldLadyAskedForPlayer
	msgbox HoennStory_Text_ArchieMaxieFarewell
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
	.string "MAXIE: The first message through the\n"
	.string "cable was for you, from BLAINE.$"

@ What waits in KANTO now (the rematches open with FLAG_HOENN_KANTO_LINK, see docs/mega_island.md)
HoennStory_Text_BlaineMessage:
	.string "“{PLAYER}, while you were away,\n"
	.string "KANTO's GYM LEADERS and the ELITE\l"
	.string "FOUR trained new teams.\p"
	.string "“A stranger in a long coat left each\n"
	.string "of them a MEGA STONE, for the one\l"
	.string "who beats them.\p"
	.string "“The SAFFRON DOJO's master has one\n"
	.string "too, and two old friends of yours\l"
	.string "wait by CERULEAN CAVE.$"

@ The LEAGUE also needs the Sevii Islands' network (FireRed's FLAG_SYS_CAN_LINK_WITH_RS)
HoennStory_EventScript_BlaineMessageSevii::
	msgbox HoennStory_Text_BlaineMessageSevii
	return

HoennStory_Text_BlaineMessageSevii:
	.string "“The LEAGUE stays closed until\n"
	.string "LORELEI comes back. BILL says she's\l"
	.string "on the SEVII ISLANDS.$"

HoennStory_Text_BlaineMessageEnd:
	.string "“Come home soon, CHAMPION!”$"

@ Hoenn's post-game: BIRCH's MEGA STONE, and the ORBS at MT. PYRE (HoennStory_EventScript_TryGivePrimalOrbs)
HoennStory_EventScript_MeetAtMtPyre::
	msgbox HoennStory_Text_MeetAtMtPyre
	return

HoennStory_EventScript_OldLadyAskedForPlayer::
	msgbox HoennStory_Text_OldLadyAskedForPlayer
	return

HoennStory_Text_SeeBirch:
	.string "ARCHIE: Before you go, see PROF.\n"
	.string "BIRCH. He has something for you.$"

HoennStory_Text_MeetAtMtPyre:
	.string "MAXIE: And meet us at the summit of\n"
	.string "MT. PYRE. We owe its old lady the\l"
	.string "ORBS, and you should be there.$"

HoennStory_Text_OldLadyAskedForPlayer:
	.string "MAXIE: And the old lady of MT. PYRE\n"
	.string "asked for you. Go and see her.$"

HoennStory_Text_ArchieMaxieFarewell:
	.string "MAXIE: You beat HOENN's LEAGUE with\n"
	.string "POKéMON you met here…\p"
	.string "Perhaps that's what we should have\n"
	.string "learned, too. Farewell, {PLAYER}.$"

@ Act 4 (docs/hoenn_plan.md): once the ORBS are back at MT. PYRE and the player is HOENN's CHAMPION, the old lady
@ trusts them with both. Held, they bring out PRIMAL KYOGRE and PRIMAL GROUDON in battle.
HoennStory_EventScript_TryGivePrimalOrbs::
	goto_if_unset FLAG_HOENN_GAME_CLEAR, Common_EventScript_NopReturn
	goto_if_set FLAG_RECEIVED_PRIMAL_ORBS, Common_EventScript_NopReturn
	goto_if_lt VAR_MT_PYRE_STATE, 3, Common_EventScript_NopReturn
	msgbox HoennStory_Text_OldLadyTrustsPlayer
	giveitem ITEM_BLUE_ORB
	goto_if_eq VAR_RESULT, FALSE, HoennStory_EventScript_OrbsNoRoom
	giveitem ITEM_RED_ORB
	setflag FLAG_RECEIVED_PRIMAL_ORBS
	setflag FLAG_RETURNED_RED_OR_BLUE_ORB
	msgbox HoennStory_Text_OldLadyOrbsExplained
	release
	end

HoennStory_EventScript_OrbsNoRoom::
	msgbox gText_TooBadBagIsFull
	release
	end

HoennStory_Text_OldLadyTrustsPlayer:
	.string "Ah, the CHAMPION of HOENN…\p"
	.string "RAYQUAZA came down from the sky when\n"
	.string "your RING called it. It trusts you.\p"
	.string "The one RAYQUAZA trusts may keep\n"
	.string "the ORBS. Please, take them both.$"

HoennStory_Text_OldLadyOrbsExplained:
	.string "KYOGRE and GROUDON still sleep in\n"
	.string "the depths, it is said…\p"
	.string "Should you meet them, the ORBS will\n"
	.string "show you their PRIMAL forms.\p"
	.string "Only let them hold the ORBS for\n"
	.string "the one who calms them, child.$"

@ Act 4: after Hoenn's Hall of Fame, LILYCOVE's SEAGALLOP sailor also sails to SOUTHERN ISLAND, instead of
@ Emerald's EON TICKET. LATIAS waits there, then LATIOS, and their MEGA STONES (SouthernIsland_Interior).
HoennStory_EventScript_LilycoveSailor::
	lock
	faceplayer
	goto_if_unset FLAG_HOENN_GAME_CLEAR, HoennStory_EventScript_SeagallopSailorSkipLock
	msgbox HoennStory_Text_SailToSouthernIsland, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennStory_EventScript_SeagallopSailorSkipLock
	msgbox HoennStory_Text_SouthernIslandAllAboard
	closemessage
	warp MAP_SOUTHERN_ISLAND_EXTERIOR, 13, 22
	waitstate
	release
	end

HoennStory_Text_SailToSouthernIsland:
	.string "Ahoy, CHAMPION! Folks say two rare\n"
	.string "POKéMON were seen on SOUTHERN\l"
	.string "ISLAND, a speck of land to the south.\p"
	.string "Shall I sail you there?$"

HoennStory_Text_SouthernIslandAllAboard:
	.string "Hold on tight, then!$"

@ Act 4, optional: once the player holds the ORBS and has caught both LATI, the Space Center's director has seen
@ the MILLENNIUM COMET, and what fell with it
HoennStory_EventScript_TryGiveJirachi::
	goto_if_set FLAG_RECEIVED_JIRACHI, Common_EventScript_NopReturn
	goto_if_unset FLAG_RECEIVED_PRIMAL_ORBS, Common_EventScript_NopReturn
	goto_if_unset FLAG_CAUGHT_SOUTHERN_ISLAND_LATIOS, Common_EventScript_NopReturn
	lock
	faceplayer
	msgbox HoennStory_Text_MillenniumComet
	givemon SPECIES_JIRACHI, 5
	goto_if_eq VAR_RESULT, MON_CANT_GIVE, HoennStory_EventScript_JirachiNoRoom
	playfanfare MUS_OBTAIN_ITEM
	message HoennStory_Text_ReceivedJirachi
	waitmessage
	waitfanfare
	setflag FLAG_RECEIVED_JIRACHI
	msgbox HoennStory_Text_JirachiWish
	release
	end

HoennStory_EventScript_JirachiNoRoom::
	msgbox HoennStory_Text_JirachiNoRoom
	release
	end

@ Added (docs/catch_them_all.md, "Johto and Hoenn"): after JIRACHI, the director has tracked a meteorite to
@ Kanto's BIRTH ISLAND and gives the AURORA TICKET, which only Mystery Gift gave. VERMILION's SEAGALLOP then
@ sails there (DEOXYS), as for the ticket from the gift (data/scripts/gift_aurora_ticket.inc).
HoennStory_EventScript_TryGiveAuroraTicket::
	goto_if_unset FLAG_RECEIVED_JIRACHI, Common_EventScript_NopReturn
	goto_if_set FLAG_ENABLE_SHIP_BIRTH_ISLAND, Common_EventScript_NopReturn
	lock
	faceplayer
	msgbox HoennStory_Text_MeteoriteBirthIsland
	giveitem ITEM_AURORA_TICKET
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag FLAG_ENABLE_SHIP_BIRTH_ISLAND
	msgbox HoennStory_Text_AuroraTicketVermilion
	release
	end

HoennStory_Text_MillenniumComet:
	.string "DIRECTOR: Ah, the CHAMPION who\n"
	.string "met KYOGRE, GROUDON and RAYQUAZA!\p"
	.string "Last night, the MILLENNIUM COMET\n"
	.string "passed over MOSSDEEP. It comes once\l"
	.string "in a thousand years.\p"
	.string "And this little one was asleep on\n"
	.string "our launch pad this morning.\p"
	.string "It wakes for seven days every\n"
	.string "thousand years, they say…\p"
	.string "I think it came looking for you.\n"
	.string "Will you look after it?$"

HoennStory_Text_ReceivedJirachi:
	.string "{PLAYER} received JIRACHI!$"

HoennStory_Text_JirachiWish:
	.string "DIRECTOR: Legend says JIRACHI grants\n"
	.string "wishes. What would you wish for?$"

HoennStory_Text_JirachiNoRoom:
	.string "DIRECTOR: Oh, your party and your\n"
	.string "BOXES are full. Make some room!$"

HoennStory_Text_MeteoriteBirthIsland:
	.string "DIRECTOR: Our telescope followed\n"
	.string "a meteorite that fell with the\l"
	.string "MILLENNIUM COMET.\p"
	.string "It came down on BIRTH ISLAND, far\n"
	.string "to the south of KANTO's islands.\p"
	.string "There's a strange power in that\n"
	.string "rock. Will you go and see?\p"
	.string "The SEAGALLOP's company sent us\n"
	.string "this ticket. It's yours.$"

HoennStory_Text_AuroraTicketVermilion:
	.string "DIRECTOR: Show it to the SEAGALLOP's\n"
	.string "crew in VERMILION.$"

@ Act 5: BIRCH, after Hoenn's Hall of Fame, gives the MEGA STONE of the starter he gave (VAR_HOENN_STARTER)
HoennStory_EventScript_TryGiveStarterMegaStone::
	goto_if_unset FLAG_HOENN_GAME_CLEAR, Common_EventScript_NopReturn
	goto_if_set FLAG_RECEIVED_HOENN_STARTER_MEGA_STONE, Common_EventScript_NopReturn
	msgbox HoennStory_Text_BirchStarterStone
	switch VAR_HOENN_STARTER
	case 0, HoennStory_EventScript_GiveSceptilite
	case 1, HoennStory_EventScript_GiveBlazikenite
	goto HoennStory_EventScript_GiveSwampertite

HoennStory_EventScript_GiveSceptilite::
	giveitem ITEM_SCEPTILITE
	goto HoennStory_EventScript_GaveStarterMegaStone

HoennStory_EventScript_GiveBlazikenite::
	giveitem ITEM_BLAZIKENITE
	goto HoennStory_EventScript_GaveStarterMegaStone

HoennStory_EventScript_GiveSwampertite::
	giveitem ITEM_SWAMPERTITE
HoennStory_EventScript_GaveStarterMegaStone::
	goto_if_eq VAR_RESULT, FALSE, HoennStory_EventScript_StoneNoRoom
	setflag FLAG_RECEIVED_HOENN_STARTER_MEGA_STONE
	msgbox HoennStory_Text_BirchStarterStoneAfter
	goto HoennStory_EventScript_GiveOtherHoennStarters

@ Act 5: right after the MEGA STONE, BIRCH gives the two Hoenn starters the player didn't pick, at level 5,
@ one after the other. Each has its flag, so with no room the player gets the rest the next time.
HoennStory_EventScript_TryGiveOtherHoennStarters::
	goto_if_unset FLAG_RECEIVED_HOENN_STARTER_MEGA_STONE, Common_EventScript_NopReturn
	goto_if_set FLAG_RECEIVED_OTHER_HOENN_STARTER_2, Common_EventScript_NopReturn
HoennStory_EventScript_GiveOtherHoennStarters::
	goto_if_set FLAG_RECEIVED_OTHER_HOENN_STARTER_1, HoennStory_EventScript_GiveSecondOtherStarter
	msgbox HoennStory_Text_BirchOtherStarters
	switch VAR_HOENN_STARTER
	case 0, HoennStory_EventScript_FirstOtherStarterTorchic
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TREECKO
	goto HoennStory_EventScript_GiveFirstOtherStarter

HoennStory_EventScript_FirstOtherStarterTorchic::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TORCHIC
HoennStory_EventScript_GiveFirstOtherStarter::
	call HoennStory_EventScript_GiveOtherStarter
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, HoennStory_EventScript_OtherStarterNoRoom
	setflag FLAG_RECEIVED_OTHER_HOENN_STARTER_1
HoennStory_EventScript_GiveSecondOtherStarter::
	switch VAR_HOENN_STARTER
	case 2, HoennStory_EventScript_SecondOtherStarterTorchic
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_MUDKIP
	goto HoennStory_EventScript_GiveSecondOtherStarterMon

HoennStory_EventScript_SecondOtherStarterTorchic::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_TORCHIC
HoennStory_EventScript_GiveSecondOtherStarterMon::
	call HoennStory_EventScript_GiveOtherStarter
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, HoennStory_EventScript_OtherStarterNoRoom
	setflag FLAG_RECEIVED_OTHER_HOENN_STARTER_2
	msgbox HoennStory_Text_BirchOtherStartersAfter
	release
	end

@ Gives VAR_TEMP_TRANSFERRED_SPECIES; the result (MON_GIVEN_TO_PARTY, _TO_PC or MON_CANT_GIVE) is in VAR_0x8008.
HoennStory_EventScript_GiveOtherStarter::
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	givemon VAR_TEMP_TRANSFERRED_SPECIES, 5
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, Common_EventScript_NopReturn
	playfanfare MUS_OBTAIN_ITEM
	message HoennStory_Text_ReceivedOtherStarter
	waitmessage
	waitfanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennStory_EventScript_OtherStarterNamed
	goto_if_eq VAR_0x8008, MON_GIVEN_TO_PC, HoennStory_EventScript_NameOtherStarterInPC
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto HoennStory_EventScript_OtherStarterNamed

HoennStory_EventScript_NameOtherStarterInPC::
	call Common_EventScript_NameReceivedBoxMon
HoennStory_EventScript_OtherStarterNamed::
	call_if_eq VAR_0x8008, MON_GIVEN_TO_PC, Common_EventScript_TransferredToPC
	return

HoennStory_EventScript_OtherStarterNoRoom::
	msgbox HoennStory_Text_OtherStarterNoRoom
	release
	end

HoennStory_EventScript_StoneNoRoom::
	msgbox gText_TooBadBagIsFull
	release
	end

HoennStory_Text_BirchStarterStone:
	.string "PROF. BIRCH: {PLAYER}{KUN}! The new\n"
	.string "CHAMPION of HOENN!\p"
	.string "BLAINE sent this from KANTO, with\n"
	.string "his thanks. He made it from what the\l"
	.string "MANSION's file taught him.\p"
	.string "It's for the POKéMON I gave you, the\n"
	.string "first partner of your HOENN journey.$"

HoennStory_Text_BirchStarterStoneAfter:
	.string "PROF. BIRCH: With your MEGA RING,\n"
	.string "your partner can Mega Evolve now.\l"
	.string "Show me what it can do!\p"
	.string "Oh, and the SEAGALLOP sailor in\n"
	.string "LILYCOVE says two rare POKéMON were\l"
	.string "seen on SOUTHERN ISLAND.$"

HoennStory_Text_BirchOtherStarters:
	.string "PROF. BIRCH: There's one more thing.\p"
	.string "The two POKéMON you didn't choose\n"
	.string "that day are still in my bag.\p"
	.string "They've grown restless watching you\n"
	.string "on TV. They want to go with the\l"
	.string "CHAMPION. Here, take them both!$"

HoennStory_Text_ReceivedOtherStarter:
	.string "{PLAYER} received the {STR_VAR_1}\n"
	.string "from PROF. BIRCH!$"

HoennStory_Text_BirchOtherStartersAfter:
	.string "PROF. BIRCH: Now all three of my\n"
	.string "POKéMON are with you. Take good\l"
	.string "care of them, {PLAYER}{KUN}!$"

HoennStory_Text_OtherStarterNoRoom:
	.string "PROF. BIRCH: Your party and your\n"
	.string "BOXES are full! Make some room, and\l"
	.string "come back for the rest.$"

@ Act 5: BLAINE, the first time the player is back in his gym with the link mended
HoennStory_EventScript_BlaineHoennNews::
	goto_if_unset FLAG_HOENN_KANTO_LINK, Common_EventScript_NopReturn
	goto_if_set FLAG_BLAINE_HEARD_ABOUT_HOENN, Common_EventScript_NopReturn
	setflag FLAG_BLAINE_HEARD_ABOUT_HOENN
	goto_if_set FLAG_GAVE_PRIMAL_NOTES_TO_BIRCH, HoennStory_EventScript_BlaineGotNotes
	msgbox HoennStory_Text_BlaineHoennNews
	return

HoennStory_EventScript_BlaineGotNotes::
	msgbox HoennStory_Text_BlaineGotNotes
	return

HoennStory_Text_BlaineGotNotes:
	.string "BLAINE: Hah! The CHAMPION of two\n"
	.string "regions! BIRCH sent me the file.\p"
	.string "PRIMAL REVERSION, and that last page\n"
	.string "about RAYQUAZA… The missing piece of\l"
	.string "my lab's research, back at last.\p"
	.string "You've done KANTO proud. Now, how\n"
	.string "about a hot battle?$"

HoennStory_Text_BlaineHoennNews:
	.string "BLAINE: Hah! The CHAMPION of two\n"
	.string "regions! BIRCH told me everything.\p"
	.string "The file's half may be lost, but\n"
	.string "KYOGRE and GROUDON sleep again.\l"
	.string "That's what mattered.\p"
	.string "Now, how about a hot battle?$"

@ Act 5: STEVEN in MR. M's tournament remembers the player from HOENN
HoennStory_EventScript_StevenTournamentIntro::
	goto_if_set FLAG_HOENN_GAME_CLEAR, HoennStory_EventScript_StevenRemembers
	msgbox MegaIsland_Arena_Text_StevenIntro
	return

HoennStory_EventScript_StevenReplayIntro::
	goto_if_set FLAG_HOENN_GAME_CLEAR, HoennStory_EventScript_StevenRemembers
	msgbox MegaIsland_Arena_Text_ReplayStevenIntro
	return

HoennStory_EventScript_StevenRemembers::
	msgbox HoennStory_Text_StevenRemembers
	return

HoennStory_Text_StevenRemembers:
	.string "STEVEN: {PLAYER}! We meet again, far\n"
	.string "from SOOTOPOLIS.\p"
	.string "The trainer who calmed HOENN's storm\n"
	.string "and won its LEAGUE…\p"
	.string "METAGROSS and I won't hold back.\n"
	.string "Show us that power again!$"
