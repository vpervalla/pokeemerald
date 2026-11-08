#ifndef GUARD_FAME_CHECKER_H
#define GUARD_FAME_CHECKER_H

#include "main.h"
#include "constants/fame_checker.h"

enum {
    FCWINDOWID_LIST,
    FCWINDOWID_UIHELP,
    FCWINDOWID_MSGBOX,
    FCWINDOWID_ICONDESC
};

void ResetFameChecker(void);
void FullyUnlockFameChecker(void);
void UseFameChecker(MainCallback savedCallback);
void SetFlavorTextFlagFromSpecialVars(void);
void UpdatePickStateFromSpecialVar8005(void);
void CB2_OpenFameCheckerFromBag(void);
void CB2_OpenFameCheckerFromField(void);

#endif //GUARD_FAME_CHECKER_H
