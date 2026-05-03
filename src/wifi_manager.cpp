#include "wifi_manager.h"

bool tryConnectSavedWifi() {
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    if (ssid.length() == 0) return false;

    Serial.printf("Connecting to %s\n", ssid.c_str());
    displayStatus("WiFi", ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}
