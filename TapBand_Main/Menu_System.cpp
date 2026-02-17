#include "Menu_System.h"
#include "Display_Functions.h"
#include "WiFi_MQTT.h"
#include "Alert_System.h"
#include "Vibration_System.h"  // ADD THIS

// Menu items
const char* mainMenuItems[] = { "Alert Mode", "Assist Mode", "Normal Mode", "Settings", "Back" };
const int mainMenuLength = sizeof(mainMenuItems)/sizeof(mainMenuItems[0]);

const char* settingsMenuItems[] = { "Wi-Fi Setup", "Reset Wi-Fi", "Back" };
const int settingsMenuLength = sizeof(settingsMenuItems) / sizeof(settingsMenuItems[0]);

// Menu state
ScreenState currentScreen = HOME;
int currentMenu = 0;
int menuIndex = 0;
unsigned long lastMenuInteraction = 0;

void handleButtonPress() {
  switch (currentScreen) {
    case HOME:
      currentScreen = MAIN_MENU;
      currentMenu = 0;
      menuIndex = 0;
      lastMenuInteraction = millis();
      vibeScreenChange();  // ADD THIS
      drawMenuScreen(mainMenuItems, mainMenuLength, true);
      break;
      
    case MAIN_MENU:
      handleMainMenuSelection();
      break;
      
    case SETTINGS_MENU:
      handleSettingsMenuSelection();
      break;
      
    case ALERT_MODE:
      sendAlertSignal();
      currentScreen = HOME;
      drawHomeScreen();
      break;
      
    case ASSIST_MODE:
      sendAssistRequest();
      currentScreen = HOME;
      drawHomeScreen();
      break;
      
    case NORMAL_MODE:
      sendNormalSignal();
      currentScreen = HOME;
      drawHomeScreen();
      break;
      
    case WIFI_SETUP_MODE:
      stopWiFiHotspot();
      break;
  }
}

void handleMainMenuSelection() {
  switch (menuIndex) {
    case 0:  // Alert Mode
      currentScreen = ALERT_MODE;
      vibeConfirmation();  // ADD THIS
      drawModeScreen("ALERT MODE", "Press To Send Alert");
      break;
    case 1:  // Assist Mode
      currentScreen = ASSIST_MODE;
      vibeConfirmation();  // ADD THIS
      drawModeScreen("ASSIST MODE", "Press To Request Help");
      break;
    case 2:  // Normal Mode
      currentScreen = NORMAL_MODE;
      vibeConfirmation();  // ADD THIS
      drawModeScreen("NORMAL MODE", "Press To Send Normal");
      break;
    case 3:  // Settings
      currentScreen = SETTINGS_MENU;
      currentMenu = 1;
      menuIndex = 0;
      vibeScreenChange();  // ADD THIS
      drawMenuScreen(settingsMenuItems, settingsMenuLength, false);
      break;
    case 4:  // Back
      currentScreen = HOME;
      vibeMenuBack();  // ADD THIS
      drawHomeScreen();
      break;
  }
}

void handleSettingsMenuSelection() {
  switch (menuIndex) {
    case 0:  // Wi-Fi Setup
      currentScreen = WIFI_SETUP_MODE; // Set this before starting hotspot
      vibeConfirmation();  // ADD THIS
      startWiFiHotspot();
      break;
    case 1:  // Reset Wi-Fi
      resetWiFiCredentials();
      currentScreen = HOME;
      drawHomeScreen();
      break;
    case 2:  // Back
      currentScreen = MAIN_MENU;
      currentMenu = 0;
      menuIndex = 3;  // Return to Settings option
      vibeMenuBack();  // ADD THIS
      drawMenuScreen(mainMenuItems, mainMenuLength, true);
      break;
  }
}

void checkMenuTimeout() {
  if (currentScreen != HOME && currentScreen != WIFI_SETUP_MODE && 
      millis() - lastMenuInteraction > 10000) {
    currentScreen = HOME;
    vibeMenuBack();  // ADD THIS
    drawHomeScreen();
  }
}