#ifndef GUARD_DAY_NIGHT_H
#define GUARD_DAY_NIGHT_H

#include "constants/day_night.h"

u8 DayNight_GetPhase(void);
void DayNight_OnTilesetPalettesLoaded(void);
void DayNight_UpdateField(void);
void DayNight_TransferPlttBuffer(void);

#endif // GUARD_DAY_NIGHT_H
