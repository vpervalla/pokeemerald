@ The trainers who took the Kanto starters from OAK's lab (see docs/catch_them_all.md). LEAF took
@ BULBASAUR, and battles the player four times: ROUTE 24, CELADON CITY, ROUTE 23 by VICTORY ROAD, and
@ by CERULEAN CAVE after the Hall of Fame. RED took CHARMANDER, and waits deep in CERULEAN CAVE (where
@ MEWTWO was, before it left for APEX ISLAND) after the Hall of Fame. When the player is dressed as LEAF, KRIS takes her place, and when they're
@ dressed as RED, ETHAN takes his (their sprites do the same, see GetObjectEventGraphicsInfo).
@ Talking to them starts the battle. Afterwards they leave, and each LEAF also hides the earlier ones.
@ The post-game LEAF and RED give the player the MEGA STONE of their ace before leaving (with a full
@ BAG they wait for the player to make room).

@ STR_VAR_1 is LEAF or KRIS, and VAR_0x8009 is TRUE for KRIS.
LabTrainers_EventScript_BufferLeafName::
	setvar VAR_0x8009, FALSE
	bufferstring STR_VAR_1, LabTrainers_Text_Leaf
	specialvar VAR_RESULT, GetPlayerCostume
	goto_if_ne VAR_RESULT, PLAYER_COSTUME_FRLG, LabTrainers_EventScript_Return
	checkplayergender
	goto_if_ne VAR_RESULT, FEMALE, LabTrainers_EventScript_Return
	setvar VAR_0x8009, TRUE
	bufferstring STR_VAR_1, LabTrainers_Text_Kris
	return

@ STR_VAR_1 is RED or ETHAN, and VAR_0x8009 is TRUE for ETHAN.
LabTrainers_EventScript_BufferRedName::
	setvar VAR_0x8009, FALSE
	bufferstring STR_VAR_1, LabTrainers_Text_Red
	specialvar VAR_RESULT, GetPlayerCostume
	goto_if_ne VAR_RESULT, PLAYER_COSTUME_FRLG, LabTrainers_EventScript_Return
	checkplayergender
	goto_if_ne VAR_RESULT, MALE, LabTrainers_EventScript_Return
	setvar VAR_0x8009, TRUE
	bufferstring STR_VAR_1, LabTrainers_Text_Ethan
	return

LabTrainers_EventScript_Return::
	return

@ Called on transition by CERULEAN CITY and CERULEAN CAVE B1F: the post-game trainers appear once the
@ game is clear, until they've given the player their MEGA STONE.
LabTrainers_EventScript_UpdatePostgameTrainers::
	setflag FLAG_HIDE_LEAF_CERULEAN_CAVE
	setflag FLAG_HIDE_RED_CERULEAN_CAVE
	goto_if_unset FLAG_SYS_GAME_CLEAR, LabTrainers_EventScript_Return
	call_if_unset FLAG_GOT_VENUSAURITE_FROM_LEAF, LabTrainers_EventScript_ShowPostgameLeaf
	call_if_unset FLAG_GOT_CHARIZARDITE_X_FROM_RED, LabTrainers_EventScript_ShowPostgameRed
	return

LabTrainers_EventScript_ShowPostgameLeaf::
	clearflag FLAG_HIDE_LEAF_CERULEAN_CAVE
	return

LabTrainers_EventScript_ShowPostgameRed::
	clearflag FLAG_HIDE_RED_CERULEAN_CAVE
	return

LabTrainers_EventScript_LeafLeaves::
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	release
	end

@ ROUTE 24, past NUGGET BRIDGE
LabTrainers_EventScript_LeafRoute24::
	lock
	faceplayer
	playbgm MUS_RG_ENCOUNTER_GIRL, 0
	call LabTrainers_EventScript_BufferLeafName
	msgbox LabTrainers_Text_LeafRoute24Intro
	closemessage
	goto_if_eq VAR_0x8009, TRUE, LabTrainers_EventScript_KrisRoute24Battle
	trainerbattle_no_intro TRAINER_LEAF_ROUTE24, LabTrainers_Text_LeafRoute24Defeat
	goto LabTrainers_EventScript_LeafRoute24After

LabTrainers_EventScript_KrisRoute24Battle::
	trainerbattle_no_intro TRAINER_KRIS_ROUTE24, LabTrainers_Text_LeafRoute24Defeat
LabTrainers_EventScript_LeafRoute24After::
	msgbox LabTrainers_Text_LeafRoute24After
	setflag FLAG_HIDE_LEAF_ROUTE24
	goto LabTrainers_EventScript_LeafLeaves

@ CELADON CITY, outside the DEPARTMENT STORE
LabTrainers_EventScript_LeafCeladon::
	lock
	faceplayer
	playbgm MUS_RG_ENCOUNTER_GIRL, 0
	call LabTrainers_EventScript_BufferLeafName
	msgbox LabTrainers_Text_LeafCeladonIntro
	closemessage
	goto_if_eq VAR_0x8009, TRUE, LabTrainers_EventScript_KrisCeladonBattle
	trainerbattle_no_intro TRAINER_LEAF_CELADON, LabTrainers_Text_LeafCeladonDefeat
	goto LabTrainers_EventScript_LeafCeladonAfter

