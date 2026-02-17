# TapBand

End‑to‑end project for a wearable alert band: ESP32 firmware, a small Flask MQTT dashboard, and full PCB design files. This README is the map and “why” behind the code.

## What’s here (bird’s‑eye)
- TapBand_Main — Main ESP32 firmware (menus, OLED UI, Wi‑Fi provisioning, MQTT alerts, vibration haptics)
- WIFI_PROVISIONIG_MQTT_OLED — Minimal ESP32 demo of captive portal provisioning + MQTT + OLED
- OLED_MQTT — Menu + OLED + MQTT prototype using a rotary encoder
- MQTT — Very small MQTT connectivity sample (for sanity checks)
- Board_Vibrator_Test — DRV2605 haptics validation
- WS2812_Test — NeoPixel LED test
- OLED_INTERFAACE — OLED rendering tests
- PowerSave_AlertBand_Test — Sleep/low‑power experiments
- mqtt_dashboard — Flask app with SQLite for managing “shops” and “bands”, plus MQTT control/alerts
- TapBand_Design — Altium project, logs and fabrication outputs (Gerbers/CSV/BOM)
- Misc — Screenshots, PDFs, BOM spreadsheets, zipped server/outputs

## Quick start
1) Firmware (recommended: TapBand_Main)
- Board: ESP32 Dev Module in Arduino IDE
- Install libraries: Adafruit GFX, Adafruit SSD1306, Adafruit DRV2605, PubSubClient, WebServer, Preferences
- Pins (default): SDA 21, SCL 22, Buttons UP/DOWN/SELECT 27/34/35, Vibrator EN 33, Battery ADC 19, OLED addr 0x3C
- Build and flash `TapBand_Main/TapBand_Main.ino`. See `TapBand_Main/README.md` for Wi‑Fi provisioning and menu map

2) Dashboard
- Python 3.x, install: pip install flask paho-mqtt
- Configure MQTT in `mqtt_dashboard/app.py` (broker, TLS, username/password, CA path)
- Run: python mqtt_dashboard/app.py, then open http://localhost:5000

3) Hardware
- Open `TapBand_Design/TapBand/TapBand.PrjPcb` in Altium. Gerbers live under “Project Outputs for TapBand”.

## How the system fits together
- The band publishes/consumes MQTT messages (default public broker: broker.hivemq.com, topic: band/alert in firmware demos). The dashboard uses the namespace tapband/# and tapband/control/{shop}/{band} for management.
- A captive Wi‑Fi portal lets you provision SSID/password on‑device without rebuilding firmware (Preferences NVS storage).
- UI is rendered on a 128x64 OLED; haptics via DRV2605 preset effects for feedback.

## Practical notes (from experience)
- OLED I2C address is 0x3C on most modules; ensure SDA=21/SCL=22 or adapt in Config.h.
- Battery sensing: divider R1=220k, R2=100k to ADC pin 19. Formula: Vbat = (ADC/4095*3.3V) * (R1+R2)/R2 + offset. Tune VOLTAGE_OFFSET for your board.
- DRV2605: set library 1 and INTTRIG; effect IDs differ by library. Start with small effects to confirm orientation.
- Wi‑Fi RSSI → bars is heuristic. Tweak WIFI_RSSI_MIN/MAX for your site to get stable UI.
- MQTT on public brokers is fine for testing; for production, use your own broker with TLS and auth.

## Repository guide
- Firmware READMEs: build steps, pins, libraries, and “how to use it” per sketch
- Dashboard README: setup, routes, topic layout, and how device status is derived
- Design README: what each schematic page covers, outputs, and how to regenerate CAM

## Troubleshooting
- OLED blank: check 3.3V and I2C wiring; try scanning for 0x3C; verify pull‑ups.
- No haptics: check DRV2605 VIN/3V3, I2C, EN pin, and that VIBRATOR_EN_PIN is set HIGH when driving.
- MQTT not connecting: broker reachable? correct port/TLS? unique client IDs? firewall?
- Provisioning page not loading: ensure device is in AP mode (WIFI_SETUP_MODE) and connect to TapBand_<ID> AP.

## Next steps
- Add device heartbeat topics and retained device metadata
- Persist settings in a JSON block (not just SSID/pass)
- OTA updates over Wi‑Fi
