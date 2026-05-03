# Smart AC Controller

ESP32-C3 IR blaster with OLED, SHT31 temp/humidity sensor, captive portal setup, and Home Assistant integration.

## Commands

```bash
~/.platformio/penv/bin/pio run              # build
~/.platformio/penv/bin/pio run --target clean && ~/.platformio/penv/bin/pio run  # clean build
~/.platformio/penv/bin/pio run --target upload  # flash
~/.platformio/penv/bin/pio device monitor --baud 115200  # serial monitor
```

## Architecture

```
src/
├── main.cpp          # setup() + loop() only (~79 lines)
├── hw_config.h       # ALL pin/config/I2C constants — only file to change for Phase B (C6 port)
├── app_state.h/.cpp  # shared globals (extern pattern); DeviceState enum
├── display.h/.cpp    # displayStatus(), displayBig()
├── wifi_manager.h/.cpp
├── setup_portal.h/.cpp  # captive portal HTML + HTTP handlers (static) + enterSetupMode()
├── normal_mode.h/.cpp   # enterNormalMode(), normalLoop(), sendACCommand(); AcState struct
├── mqtt_handler.h/.cpp  # mqttSetup/Loop, publishState/Telemetry, HA discovery + command sub
├── ir_handler.h/.cpp    # processIR() — setup mode only (learn); called only in STATE_SETUP
└── button_handler.h/.cpp # handleResetButton(); factoryReset() is static (not exported)
```

State machine: `STATE_BOOT → STATE_SETUP | STATE_NORMAL`

## Key Constraints

- **NVS namespace:** `"acconfig"` — keys: `ssid`, `pass`, `proto`, `bits`, `mode`, `provisioned`, `force_setup`, `mqtt_host`, `mqtt_port`, `mqtt_id`
- `volatile` on `detectedProtocol`, `detectedBits`, `isAcSupported` — do not remove
- Button state vars are `static` in `button_handler.cpp` — not in `app_state`
- `acController`, `acMode`, `lastSensorUpdate` are `static` in `normal_mode.cpp`
- `acState` (AcState) is a non-static global in `normal_mode.cpp`, shared with `mqtt_handler.cpp`
- IR receiver active **only in STATE_SETUP**; `irrecv.disableIRIn()` called on entering normal mode

## Benchmarks

| Metric | Value |
|---|---|
| Flash | 81.7% (1,071,400 / 1,310,720 bytes) |
| RAM | 12.7% (41,500 / 327,680 bytes) |
| Clean build time | ~26s |

## Docs

- read under /docs to find roadmap, pin setup, etc.
