#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// ======== CONFIG ========
#define WIFI_SSID "VAASA-65200"
#define WIFI_PASS "01234567890"
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC_SUB "alertband/test"
#define MQTT_TOPIC_PUB "alertband/test"

#define BUTTON_PIN 27
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define uS_TO_S_FACTOR 1000000ULL
#define SLEEP_DELAY_SECONDS 300

RTC_DATA_ATTR int buttonPressCount = 0;
RTC_DATA_ATTR int bootCount = 0;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// ======================== SETUP ========================
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  bootCount++;
  Serial.printf("\n[BOOT] Wakeup #%d\n", bootCount);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] OLED init failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
  Serial.printf("[WAKE] Reason: %d\n", wakeReason);

  initWiFi();
  initMQTT();

  if (wakeReason == ESP_SLEEP_WAKEUP_EXT0) {
    // Button woke device
    buttonPressCount++;
    handleButtonPress();
  } else {
    // First boot or timer wake
    showMessage("System Ready");
  }

  delay(3000);
  goToSleep();
}

// ======================== LOOP ========================
void loop() {
  // Not used — device goes to sleep after setup
}

// ======================== DISPLAY HELPERS ========================
void showMessage(String msg) {
  Serial.println("[OLED] " + msg);
  display.clearDisplay();
  display.setCursor(0, 25);
  display.println(msg);
  display.display();
}

void showConfirmationScreen() {
  display.clearDisplay();
  display.setCursor(0, 15);
  display.println("Press again to");
  display.println("SEND ALERT");
  display.display();
}

// ======================== WIFI / MQTT ========================
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WIFI] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected");
}

void initMQTT() {
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(mqttCallback);
  reconnectMQTT();
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("[MQTT] Connecting...");
    if (client.connect("ESP32TapBand")) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_SUB);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
  client.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println("[MQTT IN] " + msg);
  showMessage("MQTT Msg: " + msg);
  delay(3000);  // Keep it visible briefly
}

// ======================== BUTTON LOGIC ========================
void handleButtonPress() {
  switch (buttonPressCount % 3) {
    case 1:
      showMessage("Tap Band Active");
      break;
    case 2:
      showConfirmationScreen();
      break;
    case 0:
      sendMQTTMessage("BUTTON_ALERT");
      showMessage("Alert Sent!");
      delay(2000);
      break;
  }
}

void sendMQTTMessage(String message) {
  if (!client.connected()) reconnectMQTT();
  client.publish(MQTT_TOPIC_PUB, message.c_str());
  Serial.println("[MQTT OUT] " + message);
}

// ======================== SLEEP ========================
void goToSleep() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  Serial.println("[SLEEP] Going to deep sleep...");
  delay(500);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, LOW);  // Wake on button
  esp_sleep_enable_timer_wakeup(SLEEP_DELAY_SECONDS * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}
