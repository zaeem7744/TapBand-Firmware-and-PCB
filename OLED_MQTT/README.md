# OLED_MQTT

Prototype: rotary encoder menu + OLED UI + MQTT publish/subscribe.

## Features
- Home screen with battery and Wi‑Fi indicators; shows last alert for ~8s
- Menu: Assist Mode, Alert Mode, Settings, Back (encoder to navigate, press to select)
- MQTT subscribe to band/alert; publishes ASSIST/ALERT on action

## Build
- Board: ESP32, OLED on I2C (SDA 21, SCL 22, 0x3C)
- Encoder pins: CLK 34, DT 35, SW 32 (INPUT_PULLUP)
- Libraries: Adafruit GFX, Adafruit SSD1306, WiFi, PubSubClient

## Notes
- Replace hardcoded SSID/password with your own before flashing
- For smoother UI, debounce the encoder and avoid heavy blocking delays (a few remain in this prototype)
