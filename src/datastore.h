#pragma once

#include "dustsensor.h"
#include "windsensor.h"

#define MAX_NUM_SENSORS 16
#define DATA_LIFETIME 1000  // 1 sec

DustSensorData sensorNetworkData[MAX_NUM_SENSORS];
unsigned long dataTimestamps[MAX_NUM_SENSORS];

WindSensorData remoteWindSensorData;
unsigned long remoteWindSensorTimestamp;

void initRemoteSensorData()
{
    for (uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
    {
        dataTimestamps[i] = 0;
    }
    remoteWindSensorTimestamp = 0;
}

void registerDustSensorData(uint8_t sensor, const DustSensorData &data)
{
    if (sensor >= MAX_NUM_SENSORS)
        return;

    dataTimestamps[sensor] = millis();

    sensorNetworkData[sensor] = data;
}

void registerWindSensorData(uint8_t sensor, const WindSensorData &data)
{
    remoteWindSensorTimestamp = millis();

    remoteWindSensorData = data;
}

bool dataValid(ulong timestamp)
{
    return millis() - timestamp < DATA_LIFETIME;
}

int countValidConnections()
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < MAX_NUM_SENSORS; i++)
        if (dataValid(dataTimestamps[i]))
            c++;

    if (dataValid(remoteWindSensorTimestamp))
        c++;

    return c;
}