#ifndef VIBRATION_SYSTEM_H
#define VIBRATION_SYSTEM_H

#include <Adafruit_DRV2605.h>
#include "Config.h"

// Vibration effect definitions
enum VibrationEffect {
  VIBE_NONE = 0,
  VIBE_BUTTON_PRESS = 2,        // Strong Click - 60% (for button presses)
  VIBE_MENU_NAVIGATE = 4,       // Sharp Click - 100% (for menu navigation)
  VIBE_ALERT_SENT = 14,         // Strong Buzz - 100% (for alert sent)
  VIBE_ASSIST_SENT = 12,        // Triple Click - 100% (for assist request)
  VIBE_NORMAL_SENT = 10,        // Double Click - 100% (for normal signal)
  VIBE_INCOMING_ALERT = 16,     // 1000ms Alert - 100% (for incoming alerts)
  VIBE_WIFI_CONNECTED = 17,     // Strong Click 1 - 100% (for WiFi connection)
  VIBE_WIFI_DISCONNECTED = 13,  // Soft Fuzz - 60% (for WiFi disconnect)
  VIBE_ERROR = 47,              // Buzz 1 - 100% (for errors)
  VIBE_CONFIRMATION = 58,       // Transition Click 1 - 100% (for confirmations)
  VIBE_LOW_BATTERY = 52,        // Pulsing Strong 1 - 100% (for low battery)
  VIBE_HOTSPOT_START = 64,      // Transition Hum 1 - 100% (for hotspot start)
  VIBE_MENU_BACK = 6,           // Sharp Click - 30% (for menu back)
  VIBE_SCREEN_CHANGE = 24,      // Sharp Tick 1 - 100% (for screen transitions)
  VIBE_CONNECTION_SUCCESS = 21, // Medium Click 1 - 100% (for successful connections)
  VIBE_WEAK_SIGNAL = 51         // Buzz 5 - 20% (for weak signal warnings)
};

// Function declarations
void initVibration();
void playVibrationEffect(uint8_t effect);
void vibeButtonPress();
void vibeMenuNavigate();
void vibeAlertSent();
void vibeAssistSent();
void vibeNormalSent();
void vibeIncomingAlert();
void vibeWiFiConnected();
void vibeWiFiDisconnected();
void vibeError();
void vibeConfirmation();
void vibeLowBattery();
void vibeHotspotStart();
void vibeMenuBack();
void vibeScreenChange();
void vibeConnectionSuccess();
void vibeWeakSignal();

#endif