#pragma once

#include <esp_now.h>

#include "DustSensor.h"
#include "datastore.h"

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

uint8_t masterAddress[] = {0xD8, 0xA0, 0x1D, 0x60, 0xD1, 0x3C};

esp_now_peer_info_t peerInfo;

enum RemoteCommand
{
    StartLogging,
    StopLogging
};

struct DataPacket
{
    uint8_t sensor;
    DustSensorData data;
};

void (*commandCallback)(RemoteCommand) = nullptr;

void sendSensorData(uint8_t sensor, const DustSensorData &data)
{
    DataPacket p;
    p.sensor = sensor;
    p.data = data;

    esp_err_t result = esp_now_send(masterAddress, (uint8_t *)&p, sizeof(p));

    if (result != ESP_OK)
        Serial.println("Error sending the data");
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
    if (len != sizeof(DataPacket))
    {
        Serial.println("Invalid data lenght.");
        return;
    }

    DataPacket *recieved = (DataPacket *)incomingData;

    Serial.printf("pm2.5: %.1f pm10: %.1f\n", recieved->data.pm2_5, recieved->data.pm10);

    registerSensorData(recieved->sensor, recieved->data);
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