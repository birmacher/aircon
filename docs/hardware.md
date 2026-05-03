# Hardware

ESP32-C3 SuperMini-based IR remote with 3 buttons, an OLED display, and an
SHT30 temperature/humidity sensor. The board listens for incoming IR codes during installation
on GPIO0 and detect and stores the AC protocal.
After setup it uses two IR LEDs driven by an NPN transistor on GPIO6 for communication with the ACs.

## Bill of materials

| Ref     | Qty | Part                       | Notes                              |
|---------|----:|----------------------------|------------------------------------|
| U1      |   1 | ESP32-C3 SuperMini         | MCU board, 3V3 logic               |
| U2      |   1 | SHT30-M                    | I²C temp/humidity sensor           |
| DS1     |   1 | 0.96" OLED 128×64 (SSD1306)| I²C, model D096-12864-I2C-GV-WH    |
| Q1      |   1 | 2N2222A                    | NPN, low-side IR-LED driver        |
| D1, D2  |   2 | IRE940-5                   | IR LED, 940 nm                     |
| D3      |   1 | LED 5 mm red               | Status LED                         |
| U3      |   1 | JS-1838B (VS1838B)         | IR receiver, 38 kHz                |
| SW1–SW3 |   3 | TACTR-12×12×7.5            | Tactile button                     |
| R1, R2  |   2 | 1 kΩ ±5%                   | In parallel → ≈500 Ω base resistor |
| R3, R4  |   2 | 27 Ω ±5%                   | IR LED current limit (one per LED) |
| R5      |   1 | 220 Ω ±10%                 | Status LED current limit           |

## Pin assignments

| Pin    | Net      | Connection                      |
|--------|----------|---------------------------------|
| 3V3    | +3V3     | U2 VCC, U3 VCC, DS1 VCC, IR LED anodes (via R3/R4) |
| GND    | GND      | common ground                   |
| GPIO0  | IR_RX    | U3 OUT                          |
| GPIO3  | BTN3     | SW3 → GND (`INPUT_PULLUP`)      |
| GPIO4  | BTN2     | SW2 → GND (`INPUT_PULLUP`)      |
| GPIO5  | BTN1     | SW1 → GND (`INPUT_PULLUP`)      |
| GPIO6  | IR_TX    | Q1 base via R1∥R2; D3 via R5    |
| GPIO8  | I²C SDA  | U2 SDA, DS1 SDA                 |
| GPIO9  | I²C SCL  | U2 SCL, DS1 SCL                 |

## Schematic

```mermaid
flowchart LR
    classDef rail fill:#f5e6e0,stroke:#c0432e,color:#5a2114
    classDef gnd  fill:#e8e8e8,stroke:#444,color:#222
    classDef mcu  fill:#dde7f3,stroke:#2a6cb8,color:#0e2a4a
    classDef comp fill:#fff,stroke:#666,color:#222

    V3[+3V3]:::rail
    GND[GND]:::gnd

    subgraph U1[ESP32-C3 SuperMini]
        direction TB
        G0[GPIO0]:::mcu
        G3[GPIO3]:::mcu
        G4[GPIO4]:::mcu
        G5[GPIO5]:::mcu
        G6[GPIO6]:::mcu
        G8[GPIO8]:::mcu
        G9[GPIO9]:::mcu
    end

    SW1[SW1]:::comp --- G5
    SW1 --- GND
    SW2[SW2]:::comp --- G4
    SW2 --- GND
    SW3[SW3]:::comp --- G3
    SW3 --- GND

    U3[U3 JS-1838B]:::comp
    U3 -- VCC --- V3
    U3 -- GND --- GND
    U3 -- OUT --- G0

    Rb["R1∥R2 (≈500Ω)"]:::comp
    Q1[Q1 2N2222A]:::comp
    R3[R3 27Ω]:::comp
    R4[R4 27Ω]:::comp
    D1[D1 IR LED]:::comp
    D2[D2 IR LED]:::comp
    R5[R5 220Ω]:::comp
    D3[D3 red LED]:::comp

    G6 --- Rb --- Q1
    Q1 -- E --- GND
    Q1 -- C --- D1 --- R3 --- V3
    Q1 -- C --- D2 --- R4 --- V3
    G6 --- R5 --- D3 --- GND

    U2[U2 SHT30-M]:::comp
    DS1[DS1 OLED]:::comp
    G8 -- SDA --- U2
    G8 -- SDA --- DS1
    G9 -- SCL --- U2
    G9 -- SCL --- DS1
    U2  -- VCC --- V3
    U2  -- GND --- GND
    DS1 -- VCC --- V3
    DS1 -- GND --- GND
```

## Notes

- **I²C pull-ups.** Both the SHT30-M and the OLED breakout boards include
  built-in pull-ups on SDA/SCL. With both connected, the effective pull-up
  is the parallel combination — fine for 100 kHz and 400 kHz operation.
  No external resistors needed.
- **GPIO9 is a boot strap pin** on the ESP32-C3. The I²C pull-up on SCL
  keeps it high at boot, which is what the chip wants. Don't add anything
  that pulls GPIO9 low at startup.
- **Status LED tracks IR transmission.** R5 + D3 hang off the same GPIO6
  that drives the transistor base, so the red LED flickers visibly on
  every IR burst.
- **IR LED current.** With V_LED ≈ 1.4 V, V_CE(sat) ≈ 0.2 V, and 27 Ω in
  series, peak current per LED is ≈ 63 mA. IR remote protocols pulse at
  ~33% duty over short bursts, keeping the average well within the LED's
  continuous rating.
