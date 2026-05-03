# Roadmap

Plan for evolving the project from the current ESP32-C3 SuperMini prototype
toward a custom-PCB, battery-powered ESP32-C6 design with Zigbee or
Matter-over-Thread integration.

## Targets

- **Single AC controlled** end-to-end via Home Assistant.
- **Hybrid power**: USB when plugged in, LiPo battery when not.
- **Battery life**: ≥ 1 year on a 1000 mAh LiPo (≤ ~100 µA average).
- **Custom PCB** as the final form, built around an Espressif WROOM-1 module
  so the same module can be reused without redesign.

## Hard constraint

Battery target rules out WiFi during normal operation. The radio in normal
use must be **802.15.4 only** (Zigbee end-device or Thread sleepy end-device).
WiFi may run only at commissioning or for OTA, not as the main control path.

## Phases

### A — Stabilize the C3 baseline (current hardware)

Get the prototype working end-to-end before changing anything physical.

- [x] IR learn + decode on GPIO0 (IRremoteESP8266)
- [ ] IR transmit on GPIO6, range > 3 m measured
- [x] SHT30 reads on I²C
- [x] OLED renders status
- [x] Buttons handled with debounce
- [x] Learned AC code persisted in NVS
- [ ] Minimal HA integration (WiFi + MQTT is fine here — throwaway)

**Done when:** a HA command turns the AC on/off and changes its state.

### B — Port to ESP32-C6-WROOM-1 DevKitC-1 (USB only)

Move to the target SoC with the existing functionality.

- [ ] Acquire ESP32-C6-DevKitC-1 (Hestore or alt source)
- [ ] Arduino-ESP32 core 3.0+ or ESP-IDF setup for C6
- [ ] Pin remap to C6 (different boot strap layout)
- [ ] Phase A code runs on C6 with same behavior
- [ ] IRremoteESP8266 + RMT verified on C6

**Done when:** the C6 DevKit replicates the C3 prototype's behavior.

### C — Zigbee or Matter-over-Thread integration

Decision deferred. Both run on the same 802.15.4 radio, so the choice is
firmware-only and does not affect hardware design.

- [ ] Decide: Zigbee (mature, Z2M) vs Matter-over-Thread (future-proof, multi-ecosystem)
- [ ] SDK setup (ESP-Zigbee-SDK or ESP-Matter)
- [ ] Pick device class: Thermostat (Zigbee cluster) or Thermostat device type (Matter)
- [ ] Commissioning works against HA (Z2M or Matter server)
- [ ] WiFi disabled in normal operation

**Done when:** AC is controlled from HA over 802.15.4 only, with no WiFi traffic.

### D — Battery prototype on the C6 DevKit

Validate hybrid power and battery life before designing a PCB.

- [ ] TP4056 (or MCP73831) charger module + LiPo (1000+ mAh)
- [ ] MCP1700-3302 LDO bypassing the DevKit's onboard regulator
- [ ] USB↔battery power-path (Schottky or P-FET)
- [ ] Battery voltage divider on a free ADC pin
- [ ] Switchable IR receiver supply (GPIO-controlled, off in normal operation)
- [ ] OLED auto-off after inactivity, on-demand wake
- [ ] 2N2222 → AO3400 logic-level N-MOSFET (saves base current, simpler drive)
- [ ] Bulk decoupling caps on 3V3 and at the IR receiver
- [ ] Deep sleep: average current ≤ 100 µA verified on a meter
- [ ] 1-week soak test, battery drop extrapolated to ≥ 1 year

**Done when:** measured average current and 1-week soak test confirm the
1-year-on-1000-mAh target. **No PCB work before this passes.**

### E — Custom PCB (KiCad)

Integrate the proven D architecture onto a single board.

- [ ] KiCad project, WROOM-1 reference schematic adapted
- [ ] Charger (MCP73831 preferred — small SOIC, reliable)
- [ ] Power-path P-FET, low-Iq LDO (MCP1700)
- [ ] IR driver (AO3400 + 27 Ω + IR LEDs)
- [ ] I²C bus to SHT30 + OLED (connectors or direct)
- [ ] USB-C with proper CC pull-down (5.1 kΩ × 2)
- [ ] Boot/reset buttons, programming-friendly layout
- [ ] GND plane, decoupling on each IC
- [ ] Manufactured (JLCPCB / PCBWay), assembled, brought up
- [ ] Power figures match phase D measurements

**Done when:** the custom PCB is functionally and energetically equivalent
to the phase D bench setup.

### F — Polish

- [ ] Enclosure (3D-printed)
- [ ] OTA updates (Matter OTA or ESP-IDF native)
- [ ] Optional: fuel gauge IC (MAX17048) for accurate battery %
- [ ] Optional: scale to support multiple ACs from a single device
