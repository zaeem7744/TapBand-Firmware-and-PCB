#include "WiFi_MQTT.h"
#include "Display_Functions.h"
#include "Alert_System.h"
#include "Vibration_System.h"  // ADD THIS

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);
Preferences preferences;

// External variables
extern String deviceID;
extern String macAddress;
extern bool inHotspotMode;
extern unsigned long wifiHotspotStartTime;
extern int wifiStrength;
extern bool wifiConnected;
extern ScreenState currentScreen;
extern String incomingAlert;
extern unsigned long lastAlertTime;
extern int menuIndex;

void initWiFi() {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      macAddress = WiFi.macAddress();
      vibeWiFiConnected();  // ADD THIS
    }
  }
}

void initMQTT() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback([](char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    handleIncomingAlert(message);
  });
}

void startWiFiHotspot() {
  
  if (currentScreen != WIFI_SETUP_MODE) {
    return;
  }

  // Initialize WiFi in AP mode
  WiFi.mode(WIFI_AP);
  String apSSID = "TapBand_" + String(DEVICE_ID);
  WiFi.softAP(apSSID.c_str());
  macAddress = WiFi.softAPmacAddress();
   vibeHotspotStart();  // ADD THIS
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>TapBand Wi-Fi Setup</title>"
                  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                  "<style>"
                  "body {font-family: Arial, sans-serif; background-color: #f8f9fa; color: #333; max-width: 500px; margin: 0 auto; padding: 20px;}"
                  ".container {background: white; border-radius: 8px; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.1);}"
                  "h2 {color: #4a6fa5; text-align: center;}"
                  ".device-id {background: #4a6fa5; color: white; padding: 10px; border-radius: 5px; text-align: center; margin-bottom: 20px;}"
                  "input[type=text], input[type=password] {width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box;}"
                  "button {background-color: #4a6fa5; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; width: 100%; font-size: 16px;}"
                  "button:hover {background-color: #3a5a8f;}"
                  ".loader {border: 4px solid #f3f3f3; border-top: 4px solid #4a6fa5; border-radius: 50%; width: 30px; height: 30px; animation: spin 1s linear infinite; margin: 20px auto;}"
                  "@keyframes spin {0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); }}"
                  "</style></head><body>"
                  "<div class=\"container\">"
                  "<div class=\"device-id\">Band ID: "
                  + deviceID + "<br>MAC: " + macAddress + "</div>"
                                                          "<h2>Wi-Fi Setup</h2>"
                                                          "<form id=\"wifiForm\">"
                                                          "<label for=\"ssid\">Network Name (SSID)</label>"
                                                          "<input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"Enter Wi-Fi name\" required>"
                                                          "<label for=\"password\">Password</label>"
                                                          "<input type=\"password\" id=\"password\" name=\"password\" placeholder=\"Enter Wi-Fi password\">"
                                                          "<button type=\"submit\">Connect</button>"
                                                          "</form>"
                                                          "<div id=\"loading\" style=\"display:none;\"><div class=\"loader\"></div><p>Connecting...</p></div>"
                                                          "<script>"
                                                          "document.getElementById('wifiForm').addEventListener('submit', function(e) {"
                                                          "e.preventDefault();"
                                                          "document.getElementById('wifiForm').style.display = 'none';"
                                                          "document.getElementById('loading').style.display = 'block';"
                                                          "fetch('/connect', {"
                                                          "method: 'POST',"
                                                          "headers: {'Content-Type': 'application/x-www-form-urlencoded',},"
                                                          "body: 'ssid=' + encodeURIComponent(document.getElementById('ssid').value) + "
                                                          "'&password=' + encodeURIComponent(document.getElementById('password').value)"
                                                          "}).then(response => response.text()).then(data => {"
                                                          "document.getElementById('loading').innerHTML = '<p>' + data + '</p>';"
                                                          "if(data.includes('Success')) { setTimeout(() => { window.location.href = '/'; }, 2000); }"
                                                          "});"
                                                          "});"
                                                          "</script></div></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/connect", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.end();

    server.send(200, "text/plain", "Success! Device will restart and attempt to connect.");
    delay(1000);
    ESP.restart();
  });

  server.begin();
  wifiHotspotStartTime = millis();
  inHotspotMode = true;
  currentScreen = WIFI_SETUP_MODE;
}


