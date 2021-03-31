#pragma once

#include "Arduino.h"
#include "pins.h"

#define BAT_VOLTAGE_DIVIDER 2  // voltage divider 100k/100k on board

void initBattery()
{
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);
}

float readBatteryVoltage()
{
    const int numSamples = 64;
    int analogValue = 0;
    for (int i = 0; i < numSamples; i++)
        analogValue += analogRead(PIN_BATTERY_VOLTAGE);
    analogValue /= numSamples;

    float voltage = analogValue * (3.3 / 4096) * BAT_VOLTAGE_DIVIDER;
    return voltage;
}