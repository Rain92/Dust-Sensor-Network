#pragma once

#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>
#include "time.h"

#include "wificredentials.h"

int32_t getWiFiChannel(const char *ssid)
{
    if (int32_t n = WiFi.scanNetworks())
    {
        for (uint8_t i = 0; i < n; i++)
        {
            if (!strcmp(ssid, WiFi.SSID(i).c_str()))
            {
                return WiFi.channel(i);
            }
        }
    }
    return 0;
}

void fixIP(int n)
{
    IPAddress staticIP(192, 168, 159, 150);
    IPAddress gateway(192, 168, 159, 3);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 159, 3);
    if (WiFi.localIP()[3] == 3)
    {
        if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false)
            Serial.println("IP Configuration failed.");

        WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);
    }
}

bool connectToWifi(int timeoutSeconds = 5)
{

    if (WiFi.status() == WL_CONNECTED)
        return true;

    Serial.println("Starting WiFi..");

    WiFi.mode(WIFI_AP_STA);

    int nbVisibleNetworks = WiFi.scanNetworks();

    boolean wifiFound = false;
    int n;
    for (n = 0; n < KNOWN_SSID_COUNT; n++)
    {
        for (int i = 0; i < nbVisibleNetworks; ++i)
        {
            if (!strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str()))
            {
                wifiFound = true;
                break;
            }
        }
        if (wifiFound)
            break;
    }

    if (wifiFound)
        WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);

    int loopcounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        if (++loopcounter > timeoutSeconds)
            return false;
        delay(1000);
        Serial.print("Establishing connection to WiFi: ");
        Serial.println(KNOWN_SSID[n]);
    }

    // fixIP(n);

    Serial.println("Connected to network");
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    return true;
}

// void initComWifi()
// {
//     WiFi.mode(WIFI_STA);

//     int32_t channel = getWiFiChannel(WIFI_SSID);

//     esp_wifi_set_promiscuous(true);
//     esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
//     esp_wifi_set_promiscuous(false);
// }

void endWifi()
{
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
}

void syncTime()
{
    const char *ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}