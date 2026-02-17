#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 4

CRGB leds[NUM_LEDS];

// --- Function prototypes (optional if functions are below loop) ---
void patternRainbow();
void patternBlinkRed();
void patternBreathingBlue();

int pattern = 0;         
unsigned long lastChange = 0;
const unsigned long interval = 3000; // change pattern every 3 seconds

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void loop() {
  // Change pattern every interval
  if (millis() - lastChange > interval) {
    pattern = (pattern + 1) % 3; // cycles 0,1,2
    lastChange = millis();
  }

  switch(pattern) {
    case 0:
      patternRainbow();
      break;
    case 1:
      patternBlinkRed();
      break;
    case 2:
      patternBreathingBlue();
      break;
  }

  FastLED.show();
  delay(20);
}

// --- Pattern 0: Rainbow ---
void patternRainbow() {
  static uint8_t hue = 0;
  leds[0] = CHSV(hue, 255, 255);
  hue++;
}

// --- Pattern 1: Blinking Red ---
void patternBlinkRed() {
  static bool on = false;
  on = !on;
  leds[0] = on ? CRGB::Red : CRGB::Black;  // fixed typo here
}

// --- Pattern 2: Breathing Blue ---
void patternBreathingBlue() {
  static uint8_t brightness = 0;
  static int8_t delta = 5;

  leds[0] = CRGB(0, 0, brightness);
  brightness += delta;

  if (brightness == 0 || brightness == 255) delta = -delta;
}
