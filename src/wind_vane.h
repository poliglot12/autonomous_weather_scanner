#pragma once

#include <Arduino.h>

bool initWindVane();

float getWindDirectionDegrees();

const char* getWindDirectionCardinal();