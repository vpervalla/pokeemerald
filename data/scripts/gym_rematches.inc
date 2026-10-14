@ The KANTO GYM LEADERS' rematches, after MR. M.'s invitation (VAR_MEGA_STORY_STATE 2 or more).
@ MR. M. (MEWTWO, seen as a tall man in a long coat) gave each leader one of the MEGA STONES taken
@ from the POKéMON MANSION, to pass on to the trainer who beats them. Their ace holds it in the
@ rematch (TRAINER_REMATCH_*, see src/data/gym_leader_rematch_trainers.h). The player needs a
@ POKéMON that can Mega Evolve to enter the tournament (CanPlayerMegaEvolve).
@ Each leader's script jumps here from the gym's own script.

GymRematch_EventScript_BagFull::
	msgbox GymRematch_Text_BagFull
	release
	end

@ BROCK, PEWTER CITY GYM: AERODACTYL and the AERODACTYLITE
GymRematch_EventScript_Brock::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_BROCK, GymRematch_EventScript_BrockDefeated
	msgbox GymRematch_Text_BrockIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_BrockDeclined
	msgbox GymRematch_Text_BrockAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_BROCK, GymRematch_Text_BrockDefeat
	goto GymRematch_EventScript_BrockGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_BrockGiveStone::
	msgbox GymRematch_Text_BrockGiveStone
	giveitem ITEM_AERODACTYLITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_BROCK
	bufferspeciesname STR_VAR_1, SPECIES_AERODACTYL
	bufferitemname STR_VAR_2, ITEM_AERODACTYLITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_BrockAfterGift
	release
	end

GymRematch_EventScript_BrockDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_BROCK, GymRematch_EventScript_BrockGiveStone
	msgbox GymRematch_Text_BrockAfterGift
	release
	end

GymRematch_EventScript_BrockDeclined::
	msgbox GymRematch_Text_BrockDeclined
	release
	end

@ MISTY, CERULEAN CITY GYM: BLASTOISE and the BLASTOISINITE
GymRematch_EventScript_Misty::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_MISTY, GymRematch_EventScript_MistyDefeated
	msgbox GymRematch_Text_MistyIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_MistyDeclined
	msgbox GymRematch_Text_MistyAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_MISTY, GymRematch_Text_MistyDefeat
	goto GymRematch_EventScript_MistyGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_MistyGiveStone::
	msgbox GymRematch_Text_MistyGiveStone
	giveitem ITEM_BLASTOISINITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_MISTY
	bufferspeciesname STR_VAR_1, SPECIES_BLASTOISE
	bufferitemname STR_VAR_2, ITEM_BLASTOISINITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_MistyAfterGift
	release
	end

GymRematch_EventScript_MistyDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_MISTY, GymRematch_EventScript_MistyGiveStone
	msgbox GymRematch_Text_MistyAfterGift
	release
	end

GymRematch_EventScript_MistyDeclined::
	msgbox GymRematch_Text_MistyDeclined
	release
	end

@ LT. SURGE, VERMILION CITY GYM: AMPHAROS and the AMPHAROSITE
GymRematch_EventScript_LtSurge::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_LT_SURGE, GymRematch_EventScript_LtSurgeDefeated
	msgbox GymRematch_Text_LtSurgeIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_LtSurgeDeclined
	msgbox GymRematch_Text_LtSurgeAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_LT_SURGE, GymRematch_Text_LtSurgeDefeat
	goto GymRematch_EventScript_LtSurgeGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_LtSurgeGiveStone::
	msgbox GymRematch_Text_LtSurgeGiveStone
	giveitem ITEM_AMPHAROSITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_LT_SURGE
	bufferspeciesname STR_VAR_1, SPECIES_AMPHAROS
	bufferitemname STR_VAR_2, ITEM_AMPHAROSITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_LtSurgeAfterGift
	release
	end

GymRematch_EventScript_LtSurgeDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_LT_SURGE, GymRematch_EventScript_LtSurgeGiveStone
	msgbox GymRematch_Text_LtSurgeAfterGift
	release
	end

