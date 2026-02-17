#include <WiFi.h>
#include <PubSubClient.h>

/* ========== CONFIG ========== */
#define WIFI_SSID     "TP-Link_5F74"
#define WIFI_PASSWORD "73060420"

#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "band/alert"

#define BUTTON_PIN 18
#define LED_PIN LED_BUILTIN

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 300;
bool lastButtonState = HIGH;
bool ledBlink = false;
unsigned long blinkStart = 0;

/* ========== Connect to Wi-Fi ========== */
void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP address: " + WiFi.localIP().toString());
}

/* ========== MQTT Callback ========== */
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.println("[MQTT Received] Topic: " + String(topic) + " | Message: " + msg);

  // Blink LED for alert messages
  if (msg == "ALERT" || msg == "ASSIST") {
    ledBlink = true;
    blinkStart = millis();
    digitalWrite(LED_PIN, HIGH);
  }
}

/* ========== MQTT Reconnect ========== */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP32Client-" + String(random(1000, 9999));  // Generate random client ID
    if (client.connect(clientId.c_str())) {  // Use c_str() to convert to const char*
      Serial.println("connected.");
      client.subscribe(MQTT_TOPIC);
      Serial.println("Subscribed to topic: " + String(MQTT_TOPIC));
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s...");
      delay(5000);
    }
  }
}

/* ========== Setup ========== */
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setup_wifi();

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

/* ========== Main Loop ========== */
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Button Press Handling
  bool buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH && millis() - lastDebounceTime > debounceDelay) {
    lastDebounceTime = millis();
    Serial.println("[MQTT Publish] Sending ALERT...");
    client.publish(MQTT_TOPIC, "ALERT");
  }
  lastButtonState = buttonState;

  // Blink timer
  if (ledBlink && millis() - blinkStart > 500) {
    digitalWrite(LED_PIN, LOW);
    ledBlink = false;
  }
}
