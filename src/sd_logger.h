#pragma once

#include <Arduino.h>

bool initSD();
bool writeTestFile();

bool createWeatherFile();
bool logWeatherData(
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv
);