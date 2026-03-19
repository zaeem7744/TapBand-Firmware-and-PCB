# âŒš TapBand â€” MQTT-Based Battery-Powered IoT Wearable Device

![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=flat-square&logo=espressif&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-660066?style=flat-square&logo=mqtt&logoColor=white)
![C++](https://img.shields.io/badge/C/C++-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![Altium Designer](https://img.shields.io/badge/Altium_Designer-A5915F?style=flat-square&logo=altiumdesigner&logoColor=white)
![WiFi](https://img.shields.io/badge/Wi--Fi-4285F4?style=flat-square&logo=wifi&logoColor=white)

Compact **5cm Ã— 3cm** battery-powered wearable device with MQTT communication, OLED display, and deep-sleep power optimization. Built for a commercial security application â€” **currently in mass production**.

> See also: [TapBand Web Dashboard](https://github.com/zaeem7744/TapBand-Web-Dashboard)

---

## ðŸ”§ Features

- **Ultra-Compact Design** â€” 5cm Ã— 3cm form factor optimized for wearable use
- **MQTT Communication** â€” Real-time bidirectional messaging via MQTT protocol (HiveMQ)
- **OLED Display** â€” Menu-driven interface with navigation buttons
- **Deep-Sleep Optimization** â€” Battery-powered with aggressive power saving for extended life
- **Wi-Fi Provisioning** â€” Hotspot-based setup for easy deployment without hardcoding credentials
- **WS2812B LED Indicators** â€” Visual status feedback with addressable LEDs
- **Vibration Motor** â€” Haptic feedback for alerts and notifications
- **Real-Time Admin Panel** â€” Subscription management via web dashboard
- **Custom PCB** â€” Designed in Altium Designer, production-ready with Gerber files
- **Mass Production Ready** â€” Currently manufactured commercially

---

## ðŸ“‚ Project Structure

```
â”œâ”€â”€ TapBand_Main/              # Main production firmware
â”œâ”€â”€ TapBand_Design/            # PCB design files (Altium Designer)
â”œâ”€â”€ MQTT/                      # MQTT communication module
â”œâ”€â”€ OLED_MQTT/                 # OLED + MQTT integration
â”œâ”€â”€ OLED_INTERFAACE/           # OLED display driver
â”œâ”€â”€ WIFI_PROVISIONIG_MQTT_OLED/ # Wi-Fi provisioning
â”œâ”€â”€ PowerSave_AlertBand_Test/  # Deep-sleep power testing
â”œâ”€â”€ WS2812_Test/               # LED strip testing
â”œâ”€â”€ Board_Vibrator_Test/       # Vibration motor testing
â”œâ”€â”€ Sync_Mqtt_Hive/            # MQTT broker sync module
â””â”€â”€ Web App/                   # Companion web application
```

---

## ðŸ› ï¸ Tech Stack

- **MCU:** ESP32
- **Language:** C/C++ (PlatformIO)
- **Communication:** MQTT (HiveMQ), Wi-Fi
- **Display:** SSD1306 OLED
- **LEDs:** WS2812B addressable LEDs
- **PCB Design:** Altium Designer
- **Power:** Battery with deep-sleep optimization
- **Cloud:** MQTT Broker (HiveMQ Cloud)

---

## ðŸ‘¤ Author

**Muhammad Zaeem Sarfraz**
- ðŸ”— [LinkedIn](https://www.linkedin.com/in/zaeemsarfraz7744/)
- ðŸ“§ Zaeem.7744@gmail.com
- ðŸŒ Vaasa, Finland
