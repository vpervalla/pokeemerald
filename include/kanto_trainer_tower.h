#ifndef GUARD_KANTO_TRAINER_TOWER_H
#define GUARD_KANTO_TRAINER_TOWER_H

// Kanto (FRLG): TRAINER TOWER on SEVEN ISLAND, see src/kanto_trainer_tower.c. Its battles run as
// Emerald's BATTLE_TYPE_TRAINER_HILL (Emerald has no free battle type), and Trainer Hill's battle
// functions (src/trainer_hill.c) hand over to these while KantoTower_InBattle() is TRUE.
#include "constants/kanto_trainer_tower.h"

struct TrainerTowerTrainer
{
    u8 name[TRAINER_NAME_LENGTH + 1];
    u8 facilityClass; // TT_CLASS_*
    u8 textColor;     // FRLG's, unused here
    u16 speechBefore[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 speechWin[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 speechLose[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 speechAfter[EASY_CHAT_BATTLE_WORDS_COUNT];
    struct BattleTowerPokemon mons[PARTY_SIZE];
};

struct TrainerTowerFloor
{
    u8 id;
    u8 floorIdx;
    u8 challengeType; // CHALLENGE_TYPE_*
    u8 prize;         // TTPRIZE_*
    struct TrainerTowerTrainer trainers[MAX_TRAINERS_PER_FLOOR];
};

void CallTrainerTowerFunc(void);
void ResetTrainerTowerResults(void);
bool8 KantoTower_InBattle(void);
u8 KantoTower_GetOpponentClass(void);
void KantoTower_GetOpponentName(u8 *dst);
u8 KantoTower_GetFrontPic(void);
void KantoTower_GetBattleText(u8 *dst, bool8 playerWon);
void KantoTower_InitBattle(void);
void KantoTower_EndBattle(void);

#endif // GUARD_KANTO_TRAINER_TOWER_H
