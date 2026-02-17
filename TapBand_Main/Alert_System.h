#ifndef ALERT_SYSTEM_H
#define ALERT_SYSTEM_H

#include "Config.h"

// Function declarations
void sendAlertSignal();
void sendAssistRequest();
void sendNormalSignal();
void handleIncomingAlert(const String& message);

#endif