LabTrainers_EventScript_KrisCeladonBattle::
	trainerbattle_no_intro TRAINER_KRIS_CELADON, LabTrainers_Text_LeafCeladonDefeat
LabTrainers_EventScript_LeafCeladonAfter::
	msgbox LabTrainers_Text_LeafCeladonAfter
	setflag FLAG_HIDE_LEAF_ROUTE24
	setflag FLAG_HIDE_LEAF_CELADON
	goto LabTrainers_EventScript_LeafLeaves

@ ROUTE 23, by the entrance of VICTORY ROAD
LabTrainers_EventScript_LeafRoute23::
	lock
	faceplayer
	playbgm MUS_RG_ENCOUNTER_GIRL, 0
	call LabTrainers_EventScript_BufferLeafName
	msgbox LabTrainers_Text_LeafRoute23Intro
	closemessage
	goto_if_eq VAR_0x8009, TRUE, LabTrainers_EventScript_KrisRoute23Battle
	trainerbattle_no_intro TRAINER_LEAF_ROUTE23, LabTrainers_Text_LeafRoute23Defeat
	goto LabTrainers_EventScript_LeafRoute23After

LabTrainers_EventScript_KrisRoute23Battle::
	trainerbattle_no_intro TRAINER_KRIS_ROUTE23, LabTrainers_Text_LeafRoute23Defeat
LabTrainers_EventScript_LeafRoute23After::
	msgbox LabTrainers_Text_LeafRoute23After
	setflag FLAG_HIDE_LEAF_ROUTE24
	setflag FLAG_HIDE_LEAF_CELADON
	setflag FLAG_HIDE_LEAF_ROUTE23
	goto LabTrainers_EventScript_LeafLeaves

@ CERULEAN CITY, by CERULEAN CAVE, after the Hall of Fame
LabTrainers_EventScript_LeafCeruleanCave::
	lock
	faceplayer
	goto_if_set FLAG_DEFEATED_LEAF_POSTGAME, LabTrainers_EventScript_LeafGiveVenusaurite
	playbgm MUS_RG_ENCOUNTER_GIRL, 0
	call LabTrainers_EventScript_BufferLeafName
	msgbox LabTrainers_Text_LeafCeruleanCaveIntro
	closemessage
	goto_if_eq VAR_0x8009, TRUE, LabTrainers_EventScript_KrisCeruleanCaveBattle
	trainerbattle_no_intro TRAINER_LEAF_CERULEAN_CAVE, LabTrainers_Text_LeafCeruleanCaveDefeat
	goto LabTrainers_EventScript_LeafCeruleanCaveAfter

LabTrainers_EventScript_KrisCeruleanCaveBattle::
	trainerbattle_no_intro TRAINER_KRIS_CERULEAN_CAVE, LabTrainers_Text_LeafCeruleanCaveDefeat
LabTrainers_EventScript_LeafCeruleanCaveAfter::
	setflag FLAG_HIDE_LEAF_ROUTE24
	setflag FLAG_HIDE_LEAF_CELADON
	setflag FLAG_HIDE_LEAF_ROUTE23
	setflag FLAG_DEFEATED_LEAF_POSTGAME
	msgbox LabTrainers_Text_LeafTakeVenusaurite
LabTrainers_EventScript_LeafGiveVenusaurite::
	giveitem ITEM_VENUSAURITE
	goto_if_eq VAR_RESULT, FALSE, LabTrainers_EventScript_LeafBagFull
	setflag FLAG_GOT_VENUSAURITE_FROM_LEAF
	msgbox LabTrainers_Text_LeafCeruleanCaveAfter
	goto LabTrainers_EventScript_LeafLeaves

LabTrainers_EventScript_LeafBagFull::
	msgbox LabTrainers_Text_LeafBagFull
	release
	end

@ CERULEAN CAVE B1F, where MEWTWO was, after the Hall of Fame
LabTrainers_EventScript_RedCeruleanCave::
	lock
	faceplayer
	call LabTrainers_EventScript_BufferRedName
	goto_if_set FLAG_DEFEATED_RED_POSTGAME, LabTrainers_EventScript_RedGiveCharizarditeX
	msgbox LabTrainers_Text_RedIntro
	closemessage
	goto_if_eq VAR_0x8009, TRUE, LabTrainers_EventScript_EthanCeruleanCaveBattle
	trainerbattle_no_intro TRAINER_RED_CERULEAN_CAVE, LabTrainers_Text_RedDefeat
	goto LabTrainers_EventScript_RedCeruleanCaveAfter

LabTrainers_EventScript_EthanCeruleanCaveBattle::
	trainerbattle_no_intro TRAINER_ETHAN_CERULEAN_CAVE, LabTrainers_Text_RedDefeat
