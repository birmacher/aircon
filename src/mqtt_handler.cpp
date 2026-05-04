#include "mqtt_handler.h"
#include "normal_mode.h"

static char     mqttHost[64]          = {};
static uint16_t mqttPort              = 1883;
static char     mqttId[32]            = "aircon";
static unsigned long lastReconnect    = 0;

static void buildTopic(char* buf, size_t len, const char* suffix) {
    snprintf(buf, len, "%s/%s", mqttId, suffix);
}

static const char* modeStr(stdAc::opmode_t mode) {
    switch (mode) {
        case stdAc::opmode_t::kCool: return "cool";
        case stdAc::opmode_t::kHeat: return "heat";
        case stdAc::opmode_t::kDry:  return "dry";
        case stdAc::opmode_t::kFan:  return "fan_only";
        default:                     return "auto";
    }
}

void publishState() {
    if (!mqttClient.connected()) return;
    char payload[64];
    if (!acState.power) {
        snprintf(payload, sizeof(payload), "{\"mode\":\"off\",\"temp\":%d}", (int)acState.temp);
    } else {
        snprintf(payload, sizeof(payload), "{\"mode\":\"%s\",\"temp\":%d}",
                 modeStr(acState.mode), (int)acState.temp);
    }
    char topic[72];
    buildTopic(topic, sizeof(topic), "state");
    mqttClient.publish(topic, payload, true);
}

void publishTelemetry(float t, float h) {
    if (!mqttClient.connected()) return;
    char val[16], topic[72];

    snprintf(val, sizeof(val), "%.1f", t);
    buildTopic(topic, sizeof(topic), "sensor/temperature");
    mqttClient.publish(topic, val);

    snprintf(val, sizeof(val), "%.1f", h);
    buildTopic(topic, sizeof(topic), "sensor/humidity");
    mqttClient.publish(topic, val);
}

static void publishHADiscovery() {
    // Build HA MQTT climate discovery payload
    char uid[48];
    snprintf(uid, sizeof(uid), "aircon_%s", deviceCode);

    static char disco[640];
    snprintf(disco, sizeof(disco),
        "{\"name\":\"Smart AC\","
        "\"uniq_id\":\"%s\","
        "\"modes\":[\"off\",\"cool\",\"heat\",\"auto\",\"dry\",\"fan_only\"],"
        "\"mode_cmd_t\":\"%s/set/mode\","
        "\"mode_stat_t\":\"%s/state\","
        "\"mode_stat_tpl\":\"{{value_json.mode}}\","
        "\"temp_cmd_t\":\"%s/set/temp\","
        "\"temp_stat_t\":\"%s/state\","
        "\"temp_stat_tpl\":\"{{value_json.temp}}\","
        "\"curr_temp_t\":\"%s/sensor/temperature\","
        "\"min_temp\":16,\"max_temp\":30,\"temp_step\":1}",
        uid, mqttId, mqttId, mqttId, mqttId, mqttId);

    char discTopic[80];
    snprintf(discTopic, sizeof(discTopic), "homeassistant/climate/%s/config", mqttId);
    mqttClient.publish(discTopic, disco, true);
}

static void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String val((char*)payload, length);
    val.trim();

    char modeTopic[72], tempTopic[72];
    buildTopic(modeTopic, sizeof(modeTopic), "set/mode");
    buildTopic(tempTopic, sizeof(tempTopic), "set/temp");

    if (strcmp(topic, modeTopic) == 0) {
        Serial.printf("MQTT cmd: mode=%s\n", val.c_str());
        if (val == "off") {
            acState.power = false;
        } else {
            acState.power = true;
            if      (val == "cool")     acState.mode = stdAc::opmode_t::kCool;
            else if (val == "heat")     acState.mode = stdAc::opmode_t::kHeat;
            else if (val == "dry")      acState.mode = stdAc::opmode_t::kDry;
            else if (val == "fan_only") acState.mode = stdAc::opmode_t::kFan;
            else                        acState.mode = stdAc::opmode_t::kAuto;
        }
        sendACCommand();
        publishState();
    } else if (strcmp(topic, tempTopic) == 0) {
        float newTemp = val.toFloat();
        Serial.printf("MQTT cmd: temp=%s\n", val.c_str());
        if (newTemp >= 16.0f && newTemp <= 30.0f) {
            acState.temp = newTemp;
            if (acState.power) sendACCommand();
            publishState();
        } else {
            Serial.printf("MQTT cmd: temp out of range, ignored\n");
        }
    }
}

static bool mqttConnect() {
    Serial.printf("MQTT connecting to %s:%u as '%s'...\n", mqttHost, mqttPort, mqttId);
    if (mqttClient.connect(mqttId)) {
        Serial.println("MQTT connected");
        char modeTopic[72], tempTopic[72];
        buildTopic(modeTopic, sizeof(modeTopic), "set/mode");
        buildTopic(tempTopic, sizeof(tempTopic), "set/temp");
        mqttClient.subscribe(modeTopic);
        mqttClient.subscribe(tempTopic);
        publishHADiscovery();
        publishState();
        return true;
    }
    Serial.printf("MQTT connect failed, rc=%d\n", mqttClient.state());
    return false;
}

void mqttSetup() {
    String host = prefs.getString("mqtt_host", "");
    mqttPort    = prefs.getUShort("mqtt_port", 1883);
    String id   = prefs.getString("mqtt_id", "aircon");

    if (host.length() == 0) {
        Serial.println("MQTT: no broker configured, skipping");
        return;
    }

    host.toCharArray(mqttHost, sizeof(mqttHost));
    id.toCharArray(mqttId, sizeof(mqttId));

    mqttClient.setServer(mqttHost, mqttPort);
    mqttClient.setCallback(onMqttMessage);
    mqttClient.setBufferSize(640);

    mqttConnect();
}

void mqttLoop() {
    if (mqttHost[0] == '\0') return;
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnect > 5000UL) {
            lastReconnect = now;
            mqttConnect();
        }
    }
    mqttClient.loop();
}
