#pragma once

#include <Arduino.h>

void initAnemometer();
void updateAnemometer();

unsigned long getRevolutionCount();

float getRPM();
float getWindSpeedMPS();
float getWindSpeedMPH();