#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <Preferences.h>

/* ========== HARDWARE CONFIGURATION ========== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SDA_PIN 21
#define SCL_PIN 22

#define ENCODER_CLK 34
#define ENCODER_DT 35
#define ENCODER_SW 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ========== SYSTEM VARIABLES ========== */
// Main menu items
const char* mainMenuItems[] = { "Assist Mode", "Alert Mode", "Settings", "Back" };
const int mainMenuLength = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);

// Settings menu items
const char* settingsMenuItems[] = { "Wi-Fi Setup", "Reset Wi-Fi", "Back" };
const int settingsMenuLength = sizeof(settingsMenuItems) / sizeof(settingsMenuItems[0]);

enum ScreenState {
  HOME,
  MAIN_MENU,
  ASSIST_MODE,
  ALERT_MODE,
  SETTINGS_MENU,
  WIFI_SETUP_MODE
};

ScreenState currentScreen = HOME;
int currentMenu = 0;  // 0 = main menu, 1 = settings menu
int menuIndex = 0;

// Encoder variables
int currentStateCLK;
int lastStateCLK;
unsigned long lastButtonPress = 0;
bool buttonPressed = false;
unsigned long lastMenuInteraction = 0;

// System status
int batteryLevel = 85;
int wifiStrength = 0;
bool wifiConnected = false;
unsigned long wifiHotspotStartTime = 0;
const unsigned long WIFI_HOTSPOT_TIMEOUT = 180000;  // 3 minutes (180000ms)
bool inHotspotMode = false;

String deviceID = "BAND-001X";
String macAddress = "";

/* ========== WiFi + MQTT CONFIGURATION ========== */
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "band/alert";

WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);
Preferences preferences;
String incomingAlert = "";
unsigned long lastAlertTime = 0;

/* ========== OLED DISPLAY FUNCTIONS ========== */
void drawHomeScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header with battery and WiFi indicators
  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_BLACK);
  display.drawRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);

  // Battery indicator
  display.drawRect(2, 4, 30, 8, SSD1306_WHITE);
  display.fillRect(32, 6, 2, 4, SSD1306_WHITE);
  int batteryFill = map(batteryLevel, 0, 100, 0, 28);
  display.fillRect(3, 5, batteryFill, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(38, 4);
  display.print(batteryLevel);
  display.print("%");

  // WiFi indicator
  int wifiX = SCREEN_WIDTH - 25;
  for (int i = 0; i < 4; i++) {
    int barHeight = (i + 1) * 2 + 2;  // Heights: 4,6,8,10 pixels
    int barY = 14 - barHeight;

    if (wifiStrength > i) {
      // Filled bar for active strength
      display.fillRect(wifiX + i * 5, barY, 3, barHeight, SSD1306_WHITE);
    } else {
      // Outline for inactive bars
      display.drawRect(wifiX + i * 5, barY, 3, barHeight, SSD1306_WHITE);
    }
  }


  // Main status text
  display.setTextSize(2);
  display.setCursor(30, 25);

  if (millis() - lastAlertTime < 8000 && incomingAlert.length() > 0) {
    display.print(incomingAlert);
  } else {
    if (wifiConnected) {
      display.print("NORMAL");
    } else {
      display.print("NO WIFI");
    }
  }

  // Footer
  display.setTextSize(1);
  display.setCursor(25, 45);
  display.print("TAP BAND V1.1");

  display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, SSD1306_WHITE);
  display.setCursor(0, SCREEN_HEIGHT - 8);
  display.print("Press Button For Menu");

  display.display();
}

void drawWiFiHotspotScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.println("Wi-Fi Hotspot Active");

  display.setCursor(5, 20);
  display.print("SSID: TapBand_");
  display.println(deviceID.substring(5));

  display.setCursor(5, 35);
  display.print("IP: 192.168.4.1");

  display.setTextSize(1);
  display.setCursor(20, 50);
  display.print("Timeout: ");
  display.print((WIFI_HOTSPOT_TIMEOUT - (millis() - wifiHotspotStartTime)) / 1000);
  display.print("s");

  // Show back option
  //display.setCursor(SCREEN_WIDTH - 30, SCREEN_HEIGHT-8);
  //display.print("BACK");

  display.display();
}

void drawWiFiStatusScreen(const char* status) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print(status);
  display.display();
  delay(2000);
  drawHomeScreen();
}

void drawMenuScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print(currentMenu == 0 ? "MAIN MENU" : "SETTINGS");
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  int itemHeight = 13;
  int startY = 15;
  int menuLength = currentMenu == 0 ? mainMenuLength : settingsMenuLength;
  const char** menuItems = currentMenu == 0 ? mainMenuItems : settingsMenuItems;

  for (int i = 0; i < menuLength; i++) {
    int y = startY + i * itemHeight;
    if (i == menuIndex) {
      display.fillRect(0, y, SCREEN_WIDTH, itemHeight, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(10, y + 3);
    display.print(menuItems[i]);
    if (i == menuIndex) {
      display.drawTriangle(110, y + 3, 120, y + 6, 110, y + 9, SSD1306_BLACK);
    }
  }

  int footerY = startY + menuLength * itemHeight;
  if (footerY < SCREEN_HEIGHT - 12) {
    display.drawLine(0, footerY, SCREEN_WIDTH, footerY, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, footerY + 2);
    display.print("Rotate to select");
  }

  display.display();
}

void drawModeScreen(const char* title, const char* instruction) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH / 2 - strlen(title) * 3, 2);
  display.print(title);
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  display.setCursor(SCREEN_WIDTH / 2 - strlen(instruction) * 3, 30);
  display.print(instruction);

  display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, SSD1306_WHITE);
  display.setCursor(30, SCREEN_HEIGHT - 7);
  display.print("Press To confirm");
  display.setCursor(SCREEN_WIDTH - 30, SCREEN_HEIGHT - 7);
  // display.print("BACK");

  display.display();
}

/* ========== WIFI MANAGEMENT FUNCTIONS ========== */
void startWiFiHotspot() {
  WiFi.mode(WIFI_AP);
  String apSSID = "TapBand_" + deviceID.substring(5);
  WiFi.softAP(apSSID.c_str());
  macAddress = WiFi.softAPmacAddress();

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
}

void resetWiFiCredentials() {
  if (WiFi.status() != WL_CONNECTED) {
    display.setCursor(8, 30);
    drawWiFiStatusScreen("No WiFi Connected");
    return;
  }

  preferences.begin("wifi", false);
  preferences.remove("ssid");
  preferences.remove("password");
  preferences.end();

  WiFi.disconnect();
  wifiConnected = false;
  drawWiFiStatusScreen("Wi-Fi Deleted!");
}

void checkWiFiConnection() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 10000) return;  // Check every 10 seconds
  lastCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    Serial.print("RSSI: ");
    Serial.println(rssi);

    // Adjusted thresholds based on your actual readings
    if (rssi > -30) {
      wifiStrength = 4;  // Excellent (-29 to 0)
      Serial.println("4 bars - Excellent");
    } else if (rssi > -50) {
      wifiStrength = 3;  // Good (-49 to -30)
      Serial.println("3 bars - Good");
    } else if (rssi > -65) {
      wifiStrength = 2;  // Fair (-64 to -50)
      Serial.println("2 bars - Fair");
    } else if (rssi > -80) {
      wifiStrength = 1;  // Weak (-79 to -65)
      Serial.println("1 bar - Weak");
    } else {
      wifiStrength = 0;  // Very weak/No signal (< -80)
      Serial.println("0 bars - No signal");
    }

    if (!wifiConnected) {
      wifiConnected = true;
      drawHomeScreen();  // Force redraw when first connecting
    }
  } else {
    wifiConnected = false;
    wifiStrength = 0;
  }
}

/* ========== MQTT FUNCTIONS ========== */
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(deviceID.c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == mqtt_topic && (message == "ASSIST" || message == "ALERT")) {
    incomingAlert = message;
    lastAlertTime = millis();

    // Visual feedback for alert
    for (int i = 0; i < 5; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);
      delay(300);
    }

    drawHomeScreen();
  }
}

/* ========== ENCODER & MENU FUNCTIONS ========== */
void handleButtonPress() {
  switch (currentScreen) {
    case HOME:
      currentScreen = MAIN_MENU;
      currentMenu = 0;
      menuIndex = 0;
      lastMenuInteraction = millis();
      drawMenuScreen();
      break;

    case MAIN_MENU:
      handleMainMenuSelection();
      break;

    case SETTINGS_MENU:
      handleSettingsMenuSelection();
      break;

    case ASSIST_MODE:
      sendAssistRequest();
      currentScreen = HOME;
      drawHomeScreen();
      break;

    case ALERT_MODE:
      sendAlertSignal();
      currentScreen = HOME;
      drawHomeScreen();
      break;

    case WIFI_SETUP_MODE:
      // Manual back from WiFi setup
      stopWiFiHotspot();
      break;
  }
}

