#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#define SD_MISO 14
#define SD_MOSI 27
#define SD_SCK  26
#define SD_CS   25

SPIClass sdSPI(HSPI);

bool initSD() {

    Serial.println("Initializing SD card...");

    // Start our custom SPI bus
    sdSPI.begin(
        SD_SCK,
        SD_MISO,
        SD_MOSI,
        SD_CS
    );

    // Try mounting the SD card
    if (!SD.begin(SD_CS, sdSPI, 4000000)) {
        Serial.println("SD initialization FAILED");
        return false;
    }

    Serial.println("SD initialized successfully");

    // Check whether an actual card exists
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card detected");
        return false;
    }

    // Print size
    uint64_t cardSizeMB =
        SD.cardSize() / (1024 * 1024);

    Serial.print("SD card size: ");
    Serial.print(cardSizeMB);
    Serial.println(" MB");

    return true;
}


bool writeTestFile() {

    File file = SD.open(
        "/test.txt",
        FILE_WRITE
    );

    if (!file) {
        Serial.println("Could not open test.txt");
        return false;
    }

    file.println("Weather station SD test");
    file.println("SD logger module works!");

    file.close();

    Serial.println("Test file written");

    return true;
}


bool createWeatherFile() {

    // If the file already exists,
    // don't write the header again.
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
        "temperature_C,humidity_percent,pressure_hPa,uv_raw"
    );

    file.close();

    Serial.println("Created weather.csv");

    return true;
}


bool logWeatherData(
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