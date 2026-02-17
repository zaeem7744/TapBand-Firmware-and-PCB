# Board_Vibrator_Test

Validate the DRV2605 haptic driver and actuator wiring.

## Build
- Board: ESP32
- Libraries: Adafruit DRV2605, Wire
- I2C: SDA 21, SCL 22; ensure 3.3V and GND connected; typical DRV2605 addr 0x5A

## What to look for
- Short clicks/buzzes for different effects; if it feels weak or inverted, re‑mount the actuator or try other effects
- If nothing happens: check I2C wiring, selectLibrary(1), and INTTRIG mode
