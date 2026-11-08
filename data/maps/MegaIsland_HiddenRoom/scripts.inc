MegaIsland_HiddenRoom_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, MegaIsland_HiddenRoom_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, MegaIsland_HiddenRoom_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, MegaIsland_HiddenRoom_OnFrame
	.byte 0

@ The door behind MR. M. shuts behind the player, as it was shut for LEON and CYNTHIA: the way out of
@ this room has no warps (the red mat with none), so after MEWTWO the only way out is the passage
@ behind it and the island's cave, where the two champions meet the player (MegaIsland). Coming in
@ from the arena (on the way out, 4-6,7), the player hears it shut.
MegaIsland_HiddenRoom_OnLoad:
	setmetatile 4, 7, METATILE_MegaArena_RedMat_Left, FALSE
	setmetatile 5, 7, METATILE_MegaArena_RedMat_Mid, FALSE
	setmetatile 6, 7, METATILE_MegaArena_RedMat_Right, FALSE
	end

MegaIsland_HiddenRoom_OnTransition:
	setvar VAR_TEMP_1, 0
	getplayerxy VAR_0x8004, VAR_0x8005
	call_if_eq VAR_0x8005, 7, MegaIsland_HiddenRoom_EventScript_FromTheArena
	end

MegaIsland_HiddenRoom_EventScript_FromTheArena::
	setvar VAR_TEMP_1, 1
	return

MegaIsland_HiddenRoom_OnFrame:
	map_script_2 VAR_TEMP_1, 1, MegaIsland_HiddenRoom_EventScript_DoorShuts
	.2byte 0

MegaIsland_HiddenRoom_EventScript_DoorShuts::
	lockall
	setvar VAR_TEMP_1, 2
	delay 16
	playse SE_DOOR
	waitse
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	msgbox MegaIsland_HiddenRoom_Text_DoorShut
	releaseall
	end

MegaIsland_HiddenRoom_EventScript_DoorShut::
	msgbox MegaIsland_HiddenRoom_Text_DoorWontOpen, MSGBOX_SIGN
	end

MegaIsland_HiddenRoom_Text_DoorShut:
	.string "The door shut behind {PLAYER}!$"

MegaIsland_HiddenRoom_Text_DoorWontOpen:
	.string "The door is shut tight.\n"
	.string "It won't budge.$"
