#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <Preferences.h>

/* Hardware Configuration */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SDA_PIN 21
#define SCL_PIN 22
#define VIBRATOR_EN_PIN 33

// ========================
// BATTERY CONFIG
// ========================
#define BATTERY_ADC_PIN 19
#define R1 220000.0  // 220k resistor
#define R2 100000.0  // 100k resistor
#define BATTERY_FULL 4.2
#define BATTERY_EMPTY 3.7
#define VOLTAGE_OFFSET 0.02
#define ADC_REF_VOLTAGE 3.3
#define ADC_RESOLUTION 4095.0

// ========================
// WIFI CONFIG 
// ========================
#define WIFI_RSSI_MAX -50  // Best signal
#define WIFI_RSSI_MIN -100  // Weakest usable signal
/* System Constants */
#define WIFI_HOTSPOT_TIMEOUT 180000  // 3 minutes
#define MQTT_SERVER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC "band/alert"
#define BATTERY_PIN 19        // Analog pin for battery voltage
#define BATTERY_READ_INTERVAL 10000  // 10 seconds
#define DEVICE_ID "BAND-001X"  // Unified device ID
/* Screen States */
enum ScreenState {
  HOME,
  MAIN_MENU,
  ALERT_MODE,
  ASSIST_MODE,
  NORMAL_MODE,
  SETTINGS_MENU,
  WIFI_SETUP_MODE
};

#endif