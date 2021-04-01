#include <SPI.h>
#include <string.h>

#include "BME280Thermometer.h"
#include "Display.h"
#include "DustSensor.h"
#include "WifiManager.h"
#include "WifiServer.h"
#include "battery.h"
#include "communicator.h"
#include "logger.h"
#include "mqtt.h"
#include "nodeconfig.h"
#include "nvsstore.h"
#include "otamanager.h"
#include "touchmanager.h"
#include "windsensor.h"

ThermometerData thermometerData;
DustSensorData dustSensorData;
WindSensorData windSensorData;

struct tm timeinfo;

bool nodeRunnig = false;

int lastLogSec = -1;

unsigned long minCycleTime = 200;

bool isMaster()
{
    return settings.nodeId == 0;
}

void handleCommand(RemoteCommand c)
{
    if (c == StartLogging)
    {
        sds.wakeup();
        nodeRunnig = true;
    }
    else if (c == StopLogging)
    {
        nodeRunnig = false;
        sds.sleep();
    }
}

void updateTime()
{
    if (!getLocalTime(&timeinfo, 200))
        ;
    // Serial.println("Failed to obtain time");
}

void toggleSensorRunning(bool start)
{
    if (start && !nodeRunnig)
    {
        nodeRunnig = true;
        if (isMaster())
        {
            if (USE_MQTT)
                connectMqtt();
            sendCommand(StartLogging);
        }

        if (settings.nodeType == NodeTypeDustSensor)
            sds.wakeup();

        if (isMaster())
            currentLogfilePath = createLogFile(timeinfo);
        Serial.println("Starting Sensor.");
    }
    else if (!start && nodeRunnig)
    {
        nodeRunnig = false;
        if (isMaster())
        {
            sendCommand(StopLogging);
            if (USE_MQTT)
                disconnectMqtt();
        }

        if (settings.nodeType == NodeTypeDustSensor)
            sds.sleep();
        Serial.println("Stopping Sensor.");
    }
}

bool getStatus()
{
    return nodeRunnig;
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    initBattery();

    initNvs();

    initDisplay();

    if (settings.nodeType == NodeTypeDustSensor)
        initSDS011();
    else if (settings.nodeType == NodeTypeWindSensor)
        initWS200();

    if (isMaster())
    {
        initThermometer();

        initSdCard();
    }

    if (connectToWifi())
    {
        setupOTA();
        syncTime();
    }

    if (isMaster())
    {
        initWebServer();
        toggleCallback = &toggleSensorRunning;
        statusCallback = &getStatus;
        tagCallback = &addLoggingTag;

        if (USE_MQTT)
            initMqtt();
    }
    initEspNow(isMaster());

    if (!isMaster())
        commandCallback = &handleCommand;
}

void printLocalTime()
{
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay, 10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
}

void displayDustSensorData()
{
    if (dustSensorData.pm10 != -1)
        display.printf("PM2.5:%.1f PM10:%.1f\n", dustSensorData.pm2_5, dustSensorData.pm10);
    else
        display.println("error reading Sensor");
}

void displayWindData()
{
    display.printf("Wind speed: %.2f\n", windSensorData.windspeed);
    display.printf("Wind direction: %d\n", windSensorData.winddirection);
    if (windSensorData.status != 0 && windSensorData.status != -1)
        display.println(ws200.checkStatus(windSensorData.status));
}

void displayTemp()
{
    display.printf("Temp: %.1f Hum: %.1f\n", thermometerData.temperature, thermometerData.humidity);
}

void masterLoop()
{
    thermometerData = updateTemperature();

    displayTemp();

    // display.printf("touch value: %d\n", latestTouchValue);

    // if (processTouchPin())
    //     toggleSensorRunning();

    if (nodeRunnig)
    {
        dustSensorData = updateDustSensorData();
        registerDustSensorData(settings.nodeId, dustSensorData);
        displayDustSensorData();
        if (lastLogSec != timeinfo.tm_sec)
        {
            lastLogSec = timeinfo.tm_sec;
            auto str = makeMeasurementString(&timeinfo, &thermometerData);

            logString(str);

            if (USE_MQTT)
                publishMqtt(str);
        }
        display.printf("Connected Sensors: %d\n", countValidConnections());
    }
    else
        display.println("Node inactive");

    handleRequests();
}

void servantLoop()
{
    if (settings.nodeType == NodeTypeWindSensor)
    {
        windSensorData = updateWindSensorData();
        displayWindData();
    }

    if (nodeRunnig)
    {
        if (settings.nodeType == NodeTypeDustSensor)
        {
            dustSensorData = updateDustSensorData();
            displayDustSensorData();
            sendDustSensorData(settings.nodeId, dustSensorData);
        }
        if (settings.nodeType == NodeTypeWindSensor)
        {
            sendWindSensorData(settings.nodeId, windSensorData);
        }
    }
    else
    {
        display.println("Node inactive");
    }
}

void loop()
{
    static unsigned long lastmillis = 0;
    auto nm = millis();
    auto cycleTime = nm - lastmillis;
    if (cycleTime < minCycleTime)
        return;
    lastmillis = nm;

    if (WiFi.isConnected())
    {
        ArduinoOTA.handle();
        updateTime();
    }

    display.clearDisplay();
    display.setCursor(0, 0);

    display.printf("Sensor Id: %d\n", settings.nodeId);

    display.println(WiFi.isConnected() ? WiFi.localIP().toString() : String("No Connection"));

    display.println(&timeinfo, "%H:%M:%S");

    if (isMaster())
        masterLoop();
    else
        servantLoop();

    // display.printf("Battery voltage: %.1fv\n", readBatteryVoltage());
    display.printf("Cycle time: %lu", cycleTime);

    display.display();
}