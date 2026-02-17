#include "Vibration_System.h"
#include "Config.h"

Adafruit_DRV2605 drv;

void initVibration() {
  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    return;
  }
  
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
  
  // Set default parameters
  drv.setRealtimeValue(0x00); // Turn off real-time playback
}

void playVibrationEffect(uint8_t effect) {
  if (effect == VIBE_NONE) return;
  
  drv.setWaveform(0, effect);
  drv.setWaveform(1, 0); // End waveform
  drv.go();
}

// Short click for button presses (Strong Click - 60%)
void vibeButtonPress() {
  playVibrationEffect(VIBE_BUTTON_PRESS);
}

// Sharp click for menu navigation (Sharp Click - 100%)
void vibeMenuNavigate() {
  playVibrationEffect(VIBE_MENU_NAVIGATE);
}

// Strong buzz for alert sent (Strong Buzz - 100%)
void vibeAlertSent() {
  playVibrationEffect(VIBE_ALERT_SENT);
}

// Triple click for assist request (Triple Click - 100%)
void vibeAssistSent() {
  playVibrationEffect(VIBE_ASSIST_SENT);
}

// Double click for normal signal (Double Click - 100%)
void vibeNormalSent() {
  playVibrationEffect(VIBE_NORMAL_SENT);
}

// 1000ms alert for incoming alerts
void vibeIncomingAlert() {
  playVibrationEffect(VIBE_INCOMING_ALERT);
}

// Strong click for successful WiFi connection
void vibeWiFiConnected() {
  playVibrationEffect(VIBE_WIFI_CONNECTED);
}

// Soft fuzz for WiFi disconnect
void vibeWiFiDisconnected() {
  playVibrationEffect(VIBE_WIFI_DISCONNECTED);
}

// Buzz for errors
void vibeError() {
  playVibrationEffect(VIBE_ERROR);
}

// Transition click for confirmations
void vibeConfirmation() {
  playVibrationEffect(VIBE_CONFIRMATION);
}

// Pulsing for low battery warning
void vibeLowBattery() {
  playVibrationEffect(VIBE_LOW_BATTERY);
}

// Transition hum for hotspot start
void vibeHotspotStart() {
  playVibrationEffect(VIBE_HOTSPOT_START);
}

// Soft click for menu back actions
void vibeMenuBack() {
  playVibrationEffect(VIBE_MENU_BACK);
}

// Sharp tick for screen transitions
void vibeScreenChange() {
  playVibrationEffect(VIBE_SCREEN_CHANGE);
}

// Medium click for successful connections
void vibeConnectionSuccess() {
  playVibrationEffect(VIBE_CONNECTION_SUCCESS);
}

// Weak buzz for signal warnings
void vibeWeakSignal() {
  playVibrationEffect(VIBE_WEAK_SIGNAL);
}