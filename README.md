# ⌚ TapBand — MQTT-Based Battery-Powered IoT Wearable Device

![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=flat-square&logo=espressif&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-660066?style=flat-square&logo=mqtt&logoColor=white)
![C++](https://img.shields.io/badge/C/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Altium Designer](https://img.shields.io/badge/Altium_Designer-A5915F?style=flat-square&logo=altiumdesigner&logoColor=white)
![WiFi](https://img.shields.io/badge/Wi--Fi-4285F4?style=flat-square&logo=wifi&logoColor=white)

Compact **5cm × 3cm** battery-powered wearable device with MQTT communication, OLED display, and deep-sleep power optimization. Built for a commercial security application — **currently in mass production**.

> See also: [TapBand Web Dashboard](https://github.com/zaeem7744/TapBand-Web-Dashboard)

---

## 🔧 Features

- **Ultra-Compact Design** — 5cm × 3cm form factor optimized for wearable use
- **MQTT Communication** — Real-time bidirectional messaging via MQTT protocol (HiveMQ)
- **OLED Display** — Menu-driven interface with navigation buttons
- **Deep-Sleep Optimization** — Battery-powered with aggressive power saving for extended life
- **Wi-Fi Provisioning** — Hotspot-based setup for easy deployment without hardcoding credentials
- **WS2812B LED Indicators** — Visual status feedback with addressable LEDs
- **Vibration Motor** — Haptic feedback for alerts and notifications
- **Real-Time Admin Panel** — Subscription management via web dashboard
- **Custom PCB** — Designed in Altium Designer, production-ready with Gerber files
- **Mass Production Ready** — Currently manufactured commercially

---

## 📂 Project Structure

```
├── TapBand_Main/              # Main production firmware
├── TapBand_Design/            # PCB design files (Altium Designer)
├── MQTT/                      # MQTT communication module
├── OLED_MQTT/                 # OLED + MQTT integration
├── OLED_INTERFAACE/           # OLED display driver
├── WIFI_PROVISIONIG_MQTT_OLED/ # Wi-Fi provisioning
├── PowerSave_AlertBand_Test/  # Deep-sleep power testing
├── WS2812_Test/               # LED strip testing
├── Board_Vibrator_Test/       # Vibration motor testing
├── Sync_Mqtt_Hive/            # MQTT broker sync module
└── Web App/                   # Companion web application
```

---

## 🛠️ Tech Stack

- **MCU:** ESP32
- **Language:** C/C++ (PlatformIO)
- **Communication:** MQTT (HiveMQ), Wi-Fi
- **Display:** SSD1306 OLED
- **LEDs:** WS2812B addressable LEDs
- **PCB Design:** Altium Designer
- **Power:** Battery with deep-sleep optimization
- **Cloud:** MQTT Broker (HiveMQ Cloud)

---

## 👤 Author

**Muhammad Zaeem Sarfraz**
- 🔗 [LinkedIn](https://www.linkedin.com/in/zaeemsarfraz7744/)
- 📧 Zaeem.7744@gmail.com
- 🌍 Vaasa, Finland
