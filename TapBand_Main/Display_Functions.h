#ifndef DISPLAY_FUNCTIONS_H
#define DISPLAY_FUNCTIONS_H

#include "Config.h"

// Forward declarations
extern Adafruit_SSD1306 display;
extern int batteryLevel;
extern int wifiStrength;
extern bool wifiConnected;
extern String incomingAlert;
extern unsigned long lastAlertTime;
extern String deviceID;
extern bool inHotspotMode;
extern unsigned long wifiHotspotStartTime;
extern int menuIndex;  // <-- ADD THIS LINE

// Function declarations
void initDisplay();
void drawHomeScreen();
void drawWiFiHotspotScreen();
void drawWiFiStatusScreen(const char* status);
void drawMenuScreen(const char** menuItems, int menuLength, bool isMainMenu);
void drawModeScreen(const char* title, const char* instruction);

#endif