#pragma once

#include <mySD.h>

#include "pins.h"

enum FileType
{
    NotFound,
    IsFile,
    IsDirectory
};

FileType checkPath(const String &path)
{
    FileType res;
    auto file = SD.open(path.c_str(), FILE_READ);
    if (!file)
        res = NotFound;
    else if (!file.isDirectory())
        res = IsFile;
    else
        res = IsDirectory;

    file.close();
    return res;
}

bool initSdCard()
{
    if (!SD.begin(SDCARD_CS, SDCARD_MOSI, SDCARD_MISO, SDCARD_SCLK))
    {
        Serial.println("Card Mount Failed");
        return false;
    }
    else
    {
        Serial.println("Card Mount Succeeded!");

        return true;
    }
}
