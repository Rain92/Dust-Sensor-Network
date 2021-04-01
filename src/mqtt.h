#pragma once

#include <AsyncMqttClient.h>

#define MQTT_HOST "broker.mqttdashboard.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "dust-sensor-network"

#define MQTT_TOPIC_MAIN "dust-sensor-network/measurement"

AsyncMqttClient mqttClient;

void connectMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void disconnectMqtt()
{
    Serial.println("Disconnectieng to MQTT...");
    mqttClient.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");
}

void onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
}

void initMqtt()
{
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setClientId(MQTT_CLIENT_ID);
}

void publishMqtt(const String& str)
{
    mqttClient.publish(MQTT_TOPIC_MAIN, 0, true, str.c_str());
}