#include "weather_sensors.h"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>

Adafruit_BME280 bme;
Adafruit_LTR390 ltr;

bool bmeReady = false;
bool ltrReady = false;

uint32_t lastUV = 0;

bool initWeatherSensors() {

    // -------------------------
    // BME280
    // -------------------------

    bmeReady = bme.begin(0x76);

    if (bmeReady) {
        Serial.println("BME280 found");
    }
    else {
        Serial.println("BME280 not found");
    }


    // -------------------------
    // LTR390
    // -------------------------

    ltrReady = ltr.begin();

    if (ltrReady) {

        Serial.println("LTR390 found");

        ltr.setMode(LTR390_MODE_UVS);
        ltr.setGain(LTR390_GAIN_3);
        ltr.setResolution(LTR390_RESOLUTION_18BIT);
    }
    else {
        Serial.println("LTR390 not found");
    }


    // True only if BOTH are connected
    return bmeReady && ltrReady;
}

WeatherData readWeatherSensors() {

    WeatherData data;

    // Default values if sensor is disconnected
    data.temperature = NAN;
    data.humidity = NAN;
    data.pressure = NAN;
    data.uv = 0;


    // -------------------------
    // BME280
    // -------------------------

    if (bmeReady) {

        data.temperature = bme.readTemperature();
        data.humidity = bme.readHumidity();
        data.pressure = bme.readPressure() / 100.0F;
    }


    // -------------------------
    // LTR390
    // -------------------------

    if (ltrReady) {

        if (ltr.newDataAvailable()) {
            lastUV = ltr.readUVS();
        }

        data.uv = lastUV;
    }


    return data;
}