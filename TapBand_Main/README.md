# TapBand_Main

Feature‑complete ESP32 firmware for the band: OLED UI, menu system, Wi‑Fi provisioning via captive portal, MQTT alerts, and haptics through DRV2605.

## Hardware/pins (defaults)
- OLED I2C: SDA 21, SCL 22, addr 0x3C
- Buttons: UP 27, DOWN 34, SELECT 35 (INPUT_PULLUP)
- Vibration: EN 33 (DRV2605 on I2C)
- Battery sense: ADC 19 via divider R1=220k, R2=100k
- Built‑in LED used for alert blink feedback

## Libraries
- Adafruit GFX, Adafruit SSD1306, Adafruit DRV2605
- PubSubClient (MQTT), WiFi, WebServer, Preferences

## Build (Arduino IDE)
1) Board: “ESP32 Dev Module” (or your ESP32 variant)
2) Install libraries above from Library Manager
3) Open `TapBand_Main.ino` and flash; Serial Monitor: 115200 baud
4) If first boot has no Wi‑Fi creds, use Menu → Settings → Wi‑Fi Setup

## Menu map and flow
- Home → shows battery, Wi‑Fi bars, and last alert for ~8s
- Main Menu: Alert Mode, Assist Mode, Normal Mode, Settings, Back
- Settings: Wi‑Fi Setup (starts AP + portal), Reset Wi‑Fi, Back
- In modes, pressing SELECT publishes the corresponding MQTT message and returns Home

## Wi‑Fi provisioning (captive portal)
- When Wi‑Fi Setup is selected, device starts AP TapBand_<DEVICE_ID> and a minimal web server
- Open the root page, submit SSID/password; device stores them in NVS (Preferences) and restarts
- Hotspot auto‑times‑out after 180s (WIFI_HOTSPOT_TIMEOUT)

## MQTT
- Defaults: server broker.hivemq.com, port 1883, topic band/alert (see Config.h)
- Subscribes to band/alert; publishes NORMAL/ALERT/ASSIST depending on mode
- Tip: in production, change to your broker, use TLS, and unique client IDs; consider Last‑Will and retained metadata

## Battery model
- Voltage: Vadc = raw/4095*3.3V; Vbat = Vadc*(R1+R2)/R2 + VOLTAGE_OFFSET
- Percentage mapping clamps between BATTERY_EMPTY (3.7V) and BATTERY_FULL (4.2V). Adjust these and VOLTAGE_OFFSET per your cells/board.

## Haptics
- DRV2605 library 1, INTTRIG mode. Effects are mapped in `Vibration_System.h`:
  - Wi‑Fi connect/disconnect, menu navigate/back/screen change, confirmations, alerts, low battery, hotspot start
- Start with short effects to verify actuator polarity and mounting

## Extending
- New menu items: add labels to Menu_System.cpp, handle in switch, and implement action.
- New MQTT topics: update Config.h and WiFi_MQTT.cpp callback/publish sites.
- UI: tweak Display_Functions.cpp; keep draw calls minimal per frame to avoid flicker.

## Troubleshooting
- “SSD1306 allocation failed”: power/I2C wiring or wrong address
- No MQTT: check Wi‑Fi status, broker reachability, and client.loop() in the main loop
- ADC jumps: add a small RC filter, sample average (already 5x), and re‑tune VOLTAGE_OFFSET
