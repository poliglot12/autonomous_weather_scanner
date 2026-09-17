#include "db_logger.h"

#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* NEON_URL =
    "YOUR_NEON_DATA_API_URL/weather_readings";

// We'll fill this in once we configure Data API authentication.
const char* NEON_TOKEN =
    "YOUR_TOKEN";


bool connectWiFi() {

    WiFi.mode(WIFI_STA);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    Serial.print("Connecting to WiFi");

    unsigned long startTime = millis();

    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 15000
    ) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {

        Serial.println("WiFi connection failed");

        return false;
    }

    Serial.println("WiFi connected");

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    return true;
}


bool uploadWeatherToNeon(
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv,
    float windSpeed,
    float windDirection,
    float rpm
) {

    if (WiFi.status() != WL_CONNECTED) {

        if (!connectWiFi()) {
            return false;
        }
    }


    HTTPClient http;

    http.begin(NEON_URL);

    http.addHeader(
        "Content-Type",
        "application/json"
    );

    http.addHeader(
        "Authorization",
        String("Bearer ") + NEON_TOKEN
    );


    String json = "{";

    json += "\"temperature_c\":";
    json += String(temperature, 2);
    json += ",";

    json += "\"humidity_percent\":";
    json += String(humidity, 2);
    json += ",";

    json += "\"pressure_hpa\":";
    json += String(pressure, 2);
    json += ",";

    json += "\"uv_raw\":";
    json += String(uv);
    json += ",";

    json += "\"wind_speed_mps\":";
    json += String(windSpeed, 2);
    json += ",";

    json += "\"wind_direction_deg\":";
    json += String(windDirection, 1);
    json += ",";

    json += "\"rpm\":";
    json += String(rpm, 1);

    json += "}";


    int responseCode =
        http.POST(json);


    Serial.print("Neon response: ");
    Serial.println(responseCode);


    if (responseCode > 0) {

        String response =
            http.getString();

        Serial.println(response);
    }


    http.end();


    return (
        responseCode >= 200 &&
        responseCode < 300
    );
}