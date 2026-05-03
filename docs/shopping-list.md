# Shopping list

Components to source for the next phases of the project. See
[roadmap.md](./roadmap.md) for the phase definitions.

Primary supplier: **hestore.hu**. Mouser / Farnell as fallback for anything
not in stock.

## Buy now (phases B + D)

### Microcontroller — phase B

- [ ] **ESP32-C6-DevKitC-1** × 1–2 — official Espressif DevKit with
      ESP32-C6-WROOM-1 module. Buy two if cheap, so a bricked board
      isn't a blocker.

Alternative if not stocked: any WROOM-1-based C6 board with most GPIOs
broken out (e.g. Waveshare ESP32-C6 Mini). Avoid heavily-stripped boards
where pins aren't exposed.

### Charging and power — phase D

- [ ] **TP4056 charger module with protection** × 2–3 — TP4056 + DW01A +
      8205A combo. USB-C input preferred over micro-USB.
- [ ] **LiPo battery 1000–2000 mAh** × 1–2 — 1S, 3.7 V nominal, JST-PH
      (2.0 mm) connector.
- [ ] **MCP1700-3302E** × 3–5 — low-Iq 3.3 V LDO. TO-92 for breadboard,
      SOT-23 for the future PCB. Buy a few of each.
- [ ] **HT7333** × 3–5 — backup if MCP1700 not stocked. Similar Iq.
- [ ] **JST-PH 2-pin connectors** (male + female) × 2–3 sets — for
      hooking the LiPo to the DevKit.

### IR driver upgrade — phase D

- [ ] **AO3400** (SOT-23) × 5–10 — logic-level N-MOSFET to replace the
      2N2222A. SMD, easy to lose, buy plenty.
- [ ] **2N7002** × 5–10 — backup if AO3400 not stocked. Lower current
      rating (200 mA) but enough for our IR LEDs.

### Power-path — phase D

- [ ] **Schottky diode 1N5817** (THT) or **SS14** (SMD) × 5–10 — USB ↔
      battery power path. Low forward drop matters.
- [ ] **P-channel MOSFET AO3401 / SI2301** × 3–5 — optional, for an
      "ideal diode" power-path with a smaller voltage drop than a
      Schottky.

### Decoupling — phase D

- [ ] **100 nF ceramic** (X7R) × 10–20 — IR receiver decoupling, per-IC
      bypass caps. THT and/or 0805 SMD.
- [ ] **10 µF ceramic** × 5–10 — LDO input/output.
- [ ] **100–220 µF electrolytic** × 3–5 — bulk on the 3V3 rail.

### Resistors — phase D

- [ ] **470 kΩ** × 5–10 — battery voltage divider (~4 µA leakage).
- [ ] **5.1 kΩ** × 5 — USB-C CC pull-down (will be needed for phase E).
- [ ] **0805 SMD resistor variety pack** × 1 — if not already on hand.

### Measurement — recommended

- [ ] **µCurrent Gold** (or equivalent) — accurate µA measurement for
      deep-sleep validation. A regular DMM is unreliable below ~100 µA.
      Optional but strongly recommended before signing off phase D.

## Buy later (phase E — own PCB)

Defer until the PCB is actively being designed; the BOM may shift.

- [ ] **ESP32-C6-WROOM-1-N8** (or **-N4**) module — standalone, for the
      custom PCB.
- [ ] **MCP73831T-2ATI/OT** (SOT-23-5) — charger IC, smaller than the
      TP4056 module form-factor.
- [ ] **MAX17048** — fuel-gauge IC, optional. Adds accurate battery %.
- [ ] **USB-C connector** (SMD, 16-pin or 24-pin) — for the PCB's USB.
- [ ] ESD protection diodes, ferrite beads, additional passives — finalize
      with the schematic.

## Purchase notes

- **JST connector compatibility.** Verify the LiPo's connector pitch
  matches the TP4056 module's connector — both are commonly 2.0 mm PH
  but some are 1.25 mm GH. Mismatch means an adapter or a re-crimp.
- **Protection placement.** Either the TP4056 module *or* the LiPo cell
  must include over-discharge protection (DW01A + 8205A on the module,
  or a built-in PCM on the cell). Both is fine. Neither is dangerous.
- **Minimum kit to start.** If buying everything at once isn't practical,
  the smallest set to begin phases B + D is: DevKitC-1, TP4056 module,
  LiPo, MCP1700, AO3400, plus a few caps. The rest can wait.
- **Phase A overlap.** AO3400, 100 nF and bulk caps can already be added
  to the existing C3 prototype as small phase-A improvements — no need
  to wait for the C6 board.
