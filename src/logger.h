#pragma once

#include "bme280thermometer.h"
#include "datastore.h"
#include "dustsensor.h"
#include "sdcard.h"

String currentLogfilePath;

// tag counter for the active experiment
int tagCounter = 0;
// tag to be written to sdcard if not zero
int loggingTag = 0;

// active Sensor counter
int sensorCounter = 0;

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
    sensorCounter = 0;

    return logfilePath;
}

String makeMeasurementString(tm *timeinfo, ThermometerData *thermometerData)
{
    char buf[200];

    int idx = 0;
    idx += strftime(buf + idx, 200, "%H:%M:%S, ", timeinfo);
    idx += sprintf(buf + idx, "%d, %.1f, %.1f", loggingTag, thermometerData->temperature, thermometerData->humidity);

    for (int i = 0; i < MAX_NUM_SENSORS; i++)
    {
        if (dataValid(dataTimestamps[i]))
        {
            idx += sprintf(buf + idx, ", %d, %.1f, %.1f", i, sensorNetworkData[i].pm2_5, sensorNetworkData[i].pm10);
            sensorCounter = max(sensorCounter, i);
        }
        else if (i <= sensorCounter)
            idx += sprintf(buf + idx, ", %d, %d, %d", i, -1, -1);
    }

    if (dataValid(remoteWindSensorTimestamp))
        idx += sprintf(buf + idx, ", W, %.2f, %d", remoteWindSensorData.windspeed, remoteWindSensorData.winddirection);

    return String(buf);
}

void logString(const String &str)
{
    auto logFile = SD.open(currentLogfilePath.c_str(), FILE_WRITE);

    logFile.println(str);

    logFile.close();

    loggingTag = 0;
}
