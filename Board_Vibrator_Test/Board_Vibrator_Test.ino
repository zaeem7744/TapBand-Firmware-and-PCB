#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DRV2605
Adafruit_DRV2605 drv;

// Menu buttons
#define BUTTON_UP     27
#define BUTTON_DOWN   34
#define BUTTON_SELECT 35

// NeoPixel
#define LED_PIN 4
Adafruit_NeoPixel led(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// Selected effects
const uint8_t effects[4] = {1, 14, 52, 118};
const char* effectNames[4] = {"Strong Click", "Strong Buzz", "Pulsing Strong", "Long Buzz"};
uint8_t menuIndex = 0;

void setup() {
  Serial.begin(9600);

  // Buttons
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  // I2C
  Wire.begin(21, 22);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("DRV2605 Menu");
  display.display();

  // DRV2605 init
  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    display.println("DRV2605 not found!");
    display.display();
    while (1) delay(10);
  }

  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
  delay(1000);

  // NeoPixel init
  led.begin();
  led.show();
}

void loop() {
  handleButtons();
  displayMenu();
  idleLed(); // run slow idle animation when not vibrating
}

// Handle button presses
void handleButtons() {
  static uint32_t lastDebounce = 0;
  if (millis() - lastDebounce < 150) return; // debounce

  if (!digitalRead(BUTTON_UP)) {
    if (menuIndex == 0) menuIndex = 3;
    else menuIndex--;
    lastDebounce = millis();
  }

  if (!digitalRead(BUTTON_DOWN)) {
    menuIndex++;
    if (menuIndex > 3) menuIndex = 0;
    lastDebounce = millis();
  }

  if (!digitalRead(BUTTON_SELECT)) {
    runEffect(effects[menuIndex]);
    lastDebounce = millis();
  }
}

// Display menu on OLED
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  for (int i = 0; i < 4; i++) {
    display.setCursor(0, i * 14);
    if (i == menuIndex) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // highlight
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.println(effectNames[i]);
  }
  display.display();
}

// Run vibration and LED blink for 20 seconds
void runEffect(uint8_t effect) {
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    // Vibrate
    drv.setWaveform(0, effect);
    drv.setWaveform(1, 0);
    drv.go();

    // Fast LED blink
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.show();
    delay(100);
    led.setPixelColor(0, 0);
    led.show();
    delay(100);
  }
}

// Idle LED animation (slow red fade)
void idleLed() {
  static uint8_t brightness = 0;
  static int8_t direction = 5;

  brightness += direction;
  if (brightness == 0 || brightness == 255) direction = -direction;

  led.setPixelColor(0, led.Color(brightness, 0, 0));
  led.show();
  delay(30);
}
