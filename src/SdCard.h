#pragma once

#include <mySD.h>
#include <algorithm>
#include <tuple>
#include <vector>

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

std::tuple<std::vector<String>, std::vector<String>> listFiles(File &dir)
{
    std::vector<String> dirs;
    std::vector<String> files;

    if (dir.isDirectory())
    {
        dir.rewindDirectory();
        auto file = dir.openNextFile();

        while (file)
        {
            if (file.isDirectory())
                dirs.push_back(file.name());
            else
                files.push_back(file.name());

            file = dir.openNextFile();
        }
    }
    std::sort(dirs.begin(), dirs.end());
    std::sort(files.begin(), files.end());

    return std::tuple<std::vector<String>, std::vector<String>>(std::move(dirs), std::move(files));
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
