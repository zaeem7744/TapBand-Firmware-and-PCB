# MQTT

Tiny sketch to sanity‑check Wi‑Fi + MQTT connectivity on ESP32.

## Use
- Open `MQTT.ino`
- Set your Wi‑Fi SSID/password and broker host/port/topic
- Flash and open Serial Monitor (115200). You should see connect logs and a periodic publish every 5s

## Tips
- Use a unique client ID per device to avoid broker kicks
- Public brokers are for testing; for real use, switch to a private/TLS broker and don’t hardcode secrets in source
