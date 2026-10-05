#ifndef GUARD_BATTLE_BG_H
#define GUARD_BATTLE_BG_H

void BattleInitBgsAndWindows(void);
void InitBattleBgsVideo(void);
void LoadBattleMenuWindowGfx(void);
extern bool8 gBattleBgShowsEnvironment;

void DrawMainBattleBackground(void);
void LoadBattleTextboxAndBackground(void);
void InitLinkBattleVsScreen(u8 taskId);
void DrawBattleEntryBackground(void);
bool8 LoadChosenBattleElement(u8 caseId);

#endif // GUARD_BATTLE_BG_H