GymRematch_EventScript_LtSurgeDeclined::
	msgbox GymRematch_Text_LtSurgeDeclined
	release
	end

@ ERIKA, CELADON CITY GYM: BEEDRILL and the BEEDRILLITE
GymRematch_EventScript_Erika::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_ERIKA, GymRematch_EventScript_ErikaDefeated
	msgbox GymRematch_Text_ErikaIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_ErikaDeclined
	msgbox GymRematch_Text_ErikaAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_ERIKA, GymRematch_Text_ErikaDefeat
	goto GymRematch_EventScript_ErikaGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_ErikaGiveStone::
	msgbox GymRematch_Text_ErikaGiveStone
	giveitem ITEM_BEEDRILLITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_ERIKA
	bufferspeciesname STR_VAR_1, SPECIES_BEEDRILL
	bufferitemname STR_VAR_2, ITEM_BEEDRILLITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_ErikaAfterGift
	release
	end

GymRematch_EventScript_ErikaDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_ERIKA, GymRematch_EventScript_ErikaGiveStone
	msgbox GymRematch_Text_ErikaAfterGift
	release
	end

GymRematch_EventScript_ErikaDeclined::
	msgbox GymRematch_Text_ErikaDeclined
	release
	end

@ KOGA, FUCHSIA CITY GYM: SCIZOR and the SCIZORITE
GymRematch_EventScript_Koga::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_KOGA, GymRematch_EventScript_KogaDefeated
	msgbox GymRematch_Text_KogaIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_KogaDeclined
	msgbox GymRematch_Text_KogaAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_KOGA, GymRematch_Text_KogaDefeat
	goto GymRematch_EventScript_KogaGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_KogaGiveStone::
	msgbox GymRematch_Text_KogaGiveStone
	giveitem ITEM_SCIZORITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_KOGA
	bufferspeciesname STR_VAR_1, SPECIES_SCIZOR
	bufferitemname STR_VAR_2, ITEM_SCIZORITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_KogaAfterGift
	release
	end

GymRematch_EventScript_KogaDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_KOGA, GymRematch_EventScript_KogaGiveStone
	msgbox GymRematch_Text_KogaAfterGift
	release
	end

GymRematch_EventScript_KogaDeclined::
	msgbox GymRematch_Text_KogaDeclined
	release
	end

@ SABRINA, SAFFRON CITY GYM: ALAKAZAM and the ALAKAZITE
GymRematch_EventScript_Sabrina::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_SABRINA, GymRematch_EventScript_SabrinaDefeated
	msgbox GymRematch_Text_SabrinaIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_SabrinaDeclined
	msgbox GymRematch_Text_SabrinaAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_SABRINA, GymRematch_Text_SabrinaDefeat
	goto GymRematch_EventScript_SabrinaGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_SabrinaGiveStone::
	msgbox GymRematch_Text_SabrinaGiveStone
	giveitem ITEM_ALAKAZITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_SABRINA
	bufferspeciesname STR_VAR_1, SPECIES_ALAKAZAM
	bufferitemname STR_VAR_2, ITEM_ALAKAZITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_SabrinaAfterGift
	release
	end

GymRematch_EventScript_SabrinaDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_SABRINA, GymRematch_EventScript_SabrinaGiveStone
	msgbox GymRematch_Text_SabrinaAfterGift
	release
	end

GymRematch_EventScript_SabrinaDeclined::
	msgbox GymRematch_Text_SabrinaDeclined
	release
	end

@ BLAINE, CINNABAR ISLAND GYM: HOUNDOOM and the HOUNDOOMINITE
GymRematch_EventScript_Blaine::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_BLAINE, GymRematch_EventScript_BlaineDefeated
	msgbox GymRematch_Text_BlaineIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_BlaineDeclined
	msgbox GymRematch_Text_BlaineAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_BLAINE, GymRematch_Text_BlaineDefeat
	goto GymRematch_EventScript_BlaineGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_BlaineGiveStone::
	msgbox GymRematch_Text_BlaineGiveStone
	giveitem ITEM_HOUNDOOMINITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_BLAINE
	bufferspeciesname STR_VAR_1, SPECIES_HOUNDOOM
	bufferitemname STR_VAR_2, ITEM_HOUNDOOMINITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_BlaineAfterGift
	release
	end

