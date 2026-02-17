#include "Config.h"
#include "Display_Functions.h"
#include "WiFi_MQTT.h"
#include "Menu_System.h"
#include "Buttons.h"
#include "Alert_System.h"
#include "Vibration_System.h"  // ADD THIS

/* System Variables */
int batteryLevel = 85;
String deviceID = DEVICE_ID;
String macAddress = "";
String incomingAlert = "";
unsigned long lastAlertTime = 0;
unsigned long lastBatteryRead = 0;

/* WiFi Status */
int wifiStrength = 0;
bool wifiConnected = false;
bool inHotspotMode = false;
unsigned long wifiHotspotStartTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VIBRATOR_EN_PIN, OUTPUT);
  pinMode(BATTERY_PIN, INPUT);
  initButtons();
  initDisplay();
  initVibration();  // ADD THIS
  initWiFi();
  initMQTT();
  drawHomeScreen();
}

void loop() {
  digitalWrite(VIBRATOR_EN_PIN, LOW);
  updateBatteryLevel();
  // Handle WiFi hotspot timeout
   if (inHotspotMode && millis() - wifiHotspotStartTime > WIFI_HOTSPOT_TIMEOUT) {
    stopWiFiHotspot();
    currentScreen = HOME; // Ensure we return to home screen
    drawHomeScreen();
  }

  // Handle WiFi and MQTT
  checkWiFiConnection();
  if (wifiConnected && !inHotspotMode) {
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop();
  }

  // Handle encoder and menus
  checkButtons();
  checkMenuTimeout();

  // Handle web server in hotspot mode
  handleWebServer();

  // Update display if in hotspot mode
  static unsigned long lastDisplayUpdate = 0;
  if (inHotspotMode && millis() - lastDisplayUpdate > 1000) {
    drawWiFiHotspotScreen();
    lastDisplayUpdate = millis();
  }
} 