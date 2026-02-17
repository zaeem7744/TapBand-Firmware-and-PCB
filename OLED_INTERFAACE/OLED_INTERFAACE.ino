#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ========== HARDWARE CONFIGURATION ========== */
// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SDA_PIN 5
#define SCL_PIN 6

// Rotary Encoder with ESP32-C6 compatible pins
#define ENCODER_CLK 34 // CLK pin
#define ENCODER_DT  35  // DT pin
#define ENCODER_SW  32  // Switch pin

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ========== SYSTEM VARIABLES ========== */
// Menu configuration
const char* menuItems[] = { "Assist Mode", "Alert Mode", "Settings", "Back" };
const int menuLength = sizeof(menuItems) / sizeof(menuItems[0]);

enum ScreenState { HOME, MENU, ASSIST_MODE, ALERT_MODE, SETTINGS_MODE };
ScreenState currentScreen = HOME;
int menuIndex = 0;

// Rotary encoder variables
int counter = 0;
int currentStateCLK;
int lastStateCLK;
unsigned long lastButtonPress = 0;
bool buttonPressed = false;

// Status indicators
int batteryLevel = 85;
int wifiStrength = 3; // 0-3 scale

/* ========== MAIN SETUP ========== */
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  while(!Serial); // Wait for serial connection on native USB
  
  // Set encoder pins as inputs with internal pullups
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  // Read the initial state of CLK
  lastStateCLK = digitalRead(ENCODER_CLK);
  
  // Initialize display
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  // Initial display setup
  display.clearDisplay();
  display.display();
  delay(100); // Display stabilization
  
  // Draw initial screen
  drawHomeScreen();
}

/* ========== MAIN LOOP ========== */
void loop() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(ENCODER_CLK);
  
  // Handle rotary encoder rotation
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    // Only process rotation when in menu screens
    if (currentScreen == MENU || currentScreen == ASSIST_MODE || 
        currentScreen == ALERT_MODE || currentScreen == SETTINGS_MODE) {
      
      // Determine direction
      if (digitalRead(ENCODER_DT) != currentStateCLK) {
        // Counter-clockwise
        menuIndex = constrain(menuIndex + 1, 0, menuLength - 1);
      } else {
        // Clockwise
        menuIndex = constrain(menuIndex - 1, 0, menuLength - 1);
      }
      
      // Update menu display if in menu
      if (currentScreen == MENU) {
        drawMenuScreen();
      }
    }
  }
  lastStateCLK = currentStateCLK;

  // Handle button press
  if (digitalRead(ENCODER_SW) == LOW) {
    // Debounce check
    if (millis() - lastButtonPress > 250 && !buttonPressed) {
      buttonPressed = true;
      lastButtonPress = millis();
      
      handleButtonPress();
    }
  } else {
    buttonPressed = false;
  }

  // Add any periodic checks here if needed
}

/* ========== BUTTON PRESS HANDLER ========== */
void handleButtonPress() {
  switch (currentScreen) {
    case HOME:
      // Enter menu from home screen
      currentScreen = MENU;
      menuIndex = 0;
      drawMenuScreen();
      break;
      
    case MENU:
      // Handle menu selection
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

/* ========== MENU FUNCTIONS ========== */
void handleMenuSelection(int index) {
  switch (index) {
    case 0: // Assist Mode
      currentScreen = ASSIST_MODE;
      drawModeScreen("ASSIST MODE", "Press To Request Help");
      break;
    case 1: // Alert Mode
      currentScreen = ALERT_MODE;
      drawModeScreen("ALERT MODE", "Press To Send Alert");
      break;
    case 2: // Settings
      currentScreen = SETTINGS_MODE;
      handleSettings();
      break;
    case 3: // Back
      currentScreen = HOME;
      drawHomeScreen();
      break;
  }
}

/* ========== DISPLAY FUNCTIONS ========== */
void drawHomeScreen() {
  display.clearDisplay();
  
  // Set text properties
  display.setTextColor(SSD1306_WHITE);
  
  // Status bar
  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_BLACK);
  display.drawRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
  
  // Battery indicator
  display.drawRect(2, 4, 30, 8, SSD1306_WHITE);
  display.fillRect(32, 6, 2, 4, SSD1306_WHITE);
  int batteryFill = map(batteryLevel, 0, 100, 0, 28);
  display.fillRect(3, 5, batteryFill, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(38, 4);
  display.print(batteryLevel);
  display.print("%");
  
  // WiFi indicator
  int wifiX = SCREEN_WIDTH - 25;
  for (int i = 0; i < 3; i++) {
    if (wifiStrength > i) {
      display.fillRect(wifiX + i*6, 12 - i*3, 4, i*3 + 4, SSD1306_WHITE);
    } else {
      display.drawRect(wifiX + i*6, 12 - i*3, 4, i*3 + 4, SSD1306_WHITE);
    }
  }
  
  // Main content
  display.setTextSize(2);
  display.setCursor(30, 25);
  display.print("NORMAL");
  
  // Version info
  display.setTextSize(1);
  display.setCursor(25, 45);
  display.print("TAP BAND V1.1");
  
  // Footer instruction
  display.drawLine(0, SCREEN_HEIGHT-10, SCREEN_WIDTH, SCREEN_HEIGHT-10, SSD1306_WHITE);
  display.setCursor(0, SCREEN_HEIGHT-8);
  display.print("Press Button For Menu");
  
  display.display();
}

void drawMenuScreen() {
  display.clearDisplay();
  
  // Header
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print("MENU OPTIONS");
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);
  
  // Menu items
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
    
    // Selection indicator
    if (i == menuIndex) {
      display.drawTriangle(110, y+3, 120, y+6, 110, y+9, SSD1306_BLACK);
    }
  }
  
  // Footer
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
  
  // Header
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH/2 - strlen(title)*3, 2);
  display.print(title);
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);
  
  // Instruction
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH/2 - strlen(instruction)*3, 30);
  display.print(instruction);
  
  // Footer
  display.drawLine(0, SCREEN_HEIGHT-10, SCREEN_WIDTH, SCREEN_HEIGHT-10, SSD1306_WHITE);
  display.setCursor(30, SCREEN_HEIGHT-7);
  display.print("Press To confirm");
  
  display.display();
}

/* ========== APPLICATION FUNCTIONS ========== */
void sendAssistRequest() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8, 30);
  display.print("ASSIST REQUEST SENT");
  display.display();
  delay(1000);
}

void sendAlertSignal() {
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
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.print("Feature coming soon");
  display.display();
  delay(1500);
}