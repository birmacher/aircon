# Phase B — Port to ESP32-C6

Phase A (C3 SuperMini prototype) is complete. Phase B moves to the target SoC
with the same Phase A functionality, USB-powered, no battery yet.

## Board choice: ESP32-C6-DevKitC-1-N8 (8 MB)

This is the correct board. Concrete reasons tied to roadmap constraints:

| Constraint | Why C6-DevKitC-1-N8 satisfies it |
|---|---|
| Final form is a **WROOM-1 module** so it carries to the PCB unchanged | DevKitC-1 carries the **ESP32-C6-WROOM-1** module; DevKitM-1 uses the smaller MINI-1 module — wrong path for PCB reuse |
| 802.15.4 radio for Phase C (Zigbee or Thread) | C6 has Zigbee 3.0 + Thread 1.3 + BLE 5 + Wi-Fi 6 |
| Wi-Fi available at commissioning / OTA only | C6 has Wi-Fi 6, suitable for the Phase A captive portal flow |
| Battery target (~100 µA average, Phase D) | C6 deep sleep ≈ 7 µA, same ballpark as C3 — target is reachable |
| Headroom for Matter/Zigbee SDK + dual-OTA partitions | **N8 = 8 MB flash** (vs 4 MB on C3 SuperMini and on DevKitM-1). Current OTA layout is 1.5 MB × 2 = 3 MB; Matter SDK pushes app size noticeably, so 8 MB is the safer pick |

**Buy the N8 SKU specifically**, not the older 4 MB DevKitC-1.

## Alternatives considered and rejected

- **ESP32-C6-DevKitM-1** — MINI-1 module, only 4 MB flash. Fails the WROOM-1 reuse rule and tight on flash for Matter.
- **ESP32-H2** — 802.15.4 + BLE only, **no Wi-Fi**. The captive-portal commissioning and any Wi-Fi OTA path become impossible. The roadmap explicitly allows Wi-Fi at commissioning, so H2 over-constrains us.
- **ESP32-C5** — Wi-Fi 6 dual-band (2.4/5 GHz) + 802.15.4. Newer, less mature ecosystem, no benefit for an IR blaster on 2.4 GHz mesh.
- **Third-party C6 boards** (Seeed XIAO ESP32-C6, etc.) — use MINI module, smaller form factor but again non-WROOM-1.
- **Waveshare ESP32-C6-Zero** — bare ESP32-C6FH4/FH8 SiP chip with castellated pads. Two disqualifiers: (a) not a WROOM-1 module, fails reuse rule; (b) **GPIO10 and GPIO11 are not bonded out on the SiP-flash variants** (Waveshare wiki), so the proposed BTN1/BTN2 pin map cannot be used. SiP package also can't drop onto a WROOM-1-based PCB. Useful only as a secondary toy.

## Drawbacks of going with C6 (and how we handle each)

1. **PlatformIO official platform lags Arduino-ESP32 Core 3.0.** The official `platformio/platform-espressif32` has been slow on Core 3.0 (which is what enables C6). Use the community fork **pioarduino** (`https://github.com/pioarduino/platform-espressif32`) — it tracks Arduino-ESP32 3.0.x + ESP-IDF 5.x and has working `esp32-c6-devkitc-1` board support. Pin to a numbered release tag (e.g. `#55.03.38-1`); the repo has no `#stable` branch and `#develop` is unstable.
2. **IRremoteESP8266 v2.8.6 (currently pinned) does not compile on Arduino-ESP32 Core 3.x** because the Timer/RMT APIs changed. Fixed in **v2.9.0** (released 2026-01-02; changelog: "Esp32 Core version 3 support (#2144)"). We need to bump the lib pin.
3. **Strapping pin clash on I²C.** Current pins: `I2C_SDA=GPIO8`, `I2C_SCL=GPIO9` (`src/hw_config.h`). On C6, **GPIO8 and GPIO9 are strapping pins** that select boot mode. I²C's open-drain transients can cause boot-mode glitches. Remap to non-strapping GPIOs. Full C6 strap list for reference: GPIO4 (MTMS, SDIO sample edge), GPIO5 (MTDI, SDIO drive edge), GPIO8 (boot mode), GPIO9 (boot mode), GPIO15 (JTAG signal source). GPIO4/5 are SDIO-edge straps only — safe for IR I/O since this design uses no SDIO peripheral. GPIO15 must not float; we don't use it.
4. **USB CDC flags differ.** Current build flags `-DARDUINO_USB_MODE=1 -DARDUINO_USB_CDC_ON_BOOT=1` (`platformio.ini`) are USB-OTG-specific (S2/S3) and **must both be dropped on C6**. With `ARDUINO_USB_CDC_ON_BOOT=1` set, `HardwareSerial.h` redefines `Serial` as `USBSerial` (the USB-OTG CDC class), which doesn't exist on C6 — build fails with `'USBSerial' was not declared in this scope`. C6 routes `Serial` to USB-Serial-JTAG by default with no flags.
5. **Partition table is sized to exactly 4 MB** (`partitions.csv`). On the N8 board we should grow `app0`/`app1` (e.g., 2.5–3 MB each) and SPIFFS to use the available 8 MB. Optional for Phase B (current layout works), required by Phase C/F.
6. **Cost / availability** — DevKitC-1-N8 is ~$10–15 vs ~$5 for the C3 SuperMini. Hestore (Hungary) listed in the roadmap; Mouser/DigiKey/AliExpress also stock it.

