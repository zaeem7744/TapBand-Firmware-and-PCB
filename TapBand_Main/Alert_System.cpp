#include "Alert_System.h"
#include "Display_Functions.h"
#include "WiFi_MQTT.h"
#include "Vibration_System.h"  // ADD THIS

extern PubSubClient client;
extern String incomingAlert;
extern unsigned long lastAlertTime;

void sendAlertSignal() {
  if (client.connected()) {
    client.publish(MQTT_TOPIC, "ALERT");
    incomingAlert = "ALERT";
    lastAlertTime = millis();
    vibeAlertSent();  // ADD THIS
    drawWiFiStatusScreen("ALERT SENT");
    drawHomeScreen(); // Force immediate update
  } else {
    vibeError();  // ADD THIS
    drawWiFiStatusScreen("NO WIFI");
  }
}

void sendAssistRequest() {
  if (client.connected()) {
    client.publish(MQTT_TOPIC, "ASSIST");
    vibeAssistSent();  // ADD THIS
    drawWiFiStatusScreen("ASSIST REQUEST SENT");
  } else {
    vibeError();  // ADD THIS
    drawWiFiStatusScreen("NO WIFI CONNECTION");
  }
}

void sendNormalSignal() {
  if (client.connected()) {
    client.publish(MQTT_TOPIC, "NORMAL");
    vibeNormalSent();  // ADD THIS
    drawWiFiStatusScreen("NORMAL SIGNAL SENT");
  } else {
    vibeError();  // ADD THIS
    drawWiFiStatusScreen("NO WIFI CONNECTION");
  }
}

void handleIncomingAlert(const String& message) {
  incomingAlert = message;
  lastAlertTime = millis();
  vibeIncomingAlert();  // ADD THIS
  drawHomeScreen(); // Force screen update
  
  // Visual feedback
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(20);
    digitalWrite(LED_BUILTIN, LOW);
    delay(20);
  }
}