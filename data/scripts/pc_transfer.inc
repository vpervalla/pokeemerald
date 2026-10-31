@ VAR_0x8004 here is used by ChangePokemonNickname
Common_EventScript_GetGiftMonPartySlot::
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	return

Common_EventScript_NameReceivedBoxMon::
	fadescreen FADE_TO_BLACK
	special ChangeBoxPokemonNickname
	lock
	faceplayer
	return

Common_EventScript_TransferredToPC::
	bufferboxname STR_VAR_1, VAR_PC_BOX_TO_SEND_MON
	bufferspeciesname STR_VAR_2, VAR_TEMP_TRANSFERRED_SPECIES
	specialvar VAR_RESULT, GetPCOwner
	goto_if_eq VAR_RESULT, PC_OWNER_BILL, EventScript_TransferredBillsPC
	goto_if_eq VAR_RESULT, PC_OWNER_LANETTE, EventScript_TransferredLanettesPC
	goto EventScript_TransferredSomeonesPC

EventScript_TransferredBillsPC::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, TRUE, EventScript_BillsPCBoxFull
	msgbox gText_PkmnTransferredBillsPC, MSGBOX_DEFAULT
	return

EventScript_BillsPCBoxFull::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname STR_VAR_3, VAR_RESULT
	msgbox gText_PkmnTransferredBillsPCBoxFull, MSGBOX_DEFAULT
	return

EventScript_TransferredSomeonesPC::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, TRUE, EventScript_SomeonesPCBoxFull
	msgbox gText_PkmnTransferredSomeonesPC, MSGBOX_DEFAULT
	return

EventScript_SomeonesPCBoxFull::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname STR_VAR_3, VAR_RESULT
	msgbox gText_PkmnTransferredSomeonesPCBoxFull, MSGBOX_DEFAULT
	return

EventScript_TransferredLanettesPC::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, TRUE, EventScript_LanettesPCBoxFull
	msgbox gText_PkmnTransferredLanettesPC, MSGBOX_DEFAULT
	return

EventScript_LanettesPCBoxFull::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname STR_VAR_3, VAR_RESULT
	msgbox gText_PkmnTransferredLanettesPCBoxFull, MSGBOX_DEFAULT
	return

Common_EventScript_NoMoreRoomForPokemon::
	msgbox gText_NoMoreRoomForPokemon, MSGBOX_DEFAULT
	release
	end
