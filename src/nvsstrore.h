#pragma once

#include <ArduinoNvs.h>

#define MAGICKEY 83634762ull

#define MAGICKEY_FLASHKEY "magickey"
#define SETTINGS_FLASHKEY "settings"

#define SENSOR_ID_DEFAULT 5

#define SENSOR_ID_OVERRIDE 1

struct Settings
{
    int sensorId = SENSOR_ID_DEFAULT;
};

Settings settings;

void saveSettings()
{
    Serial.println("Saving settings.");

    bool res1 = NVS.setInt(MAGICKEY_FLASHKEY, MAGICKEY);
    bool res2 = NVS.setBlob(SETTINGS_FLASHKEY, (uint8_t *)&settings, sizeof(Settings));

    if (!res1 || !res2)
        Serial.println("Couldn't save settings.");
}

void initNvs()
{
    NVS.begin();

    uint64_t magickey = NVS.getInt(MAGICKEY_FLASHKEY);
    size_t blobsize = NVS.getBlobSize(SETTINGS_FLASHKEY);

    if (magickey != MAGICKEY || blobsize != sizeof(Settings))
    {
        NVS.eraseAll();
        saveSettings();
    }
    else
    {
        bool res = NVS.getBlob(SETTINGS_FLASHKEY, (uint8_t *)&settings, sizeof(Settings));
        if (!res)
            Serial.println("Couldn't load settings.");

        if (SENSOR_ID_OVERRIDE > 0)
        {
            settings.sensorId = SENSOR_ID_DEFAULT;
            saveSettings();
        }
    }
}