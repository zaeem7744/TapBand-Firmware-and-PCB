#include "Buttons.h"
#include "Menu_System.h"
#include "Display_Functions.h"
#include "Vibration_System.h"  // ADD THIS

// ADD THESE EXTERNAL DECLARATIONS:
extern int menuIndex;
extern int currentMenu;
extern ScreenState currentScreen;
extern unsigned long lastMenuInteraction;
extern const char* mainMenuItems[];
extern const int mainMenuLength;
extern const char* settingsMenuItems[];
extern const int settingsMenuLength;

// Debounce timing
unsigned long lastButtonPress = 0;

void initButtons() {
  pinMode(BUTTON_UP, INPUT_PULLUP);      // ADD _PULLUP
  pinMode(BUTTON_DOWN, INPUT_PULLUP);    // ADD _PULLUP  
  pinMode(BUTTON_SELECT, INPUT_PULLUP);  // ADD _PULLUP
}

void checkButtons() {
  static unsigned long lastDebounceTime = 0;
  static int lastButtonStateUp = HIGH;
  static int lastButtonStateDown = HIGH;
  static int lastButtonStateSelect = HIGH;
  
  if (millis() - lastDebounceTime < 50) return; // 50ms debounce
  
  int currentStateUp = digitalRead(BUTTON_UP);
  int currentStateDown = digitalRead(BUTTON_DOWN);
  int currentStateSelect = digitalRead(BUTTON_SELECT);
  
  // Check UP button - falling edge (HIGH to LOW)
  if (currentStateUp == LOW && lastButtonStateUp == HIGH) {
    lastDebounceTime = millis();
    menuIndex--;
    if (menuIndex < 0) menuIndex = (currentMenu == 0) ? mainMenuLength - 1 : settingsMenuLength - 1;
    vibeMenuNavigate();  // ADD THIS
    updateMenuDisplay();
  }
  
  // Check DOWN button - falling edge (HIGH to LOW)
  if (currentStateDown == LOW && lastButtonStateDown == HIGH) {
    lastDebounceTime = millis();
    menuIndex++;
    int maxIndex = (currentMenu == 0) ? mainMenuLength - 1 : settingsMenuLength - 1;
    if (menuIndex > maxIndex) menuIndex = 0;
    vibeMenuNavigate();  // ADD THIS
    updateMenuDisplay();
  }
  
  // Check SELECT button - falling edge (HIGH to LOW)
  if (currentStateSelect == LOW && lastButtonStateSelect == HIGH) {
    lastDebounceTime = millis();
    vibeButtonPress();  // ADD THIS
    handleButtonPress();
  }
  
  lastButtonStateUp = currentStateUp;
  lastButtonStateDown = currentStateDown;
  lastButtonStateSelect = currentStateSelect;
}

void updateMenuDisplay() {
  vibeScreenChange();  // ADD THIS
  
  if (currentScreen == MAIN_MENU) {
    drawMenuScreen(mainMenuItems, mainMenuLength, true);
  } else if (currentScreen == SETTINGS_MENU) {
    drawMenuScreen(settingsMenuItems, settingsMenuLength, false);
  }
  lastMenuInteraction = millis();
}