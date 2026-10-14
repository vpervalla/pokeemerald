@ The ELITE FOUR's MEGA STONES. In their rematch (once the Sevii Islands' network is linked,
@ FLAG_SYS_CAN_LINK_WITH_RS, as in FireRed), each of them has a POKéMON holding a MEGA STONE that
@ MR. M. left with them, and gives it to the player after the battle (see gym_rematches.inc for the
@ GYM LEADERS'). Each room calls its script after the post-battle text, and again whenever the
@ player talks to them, so with a full BAG the stone waits until they make room.
@ The champion (the rival, whose starter is always SQUIRTLE) gives the BLASTOISINITE of his
@ BLASTOISE. The player goes straight on to the Hall of Fame, so with a full BAG it's sent to the PC.

EliteFourMega_EventScript_Lorelei::
	goto_if_unset FLAG_SYS_CAN_LINK_WITH_RS, EliteFourMega_EventScript_Return
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_LORELEI, EliteFourMega_EventScript_Return
	msgbox EliteFourMega_Text_LoreleiGiveStone
	giveitem ITEM_SLOWBRONITE
	goto_if_eq VAR_RESULT, FALSE, EliteFourMega_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_LORELEI
	bufferspeciesname STR_VAR_1, SPECIES_SLOWBRO
	bufferitemname STR_VAR_2, ITEM_SLOWBRONITE
	msgbox GymRematch_Text_ExplainStone
	return

EliteFourMega_EventScript_Bruno::
	goto_if_unset FLAG_SYS_CAN_LINK_WITH_RS, EliteFourMega_EventScript_Return
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_BRUNO, EliteFourMega_EventScript_Return
	msgbox EliteFourMega_Text_BrunoGiveStone
	giveitem ITEM_STEELIXITE
	goto_if_eq VAR_RESULT, FALSE, EliteFourMega_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_BRUNO
	bufferspeciesname STR_VAR_1, SPECIES_STEELIX
	bufferitemname STR_VAR_2, ITEM_STEELIXITE
	msgbox GymRematch_Text_ExplainStone
	return

EliteFourMega_EventScript_Agatha::
	goto_if_unset FLAG_SYS_CAN_LINK_WITH_RS, EliteFourMega_EventScript_Return
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_AGATHA, EliteFourMega_EventScript_Return
	msgbox EliteFourMega_Text_AgathaGiveStone
	giveitem ITEM_GENGARITE
	goto_if_eq VAR_RESULT, FALSE, EliteFourMega_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_AGATHA
	bufferspeciesname STR_VAR_1, SPECIES_GENGAR
	bufferitemname STR_VAR_2, ITEM_GENGARITE
	msgbox GymRematch_Text_ExplainStone
	return

EliteFourMega_EventScript_Lance::
	goto_if_unset FLAG_SYS_CAN_LINK_WITH_RS, EliteFourMega_EventScript_Return
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_LANCE, EliteFourMega_EventScript_Return
	msgbox EliteFourMega_Text_LanceGiveStone
	giveitem ITEM_GYARADOSITE
	goto_if_eq VAR_RESULT, FALSE, EliteFourMega_EventScript_BagFull
	setflag FLAG_GOT_MEGA_STONE_FROM_LANCE
	bufferspeciesname STR_VAR_1, SPECIES_GYARADOS
	bufferitemname STR_VAR_2, ITEM_GYARADOSITE
	msgbox GymRematch_Text_ExplainStone
	return

EliteFourMega_EventScript_Champion::
	goto_if_unset FLAG_SYS_CAN_LINK_WITH_RS, EliteFourMega_EventScript_Return
	goto_if_set FLAG_GOT_MEGA_STONE_FROM_CHAMPION, EliteFourMega_EventScript_Return
	msgbox EliteFourMega_Text_ChampionGiveStone
	setflag FLAG_GOT_MEGA_STONE_FROM_CHAMPION
	giveitem ITEM_BLASTOISINITE
	goto_if_eq VAR_RESULT, FALSE, EliteFourMega_EventScript_ChampionStoneToPC
	bufferspeciesname STR_VAR_1, SPECIES_BLASTOISE
	bufferitemname STR_VAR_2, ITEM_BLASTOISINITE
	msgbox GymRematch_Text_ExplainStone
	return

EliteFourMega_EventScript_ChampionStoneToPC::
	addpcitem ITEM_BLASTOISINITE
	msgbox EliteFourMega_Text_ChampionStoneToPC
	return

EliteFourMega_EventScript_BagFull::
	msgbox EliteFourMega_Text_BagFull
	return

EliteFourMega_EventScript_Return::
	return

EliteFourMega_Text_LoreleiGiveStone:
    .string "LORELEI: Before you go…\p"
    .string "A man in a long coat visited me\n"
    .string "on FOUR ISLAND, not long ago.\p"
    .string "He left me this MEGA STONE, for\n"
    .string "the trainer who beats my SLOWBRO.\p"
    .string "That's you. Take it.$"

EliteFourMega_Text_BrunoGiveStone:
    .string "BRUNO: Wait!\p"
    .string "A stranger in a long coat found me\n"
    .string "while I was training.\p"
    .string "He gave me this MEGA STONE, for\n"
    .string "the trainer who beats my STEELIX.\p"
    .string "You earned it. Take it!$"

EliteFourMega_Text_AgathaGiveStone:
    .string "AGATHA: Hmph! One more thing.\p"
    .string "An odd man in a long coat gave me\n"
    .string "this stone. A MEGA STONE, he said.\p"
    .string "He wanted it handed to whoever\n"
    .string "beat my GENGAR. That's you…\p"
    .string "Take it, and don't you dare\n"
    .string "waste it!$"

EliteFourMega_Text_LanceGiveStone:
    .string "LANCE: One more thing.\p"
    .string "A man in a long coat entrusted\n"
    .string "me with this MEGA STONE.\p"
    .string "He asked me to give it to the\n"
    .string "trainer who defeated my GYARADOS.\p"
    .string "It belongs to you now.$"

EliteFourMega_Text_ChampionGiveStone:
    .string "…Tch. Here.\p"
    .string "Some guy in a long coat gave me\n"
    .string "this stone. A MEGA STONE.\p"
    .string "He said to hand it to whoever\n"
    .string "beats my BLASTOISE. Whatever!\p"
    .string "Take it. I've got my own stone\n"
    .string "anyway!$"

EliteFourMega_Text_ChampionStoneToPC:
    .string "Your BAG is full, so the\n"
    .string "BLASTOISINITE was sent to your PC.$"

EliteFourMega_Text_BagFull:
    .string "Your BAG is full.\n"
    .string "Make some room, then talk to me.$"
