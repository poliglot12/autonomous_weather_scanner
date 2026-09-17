#pragma once

#include <Arduino.h>

bool connectWiFi();

bool uploadWeatherToNeon(
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv,
    float windSpeed,
    float windDirection,
    float rpm
);