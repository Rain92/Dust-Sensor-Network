#pragma once

#include "pins.h"
#include "umbcommunicator.h"

WSDevice ws200;

struct WindSensorData
{
    float windspeed;
    int winddirection;
    int status;
};

void initWS200()
{
    ws200.init(WS200_RX, WS200_TX);
}

WindSensorData updateWindSensorData()
{
    WindSensorData windSensorData;
    uint16_t channels[] = {CHANNEL_WIND_SPEED_FAST_MS, CHANNEL_WIND_DIRECTION_FAST_DEG};
    auto res = ws200.requestOnlineDataMulti(channels, sizeof(channels) / 2);

    windSensorData.windspeed = res.values[0];
    windSensorData.winddirection = (int)res.values[1];
    windSensorData.status = res.status;

    return windSensorData;
}