GymRematch_EventScript_BlaineDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_BLAINE, GymRematch_EventScript_BlaineGiveStone
	msgbox GymRematch_Text_BlaineAfterGift
	release
	end

GymRematch_EventScript_BlaineDeclined::
	msgbox GymRematch_Text_BlaineDeclined
	release
	end

@ GIOVANNI, VIRIDIAN CITY GYM: KANGASKHAN and the KANGASKHANITE
GymRematch_EventScript_Giovanni::
	lock
	faceplayer
	goto_if_defeated TRAINER_REMATCH_GIOVANNI, GymRematch_EventScript_GiovanniDefeated
	msgbox GymRematch_Text_GiovanniIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GymRematch_EventScript_GiovanniDeclined
	msgbox GymRematch_Text_GiovanniAccept
	closemessage
	trainerbattle_no_intro TRAINER_REMATCH_GIOVANNI, GymRematch_Text_GiovanniDefeat
	goto GymRematch_EventScript_GiovanniGiveStone
	end

@ Also where the player comes back to if their BAG was full
GymRematch_EventScript_GiovanniGiveStone::
	msgbox GymRematch_Text_GiovanniGiveStone
	giveitem ITEM_KANGASKHANITE
	goto_if_eq VAR_RESULT, FALSE, GymRematch_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_GIOVANNI
	bufferspeciesname STR_VAR_1, SPECIES_KANGASKHAN
	bufferitemname STR_VAR_2, ITEM_KANGASKHANITE
	msgbox GymRematch_Text_ExplainStone
	msgbox GymRematch_Text_GiovanniAfterGift
	release
	end

GymRematch_EventScript_GiovanniDefeated::
	goto_if_unset FLAG_GOT_MEGA_STONE_FROM_GIOVANNI, GymRematch_EventScript_GiovanniGiveStone
	msgbox GymRematch_Text_GiovanniAfterGift
	release
	end

GymRematch_EventScript_GiovanniDeclined::
	msgbox GymRematch_Text_GiovanniDeclined
	release
	end

GymRematch_Text_ExplainStone:
    .string "Let a {STR_VAR_1} hold the\n"
    .string "{STR_VAR_2}.\p"
    .string "With a MEGA RING, it will Mega\n"
    .string "Evolve in battle!$"

GymRematch_Text_BagFull:
    .string "Your BAG is full.\n"
    .string "Make some room and come back.$"

GymRematch_Text_BrockIntro:
    .string "BROCK: {PLAYER}! Good to see\n"
    .string "you again, CHAMPION.\p"
    .string "A tall man in a long coat came\n"
    .string "by the GYM not long ago.\p"
    .string "He left me this MEGA STONE for\n"
    .string "my AERODACTYL, and asked me to\l"
    .string "give it to a trainer who beats it.\p"
    .string "My rock-hard team is at full\n"
    .string "strength now. Will you battle me?$"

GymRematch_Text_BrockAccept:
    .string "Then show me your best!$"

GymRematch_Text_BrockDefeat:
    .string "BROCK: Even a Mega-Evolved\n"
    .string "AERODACTYL couldn't break you!$"

GymRematch_Text_BrockGiveStone:
    .string "BROCK: A promise is a promise.\n"
    .string "The AERODACTYLITE is yours.$"

GymRematch_Text_BrockAfterGift:
    .string "BROCK: The OLD AMBER in the\n"
    .string "MUSEUM holds an AERODACTYL…\p"
    .string "Let it hold that stone, and you'll\n"
    .string "see what I mean!$"

GymRematch_Text_BrockDeclined:
    .string "BROCK: Come back when you're\n"
    .string "ready. I'll be here.$"

GymRematch_Text_MistyIntro:
    .string "MISTY: Oh, it's the CHAMPION!\p"
    .string "Listen to this. A quiet man in a\n"
    .string "long coat gave me a MEGA STONE…\p"
    .string "…and asked me to give it to the\n"
    .string "first trainer who can beat me.\p"
    .string "My BLASTOISE has been practicing\n"
    .string "all week! Want to battle?$"

