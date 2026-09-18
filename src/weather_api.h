#pragma once

#include <Arduino.h>

bool connectWiFi();

bool uploadWeatherToWorker(
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv,
    float windSpeed,
    float windDirection,
    float rpm
);