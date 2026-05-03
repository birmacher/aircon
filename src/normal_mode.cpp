#include "normal_mode.h"

static IRac* acController = nullptr;
static String acMode = "standard";
static unsigned long lastSensorUpdate = 0;
static const unsigned long SENSOR_UPDATE_INTERVAL = 10000;

void enterNormalMode() {
    state = STATE_NORMAL;
    Serial.println("Entering NORMAL mode");

    decode_type_t proto = (decode_type_t)prefs.getUChar("proto", (uint8_t)decode_type_t::UNKNOWN);
    uint16_t      bits  = prefs.getUShort("bits", 0);
    acMode              = prefs.getString("mode", "standard");

    Serial.printf("AC protocol: %s, bits: %u, mode: %s\n",
                  typeToString(proto).c_str(), bits, acMode.c_str());

    acController = new IRac(IR_SEND_PIN);

    sht31.begin(SHT31_I2C_ADDR);

    irrecv.enableIRIn();

    displayStatus("Ready", WiFi.localIP().toString());
}

void normalLoop() {
    if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
        lastSensorUpdate = millis();
        float t = sht31.readTemperature();
        float h = sht31.readHumidity();
        if (!isnan(t) && !isnan(h)) {
            char line[24];
            snprintf(line, sizeof(line), "%.1fC %.0f%%", t, h);
            displayStatus("Ready", line);

            // TODO: publishTelemetry(t, h);
        }
    }

    // TODO: MQTT subscribe, thermostat logic, HTTP REST API, HA autodiscovery
}
