#ifndef GUARD_VS_SEEKER_H
#define GUARD_VS_SEEKER_H

void Task_VsSeeker_0(u8 taskId);
void ClearRematchStateByTrainerId(void);
void VsSeekerResetObjectMovementAfterChargeComplete(void);
bool8 UpdateVsSeekerStepCounter(void);
void MapResetTrainerRematches(void);
bool32 IsVsSeekerTrainer(u16 trainerId);
bool8 VsSeeker_ShouldTryRematchBattle(u16 trainerId);
bool8 VsSeeker_IsTrainerReadyForRematch(u16 trainerId);
u16 VsSeeker_GetRematchTrainerId(u16 trainerId);
void VsSeeker_ClearRematchStateOfLastTalked(void);

#endif // GUARD_VS_SEEKER_H