GymRematch_Text_MistyAccept:
    .string "All right! Here we go!$"

GymRematch_Text_MistyDefeat:
    .string "MISTY: Waah! Even a Mega\n"
    .string "BLASTOISE wasn't enough?$"

GymRematch_Text_MistyGiveStone:
    .string "MISTY: Fine, fine…\n"
    .string "Here's the BLASTOISINITE!$"

GymRematch_Text_MistyAfterGift:
    .string "MISTY: There's a SQUIRTLE in\n"
    .string "CERULEAN that needs a friend.\p"
    .string "Raise it well, and it'll Mega\n"
    .string "Evolve for you one day!$"

GymRematch_Text_MistyDeclined:
    .string "MISTY: Scared of water?\n"
    .string "Come back any time!$"

GymRematch_Text_LtSurgeIntro:
    .string "LT. SURGE: Hey, kid! CHAMPION\n"
    .string "now, huh?\p"
    .string "Some guy in a long coat walked\n"
    .string "right past my traps and handed me\l"
    .string "this stone. A MEGA STONE!\p"
    .string "Said to give it to whoever beats\n"
    .string "my AMPHAROS. Wanna try, kid?$"

GymRematch_Text_LtSurgeAccept:
    .string "Let's light it up!$"

GymRematch_Text_LtSurgeDefeat:
    .string "LT. SURGE: Whoa!\n"
    .string "You're the real deal, kid!$"

GymRematch_Text_LtSurgeGiveStone:
    .string "LT. SURGE: Orders are orders.\n"
    .string "The AMPHAROSITE is all yours!$"

GymRematch_Text_LtSurgeAfterGift:
    .string "LT. SURGE: Electric POKéMON\n"
    .string "made me who I am, kid.\p"
    .string "MAREEP live in the SAFARI ZONE.\n"
    .string "Go get one!$"

GymRematch_Text_LtSurgeDeclined:
    .string "LT. SURGE: Chicken, huh?\n"
    .string "Come back when you're ready!$"

GymRematch_Text_ErikaIntro:
    .string "ERIKA: Oh, welcome back.\n"
    .string "The flowers are in full bloom.\p"
    .string "A gentleman in a long coat\n"
    .string "visited my garden recently.\p"
    .string "He left a MEGA STONE with me…\n"
    .string "for a trainer able to defeat me.\p"
    .string "My BEEDRILL guards the flowers\n"
    .string "now. Shall we battle?$"

GymRematch_Text_ErikaAccept:
    .string "Then I shall not hold back.$"

GymRematch_Text_ErikaDefeat:
    .string "ERIKA: Oh my…\n"
    .string "I concede defeat.$"

GymRematch_Text_ErikaGiveStone:
    .string "ERIKA: As he asked, this\n"
    .string "BEEDRILLITE is now yours.$"

GymRematch_Text_ErikaAfterGift:
    .string "ERIKA: BEEDRILL and flowers\n"
    .string "live side by side.\p"
    .string "WEEDLE can be found in VIRIDIAN\n"
    .string "FOREST, if you'd like one.$"

GymRematch_Text_ErikaDeclined:
    .string "ERIKA: Then please, enjoy the\n"
    .string "flowers a while longer.$"

GymRematch_Text_KogaIntro:
    .string "KOGA: Fwahahaha!\n"
    .string "The CHAMPION returns!\p"
    .string "A man in a long coat slipped past\n"
    .string "my invisible walls… Impressive.\p"
    .string "He left me a MEGA STONE, to give\n"
    .string "to one who can best my SCIZOR.\p"
    .string "Will you face the full power of\n"
    .string "a ninja master?$"

GymRematch_Text_KogaAccept:
    .string "Then prepare yourself!$"

GymRematch_Text_KogaDefeat:
    .string "KOGA: Humph!\n"
    .string "Your skill is beyond question!$"

GymRematch_Text_KogaGiveStone:
    .string "KOGA: A ninja keeps his word.\n"
    .string "This SCIZORITE is yours.$"