LabTrainers_EventScript_RedCeruleanCaveAfter::
	call LabTrainers_EventScript_BufferRedName
	setflag FLAG_DEFEATED_RED_POSTGAME
	msgbox LabTrainers_Text_RedTakeCharizarditeX
LabTrainers_EventScript_RedGiveCharizarditeX::
	giveitem ITEM_CHARIZARDITE_X
	goto_if_eq VAR_RESULT, FALSE, LabTrainers_EventScript_RedBagFull
	setflag FLAG_GOT_CHARIZARDITE_X_FROM_RED
	call LabTrainers_EventScript_BufferRedName
	msgbox LabTrainers_Text_RedAfter
	goto LabTrainers_EventScript_LeafLeaves

LabTrainers_EventScript_RedBagFull::
	msgbox LabTrainers_Text_RedBagFull
	release
	end

LabTrainers_Text_Leaf:
    .string "LEAF$"

LabTrainers_Text_Kris:
    .string "KRIS$"

LabTrainers_Text_Red:
    .string "RED$"

LabTrainers_Text_Ethan:
    .string "ETHAN$"

LabTrainers_Text_LeafRoute24Intro:
    .string "Hi! You're {PLAYER}, right?\n"
    .string "PROF. OAK told me about you!\p"
    .string "I'm {STR_VAR_1}. I'm the one who took\n"
    .string "BULBASAUR from his lab…\p"
    .string "Sorry you got there too late!\p"
    .string "Hey, how about a battle?\n"
    .string "I want to see how you're doing!$"

LabTrainers_Text_LeafRoute24Defeat:
    .string "Wow!\n"
    .string "You're really strong!$"

LabTrainers_Text_LeafRoute24After:
    .string "I'll train harder with my\n"
    .string "BULBASAUR. See you around!$"

LabTrainers_Text_LeafCeladonIntro:
    .string "{PLAYER}! Fancy meeting you here!\p"
    .string "I came to CELADON to go shopping\n"
    .string "for my POKéMON.\p"
    .string "Look, my BULBASAUR evolved into\n"
    .string "IVYSAUR!\p"
    .string "Let's see which of us has grown\n"
    .string "more!$"

LabTrainers_Text_LeafCeladonDefeat:
    .string "Aww…\n"
    .string "You're really something!$"

LabTrainers_Text_LeafCeladonAfter:
    .string "They say {RIVAL} is way ahead of\n"
    .string "everyone…\p"
    .string "Let's both keep going, {PLAYER}!$"

LabTrainers_Text_LeafRoute23Intro:
    .string "{PLAYER}! You made it here too!\p"
    .string "Before VICTORY ROAD, one last\n"
    .string "battle. Just you and me!\p"
    .string "My VENUSAUR and I won't hold\n"
    .string "back!$"

LabTrainers_Text_LeafRoute23Defeat:
    .string "That's it…\n"
    .string "You're ready for the LEAGUE.$"

LabTrainers_Text_LeafRoute23After:
    .string "Go on ahead, {PLAYER}.\n"
    .string "Show them what you've got!\p"
    .string "I'll be cheering for you!$"

LabTrainers_Text_LeafCeruleanCaveIntro:
    .string "{PLAYER}! Congratulations,\n"
    .string "CHAMPION!\p"
    .string "They say the strongest TRAINER of\n"
    .string "all waits deep in this cave…\p"
    .string "But first, I want a real rematch.\n"
    .string "My VENUSAUR has a new power too!$"

LabTrainers_Text_LeafCeruleanCaveDefeat:
    .string "Incredible…\n"
    .string "You really are the CHAMPION.$"

LabTrainers_Text_LeafTakeVenusaurite:
    .string "A tall man in a long coat gave me\n"
    .string "this stone, a MEGA STONE.\p"
    .string "He said to give it to the trainer\n"
    .string "who beat my VENUSAUR with it.\p"
    .string "So it's yours, {PLAYER}!$"

LabTrainers_Text_LeafBagFull:
    .string "Oh, your BAG is full!\n"
    .string "Make some room. I'll wait here.$"

LabTrainers_Text_LeafCeruleanCaveAfter:
    .string "If you ever raise a VENUSAUR,\n"
    .string "let it hold that stone!\p"
    .string "Thanks for battling with me all\n"
    .string "this time, {PLAYER}.\p"
    .string "The cave is all yours.\n"
    .string "Be careful in there!$"

LabTrainers_Text_RedIntro:
    .string "{STR_VAR_1}: …\p"
    .string "…\p"
    .string "…!$"

LabTrainers_Text_RedDefeat:
    .string "…$"

LabTrainers_Text_RedTakeCharizarditeX:
    .string "{STR_VAR_1}: …\p"
    .string "{STR_VAR_1} held out a MEGA STONE.$"

LabTrainers_Text_RedBagFull:
    .string "{STR_VAR_1}: …\p"
    .string "Your BAG is full. {STR_VAR_1} seems\n"
    .string "willing to wait.$"

LabTrainers_Text_RedAfter:
    .string "{STR_VAR_1} gave a small nod, and\n"
    .string "left without a word.$"
