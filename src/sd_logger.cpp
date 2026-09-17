#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include "sd_logger.h"

constexpr int SD_MISO = 14;
constexpr int SD_MOSI = 27;
constexpr int SD_SCK  = 26;
constexpr int SD_CS   = 25;

SPIClass sdSPI(HSPI);

bool initSD() {
    Serial.println("Initializing SD card...");

    sdSPI.begin(
        SD_SCK,
        SD_MISO,
        SD_MOSI,
        SD_CS
    );

    if (!SD.begin(SD_CS, sdSPI, 4000000)) {
        Serial.println("SD initialization FAILED");
        return false;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card detected");
        return false;
    }

    uint64_t cardSizeMB = SD.cardSize() / (1024ULL * 1024ULL);

    Serial.println("SD initialized successfully");

    Serial.print("SD card size: ");
    Serial.print(cardSizeMB);
    Serial.println(" MB");

    return true;
}

bool createWeatherFile() {

    if (SD.exists("/weather.csv")) {
        Serial.println("weather.csv already exists");
        return true;
    }

    File file = SD.open(
        "/weather.csv",
        FILE_WRITE
    );

    if (!file) {
        Serial.println("Could not create weather.csv");
        return false;
    }

    file.println(
        "date,time,temperature_c,humidity_percent,pressure_hpa,uv_raw"
    );

    file.close();

    Serial.println("Created weather.csv");

    return true;
}


bool logWeatherData(
    const DateTime& timestamp,
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv
) {
    File file = SD.open(
        "/weather.csv",
        FILE_APPEND
    );

    if (!file) {
        Serial.println("Could not open weather.csv");
        return false;
    }

    // Date: YYYY-MM-DD
    file.print(timestamp.year());
    file.print("-");

    if (timestamp.month() < 10) {
        file.print("0");
    }

    file.print(timestamp.month());
    file.print("-");

    if (timestamp.day() < 10) {
        file.print("0");
    }

    file.print(timestamp.day());
    file.print(",");

    // Time: HH:MM:SS
    if (timestamp.hour() < 10) {
        file.print("0");
    }

    file.print(timestamp.hour());
    file.print(":");

    if (timestamp.minute() < 10) {
        file.print("0");
    }

    file.print(timestamp.minute());
    file.print(":");

    if (timestamp.second() < 10) {
        file.print("0");
    }

    file.print(timestamp.second());
    file.print(",");

    // Weather data
    file.print(temperature, 2);
    file.print(",");

    file.print(humidity, 2);
    file.print(",");

    file.print(pressure, 2);
    file.print(",");

    file.println(uv);

    file.close();

    return true;
}