#pragma once

#include "BME280Thermometer.h"
#include "DustSensor.h"
#include "SdCard.h"
#include "datastore.h"

String currentLogfilePath;

int loggingTag = 0;
int tagCounter = 0;

void addLoggingTag()
{
    loggingTag = ++tagCounter;
}

String createLogFile(tm &timeinfo)
{
    char logfilePath[50];
    SD.mkdir((char *)"/logs/");
    strftime(logfilePath, sizeof(logfilePath), "/logs/%y-%m-%d/", &timeinfo);
    SD.mkdir(logfilePath);
    strftime(logfilePath, sizeof(logfilePath), "/logs/%y-%m-%d/%H-%M-%S.txt", &timeinfo);

    auto logFile = SD.open(logfilePath, FILE_WRITE);

    logFile.close();

    tagCounter = 0;

    return logfilePath;
}

void logSensorData(tm *timeinfo, ThermometerData *thermometerData)
{
    auto logFile = SD.open(currentLogfilePath.c_str(), FILE_WRITE);

    logFile.print(timeinfo, "%H:%M:%S, ");
    logFile.printf("%d, %.1f, %.1f", loggingTag, thermometerData->temperature, thermometerData->humidity);

    for (int i = 0; i < MAX_NUM_SENSORS; i++)
        if (dataValid(i))
            logFile.printf(", %d, %.1f, %.1f", i, sensorNetworkData[i].pm2_5, sensorNetworkData[i].pm10);

    logFile.println();

    logFile.close();

    loggingTag = 0;
}
