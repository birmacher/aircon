#ifndef AIRCON_HW_CONFIG_H
#define AIRCON_HW_CONFIG_H

// Pin map is gated on the IDF target macro (defined by the SoC selection,
// stable under both Arduino and ESP-IDF builds). Each new SoC needs its own block.

// =================== PINS ===================
#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3 SuperMini (Phase A baseline). See docs/hardware.md.
#define IR_RECV_PIN     0
#define IR_SEND_PIN     6
#define BTN1_PIN        5
#define BTN2_PIN        4
#define BTN3_PIN        3
#define I2C_SDA         8   // GPIO9 strap is held high by I²C pull-ups → fine on C3
#define I2C_SCL         9
#elif CONFIG_IDF_TARGET_ESP32C6
// ESP32-C6-DevKitC-1 (Phase B). I²C off GPIO8/9 (C6 boot-mode straps).
// GPIO12/GPIO13 are reserved: hard-wired to USB-Serial-JTAG D−/D+, not GPIO-matrix routable.
#define IR_RECV_PIN     4
#define IR_SEND_PIN     5
#define BTN1_PIN        10
#define BTN2_PIN        11
#define BTN3_PIN        22
#define I2C_SDA         6
#define I2C_SCL         7
#else
#error "Unsupported IDF target — add a pin map block for this SoC in hw_config.h"
#endif

#define BUTTON_PIN      BTN1_PIN
#define IR_BUFFER_SIZE  2048
#define IR_TIMEOUT      50

// =================== I2C ADDRESSES ===================
#define OLED_I2C_ADDR   0x3C
#define SHT31_I2C_ADDR  0x44

// =================== CONFIG ===================
#define WIFI_CONNECT_TIMEOUT_MS 20000
#define RECONFIG_HOLD_MS        3000
#define FACTORY_RESET_HOLD_MS   10000
#define BUTTON_DEBOUNCE_MS      50

#endif // AIRCON_HW_CONFIG_H
