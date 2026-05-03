#ifndef AIRCON_MQTT_HANDLER_H
#define AIRCON_MQTT_HANDLER_H

void mqttSetup();
void mqttLoop();
void publishState();
void publishTelemetry(float t, float h);

#endif // AIRCON_MQTT_HANDLER_H