## Concrete changes required for Phase B

### 1. `platformio.ini` — add C6 environment

Keep the C3 env around during the transition. Add an env using the pioarduino fork:

```ini
[env:esp32-c6-devkitc-1]
platform = https://github.com/pioarduino/platform-espressif32.git#55.03.38-1
board = esp32-c6-devkitc-1
framework = arduino
board_build.partitions = partitions.csv      ; or partitions_8mb.csv after grow
monitor_speed = 115200
lib_deps = ${common.lib_deps}                ; shared with C3 env via [common]
build_flags =
    -DCORE_DEBUG_LEVEL=3
    ; Both ARDUINO_USB_MODE and ARDUINO_USB_CDC_ON_BOOT are dropped on C6 (USB-OTG flags).
    ; With CDC_ON_BOOT=1, HardwareSerial.h aliases Serial → USBSerial (an OTG-only class) and the build fails.
```

Also bump the C3 env's `IRremoteESP8266` pin to `^2.9.0` so both envs use the same library; factor `lib_deps` into a `[common]` section to keep them in sync.

Verified: both envs build clean. C3: 78.3% flash. C6: 80.6% flash, RAM 14.1% (Wi-Fi 6 stack adds ~36 KB over C3).

### 2. `src/hw_config.h` — remap pins for C6

Strapping-safe remap (verify against the DevKitC-1 silkscreen + C6 datasheet IO_MUX when the board arrives — these are proposed defaults):

| Function | C3 pin (current) | C6 pin (proposed) | Reason |
|---|---|---|---|
| `IR_RECV_PIN` | GPIO0 | GPIO4 | GPIO0 not exposed cleanly on DevKitC-1 |
| `IR_SEND_PIN` | GPIO6 | GPIO5 | RMT-capable, no strap |
| `BTN1_PIN` | GPIO5 | GPIO10 | non-strap, no flash conflict |
| `BTN2_PIN` | GPIO4 | GPIO11 | non-strap |
| `BTN3_PIN` | GPIO3 | GPIO22 | non-strap |
| `I2C_SDA` | GPIO8 | GPIO6 | **avoid GPIO8 strap**; GPIO19 rejected (esp-idf #11975: SDIO_CLK default mux couples GPIO19↔20 at boot) |
| `I2C_SCL` | GPIO9 | GPIO7 | **avoid GPIO9 strap**; same reason as SDA |

GPIO12 and GPIO13 are reserved on C6 — they are hard-wired to USB-Serial-JTAG (D−/D+) and are not GPIO-matrix routable. Mark them as reserved in `hw_config.h` so future remaps don't clobber USB.

### 3. `partitions.csv` — optional grow to 8 MB

Defer until needed. Phase B replicates Phase A behavior; the existing 4 MB layout works. Grow before Phase C (Matter SDK) or Phase F (OTA).

### 4. `src/main.cpp` — power-up delay

The C3-SuperMini-specific LDO-stabilization delay near the top of `setup()` is harmless on C6 (a small `delay()`) — keep it or guard it with `#if CONFIG_IDF_TARGET_ESP32C3`. Do **not** use `#ifdef ARDUINO_ESP32C3_DEV`: that macro is board-variant-specific in arduino-esp32 and is not stable across cores. `CONFIG_IDF_TARGET_ESP32C3` is defined by the IDF target selection and works under both Arduino and ESP-IDF builds.

### Files that should NOT need changes

- `src/wifi_manager.*`, `src/setup_portal.*`, `src/mqtt_handler.*`, `src/normal_mode.*`, `src/display.*`, `src/button_handler.*`, `src/ir_handler.*`, `src/app_state.*` — all use `hw_config.h` constants, no SoC-specific code.
- `IRremoteESP8266` handles RMT abstraction internally; we do not configure RMT channels in user code.

## Verification (Phase B done-when criteria)

After receiving the DevKitC-1-N8:

1. **Toolchain works**: `~/.platformio/penv/bin/pio run -e esp32-c6-devkitc-1` builds clean.
2. **Flash + boot**: `pio run -e esp32-c6-devkitc-1 --target upload` flashes; serial monitor shows boot banner.
3. **OLED**: `displayBig("BOOT")` renders on the SSD1306.
4. **SHT30**: I²C scan finds 0x44; temperature/humidity readings sane.
5. **IR receive (setup mode)**: factory-reset → captive portal → point known remote → protocol + bits decoded.
6. **IR transmit**: AC turns on/off, range ≥ 3 m (same bench test as Phase A).
7. **Wi-Fi + MQTT**: device connects to home AP; Home Assistant sees the entity via discovery; HA on/off command toggles AC.
8. **Reset button**: long-press triggers factory reset → re-enters captive portal.
