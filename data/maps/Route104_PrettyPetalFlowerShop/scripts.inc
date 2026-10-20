Route104_PrettyPetalFlowerShop_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route104_PrettyPetalFlowerShop_OnTransition
	.byte 0

Route104_PrettyPetalFlowerShop_OnTransition:
	setflag FLAG_LANDMARK_FLOWER_SHOP
	goto_if_unset FLAG_MET_PRETTY_PETAL_SHOP_OWNER, Route104_PrettyPetalFlowerShop_EventScript_MoveShopOwner
	goto_if_unset FLAG_HOENN_BADGE03_GET, Route104_PrettyPetalFlowerShop_EventScript_MoveShopOwner
	setflag FLAG_TEMP_1
	end

Route104_PrettyPetalFlowerShop_EventScript_MoveShopOwner::
	setobjectxyperm LOCALID_FLOWER_SHOP_OWNER, 4, 6
	end

Route104_PrettyPetalFlowerShop_EventScript_ShopOwner::
	lock
	faceplayer
	goto_if_set FLAG_TEMP_1, Route104_PrettyPetalFlowerShop_EventScript_SellDecorations
	msgbox Route104_PrettyPetalFlowerShop_Text_ThisIsPrettyPetalFlowerShop, MSGBOX_DEFAULT
	goto_if_set FLAG_MET_PRETTY_PETAL_SHOP_OWNER, Route104_PrettyPetalFlowerShop_EventScript_AlreadyMet
	setflag FLAG_MET_PRETTY_PETAL_SHOP_OWNER
	msgbox Route104_PrettyPetalFlowerShop_Text_IntroLearnAboutBerries, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Route104_PrettyPetalFlowerShop_EventScript_ExplainBerries
	call_if_eq VAR_RESULT, NO, Route104_PrettyPetalFlowerShop_EventScript_DontExplainBerries
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_AlreadyMet::
	msgbox Route104_PrettyPetalFlowerShop_Text_LearnAboutBerries, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, Route104_PrettyPetalFlowerShop_EventScript_ExplainBerries
	call_if_eq VAR_RESULT, NO, Route104_PrettyPetalFlowerShop_EventScript_DontExplainBerries
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_ExplainBerries::
	msgbox Route104_PrettyPetalFlowerShop_Text_BerriesExplanation, MSGBOX_DEFAULT
	return

Route104_PrettyPetalFlowerShop_EventScript_DontExplainBerries::
	msgbox Route104_PrettyPetalFlowerShop_Text_FlowersBringHappiness, MSGBOX_DEFAULT
	return

Route104_PrettyPetalFlowerShop_EventScript_SellDecorations::
	message gText_PlayerWhatCanIDoForYou
	waitmessage
	pokemartdecoration2 Route104_PrettyPetalFlowerShop_Pokemart_Plants
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
Route104_PrettyPetalFlowerShop_Pokemart_Plants:
	.2byte DECOR_RED_PLANT
	.2byte DECOR_TROPICAL_PLANT
	.2byte DECOR_PRETTY_FLOWERS
	.2byte DECOR_COLORFUL_PLANT
	.2byte DECOR_BIG_PLANT
	.2byte DECOR_GORGEOUS_PLANT
	pokemartlistend

Route104_PrettyPetalFlowerShop_EventScript_WailmerPailGirl::
	lock
	faceplayer
	goto_if_unset FLAG_RECEIVED_WAILMER_PAIL, Route104_PrettyPetalFlowerShop_EventScript_GiveWailmerPail
	msgbox Route104_PrettyPetalFlowerShop_Text_WailmerPailExplanation, MSGBOX_DEFAULT
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_GiveWailmerPail::
	msgbox Route104_PrettyPetalFlowerShop_Text_YouCanHaveThis, MSGBOX_DEFAULT
	giveitem ITEM_WAILMER_PAIL
	msgbox Route104_PrettyPetalFlowerShop_Text_WailmerPailExplanation, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_WAILMER_PAIL
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_RandomBerryGirl::
	lock
	faceplayer
	dotimebasedevents
	goto_if_set FLAG_DAILY_FLOWER_SHOP_RECEIVED_BERRY, Route104_PrettyPetalFlowerShop_EventScript_AlreadyReceivedBerry
	msgbox Route104_PrettyPetalFlowerShop_Text_ImGrowingFlowers, MSGBOX_DEFAULT
	random 8
	addvar VAR_RESULT, FIRST_BERRY_INDEX
	giveitem VAR_RESULT
	goto_if_eq VAR_RESULT, 0, Common_EventScript_ShowBagIsFull
	setflag FLAG_DAILY_FLOWER_SHOP_RECEIVED_BERRY
	msgbox Route104_PrettyPetalFlowerShop_Text_MachineMixesBerries, MSGBOX_DEFAULT
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_AlreadyReceivedBerry::
	msgbox Route104_PrettyPetalFlowerShop_Text_MachineMixesBerries, MSGBOX_DEFAULT
	release
	end

