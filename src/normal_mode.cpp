#include "normal_mode.h"
#include "mqtt_handler.h"
#include "setup_portal.h"

static IRac* acController = nullptr;
static unsigned long lastSensorUpdate = 0;
static const unsigned long SENSOR_UPDATE_INTERVAL = 10000;

AcState acState;

void sendACCommand() {
    if (!acController) return;
    stdAc::state_t cmd = {};
    cmd.protocol  = detectedProtocol;
    cmd.model     = -1;
    cmd.power     = acState.power;
    cmd.mode      = acState.mode;
    cmd.degrees   = acState.temp;
    cmd.celsius   = true;
    cmd.fanspeed  = stdAc::fanspeed_t::kAuto;
    cmd.swingv    = stdAc::swingv_t::kOff;
    cmd.swingh    = stdAc::swingh_t::kOff;
    cmd.quiet     = false;
    cmd.turbo     = false;
    cmd.econo     = false;
    cmd.light     = false;
    cmd.filter    = false;
    cmd.clean     = false;
    cmd.beep      = false;
    cmd.sleep     = -1;
    cmd.clock     = -1;
    bool ok = acController->sendAc(cmd);
    Serial.printf("IR TX: power=%s mode=%d temp=%.1f ok=%d\n",
                  acState.power ? "on" : "off", (int)acState.mode, acState.temp, ok);
}

void enterNormalMode() {
    state = STATE_NORMAL;
    Serial.println("Entering NORMAL mode");

    detectedProtocol = (decode_type_t)prefs.getUChar("proto", (uint8_t)decode_type_t::UNKNOWN);
    detectedBits     = prefs.getUShort("bits", 0);

    Serial.printf("AC protocol: %s, bits: %u\n",
                  typeToString(detectedProtocol).c_str(), detectedBits);

    if (detectedProtocol == decode_type_t::UNKNOWN || !IRac::isProtocolSupported(detectedProtocol)) {
        Serial.println("Invalid AC protocol in NVS, forcing setup");
        enterSetupMode(false);
        return;
    }

    acController = new IRac(IR_SEND_PIN);

    sht31.begin(SHT31_I2C_ADDR);
    irrecv.disableIRIn();

    mqttSetup();

    displayStatus("Ready", WiFi.localIP().toString());
}

void normalLoop() {
    mqttLoop();

    if (millis() - lastSensorUpdate > SENSOR_UPDATE_INTERVAL) {
        lastSensorUpdate = millis();
        float t = sht31.readTemperature();
        float h = sht31.readHumidity();
        if (isnan(t) || isnan(h)) {
            delay(20);
            t = sht31.readTemperature();
            h = sht31.readHumidity();
        }
        if (!isnan(t) && !isnan(h)) {
            char line[24];
            snprintf(line, sizeof(line), "%.1fC %.0f%%", t, h);
            displayStatus("Ready", line);
            publishTelemetry(t, h);
        } else {
            Serial.println("SHT31 read error");
            displayStatus("Ready", "Sensor error");
        }
    }
}
