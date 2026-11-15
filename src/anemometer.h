#pragma once

#include <Arduino.h>

void initAnemometer();
void updateAnemometer();

// Lets the ESP32 wake from light sleep on reed switch pulses,
// so wind counting keeps working while the station is sleeping.
void enableAnemometerLightSleepWakeup();

unsigned long getRevolutionCount();

float getRPM();
float getWindSpeedMPS();
float getWindSpeedMPH();