GymRematch_Text_KogaAfterGift:
    .string "KOGA: SCYTHER hide in the\n"
    .string "SAFARI ZONE.\p"
    .string "With a METAL COAT, it becomes a\n"
    .string "SCIZOR, as swift as any ninja!$"

GymRematch_Text_KogaDeclined:
    .string "KOGA: A wise ninja knows when\n"
    .string "to retreat. Fwahaha!$"

GymRematch_Text_SabrinaIntro:
    .string "SABRINA: I knew you would come.\p"
    .string "A man in a long coat visited me.\n"
    .string "I could not read his mind at all.\p"
    .string "He left a MEGA STONE, meant for\n"
    .string "the one who defeats my ALAKAZAM.\p"
    .string "I foresee a battle.\n"
    .string "Will you accept it?$"

GymRematch_Text_SabrinaAccept:
    .string "Then let us begin.$"

GymRematch_Text_SabrinaDefeat:
    .string "SABRINA: This loss…\n"
    .string "I did not foresee it.$"

GymRematch_Text_SabrinaGiveStone:
    .string "SABRINA: The ALAKAZITE is yours.\n"
    .string "It was always meant to be.$"

GymRematch_Text_SabrinaAfterGift:
    .string "SABRINA: ABRA appear on ROUTES\n"
    .string "24 and 25.\p"
    .string "Raise one with care, and its mind\n"
    .string "will awaken further still.$"

GymRematch_Text_SabrinaDeclined:
    .string "SABRINA: You will return.\n"
    .string "I have seen it.$"

GymRematch_Text_BlaineIntro:
    .string "BLAINE: Hah! The CHAMPION!\n"
    .string "I've got a hot one for you!\p"
    .string "A stranger in a long coat came\n"
    .string "here. He brought me a stone…\p"
    .string "A MEGA STONE from the MANSION!\n"
    .string "I thought they had all burned!\p"
    .string "He told me to give it to whoever\n"
    .string "could beat its POKéMON.\p"
    .string "So! Ready to face a Mega-Evolved\n"
    .string "HOUNDOOM?$"

GymRematch_Text_BlaineAccept:
    .string "Hah! Burn, baby, burn!$"

GymRematch_Text_BlaineDefeat:
    .string "BLAINE: I've burned out\n"
    .string "completely!$"

GymRematch_Text_BlaineGiveStone:
    .string "BLAINE: Hah! A deal's a deal.\n"
    .string "Take the HOUNDOOMINITE!$"

GymRematch_Text_BlaineAfterGift:
    .string "BLAINE: That man… I wonder\n"
    .string "who he really was.\p"
    .string "Who else would know where our\n"
    .string "stones were? Hah!\p"
    .string "HOUNDOUR roam the SAFARI ZONE,\n"
    .string "if you want one of your own!$"

GymRematch_Text_BlaineDeclined:
    .string "BLAINE: Hah! Cold feet? Come\n"
    .string "back when you've warmed up!$"

GymRematch_Text_GiovanniIntro:
    .string "GIOVANNI: So, the CHAMPION.\n"
    .string "We meet again.\p"
    .string "I left to rebuild my strength.\n"
    .string "But a man found me…\p"
    .string "A tall man in a long coat.\n"
    .string "He gave me this MEGA STONE.\p"
    .string "He said a trainer would come to\n"
    .string "take it from me. You, no doubt.\p"
    .string "Shall we see if you can?$"

GymRematch_Text_GiovanniAccept:
    .string "Very well.$"

GymRematch_Text_GiovanniDefeat:
    .string "GIOVANNI: Ha…\n"
    .string "Nothing changes, it seems.$"

GymRematch_Text_GiovanniGiveStone:
    .string "GIOVANNI: Take it. The\n"
    .string "KANGASKHANITE is yours.$"

GymRematch_Text_GiovanniAfterGift:
    .string "GIOVANNI: KANGASKHAN live in\n"
    .string "the SAFARI ZONE.\p"
    .string "Use its power well, CHAMPION.$"

GymRematch_Text_GiovanniDeclined:
    .string "GIOVANNI: Hmph.\n"
    .string "I have no time for cowards.$"
