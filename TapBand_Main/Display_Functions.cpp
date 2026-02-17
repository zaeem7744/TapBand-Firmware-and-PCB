#include "Display_Functions.h"
#include "Config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// External variables
extern int batteryLevel;
extern int wifiStrength;
extern bool wifiConnected;
extern String incomingAlert;
extern unsigned long lastAlertTime;
extern String deviceID;
extern bool inHotspotMode;
extern unsigned long wifiHotspotStartTime;

void initDisplay() {
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();
}

void drawHomeScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ===== HEADER WITH BORDER =====
  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_BLACK);
  display.drawRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);

  // ===== BATTERY INDICATOR =====
  display.drawRect(2, 4, 30, 8, SSD1306_WHITE);           // Outer battery shape
  display.fillRect(32, 6, 2, 4, SSD1306_WHITE);           // Battery tip
  int batteryFill = map(batteryLevel, 0, 100, 0, 28);     // Calculate fill width
  display.fillRect(3, 5, batteryFill, 6, SSD1306_WHITE);  // Fill level
  display.setTextSize(1);
  display.setCursor(38, 4);
  display.print(batteryLevel);
  display.print("%");

  // ===== WIFI INDICATOR =====
  int wifiX = SCREEN_WIDTH - 25;
  if (wifiConnected) {
    // Draw 4 bars (filled based on strength)
    for (int i = 0; i < 4; i++) {
      int barHeight = (i + 1) * 2 + 2;  // Heights: 4,6,8,10 pixels
      int barY = 14 - barHeight;
      if (i < wifiStrength) {
        display.fillRect(wifiX + i * 5, barY, 3, barHeight, SSD1306_WHITE);
      }
    }
  }

  // ===== MAIN STATUS AREA =====
  display.setTextSize(2);
  display.setCursor(30, 25);

  if (!wifiConnected) {
    display.print("NO WIFI");
  } else if (millis() - lastAlertTime < 8000 && incomingAlert.length() > 0) {
    display.print(incomingAlert);  // Show alerts for 8 seconds
  } else {
    display.print("NORMAL");
  }

  // ===== FOOTER WITH BORDER =====
  display.setTextSize(1);
  display.setCursor(25, 45);
  display.print("TAP BAND V1.1");
  display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, SSD1306_WHITE);
  display.setCursor(0, SCREEN_HEIGHT - 8);
  display.print("Press Button For Menu");

  display.display();
}

void drawWiFiHotspotScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.println("Wi-Fi Hotspot Active");
  display.setCursor(5, 20);
  display.print("SSID: TapBand_");
  display.println(deviceID.substring(5));
  display.setCursor(5, 35);
  display.print("IP: ");
  display.print(WiFi.softAPIP().toString()); // Actual IP
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.print("Timeout: ");
  display.print((WIFI_HOTSPOT_TIMEOUT - (millis() - wifiHotspotStartTime)) / 1000);
  display.print("s");
  display.display();
}

void drawWiFiStatusScreen(const char* status) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print(status);
  display.display();
  delay(500);
}

void drawMenuScreen(const char** menuItems, int menuLength, bool isMainMenu) {
  display.clearDisplay();

  // Menu title
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH / 2 - (isMainMenu ? 36 : 30), 2);
  display.print(isMainMenu ? "MAIN MENU" : "SETTINGS");
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  // Calculate display parameters
  const int itemsPerPage = 4;  // Number of items visible at once
  const int itemHeight = 12;
  int firstVisibleItem = 0;

  // Handle scrolling if needed
  if (menuIndex >= itemsPerPage) {
    firstVisibleItem = menuIndex - itemsPerPage + 1;
  }

  // Display visible menu items
  for (int i = 0; i < min(itemsPerPage, menuLength - firstVisibleItem); i++) {
    int itemIdx = firstVisibleItem + i;
    int yPos = 15 + i * itemHeight;

    // Highlight selected item
    if (itemIdx == menuIndex) {
      display.fillRect(0, yPos - 1, SCREEN_WIDTH, itemHeight + 1, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);

      // LEFT-SIDE triangular indicator (pointing right ▶)
      display.drawTriangle(5, yPos + 3, 5, yPos + 9, 10, yPos + 6, SSD1306_BLACK);

      // Show scroll indicators if needed (up/down arrows)
      if (firstVisibleItem > 0) {
        display.drawTriangle(SCREEN_WIDTH / 2 - 3, 2, SCREEN_WIDTH / 2 + 3, 2, SCREEN_WIDTH / 2, 7, SSD1306_BLACK);
      }
      if (firstVisibleItem + itemsPerPage < menuLength) {
        // Changed position to right side and direction to point down
        display.drawTriangle(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 7, SCREEN_WIDTH - 5, SCREEN_HEIGHT - 7, SCREEN_WIDTH - 7, SCREEN_HEIGHT - 2,
                             (itemIdx == menuIndex) ? SSD1306_BLACK : SSD1306_WHITE);
      }
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(20, yPos);  // Adjusted to align text after the triangle
    display.print(menuItems[itemIdx]);
  }

  // Footer
  int footerY = 15 + min(itemsPerPage, menuLength) * itemHeight;
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
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH / 2 - strlen(title) * 3, 2);
  display.print(title);
  display.drawLine(0, 12, SCREEN_WIDTH, 12, SSD1306_WHITE);

  display.setCursor(SCREEN_WIDTH / 2 - strlen(instruction) * 3, 30);
  display.print(instruction);

  display.drawLine(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, SSD1306_WHITE);
  display.setCursor(30, SCREEN_HEIGHT - 7);
  display.print("Press To confirm");
  display.display();
}