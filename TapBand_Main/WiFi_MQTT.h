#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

#include "Config.h"

// Forward declarations
extern WiFiClient espClient;
extern PubSubClient client;
extern WebServer server;
extern Preferences preferences;
extern unsigned long lastBatteryRead;
extern int batteryLevel;

// Function declarations
void initWiFi();
void initMQTT();
void startWiFiHotspot();
void stopWiFiHotspot();
void resetWiFiCredentials();
void checkWiFiConnection();
void reconnectMQTT();
void handleWebServer();
void updateBatteryLevel();

#endif