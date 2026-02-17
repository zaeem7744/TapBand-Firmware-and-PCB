# TapBand_Design

Altium Designer project for the band’s electronics: MCU, power, haptics, display, and connectors.

## Open
- `TapBand/TapBand.PrjPcb` in Altium Designer

## What’s inside
- Schematics (`*.SchDoc`):
  - Main: ESP32 and core nets
  - I2C_OLED_DRV2605: OLED and haptics driver
  - VOLTAGE_REGULATOR: power tree and battery input
  - USB_UART: programming and debug
  - Menu_Navigators: buttons/encoder
- PCB: `TapBand.PcbDoc`
- Outputs: `Project Outputs for TapBand/` → Gerbers, drills, BOM, pick‑and‑place; plus zipped packs
- Logs: ECO, DRC, compilation logs under `Project Logs for TapBand/`
- CAM: example CAMtastic session in `TapBand/CAM/`

## Fabrication checklist
- Verify board stackup with your fab; ensure soldermask openings on the haptic footprint are correct
- Check mounting and keepouts for the actuator and OLED
- Use the provided pick/place and BOM CSVs; confirm part numbers for the DRV2605 variant and OLED connector pitch

## Notes
- If you change net names or variants, regenerate outputs from the OutJob to keep files consistent
