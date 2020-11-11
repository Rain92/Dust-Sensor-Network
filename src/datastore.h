#pragma once

#include "DustSensor.h"

#define MAX_NUM_SENSORS 16
#define DATA_LIFETIME 1000  // 1 sec

DustSensorData sensorNetworkData[MAX_NUM_SENSORS];
unsigned long dataTimestamps[MAX_NUM_SENSORS];

void initRemoteSensorData()
{
    for (uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
    {
        dataTimestamps[i] = 0;
    }
}

void registerSensorData(uint8_t sensor, const DustSensorData &data)
{
    if (sensor >= MAX_NUM_SENSORS)
        return;

    dataTimestamps[sensor] = millis();

    sensorNetworkData[sensor] = data;
}

bool dataValid(uint8_t sensor)
{
    if (sensor >= MAX_NUM_SENSORS)
        return false;

    return millis() - dataTimestamps[sensor] < DATA_LIFETIME;
}

int countValidConnections()
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
        if (dataValid(i))
            c++;

    return c;
}