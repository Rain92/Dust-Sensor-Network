#pragma once

#include <Arduino.h>
#include "pins.h"

#define TOUCH_THRESHOLD 10
#define TOUCH_NUM_SAMPLES 6

uint16_t latestTouchValue = 0;

bool processTouchPin()
{
    static bool touchProcessed = false;

    static uint16_t touchSamples[TOUCH_NUM_SAMPLES] = {0, 0, 0, 0, 0, 0};

    latestTouchValue = touchRead(PIN_TOUCH_1);

    for (int i = TOUCH_NUM_SAMPLES - 1; i > 0; i--)
        touchSamples[i] = touchSamples[i - 1];

    touchSamples[0] = latestTouchValue;

    bool touch_detected = (touchSamples[0] + TOUCH_THRESHOLD < touchSamples[TOUCH_NUM_SAMPLES - 2]) &&
                          (touchSamples[1] + TOUCH_THRESHOLD < touchSamples[TOUCH_NUM_SAMPLES - 1]);

    //debug
    // if (latestTouchValue < 25)
    // {
    //     for (size_t i = 0; i < TOUCH_NUM_SAMPLES; i++)
    //     {
    //         Serial.print(touchSamples[i]);
    //         Serial.print(' ');
    //     }
    //     Serial.println(touch_detected);
    // }

    if (touch_detected && !touchProcessed)
    {
        touchProcessed = true;
        return true;
    }
    else if (!touch_detected)
    {
        touchProcessed = false;
    }

    return false;
}