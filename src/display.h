#ifndef AIRCON_DISPLAY_H
#define AIRCON_DISPLAY_H

#include "app_state.h"

void displayStatus(const String& line1, const String& line2 = "");
void displayBig(const String& msg);

#endif // AIRCON_DISPLAY_H
