#ifndef BUTTONS_H
#define BUTTONS_H

#include "Config.h"

// Button pin definitions
#define BUTTON_UP     27
#define BUTTON_DOWN   34
#define BUTTON_SELECT 35

// Debounce
extern unsigned long lastButtonPress;

// Functions
void initButtons();
void checkButtons();
void updateMenuDisplay();  // <-- ADD THIS LINE

#endif