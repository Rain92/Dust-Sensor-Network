#include <SPI.h>
#include <string.h>

#include "BME280Thermometer.h"
#include "Display.h"
#include "DustSensor.h"
#include "FileWebServer.h"
#include "SdCard.h"
#include "WifiManager.h"
#include "communicator.h"
#include "nvsstrore.h"

ThermometerData thermometerData;
DustSensorData dustSensorData;

struct tm timeinfo;

bool touchProcessed = false;
int touchDetectionCounter = 0;
bool measuring = false;
int touchThreshold = 28;
int lastLogSec = -1;
String currentLogfilePath;

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
        measuring = true;
    }
    else if (c == StopLogging)
    {
        measuring = false;
        sds.sleep();
    }
}

void updateTime()
{
    if (!getLocalTime(&timeinfo), 200)
        Serial.println("Failed to obtain time");
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    initNvs();

    initDisplay();

    initSDS011();

    if (isMaster())
    {
        initThermometer();

        initSdCard();
    }

    connectToWifi();
    syncTime();

    if (isMaster())
    {
        initWebServer();
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

void printPPM()
{
    if (dustSensorData.pm10 != -1)
        display.printf("PM2.5:%.1f PM10:%.1f\n", dustSensorData.pm2_5, dustSensorData.pm10);
    else
        display.print("error");
}

void printTemp()
{
    display.printf("Temp: %.1f Hum: %.1f\n", thermometerData.temperature, thermometerData.humidity);
}

bool processTouchPin()
{
    auto touch_sensor_value = touchRead(PIN_TOUCH_1);
    bool touch_detected = touch_sensor_value < touchThreshold;

    if (touch_detected)
        touchDetectionCounter++;
    else
        touchDetectionCounter = 0;
    

    if (touchDetectionCounter > 1 && !touchProcessed)
    {
        touchProcessed = true;
        return true;
    }
    else if (!touch_detected)
    {
        touchProcessed = false;
    }

    return false;
}

void logSensorData()
{
    auto logFile = SD.open(currentLogfilePath.c_str(), FILE_WRITE);

    logFile.print(&timeinfo, "%H:%M:%S, ");
    logFile.printf("%.1f, %.1f", thermometerData.temperature, thermometerData.humidity);

    for (int i = 0; i < MAX_NUM_SENSORS; i++)
        if (dataValid(i))
            logFile.printf(", %d, %.1f, %.1f", i, sensorNetworkData[i].pm2_5, sensorNetworkData[i].pm10);

    logFile.println();

    logFile.close();
}

void masterLoop()
{
    thermometerData = updateTemperature();

    printTemp();

    display.printf("touch value: %d\n", touchRead(PIN_TOUCH_1));

    if (processTouchPin())
    {
        measuring = !measuring;
        if (measuring)
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

    if (measuring)
    {
        dustSensorData = updateDustsensorData();
        registerSensorData(settings.sensorId, dustSensorData);
        printPPM();
        if (lastLogSec != timeinfo.tm_sec)
        {
            lastLogSec = timeinfo.tm_sec;
            logSensorData();
        }
        display.printf("Connected Sensors: %d\n", countValidConnections());
    }
    else
        display.println("Sensor sleeping");

    handleRequests();
}

void servantLoop()
{
    if (measuring)
    {
        dustSensorData = updateDustsensorData();
        printPPM();
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
        updateTime();

    display.clearDisplay();
    display.setCursor(0, 0);

    display.printf("Sensor Id: %d\n", settings.sensorId);

    display.println(WiFi.isConnected() ? WiFi.localIP().toString() : String("No Connection"));

    display.println(&timeinfo, "%H:%M:%S");

    if (isMaster())
        masterLoop();
    else
        servantLoop();

    display.printf("Cycle time: %lu", cycleTime);

    display.display();
}