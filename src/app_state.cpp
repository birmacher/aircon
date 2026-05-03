#include "app_state.h"

DeviceState state = STATE_BOOT;

Preferences prefs;
IRrecv irrecv(IR_RECV_PIN, IR_BUFFER_SIZE, IR_TIMEOUT, true);
IRsend irsend(IR_SEND_PIN);
decode_results irResults;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_SHT31 sht31;
WebServer server(80);
DNSServer dnsServer;

volatile decode_type_t detectedProtocol = decode_type_t::UNKNOWN;
volatile uint16_t      detectedBits     = 0;
volatile bool          isAcSupported    = false;

char deviceCode[5];
char apSsid[32];

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
