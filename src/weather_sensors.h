#pragma once

#include <Arduino.h>

struct WeatherData {
    float temperature;
    float humidity;
    float pressure;
    uint32_t uv;
};

bool initWeatherSensors();

WeatherData readWeatherSensors();