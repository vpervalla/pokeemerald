#ifndef GUARD_MAP_PREVIEW_SCREEN_H
#define GUARD_MAP_PREVIEW_SCREEN_H

// FRLG's map preview screens: a picture of the place and its name, shown on entering some Kanto
// and Sevii dungeons from another map section. A cave's is shown on the black screen before
// the map, a forest's over the map as it fades in.
#define MPS_TYPE_CAVE   0
#define MPS_TYPE_FOREST 1
#define MPS_TYPE_ANY    2

bool8 MapHasPreviewScreen(u8 mapsec, u8 type);
bool8 MapHasPreviewScreenOnWarp(u8 type);
void MapPreview_InitBgs(void);
void MapPreview_LoadGfx(u8 mapsec);
bool32 MapPreview_IsGfxLoadFinished(void);
u16 MapPreview_CreateMapNameWindow(u8 mapsec);
void MapPreview_Unload(s32 windowId);
u16 MapPreview_GetDuration(u8 mapsec);
void MapPreview_StartForestTransition(u8 mapsec);
bool32 MapPreview_IsForestTransitionRunning(void);

#endif // GUARD_MAP_PREVIEW_SCREEN_H
