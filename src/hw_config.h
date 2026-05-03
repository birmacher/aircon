#ifndef AIRCON_HW_CONFIG_H
#define AIRCON_HW_CONFIG_H

// NOTE: These pin numbers reflect what is currently in code.
// hardware.md specifies different pins (GPIO0/6/8/9/3-5).
// Resolve this discrepancy in Phase A before Phase B (C6 port).
// For Phase B: update only this file for the new board's pin map.

// =================== PINS ===================
#define IR_RECV_PIN     4
#define IR_SEND_PIN     5
#define BUTTON_PIN      9
#define I2C_SDA         8
#define I2C_SCL         10
#define IR_BUFFER_SIZE  2048
#define IR_TIMEOUT      50

// =================== I2C ADDRESSES ===================
#define OLED_I2C_ADDR   0x3C
#define SHT31_I2C_ADDR  0x44

// =================== CONFIG ===================
#define AP_PASSWORD             "configme123"
#define WIFI_CONNECT_TIMEOUT_MS 20000
#define RECONFIG_HOLD_MS        3000
#define FACTORY_RESET_HOLD_MS   10000
#define BUTTON_DEBOUNCE_MS      50

#endif // AIRCON_HW_CONFIG_H
