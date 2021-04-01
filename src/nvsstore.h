#pragma once

#include <ArduinoNvs.h>

#include "nodeconfig.h"

#define MAGICKEY 80633562ull

#define MAGICKEY_FLASHKEY "magickey"
#define SETTINGS_FLASHKEY "settings"

struct Settings
{
    int nodeId = NODE_ID;
    NodeType nodeType = NODE_TYPE;
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

        if (OVERRIDE_NODE_SETTINGS > 0)
        {
            settings.nodeId = NODE_ID;
            settings.nodeType = NODE_TYPE;
            saveSettings();
        }
    }
}