#pragma once

#include <Arduino.h>
#include <RTClib.h>

bool initSD();

bool createWeatherFile();

bool logWeatherData(
    const DateTime& timestamp,
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv
);