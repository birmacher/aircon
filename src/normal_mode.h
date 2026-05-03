#ifndef AIRCON_NORMAL_MODE_H
#define AIRCON_NORMAL_MODE_H

#include "app_state.h"
#include "display.h"

struct AcState {
    bool power             = false;
    stdAc::opmode_t mode   = stdAc::opmode_t::kAuto;
    float temp             = 22.0f;
};

extern AcState acState;

void enterNormalMode();
void normalLoop();
void sendACCommand();

#endif // AIRCON_NORMAL_MODE_H
