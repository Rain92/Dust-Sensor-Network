#pragma once

#include <ArduinoOTA.h>
#include "display.h"

void setupOTA()
{
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "\nfilesystem";

            display.clearDisplay();
            display.setCursor(0, 0);
            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            display.println("Start updating " + type);
            display.display();
        })
        .onEnd([]() {
            display.println("\nCompleded!");
            display.display();
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.printf("Updating Firmware\nProgress: %u%%\n", (progress / (total / 100)));
            display.display();
        })
        .onError([](ota_error_t error) {
            display.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                display.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                display.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                display.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                display.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                display.println("End Failed");

            display.display();
        });

    ArduinoOTA.begin();
}