void handleMainMenuSelection() {
  switch (menuIndex) {
    case 0:  // Assist Mode
      currentScreen = ASSIST_MODE;
      drawModeScreen("ASSIST MODE", "Press To Request Help");
      break;
    case 1:  // Alert Mode
      currentScreen = ALERT_MODE;
      drawModeScreen("ALERT MODE", "Press To Send Alert");
      break;
    case 2:  // Settings
      currentScreen = SETTINGS_MENU;
      currentMenu = 1;
      menuIndex = 0;
      drawMenuScreen();
      break;
    case 3:  // Back
      currentScreen = HOME;
      drawHomeScreen();
      break;
  }
}

void handleSettingsMenuSelection() {
  switch (menuIndex) {
    case 0:  // Wi-Fi Setup
      startWiFiHotspot();
      break;
    case 1:  // Reset Wi-Fi
      resetWiFiCredentials();
      currentScreen = HOME;
      break;
    case 2:  // Back
      currentScreen = MAIN_MENU;
      currentMenu = 0;
      menuIndex = 2;  // Return to Settings option in main menu
      drawMenuScreen();
      break;
  }
}

/* ========== SETUP & LOOP ========== */
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Encoder setup
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(ENCODER_CLK);

  // Display setup
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();
  delay(100);

  // Load WiFi credentials
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  if (ssid.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      macAddress = WiFi.macAddress();
      Serial.println(" connected!");
    } else {
      Serial.println(" failed!");
    }
  } else {
    Serial.println("No WiFi credentials stored");
  }

  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  drawHomeScreen();
}

void loop() {
  // Handle WiFi hotspot timeout
  if (inHotspotMode && millis() - wifiHotspotStartTime > WIFI_HOTSPOT_TIMEOUT) {
    stopWiFiHotspot();
    drawWiFiStatusScreen("Hotspot Timeout");
  }

  // Handle WiFi connection status
  checkWiFiConnection();

  // Handle MQTT
  if (wifiConnected && !inHotspotMode) {
    if (!client.connected()) {
      reconnectMQTT();
    }
    client.loop();
  }

  // Handle encoder rotation
  currentStateCLK = digitalRead(ENCODER_CLK);
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (currentScreen == MAIN_MENU || currentScreen == SETTINGS_MENU || currentScreen == ASSIST_MODE || currentScreen == ALERT_MODE) {
      lastMenuInteraction = millis();

      // Determine which direction the encoder is rotating
      if (digitalRead(ENCODER_DT) != currentStateCLK) {
        // Clockwise rotation
        menuIndex++;
      } else {
        // Counter-clockwise rotation
        menuIndex--;
      }

      // Constrain menu index based on current menu
      if (currentScreen == MAIN_MENU) {
        menuIndex = constrain(menuIndex, 0, mainMenuLength - 1);
      } else if (currentScreen == SETTINGS_MENU) {
        menuIndex = constrain(menuIndex, 0, settingsMenuLength - 1);
      }

      // Redraw menu if needed
      if (currentScreen == MAIN_MENU || currentScreen == SETTINGS_MENU) {
        drawMenuScreen();
      }
    }
  }
  lastStateCLK = currentStateCLK;

  // Handle encoder button press
  if (digitalRead(ENCODER_SW) == LOW) {
    if (millis() - lastButtonPress > 250 && !buttonPressed) {
      buttonPressed = true;
      lastButtonPress = millis();
      handleButtonPress();
      if (currentScreen != HOME) lastMenuInteraction = millis();
    }
  } else {
    buttonPressed = false;
  }

  // Handle menu timeout (return to home after inactivity) - except in WiFi setup mode
  if (currentScreen != HOME && currentScreen != WIFI_SETUP_MODE && millis() - lastMenuInteraction > 10000) {
    currentScreen = HOME;
    drawHomeScreen();
  }

  // Handle web server requests in hotspot mode
  if (inHotspotMode) {
    server.handleClient();
    // Update the display every second
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
      drawWiFiHotspotScreen();
      lastDisplayUpdate = millis();
    }
  }
}

/* ========== APPLICATION FUNCTIONS ========== */
void sendAssistRequest() {
  if (client.connected()) {
    client.publish(mqtt_topic, "ASSIST");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 30);
    display.print("ASSIST REQUEST SENT");
    display.display();
    delay(1000);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 30);
    display.print("NO WIFI CONNECTION");
    display.display();
    delay(1000);
  }
}

void sendAlertSignal() {
  if (client.connected()) {
    client.publish(mqtt_topic, "ALERT");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 30);
    display.print("ALERT REQUEST SENT");
    display.display();
    delay(1000);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(8, 30);
    display.print("NO WIFI CONNECTION");
    display.display();
    delay(1000);
  }
}