@ The Johto starters' side quests (data/scripts/starter_quests.inc): a visitor from JOHTO and her CHIKORITA,
@ which won't eat until the player brings an ORAN, a PECHA and a CHERI BERRY.
Route104_PrettyPetalFlowerShop_EventScript_JohtoVisitor::
	lock
	faceplayer
	goto_if_set FLAG_GOT_JOHTO_CHIKORITA, Route104_PrettyPetalFlowerShop_EventScript_VisitorGaveChikorita
	goto_if_set FLAG_FED_FLOWER_SHOP_CHIKORITA, Route104_PrettyPetalFlowerShop_EventScript_OfferChikorita
	checkitem ITEM_ORAN_BERRY
	goto_if_eq VAR_RESULT, FALSE, Route104_PrettyPetalFlowerShop_EventScript_ChikoritaWontEat
	checkitem ITEM_PECHA_BERRY
	goto_if_eq VAR_RESULT, FALSE, Route104_PrettyPetalFlowerShop_EventScript_ChikoritaWontEat
	checkitem ITEM_CHERI_BERRY
	goto_if_eq VAR_RESULT, FALSE, Route104_PrettyPetalFlowerShop_EventScript_ChikoritaWontEat
	msgbox Route104_PrettyPetalFlowerShop_Text_YouHaveTheBerries, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route104_PrettyPetalFlowerShop_EventScript_KeepBerries
	removeitem ITEM_ORAN_BERRY
	removeitem ITEM_PECHA_BERRY
	removeitem ITEM_CHERI_BERRY
	playmoncry SPECIES_CHIKORITA, CRY_MODE_NORMAL
	msgbox Route104_PrettyPetalFlowerShop_Text_ChikoritaAte
	waitmoncry
	setflag FLAG_FED_FLOWER_SHOP_CHIKORITA
Route104_PrettyPetalFlowerShop_EventScript_OfferChikorita::
	msgbox Route104_PrettyPetalFlowerShop_Text_TakeChikorita, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route104_PrettyPetalFlowerShop_EventScript_DeclineChikorita
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CHIKORITA
	givemon SPECIES_CHIKORITA, 5
	call JohtoStarters_EventScript_ReceivedMon
	goto_if_eq VAR_0x8008, MON_CANT_GIVE, JohtoStarters_EventScript_NoRoomForMon
	setflag FLAG_GOT_JOHTO_CHIKORITA
	removeobject LOCALID_FLOWER_SHOP_CHIKORITA
	msgbox Route104_PrettyPetalFlowerShop_Text_TakeGoodCareOfChikorita
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_ChikoritaWontEat::
	msgbox Route104_PrettyPetalFlowerShop_Text_ChikoritaWontEat
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_KeepBerries::
	msgbox Route104_PrettyPetalFlowerShop_Text_KeepBerries
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_DeclineChikorita::
	msgbox Route104_PrettyPetalFlowerShop_Text_ChikoritaWillWait
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_VisitorGaveChikorita::
	msgbox Route104_PrettyPetalFlowerShop_Text_HowsChikorita
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_Chikorita::
	lock
	faceplayer
	playmoncry SPECIES_CHIKORITA, CRY_MODE_NORMAL
	goto_if_set FLAG_FED_FLOWER_SHOP_CHIKORITA, Route104_PrettyPetalFlowerShop_EventScript_ChikoritaHappy
	msgbox Route104_PrettyPetalFlowerShop_Text_ChikoritaDroops
	waitmoncry
	release
	end

Route104_PrettyPetalFlowerShop_EventScript_ChikoritaHappy::
	msgbox Route104_PrettyPetalFlowerShop_Text_ChikoritaHappy
	waitmoncry
	release
	end

Route104_PrettyPetalFlowerShop_Text_ChikoritaWontEat:
	.string "I came all the way from JOHTO with\n"
	.string "my CHIKORITA, but it won't eat.\p"
	.string "The sisters say it must miss the\n"
	.string "BERRIES of the gardens back home.\p"
	.string "An ORAN BERRY, a PECHA BERRY and a\n"
	.string "CHERI BERRY… They grow in HOENN too.\p"
	.string "Could you bring one of each? I saw\n"
	.string "trees with them out on ROUTE 104.$"

Route104_PrettyPetalFlowerShop_Text_YouHaveTheBerries:
	.string "Oh! You have an ORAN, a PECHA and\n"
	.string "a CHERI BERRY!\p"
	.string "May CHIKORITA have them?$"

Route104_PrettyPetalFlowerShop_Text_KeepBerries:
	.string "Oh… All right. CHIKORITA will be\n"
	.string "here if you change your mind.$"

Route104_PrettyPetalFlowerShop_Text_ChikoritaAte:
	.string "CHIKORITA ate all three BERRIES!\n"
	.string "The leaf on its head perked up!$"

Route104_PrettyPetalFlowerShop_Text_TakeChikorita:
	.string "It's eating again! Thank you!\p"
	.string "…Look at it. It won't take its eyes\n"
	.string "off you. I think it has chosen you.\p"
	.string "I'm going back to JOHTO soon, but\n"
	.string "CHIKORITA wants to stay with you.\p"
	.string "Will you take it with you?$"

Route104_PrettyPetalFlowerShop_Text_ChikoritaWillWait:
	.string "CHIKORITA will be sad… Come back\n"
	.string "for it, won't you?$"

Route104_PrettyPetalFlowerShop_Text_TakeGoodCareOfChikorita:
	.string "Take good care of CHIKORITA!\n"
	.string "Feed it plenty of BERRIES!$"

Route104_PrettyPetalFlowerShop_Text_HowsChikorita:
	.string "How's my CHIKORITA doing? I'm sure\n"
	.string "it's happy with you.$"

Route104_PrettyPetalFlowerShop_Text_ChikoritaDroops:
	.string "CHIKORITA: Chiko…\n"
	.string "The leaf on its head is drooping.$"

Route104_PrettyPetalFlowerShop_Text_ChikoritaHappy:
	.string "CHIKORITA: Chiko! Chikori!$"
