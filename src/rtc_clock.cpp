#include "rtc_clock.h"

#include <Arduino.h>

RTC_DS3231 rtc;

bool rtcReady = false;


bool initRTC() {

    rtcReady = rtc.begin();

    if (!rtcReady) {
        Serial.println("DS3231 not found");
        return false;
    }

    Serial.println("DS3231 found");


    if (rtc.lostPower()) {

        Serial.println("RTC lost power, setting compile time");

        rtc.adjust(
            DateTime(
                F(__DATE__),
                F(__TIME__)
            )
        );
    }

    return true;
}


DateTime getCurrentTime() {

    if (rtcReady) {
        return rtc.now();
    }

    // Safe placeholder if RTC is disconnected
    return DateTime(
        2000,
        1,
        1,
        0,
        0,
        0
    );
}