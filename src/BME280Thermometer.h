#pragma once

#include <BME280I2C.h>
#include <SPI.h>
#include <Wire.h>

BME280I2C::Settings bmesettings(BME280::OSR_X16, BME280::OSR_X16, BME280::OSR_X1, BME280::Mode_Forced,
                                BME280::StandbyTime_1000ms, BME280::Filter_Off, BME280::SpiEnable_False,
                                BME280I2C::I2CAddr_0x76  // I2C address. I2C specific.
);

BME280I2C bme(bmesettings);

struct ThermometerData
{
    float temperature;
    float humidity;
    float pressure;
};

void initThermometer()
{
    Wire.begin();
    if (!bme.begin())
        Serial.println(F("Could not initialize BMP280!"));
}

ThermometerData updateTemperature()
{
    ThermometerData thermometerdata;

    thermometerdata.temperature = bme.temp();
    thermometerdata.humidity = bme.hum();
    thermometerdata.pressure = bme.pres(BME280::PresUnit_bar);
    return thermometerdata;
}