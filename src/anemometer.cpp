#include <Arduino.h>
#include "anemometer.h"

constexpr int REED_PIN = 33;

// Minimum allowed time between valid magnet passes.
// 40 ms still permits up to 1500 RPM while rejecting reed bounce.
constexpr unsigned long DEBOUNCE_US = 40000;

// If we don't see another rotation for this long,
// consider the anemometer stopped.
constexpr unsigned long STOP_TIMEOUT_US = 3000000;

// --------------------------------------------------
// PHYSICAL ANEMOMETER VALUES
// --------------------------------------------------

// IMPORTANT:
// Measure from the CENTER OF THE SHAFT
// to the CENTER OF ONE CUP.
//
// Example: 7 cm radius = 0.07 meters
constexpr float CUP_RADIUS_M = 0.0889f;

// Real cup anemometers need calibration.
// Leave at 1.0 for initial testing.
// We can calibrate this later.
constexpr float CALIBRATION_FACTOR = 1.0f;


// --------------------------------------------------
// INTERRUPT DATA
// --------------------------------------------------

volatile unsigned long revolutionCount = 0;

volatile unsigned long lastPulseTimeUS = 0;
volatile unsigned long rotationIntervalUS = 0;


// --------------------------------------------------
// REED SWITCH INTERRUPT
// --------------------------------------------------

void IRAM_ATTR reedISR() {

    unsigned long now = micros();

    unsigned long elapsed = now - lastPulseTimeUS;

    // Ignore contact bounce
    if (elapsed >= DEBOUNCE_US) {

        // Don't calculate an interval on the very first pulse
        if (lastPulseTimeUS != 0) {
            rotationIntervalUS = elapsed;
        }

        lastPulseTimeUS = now;

        revolutionCount++;
    }
}


// --------------------------------------------------
// INITIALIZATION
// --------------------------------------------------

void initAnemometer() {

    pinMode(REED_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(REED_PIN),
        reedISR,
        FALLING
    );

    Serial.println("Anemometer initialized");
}


// --------------------------------------------------
// UPDATE
// --------------------------------------------------

void updateAnemometer() {

    // If the anemometer hasn't moved recently,
    // clear the old RPM measurement.

    unsigned long lastPulse;

    noInterrupts();
    lastPulse = lastPulseTimeUS;
    interrupts();

    if (
        lastPulse != 0 &&
        micros() - lastPulse > STOP_TIMEOUT_US
    ) {
        noInterrupts();
        rotationIntervalUS = 0;
        interrupts();
    }
}


// --------------------------------------------------
// TOTAL REVOLUTIONS
// --------------------------------------------------

unsigned long getRevolutionCount() {

    noInterrupts();

    unsigned long count = revolutionCount;

    interrupts();

    return count;
}


// --------------------------------------------------
// RPM
// --------------------------------------------------

float getRPM() {

    unsigned long interval;

    noInterrupts();

    interval = rotationIntervalUS;

    interrupts();

    if (interval == 0) {
        return 0.0f;
    }

    // 60,000,000 microseconds per minute
    return 60000000.0f / interval;
}


// --------------------------------------------------
// WIND SPEED
// --------------------------------------------------

float getWindSpeedMPS() {

    float rpm = getRPM();

    // rotations per second
    float rotationsPerSecond = rpm / 60.0f;

    // circumference travelled by center of cup
    float circumference =
        2.0f * PI * CUP_RADIUS_M;

    float cupSpeed =
        rotationsPerSecond * circumference;

    return cupSpeed * CALIBRATION_FACTOR;
}


// --------------------------------------------------
// MPH
// --------------------------------------------------

float getWindSpeedMPH() {

    return getWindSpeedMPS() * 2.23694f;
}