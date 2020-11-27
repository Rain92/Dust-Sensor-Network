#pragma once

#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>
#include "time.h"

#define WIFI_SSID "Andreas-PC_AP"
#define WIFI_PWD "12345678"

bool wifiConnected = false;

int32_t getWiFiChannel(const char* ssid)
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

void fixIP()
{
    IPAddress staticIP(192, 168, 159, 150);
    IPAddress gateway(192, 168, 159, 3);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 159, 3);
    if (WiFi.localIP()[3] == 3)
    {
        if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false)
            Serial.println("IP Configuration failed.");

        WiFi.begin(WIFI_SSID, WIFI_PWD);
    }
}


bool connectToWifi(int timeoutSeconds = 5)
{

    if (WiFi.status() == WL_CONNECTED)
        return true;

    Serial.println("Starting WiFi..");

    WiFi.mode(WIFI_AP_STA);

    WiFi.begin(WIFI_SSID, WIFI_PWD);

    int loopcounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        if (++loopcounter > timeoutSeconds)
            return false;
        delay(1000);
        Serial.println("Establishing connection to WiFi..");
    }

    // fixIP();


    wifiConnected = true;

    Serial.println("Connected to network");
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    return true;
}

void initComWifi()
{
    WiFi.mode(WIFI_STA);

    int32_t channel = getWiFiChannel(WIFI_SSID);

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
}

void endWifi()
{
    if (wifiConnected)
    {
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        wifiConnected = false;
    }
}

void syncTime()
{
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 3600;
    const int daylightOffset_sec = 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}