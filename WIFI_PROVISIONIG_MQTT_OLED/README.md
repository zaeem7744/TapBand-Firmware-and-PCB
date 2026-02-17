# WIFI_PROVISIONIG_MQTT_OLED

Minimal demo showing captive portal provisioning, MQTT connectivity, and a compact OLED UI.

## What it does
- Shows Home with battery and Wi‑Fi bars
- Main Menu → Settings → Wi‑Fi Setup starts AP TapBand_<ID> and hosts a simple form at “/”
- POST to "/connect" saves SSID/password to Preferences and restarts
- Hotspot timeout: 180s; display shows countdown

## Build
- Board: ESP32 Dev Module
- Libraries: Adafruit GFX, Adafruit SSD1306, WiFi, PubSubClient, WebServer, Preferences
- Pins: SDA 21, SCL 22, Encoder CLK/DT/SW 34/35/32 (if used), OLED 0x3C

## MQTT
- Default: broker.hivemq.com, topic band/alert; change for your broker/security

## Tips
- If the portal doesn’t load, ensure you are connected to the device AP, not a LAN with captive DNS
- After provisioning, the device returns to HOME automatically; use Reset Wi‑Fi to clear creds
