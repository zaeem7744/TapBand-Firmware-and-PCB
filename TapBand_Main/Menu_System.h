#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include "Config.h"

// Menu items
extern const char* mainMenuItems[];
extern const int mainMenuLength;
extern const char* settingsMenuItems[];
extern const int settingsMenuLength;

// Menu state
extern ScreenState currentScreen;
extern int currentMenu;
extern int menuIndex;
extern unsigned long lastMenuInteraction;

// Function declarations
void handleButtonPress();
void handleMainMenuSelection();
void handleSettingsMenuSelection();
void checkMenuTimeout();

#endif