void stopWiFiHotspot() {
  WiFi.softAPdisconnect(true);
  server.close();
  inHotspotMode = false;
  currentScreen = HOME;
  drawHomeScreen();
  wifiConnected = false;  // Add this line
}
void resetWiFiCredentials() {
  // Check if WiFi is already disconnected
  if (!wifiConnected) {
    drawWiFiStatusScreen("WiFi Already Reset");
    delay(2000);
    return;
  }

  preferences.begin("wifi", false);
  preferences.remove("ssid");
  preferences.remove("password");
  preferences.end();

  WiFi.disconnect();
  wifiConnected = false;
  drawWiFiStatusScreen("Wi-Fi Reset Complete");
  vibeWiFiDisconnected();  // ADD THIS
  delay(2000);
  currentScreen = HOME;  // Ensure we return to home
  drawHomeScreen();
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect(deviceID.c_str())) {
      client.subscribe(MQTT_TOPIC);
    } else {
      delay(2000);
    }
  }
}

void handleWebServer() {
  if (inHotspotMode) {
    server.handleClient();
  }
}

void updateBatteryLevel() {
  if (millis() - lastBatteryRead > 10000) {  // Check every 10 seconds
    // Read and average ADC
    int rawADC = 0;
    for (int i = 0; i < 5; i++) {
      rawADC += analogRead(BATTERY_ADC_PIN);
      delay(1);
    }
    rawADC /= 5;

    // Calculate battery voltage
    float v_adc = (rawADC / ADC_RESOLUTION) * ADC_REF_VOLTAGE;
    float batteryVoltage = (v_adc * (R1 + R2) / R2) + VOLTAGE_OFFSET;
    batteryVoltage = constrain(batteryVoltage, BATTERY_EMPTY, BATTERY_FULL);

    // Calculate percentage
    batteryLevel = round((batteryVoltage - BATTERY_EMPTY) * 100.0 / (BATTERY_FULL - BATTERY_EMPTY));
    batteryLevel = constrain(batteryLevel, 0, 100);

    // Debug output
    Serial.print("BAT - Raw:");
    Serial.print(rawADC);
    Serial.print(" V:");
    Serial.print(batteryVoltage, 2);
    Serial.print(" (");
    Serial.print(batteryLevel);
    Serial.println("%)");

    lastBatteryRead = millis();

    if (currentScreen == HOME) {
      drawHomeScreen();
    }
  }
}
void checkWiFiConnection() {
  static unsigned long lastCheck = 0;
  static bool wasConnected = false;
  
  if (millis() - lastCheck < 10000) return;
  lastCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    wifiStrength = map(constrain(rssi, WIFI_RSSI_MIN, WIFI_RSSI_MAX),
                       WIFI_RSSI_MIN, WIFI_RSSI_MAX, 1, 4);

    Serial.print("WIFI - RSSI:");
    Serial.print(rssi);
    Serial.print(" Strength:");
    Serial.println(wifiStrength);

    if (!wifiConnected) {
      wifiConnected = true;
      if (!wasConnected) {
        vibeWiFiConnected();  // ADD THIS - vibration on reconnection
        wasConnected = true;
      }
    }
  } else {
    if (wifiConnected) {
      vibeWiFiDisconnected();  // ADD THIS - vibration on disconnection
      wasConnected = false;
    }
    wifiConnected = false;
    wifiStrength = 0;
    Serial.println("WIFI - Disconnected");
  }
}