#include <Arduino.h>
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "anemometer.h"

constexpr int REED_PIN = 33;

constexpr unsigned long DEBOUNCE_US = 40000;

constexpr unsigned long STOP_TIMEOUT_US = 3000000;

// --------------------------------------------------
// PHYSICAL ANEMOMETER VALUES
constexpr float CUP_RADIUS_M = 0.0889f;

constexpr float CALIBRATION_FACTOR = 1.0f;

// INTERRUPT DATA

volatile unsigned long revolutionCount = 0;

volatile unsigned long lastPulseTimeUS = 0;
volatile unsigned long rotationIntervalUS = 0;


// REED SWITCH INTERRUPT

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

// INITIALIZATION

void initAnemometer() {

    pinMode(REED_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(REED_PIN),
        reedISR,
        FALLING
    );

    Serial.println("Anemometer initialized");
}

// LIGHT SLEEP WAKEUP

void enableAnemometerLightSleepWakeup() {

    // Light sleep only supports level-triggered GPIO wakeup
    // (edge detection needs a clock that's paused during sleep).
    // The reed switch pulls this pin low when closed.
    gpio_wakeup_enable(
        (gpio_num_t)REED_PIN,
        GPIO_INTR_LOW_LEVEL
    );

    esp_sleep_enable_gpio_wakeup();
}


// UPDATE
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

// TOTAL REVOLUTIONS

unsigned long getRevolutionCount() {

    noInterrupts();

    unsigned long count = revolutionCount;

    interrupts();

    return count;
}

// RPM

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

// WIND SPEED

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


// MPH
float getWindSpeedMPH() {

    return getWindSpeedMPS() * 2.23694f;
}