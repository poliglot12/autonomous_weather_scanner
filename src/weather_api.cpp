#include "weather_api.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"

const char* WIFI_SSID =
    WIFI_SSID_SECRET;

const char* WIFI_PASSWORD =
    WIFI_PASSWORD_SECRET;

const char* WORKER_URL =
    CF_WORKER_URL;

const char* INGEST_API_KEY =
    WEATHER_API_KEY;


bool connectWiFi() {

    // Already connected
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

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


bool uploadWeatherToWorker(
    float temperature,
    float humidity,
    float pressure,
    uint32_t uv,
    float windSpeed,
    float windDirection,
    float rpm
) {

    if (!connectWiFi()) {
        return false;
    }


    WiFiClientSecure client;

    // TEMPORARY:
    client.setInsecure();


    HTTPClient http;

    if (!http.begin(client, WORKER_URL)) {

        Serial.println("Failed to initialize HTTP connection");

        return false;
    }


    http.addHeader(
        "Content-Type",
        "application/json"
    );

    http.addHeader(
        "Authorization",
        String("Bearer ") + INGEST_API_KEY
    );


    String json = "{";

    json += "\"device_id\":\"weather-station-01\",";
    
    json += "\"temperature_c\":";
    json += String(temperature, 2);
    json += ",";

    json += "\"humidity_pct\":";
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


    Serial.println("Uploading weather data:");
    Serial.println(json);


    int responseCode =
        http.POST(json);


    Serial.print("Worker response: ");
    Serial.println(responseCode);


    if (responseCode > 0) {

        String response =
            http.getString();

        Serial.println(response);
    }
    else {

        Serial.print("HTTP error: ");
        Serial.println(
            http.errorToString(responseCode)
        );
    }


    http.end();


    return (
        responseCode >= 200 &&
        responseCode < 300
    );
}