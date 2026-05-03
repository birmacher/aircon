#include "app_state.h"
#include "display.h"
#include "wifi_manager.h"
#include "button_handler.h"
#include "ir_handler.h"
#include "setup_portal.h"
#include "normal_mode.h"

// =================== SETUP & LOOP ===================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== Smart AC Controller ===");
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Display init
    Wire.begin(I2C_SDA, I2C_SCL);
    displayReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR);
    if (displayReady) {
        displayStatus("Boot", "");
    } else {
        Serial.println("Display init failed");
    }
    
    // IR send init (vétel csak később, mode-ban)
    irsend.begin();
    
    // NVS
    prefs.begin("acconfig", false);
    
    // Eszköz kód MAC-ből
    uint64_t mac = ESP.getEfuseMac();
    uint16_t code = (uint16_t)((mac >> 32) ^ mac);
    snprintf(deviceCode, sizeof(deviceCode), "%04X", code);
    snprintf(apSsid, sizeof(apSsid), "SmartAC-%s", deviceCode);
    
    Serial.printf("Device code: %s\n", deviceCode);
    Serial.printf("AP SSID: %s\n", apSsid);
    
    // Force setup flag (re-config után jött ide)
    bool forceSetup = prefs.getBool("force_setup", false);
    if (forceSetup) {
        prefs.putBool("force_setup", false);
        Serial.println("Re-config requested");
        enterSetupMode(false);  // NVS megmarad, csak a portál
        return;
    }
    
    // Routing: provisioned + sikeres connect → NORMAL, különben SETUP
    if (prefs.getBool("provisioned", false) && tryConnectSavedWifi()) {
        enterNormalMode();
    } else {
        if (prefs.getBool("provisioned", false)) {
            Serial.println("WiFi connect failed, falling back to setup");
        } else {
            Serial.println("Not provisioned, entering setup");
        }
        enterSetupMode(false);
    }
}

void loop() {
    handleResetButton();

    switch (state) {
        case STATE_SETUP:
            processIR();
            dnsServer.processNextRequest();
            server.handleClient();
            break;
        case STATE_NORMAL:
            normalLoop();
            break;
        default:
            break;
    }
    
    delay(2);
}