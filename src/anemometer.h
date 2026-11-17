#pragma once

#include <Arduino.h>

void initAnemometer();
void updateAnemometer();

void enableAnemometerLightSleepWakeup();

unsigned long getRevolutionCount();

float getRPM();
float getWindSpeedMPS();
float getWindSpeedMPH();