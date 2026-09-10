#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LTR390.h>
#include "sd_logger.h"

Adafruit_BME280 bme;
Adafruit_LTR390 ltr;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(19, 18);

    bool sdReady = initSD();

    if (sdReady) {
     createWeatherFile();
    }

    // BME280
    if (!bme.begin(0x76)) {
        Serial.println("BME280 not found");
        while (1);
    }

    // OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED not found");
        while (1);
    }

    // LTR390
    if (!ltr.begin()) {
        Serial.println("LTR390 not found");
        while (1);
    }

    // Configure UV sensor
    ltr.setMode(LTR390_MODE_UVS);
    ltr.setGain(LTR390_GAIN_3);
    ltr.setResolution(LTR390_RESOLUTION_18BIT);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    Serial.println("All sensors initialized!");
}

uint32_t uv = 0;
const unsigned long logInterval = 60000; // Log every 60 second
unsigned long lastLogTime = -logInterval;


void loop() {

    float temp = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

    if (ltr.newDataAvailable()) {
        uv = ltr.readUVS();
    }

    // Serial output
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("UV raw: ");
    Serial.println(uv);

    Serial.println("----------------");

    // OLED
    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("Weather Test");

    display.setCursor(0, 15);
    display.print("Temp: ");
    display.print(temp, 1);
    display.println(" C");

    display.setCursor(0, 27);
    display.print("Hum:  ");
    display.print(humidity, 1);
    display.println(" %");

    display.setCursor(0, 39);
    display.print("Pres: ");
    display.print(pressure, 0);
    display.println(" hPa");

    display.setCursor(0, 51);
    display.print("UV:   ");
    display.println(uv);

    display.display();

    if (millis() - lastLogTime >= logInterval) {
        lastLogTime = millis();

        logWeatherData(
          temp,
          humidity,
          pressure,
          uv
        );
    }

    delay(250);

}