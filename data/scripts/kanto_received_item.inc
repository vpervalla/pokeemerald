@ FRLG's Std_ReceivedItem, for kanto_giveitem_msg (see asm/macros/event.inc): shows the message
@ loaded with loadword 0 with a fanfare, then says in which pocket the item was put away.
@ VAR_0x8000: item, VAR_0x8001: amount, VAR_0x8002: fanfare
Kanto_EventScript_ReceivedItem::
	call_if_eq VAR_0x8002, MUS_LEVEL_UP, Kanto_EventScript_PlayReceivedItemFanfare
	call_if_eq VAR_0x8002, MUS_RG_OBTAIN_KEY_ITEM, Kanto_EventScript_PlayReceivedKeyItemFanfare
	message NULL
	waitmessage
	waitfanfare
	call_if_eq VAR_0x8002, MUS_LEVEL_UP, Kanto_EventScript_WaitAfterReceivedItemFanfare
	bufferitemnameplural STR_VAR_2, VAR_0x8000, VAR_0x8001
	checkitemtype VAR_0x8000
	setvar VAR_0x8007, FALSE  @ buffers the pocket name without playing another fanfare
	call EventScript_BufferPocketNameAndTryFanfare
	msgbox gText_PutItemInPocket
	return

Kanto_EventScript_PlayReceivedItemFanfare::
	playfanfare MUS_LEVEL_UP
	return

Kanto_EventScript_PlayReceivedKeyItemFanfare::
	playfanfare MUS_RG_OBTAIN_KEY_ITEM
	return

Kanto_EventScript_WaitAfterReceivedItemFanfare::
	delay 50
	return
