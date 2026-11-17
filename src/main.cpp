#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "weather_sensors.h"
#include "anemometer.h"
#include "rtc_clock.h"
#include "sd_logger.h"
#include "wind_vane.h"
#include "weather_api.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

unsigned long lastUploadTime = 0;

const unsigned long uploadInterval =
    10UL * 60UL * 1000UL;   // 10 minutes

// -------------------------
// ADAPTIVE SLEEP CYCLE
// -------------------------
// Instead of a fixed 1-minute read/log cycle, each cycle's length
// depends on how much the temperature moved since the last cycle:
// fast-changing temperature -> short cycle, stable temperature -> long cycle.

const unsigned long MIN_CYCLE_INTERVAL_MS = 15UL * 1000UL;
const unsigned long MAX_CYCLE_INTERVAL_MS = 5UL * 60UL * 1000UL;

const float TEMP_DELTA_FAST_C = 0.3f;   // delta at/above this -> MIN_CYCLE_INTERVAL_MS
const float TEMP_DELTA_SLOW_C = 0.05f;  // delta at/below this -> MAX_CYCLE_INTERVAL_MS

float lastCycleTemp = NAN;

unsigned long nextCycleIntervalMs(float currentTemp) {

    // No baseline yet, or sensor unavailable: play it safe with a short cycle.
    if (isnan(currentTemp) || isnan(lastCycleTemp)) {
        return MIN_CYCLE_INTERVAL_MS;
    }

    float delta = fabs(currentTemp - lastCycleTemp);

    if (delta >= TEMP_DELTA_FAST_C) {
        return MIN_CYCLE_INTERVAL_MS;
    }

    if (delta <= TEMP_DELTA_SLOW_C) {
        return MAX_CYCLE_INTERVAL_MS;
    }

    float ratio =
        (delta - TEMP_DELTA_SLOW_C) /
        (TEMP_DELTA_FAST_C - TEMP_DELTA_SLOW_C);

    return MAX_CYCLE_INTERVAL_MS -
        (unsigned long)(ratio * (MAX_CYCLE_INTERVAL_MS - MIN_CYCLE_INTERVAL_MS));
}

// Light-sleeps for durationMs, waking briefly on every anemometer pulse

void lightSleepFor(unsigned long durationMs) {

    unsigned long elapsed = 0;

    while (elapsed < durationMs) {

        unsigned long remaining = durationMs - elapsed;
        unsigned long chunk = remaining < 1000 ? remaining : 1000;

        esp_sleep_enable_timer_wakeup((uint64_t)chunk * 1000ULL);

        unsigned long before = millis();

        esp_light_sleep_start();

        elapsed += millis() - before;

        updateAnemometer();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Shared I2C bus
    Wire.begin(19, 18);

    // RTC
    if (!initRTC()) {
        Serial.println("RTC initialization failed");
    }

    // Sensors
    if (!initWeatherSensors()) {
        Serial.println("Weather sensor initialization failed");
    }

    initAnemometer();
    enableAnemometerLightSleepWakeup();

    if (!initWindVane()) {
    Serial.println("Wind vane initialization failed");
}

    // OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED not found");
        while (1);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // SD card
    if (initSD()) {
        createWeatherFile();
    } else {
        Serial.println("SD initialization failed");
    }

    Serial.println("Weather station initialized!");
}

void loop() {

    unsigned long cycleStart = millis();

    // -------------------------
    // READ SENSORS
    // -------------------------

    WeatherData data = readWeatherSensors();

    DateTime now = getCurrentTime();

    uint32_t revolutions = getRevolutionCount();

    float rpm = getRPM();

    float windSpeedMPS = getWindSpeedMPS();
    float windSpeedMPH = getWindSpeedMPH();

    float windDirection =
        getWindDirectionDegrees();

    // -------------------------
    // SERIAL OUTPUT
    // -------------------------

    Serial.print(now.year());
    Serial.print("-");
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());

    Serial.print(" ");

    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());

    Serial.print("Temp: ");
    Serial.print(data.temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(data.humidity);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(data.pressure);
    Serial.println(" hPa");

    Serial.print("UV raw: ");
    Serial.println(data.uv);

    Serial.print("Revolutions: ");
    Serial.println(revolutions);

    Serial.print("RPM: ");
    Serial.println(rpm, 1);

    Serial.print("Wind speed: ");
    Serial.print(windSpeedMPS, 2);
    Serial.println(" m/s");

    Serial.print("Wind speed: ");
    Serial.print(windSpeedMPH, 2);
    Serial.println(" mph");

    Serial.print("Wind direction: ");
    Serial.print(windDirection, 1);
    Serial.print(" deg ");

    Serial.println(getWindDirectionCardinal());

    Serial.println("----------------");

    // -------------------------
    // OLED
    // -------------------------

    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.print("Weather ");

    if (now.hour() < 10) display.print("0");
    display.print(now.hour());

    display.print(":");

    if (now.minute() < 10) display.print("0");
    display.print(now.minute());

    display.setCursor(0, 15);
    display.print("Temp: ");
    display.print(data.temperature, 1);
    display.println(" C");

    display.setCursor(0, 27);
    display.print("Hum:  ");
    display.print(data.humidity, 1);
    display.println(" %");

    display.setCursor(0, 39);
    display.print("Pres: ");
    display.print(data.pressure, 0);
    display.println(" hPa");

    display.setCursor(0, 51);
    display.print("UV:   ");
    display.println(data.uv);

    display.display();

    // SD LOGGER

    logWeatherData(
        now,
        data.temperature,
        data.humidity,
        data.pressure,
        data.uv
    );

    // CLOUD / D1 LOGGING

    if (
        lastUploadTime == 0 ||
        millis() - lastUploadTime >= uploadInterval
    ) {

        lastUploadTime = millis();

        Serial.println(
            "Uploading weather data..."
        );

        bool uploaded =
            uploadWeatherToWorker(
                data.temperature,
                data.humidity,
                data.pressure,
                data.uv,
                windSpeedMPS,
                windDirection,
                rpm
            );


        if (uploaded) {

            Serial.println(
                "Weather data uploaded successfully"
            );

        }
        else {

            Serial.println(
                "Weather upload failed"
            );
        }

        // WiFi only needs to be up for the upload itself;
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
    }

    // ADAPTIVE LIGHT SLEEP

    unsigned long cycleInterval = nextCycleIntervalMs(data.temperature);
    lastCycleTemp = data.temperature;

    unsigned long cycleElapsed = millis() - cycleStart;

    unsigned long sleepDuration =
        cycleElapsed < cycleInterval
            ? cycleInterval - cycleElapsed
            : 0;

    Serial.print("Light-sleeping for ");
    Serial.print(sleepDuration);
    Serial.println(" ms");

    lightSleepFor(sleepDuration);
}