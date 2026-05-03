#ifndef AIRCON_HW_CONFIG_H
#define AIRCON_HW_CONFIG_H

// For Phase B (C6 port): update only this file for the new board's pin map.

// =================== PINS ===================
#define IR_RECV_PIN     0
#define IR_SEND_PIN     6
#define BTN1_PIN        5   // SW1 — used as main config/reset button
#define BTN2_PIN        4   // SW2
#define BTN3_PIN        3   // SW3
#define BUTTON_PIN      BTN1_PIN
#define I2C_SDA         8
#define I2C_SCL         9
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
