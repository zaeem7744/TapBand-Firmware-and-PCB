#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>

/* ========== HARDWARE CONFIGURATION ========== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SDA_PIN 21
#define SCL_PIN 22

#define ENCODER_CLK 34
#define ENCODER_DT  35
#define ENCODER_SW  32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ========== SYSTEM VARIABLES ========== */
const char* menuItems[] = { "Assist Mode", "Alert Mode", "Settings", "Back" };
const int menuLength = sizeof(menuItems) / sizeof(menuItems[0]);

enum ScreenState { HOME, MENU, ASSIST_MODE, ALERT_MODE, SETTINGS_MODE };
ScreenState currentScreen = HOME;
int menuIndex = 0;

int counter = 0;
int currentStateCLK;
int lastStateCLK;
unsigned long lastButtonPress = 0;
bool buttonPressed = false;
unsigned long lastMenuInteraction = 0;

int batteryLevel = 85;
int wifiStrength = 3;

/* ========== WiFi + MQTT CONFIGURATION ========== */
const char* ssid = "VAASA-65200";
const char* password = "01234567890";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "band/alert";

WiFiClient espClient;
PubSubClient client(espClient);
String incomingAlert = "";          // Stores last received alert
unsigned long lastAlertTime = 0;

/* ========== SETUP ========== */
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(ENCODER_CLK);

  // Display
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.display();
  delay(100);
  drawHomeScreen();

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" connected!");

  // MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

/* ========== LOOP ========== */
void loop() {
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.print("Connecting to MQTT...");
      if (client.connect("ESP32_TapBand")) {
        Serial.println("connected");
        client.subscribe(mqtt_topic);
      } else {
        Serial.print(" failed, rc=");
        Serial.print(client.state());
        delay(2000);
      }
    }
  }
  client.loop();

  currentStateCLK = digitalRead(ENCODER_CLK);
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (currentScreen == MENU || currentScreen == ASSIST_MODE || currentScreen == ALERT_MODE || currentScreen == SETTINGS_MODE) {
      lastMenuInteraction = millis();
      if (digitalRead(ENCODER_DT) != currentStateCLK) {
        menuIndex = constrain(menuIndex + 1, 0, menuLength - 1);
      } else {
        menuIndex = constrain(menuIndex - 1, 0, menuLength - 1);
      }
      if (currentScreen == MENU) drawMenuScreen();
    }
  }
  lastStateCLK = currentStateCLK;

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

  if (currentScreen != HOME && millis() - lastMenuInteraction > 10000) {
    currentScreen = HOME;
    drawHomeScreen();
  }
}

/* ========== MQTT CALLBACK ========== */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message received: ");
  Serial.println(message);

  if (String(topic) == mqtt_topic && (message == "ASSIST" || message == "ALERT")) {
    incomingAlert = message;          // Save message to display instead of "NORMAL"
    lastAlertTime = millis();

    // Blink LED in alert pattern for 3 seconds (non-blocking would be better, but delay used for simplicity)
    for (int i = 0; i < 5; i++) {   // 5 cycles * 600ms = 3 seconds total
      digitalWrite(LED_BUILTIN, HIGH);
      delay(300);
      digitalWrite(LED_BUILTIN, LOW);
      delay(300);
    }

    drawHomeScreen();
  }
}

/* ========== MENU HANDLERS ========== */
void handleButtonPress() {
  switch (currentScreen) {
    case HOME:
      currentScreen = MENU;
      menuIndex = 0;
      lastMenuInteraction = millis();
      drawMenuScreen();
      break;
    case MENU:
      handleMenuSelection(menuIndex);
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
    case SETTINGS_MODE:
      currentScreen = MENU;
      drawMenuScreen();
      break;
  }
}

void handleMenuSelection(int index) {
  switch (index) {
    case 0:
      currentScreen = ASSIST_MODE;
      drawModeScreen("ASSIST MODE", "Press To Request Help");
      break;
    case 1:
      currentScreen = ALERT_MODE;
      drawModeScreen("ALERT MODE", "Press To Send Alert");
      break;
    case 2:
      currentScreen = SETTINGS_MODE;
      handleSettings();
      break;
    case 3:
      currentScreen = HOME;
      drawHomeScreen();
      break;
  }
}

/* ========== DISPLAY FUNCTIONS ========== */
void drawHomeScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_BLACK);
  display.drawRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);

  display.drawRect(2, 4, 30, 8, SSD1306_WHITE);
  display.fillRect(32, 6, 2, 4, SSD1306_WHITE);
  int batteryFill = map(batteryLevel, 0, 100, 0, 28);
  display.fillRect(3, 5, batteryFill, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(38, 4);
  display.print(batteryLevel);
  display.print("%");

  int wifiX = SCREEN_WIDTH - 25;
  for (int i = 0; i < 3; i++) {
    if (wifiStrength > i) {
      display.fillRect(wifiX + i*6, 12 - i*3, 4, i*3 + 4, SSD1306_WHITE);
    } else {
      display.drawRect(wifiX + i*6, 12 - i*3, 4, i*3 + 4, SSD1306_WHITE);
    }
  }

  display.setTextSize(2);
  display.setCursor(30, 25);

  // Display incoming alert or "NORMAL"
  if (millis() - lastAlertTime < 8000 && incomingAlert.length() > 0) {
    display.print(incomingAlert);
  } else {
    display.print("NORMAL");
    incomingAlert = "";   // Clear alert after timeout
  }

  display.setTextSize(1);
  display.setCursor(25, 45);
  display.print("TAP BAND V1.1");

  display.drawLine(0, SCREEN_HEIGHT-10, SCREEN_WIDTH, SCREEN_HEIGHT-10, SSD1306_WHITE);
  display.setCursor(0, SCREEN_HEIGHT-8);
  display.print("Press Button For Menu");

  display.display();
}

void drawMenuScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print("MENU OPTIONS");
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  int itemHeight = 13;
  int startY = 15;

  for (int i = 0; i < menuLength; i++) {
    int y = startY + i * itemHeight;
    if (i == menuIndex) {
      display.fillRect(0, y, SCREEN_WIDTH, itemHeight, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.setCursor(10, y+3);
    display.print(menuItems[i]);
    if (i == menuIndex) {
      display.drawTriangle(110, y+3, 120, y+6, 110, y+9, SSD1306_BLACK);
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
  display.setCursor(SCREEN_WIDTH/2 - strlen(title)*3, 2);
  display.print(title);
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  display.setCursor(SCREEN_WIDTH/2 - strlen(instruction)*3, 30);
  display.print(instruction);

  display.drawLine(0, SCREEN_HEIGHT-10, SCREEN_WIDTH, SCREEN_HEIGHT-10, SSD1306_WHITE);
  display.setCursor(30, SCREEN_HEIGHT-7);
  display.print("Press To confirm");

  display.display();
}

/* ========== APPLICATION FUNCTIONS ========== */
void sendAssistRequest() {
  client.publish(mqtt_topic, "ASSIST");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8, 30);
  display.print("ASSIST REQUEST SENT");
  display.display();
  delay(1000);
}

void sendAlertSignal() {
  client.publish(mqtt_topic, "ALERT");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8, 30);
  display.print("ALERT REQUEST SENT");
  display.display();
  delay(1000);
}

void handleSettings() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(40, 20);
  display.print("Settings");
  display.setCursor(10, 40);
  display.print("Feature coming soon");
  display.display();
  delay(1500);
}
