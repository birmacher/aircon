#ifndef AIRCON_APP_STATE_H
#define AIRCON_APP_STATE_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRutils.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>

#include "hw_config.h"

enum DeviceState { STATE_BOOT, STATE_SETUP, STATE_NORMAL };

extern DeviceState state;

extern Preferences prefs;
extern IRrecv irrecv;
extern IRsend irsend;
extern decode_results irResults;
extern Adafruit_SSD1306 display;
extern Adafruit_SHT31 sht31;
extern WebServer server;
extern DNSServer dnsServer;

extern volatile decode_type_t detectedProtocol;
extern volatile uint16_t      detectedBits;
extern volatile bool          isAcSupported;

extern bool displayReady;

extern char deviceCode[5];
extern char apSsid[32];

extern WiFiClient wifiClient;
extern PubSubClient mqttClient;

#endif // AIRCON_APP_STATE_H
