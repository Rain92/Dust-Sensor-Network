#pragma once

#include <esp_now.h>

#include "datastore.h"
#include "dustsensor.h"

#define DATA_TYPE_DUSTSENSOR 0
#define DATA_TYPE_WINDSENSOR 1

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

uint8_t masterAddress[] = {0xD8, 0xA0, 0x1D, 0x60, 0xD1, 0x3C};  // default adress

esp_now_peer_info_t peerInfo;

enum RemoteCommand
{
    StartLogging,
    StopLogging
};

struct DustSensorDataPacket
{
    uint8_t dataType;
    uint8_t sensor;
    DustSensorData data;
};

struct WindSensorDataPacket
{
    uint8_t dataType;
    uint8_t sensor;
    WindSensorData data;
};

void (*commandCallback)(RemoteCommand) = nullptr;

void sendDataToMaster(const uint8_t *data, size_t len)
{
    esp_err_t result = esp_now_send(masterAddress, data, len);

    if (result != ESP_OK)
        Serial.println("Error sending the data");
}

void sendDustSensorData(uint8_t sensor, const DustSensorData &data)
{
    DustSensorDataPacket p;
    p.dataType = DATA_TYPE_DUSTSENSOR;
    p.sensor = sensor;
    p.data = data;

    sendDataToMaster((uint8_t *)&p, sizeof(p));
}

void sendWindSensorData(uint8_t sensor, const WindSensorData &data)
{
    WindSensorDataPacket p;
    p.dataType = DATA_TYPE_WINDSENSOR;
    p.sensor = sensor;
    p.data = data;

    sendDataToMaster((uint8_t *)&p, sizeof(p));
}

void sendCommand(RemoteCommand command)
{
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&command, sizeof(command));

    if (result != ESP_OK)
        Serial.println("Error sending the data");
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status != ESP_NOW_SEND_SUCCESS)
        Serial.println("EspNow data delivery failed!");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Serial.print("Bytes received: ");
    // Serial.println(len);
    // Serial.print("Received from MAC: ");
    // for (int i = 0; i < 6; i++)
    // {
    //     Serial.printf("%02X", mac[i]);
    //     if (i < 5)
    //         Serial.print(":");
    // }
    // Serial.println();
    if (incomingData[0] == DATA_TYPE_DUSTSENSOR)
    {
        if (len != sizeof(DustSensorDataPacket))
        {
            Serial.println("Invalid data lenght.");
            return;
        }

        DustSensorDataPacket *recieved = (DustSensorDataPacket *)incomingData;

        // Serial.printf("pm2.5: %.1f pm10: %.1f\n", recieved->data.pm2_5, recieved->data.pm10);

        registerDustSensorData(recieved->sensor, recieved->data);
    }
    if (incomingData[0] == DATA_TYPE_WINDSENSOR)
    {
        if (len != sizeof(WindSensorDataPacket))
        {
            Serial.println("Invalid data lenght.");
            return;
        }

        WindSensorDataPacket *recieved = (WindSensorDataPacket *)incomingData;

        registerWindSensorData(recieved->sensor, recieved->data);

        // Serial.printf("Windspeed: %.2f Winddirection: %d\n", recieved->data.windspeed, recieved->data.winddirection);
    }
}

void updateMaster(const uint8_t *mac)
{
    bool changed = false;

    for (int i = 0; i < 6; i++)
    {
        if (masterAddress[i] != mac[i])
        {
            changed = true;
            break;
        }
    }

    if (changed)
    {
        esp_now_del_peer(masterAddress);

        memcpy(masterAddress, mac, 6);
        memcpy(peerInfo.peer_addr, masterAddress, 6);
        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer");
            return;
        }
    }
}

void OnCommandRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Serial.print("Bytes received: ");
    // Serial.println(len);
    // Serial.print("Received from MAC: ");
    // for (int i = 0; i < 6; i++)
    // {
    //     Serial.printf("%02X", mac[i]);
    //     if (i < 5)
    //         Serial.print(":");
    // }
    // Serial.println();
    if (len != sizeof(RemoteCommand))
    {
        Serial.println("Invalid command lenght.");
        return;
    }

    RemoteCommand *command = (RemoteCommand *)incomingData;

    // Serial.printf("command: %d\n", static_cast<int>(*command));

    updateMaster(mac);

    if (commandCallback)
        commandCallback(*command);
}

void initEspNow(bool master)
{
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    if (esp_now_register_recv_cb(master ? OnDataRecv : OnCommandRecv) != ESP_OK)
    {
        Serial.println("Error initializing reciever callback");
        return;
    }

    if (esp_now_register_send_cb(OnDataSent) != ESP_OK)
    {
        Serial.println("Error initializing sender callback");
        return;
    }

    if (master)
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    else
        memcpy(peerInfo.peer_addr, masterAddress, 6);
    // peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        return;
    }
}