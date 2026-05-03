#include "setup_portal.h"
#include <ArduinoJson.h>

const char SETUP_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<title>Smart AC Setup</title>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<style>
body{font-family:sans-serif;max-width:420px;margin:1em auto;padding:1em;background:#f4f4f4}
h2{color:#333;border-bottom:2px solid #06c;padding-bottom:.3em;margin-top:1.2em}
input,button,select{padding:.7em;width:100%;margin:.4em 0;box-sizing:border-box;
  font-size:1em;border:1px solid #ccc;border-radius:6px}
button{background:#06c;color:white;border:none;cursor:pointer;font-weight:bold}
button:disabled{background:#aaa;cursor:not-allowed}
#status{padding:.8em;background:white;border-radius:6px;margin:.5em 0;
  border-left:4px solid #888}
#status.ok{border-left-color:#0a0;background:#e8f5e8}
#status.bad{border-left-color:#c00;background:#fde8e8}
.note{font-size:.85em;color:#666;margin:.3em 0 .8em}
.code{font-family:monospace;font-size:1.2em;color:#06c;font-weight:bold}
</style></head><body>
<h1>Smart AC Setup</h1>
<p>Device code: <span class='code' id='code'></span></p>

<h2>1. WiFi Network</h2>
<input id='ssid' placeholder='WiFi network name'>
<input id='pass' type='password' placeholder='WiFi password'>

<h2>2. Learn AC Remote</h2>
<p class='note'>Point your AC remote at the device and press the POWER button.</p>
<button onclick='startLearn()'>Start Listening</button>
<div id='status'>Idle — press the button above</div>

<h2>3. MQTT Broker <span style='font-weight:normal;font-size:.85em;color:#666'>(optional)</span></h2>
<input id='mqtt_host' placeholder='Broker IP (e.g. 192.168.1.100)'>
<input id='mqtt_port' type='number' value='1883' placeholder='Port'>
<input id='mqtt_id' value='aircon' placeholder='Client ID / topic prefix'>
<p class='note'>Leave broker IP empty to disable MQTT / Home Assistant integration.</p>

<h2>4. Save</h2>
<button id='finishBtn' onclick='finish()' disabled>Save & Connect</button>

<script>
let pollTimer = null;
document.getElementById('code').innerText = location.host.includes('.') ? '' : '';
fetch('/api/device-info').then(r=>r.json()).then(d=>{
  document.getElementById('code').innerText = d.code;
});

function startLearn(){
  fetch('/api/ir-start').then(()=>{
    setStatus('Listening... press your AC remote', '');
    if(pollTimer) clearInterval(pollTimer);
    pollTimer = setInterval(checkIR, 800);
  });
}
function setStatus(msg, cls){
  const s = document.getElementById('status');
  s.innerText = msg;
  s.className = cls;
}
function checkIR(){
  fetch('/api/ir-status').then(r=>r.json()).then(d=>{
    if(d.detected){
      if(d.isAc){
        setStatus('✓ Detected: '+d.protocol+' ('+d.bits+' bits)', 'ok');
        clearInterval(pollTimer);
        document.getElementById('finishBtn').disabled = false;
      } else {
        setStatus('⚠ '+d.protocol+' detected but not supported as AC. Try again.', 'bad');
      }
    }
  });
}
function finish(){
  const ssid = document.getElementById('ssid').value.trim();
  const pass = document.getElementById('pass').value;
  const mqtt_host = document.getElementById('mqtt_host').value.trim();
  const mqtt_port = parseInt(document.getElementById('mqtt_port').value) || 1883;
  const mqtt_id = document.getElementById('mqtt_id').value.trim() || 'aircon';
  if(!ssid){ alert('WiFi name required'); return; }
  document.getElementById('finishBtn').disabled = true;
  setStatus('Saving...', '');
  fetch('/api/finish', {
    method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid, pass, mqtt_host, mqtt_port, mqtt_id})
  }).then(r=>r.json()).then(d=>{
    if(d.ok) setStatus('Saved! Device rebooting and connecting...', 'ok');
    else setStatus('Error: '+d.error, 'bad');
  });
}
</script>
</body></html>
)HTML";

static void handleRoot() {
    server.send_P(200, "text/html", SETUP_HTML);
}

static void handleDeviceInfo() {
    StaticJsonDocument<128> doc;
    doc["code"] = deviceCode;
    doc["mac"] = WiFi.macAddress();
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

static void handleIrStart() {
    detectedProtocol = decode_type_t::UNKNOWN;
    detectedBits     = 0;
    isAcSupported    = false;
    irrecv.resume();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handleIrStatus() {
    StaticJsonDocument<256> doc;
    bool detected = (detectedProtocol != decode_type_t::UNKNOWN);
    doc["detected"] = detected;
    doc["protocol"] = typeToString(detectedProtocol);
    doc["bits"]     = detectedBits;
    doc["isAc"]     = isAcSupported;
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

static void handleFinish() {
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
        return;
    }

    String ssid     = doc["ssid"]      | "";
    String pass     = doc["pass"]      | "";
    String mqttHost = doc["mqtt_host"] | "";
    int    mqttPort = doc["mqtt_port"] | 1883;
    String mqttId   = doc["mqtt_id"]   | "aircon";

    if (ssid.length() == 0 || detectedProtocol == decode_type_t::UNKNOWN || !isAcSupported) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing data\"}");
        return;
    }

    bool nvsOk = true;
    nvsOk &= prefs.putString("ssid",       ssid)                              > 0;
    nvsOk &= prefs.putString("pass",       pass)                              > 0;
    nvsOk &= prefs.putUChar ("proto",      (uint8_t)detectedProtocol);
    nvsOk &= prefs.putUShort("bits",       detectedBits);
    nvsOk &= prefs.putString("mqtt_host",  mqttHost)                          > 0;
    nvsOk &= prefs.putUShort("mqtt_port",  (uint16_t)constrain(mqttPort, 1, 65535));
    nvsOk &= prefs.putString("mqtt_id",    mqttId.length() > 0 ? mqttId : "aircon") > 0;
    nvsOk &= prefs.putBool  ("provisioned", true);

    if (!nvsOk) {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"storage error\"}");
        return;
    }

    server.send(200, "application/json", "{\"ok\":true}");
    delay(1500);
    ESP.restart();
}

void enterSetupMode(bool clearCreds) {
    state = STATE_SETUP;
    Serial.println("Entering SETUP mode");

    if (clearCreds) {
        // factory reset already cleared NVS before calling here
    }

    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid, AP_PASSWORD);
    IPAddress ip = WiFi.softAPIP();
    Serial.printf("AP: %s, IP: %s\n", apSsid, ip.toString().c_str());

    dnsServer.start(53, "*", ip);

    server.on("/",                    handleRoot);
    server.on("/api/device-info",     HTTP_GET,  handleDeviceInfo);
    server.on("/api/ir-start",        HTTP_GET,  handleIrStart);
    server.on("/api/ir-status",       HTTP_GET,  handleIrStatus);
    server.on("/api/finish",          HTTP_POST, handleFinish);
    server.on("/generate_204",        handleRoot);
    server.on("/gen_204",             handleRoot);
    server.on("/hotspot-detect.html", handleRoot);
    server.on("/connecttest.txt",     handleRoot);
    server.on("/redirect",            handleRoot);
    server.on("/ncsi.txt",            handleRoot);
    server.onNotFound(handleRoot);
    server.begin();

    irrecv.enableIRIn();

    displayStatus("Setup", String("Code:") + deviceCode);
}
