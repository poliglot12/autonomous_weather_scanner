#include <Arduino.h>
#include <Wire.h>

#include "wind_vane.h"

constexpr uint8_t AS5600_ADDRESS = 0x36;

// AS5600 registers
constexpr uint8_t RAW_ANGLE_HIGH = 0x0C;
constexpr uint8_t RAW_ANGLE_LOW  = 0x0D;

// --------------------------------------------------
// CALIBRATION
// --------------------------------------------------

// Change this after physically pointing the vane NORTH.
//
// Example:
// If AS5600 reads 73 degrees while vane points north,
// set NORTH_OFFSET_DEG = 73.0f
constexpr float NORTH_OFFSET_DEG = 101.9f;


// --------------------------------------------------
// INITIALIZATION
// --------------------------------------------------

bool initWindVane() {

    Wire.beginTransmission(AS5600_ADDRESS);

    if (Wire.endTransmission() != 0) {
        Serial.println("AS5600 not found");
        return false;
    }

    Serial.println("AS5600 wind vane found");

    return true;
}


// --------------------------------------------------
// RAW ANGLE
// --------------------------------------------------

uint16_t readRawAngle() {

    Wire.beginTransmission(AS5600_ADDRESS);
    Wire.write(RAW_ANGLE_HIGH);

    if (Wire.endTransmission(false) != 0) {
        return 0;
    }

    Wire.requestFrom(AS5600_ADDRESS, (uint8_t)2);

    if (Wire.available() < 2) {
        return 0;
    }

    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();

    return ((highByte & 0x0F) << 8) | lowByte;
}


// --------------------------------------------------
// DIRECTION IN DEGREES
// --------------------------------------------------

float getWindDirectionDegrees() {

    constexpr int SAMPLES = 12;

    float sinSum = 0.0f;
    float cosSum = 0.0f;

    for (int i = 0; i < SAMPLES; i++) {

        uint16_t raw = readRawAngle();

        float degrees =
            raw * 360.0f / 4096.0f;

        float radians =
            degrees * PI / 180.0f;

        sinSum += sin(radians);
        cosSum += cos(radians);

        delay(2);
    }

    float averageRadians =
        atan2(sinSum / SAMPLES,
              cosSum / SAMPLES);

    float degrees =
        averageRadians * 180.0f / PI;

    if (degrees < 0.0f) {
        degrees += 360.0f;
    }

    // Apply calibration
    degrees -= NORTH_OFFSET_DEG;

    if (degrees < 0.0f) {
        degrees += 360.0f;
    }

    if (degrees >= 360.0f) {
        degrees -= 360.0f;
    }

    return degrees;
}


// --------------------------------------------------
// CARDINAL DIRECTION
// --------------------------------------------------

const char* getWindDirectionCardinal() {

    float degrees = getWindDirectionDegrees();

    if (degrees >= 337.5 || degrees < 22.5)
        return "N";

    if (degrees < 67.5)
        return "NE";

    if (degrees < 112.5)
        return "E";

    if (degrees < 157.5)
        return "SE";

    if (degrees < 202.5)
        return "S";

    if (degrees < 247.5)
        return "SW";

    if (degrees < 292.5)
        return "W";

    return "NW";
}