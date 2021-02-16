#pragma once

#include <HardwareSerial.h>
#include "SdsDustSensor.h"

#include "pins.h"

HardwareSerial sdsSerial(1);
SdsDustSensor sds(sdsSerial);

struct DustSensorData
{
    float pm2_5;
    float pm10;
};

void initSDS011()
{
    sdsSerial.begin(9600, SERIAL_8N1, SDS011_RX, SDS011_TX);

    // Serial.println(sds.queryFirmwareVersion().toString());
    // Serial.println(sds.setQueryReportingMode().toString());

    auto res1 = sds.queryWorkingState();

    if ((int)res1.state != 0)
        sds.sleep();
}

DustSensorData updateDustSensorData()
{
    DustSensorData dustSensorData;
    auto res = sds.queryPm();
    if (res.isOk())
    {
        dustSensorData.pm2_5 = res.pm25;
        dustSensorData.pm10 = res.pm10;
    }
    else
    {
        dustSensorData.pm2_5 = -1;
        dustSensorData.pm10 = -1;
    }

    return dustSensorData;
}
