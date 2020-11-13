#pragma once

#include "Arduino.h"
#include "pins.h"

#define BAT_VOLTAGE_DIVIDER 2  // voltage divider 100k/100k on board

#define BAT_SCALE (1.0703 * (3.3 / 4096) * BAT_VOLTAGE_DIVIDER)

#define BAT_NUM_SAMPLES 64

void initBattery()
{
    pinMode(PIN_BATTERY_VOLTAGE, INPUT);
}

float readBatteryVoltage()
{
    int analogValue = 0;
    for (int i = 0; i < BAT_NUM_SAMPLES; i++)
        analogValue += analogRead(PIN_BATTERY_VOLTAGE);
    analogValue /= BAT_NUM_SAMPLES;

    float voltage = analogValue * BAT_SCALE;
    return voltage;
}