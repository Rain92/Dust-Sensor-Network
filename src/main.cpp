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
#include "nvsstrore.h"
#include "otamanager.h"
#include "touchmanager.h"

ThermometerData thermometerData;
DustSensorData dustSensorData;

struct tm timeinfo;

bool sensorRunning = false;

int lastLogSec = -1;

unsigned long minCycleTime = 200;

bool isMaster()
{
    return settings.sensorId == 0;
}

void handleCommand(RemoteCommand c)
{
    if (c == StartLogging)
    {
        sds.wakeup();
        sensorRunning = true;
    }
    else if (c == StopLogging)
    {
        sensorRunning = false;
        sds.sleep();
    }
}

void updateTime()
{
    if (!getLocalTime(&timeinfo, 200))
        ;
    // Serial.println("Failed to obtain time");
}

void toggleSensorRunning()
{
    sensorRunning = !sensorRunning;
    if (sensorRunning)
    {
        sendCommand(StartLogging);
        sds.wakeup();
        currentLogfilePath = createLogFile(timeinfo);
    }
    else
    {
        sendCommand(StopLogging);
        sds.sleep();
    }
}

bool getStatus()
{
    return sensorRunning;
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    initBattery();

    initNvs();

    initDisplay();

    initSDS011();

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
        goCallback = &toggleSensorRunning;
        statusCallback = &getStatus;
        tagCallback = &addLoggingTag;
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

void displayPPM()
{
    if (dustSensorData.pm10 != -1)
        display.printf("PM2.5:%.1f PM10:%.1f\n", dustSensorData.pm2_5, dustSensorData.pm10);
    else
        display.print("error");
}

void displayTemp()
{
    display.printf("Temp: %.1f Hum: %.1f\n", thermometerData.temperature, thermometerData.humidity);
}

void masterLoop()
{
    thermometerData = updateTemperature();

    displayTemp();

    display.printf("touch value: %d\n", latestTouchValue);

    if (processTouchPin())
        toggleSensorRunning();

    if (sensorRunning)
    {
        dustSensorData = updateDustsensorData();
        registerSensorData(settings.sensorId, dustSensorData);
        displayPPM();
        if (lastLogSec != timeinfo.tm_sec)
        {
            lastLogSec = timeinfo.tm_sec;
            logSensorData(&timeinfo, &thermometerData);
        }
        display.printf("Connected Sensors: %d\n", countValidConnections());
    }
    else
        display.println("Sensor sleeping");

    handleRequests();
}

void servantLoop()
{
    if (sensorRunning)
    {
        dustSensorData = updateDustsensorData();
        displayPPM();
        sendSensorData(settings.sensorId, dustSensorData);
    }

    else
        display.println("Sensor sleeping");
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

    display.printf("Sensor Id: %d\n", settings.sensorId);

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