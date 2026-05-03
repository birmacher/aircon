#include "button_handler.h"

static unsigned long buttonPressedAt = 0;
static bool buttonWasPressed = false;
static bool reconfigTriggered = false;

static void factoryReset() {
    Serial.println("FACTORY RESET");
    displayBig("WIPE");
    prefs.clear();
    delay(1500);
    ESP.restart();
}

void handleResetButton() {
    bool pressed = (digitalRead(BUTTON_PIN) == LOW);

    if (pressed && !buttonWasPressed) {
        buttonPressedAt = millis();
        buttonWasPressed = true;
        reconfigTriggered = false;
    } else if (pressed && buttonWasPressed) {
        unsigned long held = millis() - buttonPressedAt;

        if (held > FACTORY_RESET_HOLD_MS) {
            displayBig("WIPE!");
        } else if (held > RECONFIG_HOLD_MS) {
            if (!reconfigTriggered) {
                displayBig("RECFG");
                reconfigTriggered = true;
            }
        }
    } else if (!pressed && buttonWasPressed) {
        unsigned long held = millis() - buttonPressedAt;
        buttonWasPressed = false;

        if (held < BUTTON_DEBOUNCE_MS) {
            // debounce — ignore
        } else if (held >= FACTORY_RESET_HOLD_MS) {
            factoryReset();
        } else if (held >= RECONFIG_HOLD_MS) {
            Serial.println("Re-config triggered");
            displayBig("RECFG");
            delay(800);
            prefs.putBool("force_setup", true);
            ESP.restart();
        } else {
            Serial.println("Short button press");
        }
    }
}
