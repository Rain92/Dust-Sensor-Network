#pragma once

#include <Arduino.h>

#define CHANNEL_WIND_SPEED_FAST_MS 401
#define CHANNEL_WIND_DIRECTION_FAST_DEG 501
#define CHANNEL_WIND_HEATER_TEMP_C 112
#define CHANNEL_COMPASS_HEADING 510
#define CHANNEL_SUPPLY_VOLTAGE 10000

HardwareSerial wsSerial(1);

struct QueryResult
{
    float value;
    int32_t status;
};

struct QueryResultMulti
{
    float* values;
    int32_t status;
};

struct UMBResponse
{
    uint8_t* payload;
    uint16_t size;
};

class WSDevice
{
private:
    uint16_t calc_crc_single(uint16_t crc_buff, uint8_t input)
    {
        uint8_t i;
        uint16_t x16;  // we’ll use this to hold the XOR mask
        for (i = 0; i < 8; i++)
        {  // XOR current D0 and next input bit to
            // determine x16 value
            if ((crc_buff & 0x0001) ^ (input & 0x01))
                x16 = 0x8408;
            else
                x16 = 0x0000;
            // shift crc buffer
            crc_buff = crc_buff >> 1;
            // XOR in the x16 value
            crc_buff ^= x16;
            // shift inputfor next iteration
            input = input >> 1;
        }
        return (crc_buff);
    }

    uint16_t calc_crc_buffer(uint8_t* buffer, uint16_t size)
    {
        // initialise startvalue FFFFh
        uint16_t crc = 0xFFFF;
        // calculation
        for (int n = 0; n < size; n++)
        {
            crc = calc_crc_single(crc, buffer[n]);
        }
        return crc;
    }

    float getValueAsFloat(uint8_t* valueptr, uint8_t type_of_value)
    {
        float value_f = -1;

        // Assume host is little endian
        if (type_of_value == 16)  // UNSIGNED_CHAR
            value_f = *((uint8_t*)valueptr);
        else if (type_of_value == 17)  // SIGNED_CHAR
            value_f = *((int8_t*)valueptr);
        else if (type_of_value == 18)  // UNSIGNED_SHORT
            value_f = *((uint16_t*)valueptr);
        else if (type_of_value == 19)  // SIGNED_SHORT
            value_f = *((int16_t*)valueptr);
        else if (type_of_value == 20)  // UNSIGNED_LONG
            value_f = *((uint32_t*)valueptr);
        else if (type_of_value == 21)  // SIGNED_LONG
            value_f = *((int32_t*)valueptr);
        else if (type_of_value == 22)  // FLOAT
            value_f = *((float*)valueptr);
        else if (type_of_value == 23)        // DOUBLE
            value_f = *((double*)valueptr);  // probably broken on arduino platfoms that don't support double

        return value_f;
    }

    uint8_t combuffer[300];
    float multichannelvalues[20];

    UMBResponse sendRequest(uint8_t receiver_id, uint8_t command, uint8_t command_version, uint8_t* payload,
                            uint8_t payload_size)
    {
        uint8_t SOH = 0x01, STX = 0x02, ETX = 0x03, EOT = 0x04;
        uint8_t VERSION = 0x10;
        uint8_t TO = receiver_id;
        uint8_t TO_CLASS = 0x70;  // class id for WS200 - WS600
        uint8_t FROM = 0x01;
        uint8_t FROM_CLASS = 0xF0;

        uint8_t COMMAND = command;
        uint8_t COMMAND_VERSION = command_version;

        uint8_t LEN = 2 + payload_size;

        uint16_t CRC = 0;

        int bs = 0;

        combuffer[bs++] = SOH;
        combuffer[bs++] = VERSION;
        combuffer[bs++] = TO;
        combuffer[bs++] = TO_CLASS;
        combuffer[bs++] = FROM;
        combuffer[bs++] = FROM_CLASS;
        combuffer[bs++] = LEN;
        combuffer[bs++] = STX;
        combuffer[bs++] = COMMAND;
        combuffer[bs++] = COMMAND_VERSION;
        for (uint8_t i = 0; i < payload_size; i++)
        {
            combuffer[bs++] = payload[i];
        }
        combuffer[bs++] = ETX;

        CRC = calc_crc_buffer(combuffer, bs);

        combuffer[bs++] = CRC % 256;
        combuffer[bs++] = CRC >> 8;

        combuffer[bs++] = EOT;

        // for (size_t i = 0; i < bs; i++)
        // {
        //     Serial.printf("%d ", buffer[i]);
        // }
        // Serial.println();

        // # Assemble transmit-frame
        // tx_frame = SOH + VERSION + TO + TO_CLASS + FROM + FROM_CLASS +
        //     LEN + STX + COMMAND + COMMAND_VERSION + payload + ETX
        // # calculate checksum for trasmit-frame and concatenate
        // tx_frame += self.calc_crc16(tx_frame).to_bytes(2, 'little') + EOT

        // # Write transmit-frame to serial

        wsSerial.write(combuffer, bs);

        int rxbs = 0;

        for (size_t timeout = 0; timeout < 100; timeout++)
        {
            if (wsSerial.available())
                break;
            delay(1);
        }
        delay(10);

        rxbs = wsSerial.available();

        if (rxbs < 10 || rxbs > 280)
        {
            Serial.printf("RX-Error! Invalid number of bytes recieved.\n");
            return {0, 0};
        }

        wsSerial.readBytes(combuffer, rxbs);

        uint16_t rxcrc = (combuffer[rxbs - 2] << 8) + combuffer[rxbs - 3];
        uint16_t rxcrc_calc = calc_crc_buffer(combuffer, rxbs - 3);

        if (rxcrc != rxcrc_calc)
        {
            Serial.printf("RX-Error! Checksum test failed. Calculated Checksum: %04X Received Checksum: %04X\n", rxcrc,
                          rxcrc_calc);
            return {0, 0};
        }

        uint8_t rxpayloadsize = combuffer[6];

        // Serial.printf("rxlength: %d\n", rxlength);

        if (combuffer[8 + rxpayloadsize] != ETX)
        {
            Serial.printf("RX-Error! Length of Payload is not valid. length-field says: %d\n", rxpayloadsize);
            return {0, 0};
        }

        if (combuffer[0] != SOH)
        {
            Serial.printf("RX-Error! No Start-of-frame Character\n");
            return {0, 0};
        }
        if (combuffer[1] != VERSION)
        {
            Serial.printf("RX-Error! Wrong Version Number\n");
            return {0, 0};
        }
        if (combuffer[2] != FROM || combuffer[3] != FROM_CLASS)
        {
            Serial.printf("RX-Error! Wrong Destination ID\n");
            return {0, 0};
        }
        if (combuffer[4] != TO || combuffer[5] != TO_CLASS)
        {
            Serial.printf("RX-Error! Wrong Source ID\n");
            return {0, 0};
        }
        if (combuffer[7] != STX)
        {
            Serial.printf("RX-Error! Missing STX field\n");
            return {0, 0};
        }
        if (combuffer[8] != COMMAND)
        {
            Serial.printf("RX-Error! Wrong Command Number\n");
            return {0, 0};
        }
        if (combuffer[9] != COMMAND_VERSION)
        {
            Serial.printf("RX-Error! Wrong Command Version Number\n");
            return {0, 0};
        }

        // uint8_t status = combuffer[10];

        return {&combuffer[10], (uint16_t)(rxpayloadsize - 2)};
    }

public:
    void init(int rxpin, int txpin) { wsSerial.begin(19200, SERIAL_8N1, rxpin, txpin); }

    QueryResult requestOnlineData(uint16_t channel, uint8_t receiver_id = 1)
    {
        uint8_t payload[2];
        payload[0] = channel % 256;
        payload[1] = channel >> 8;

        auto response = sendRequest(receiver_id, 0x23, 0x10, payload, 2);

        if (response.size == 0)
            return {-1, -1};

        int status = response.payload[0];

        if (response.size == 1)
            return {-1, status};

        auto buffer = response.payload;
        uint8_t type_of_value = buffer[3];

        uint8_t* valueptr = &buffer[4];
        float value_f = getValueAsFloat(valueptr, type_of_value);

        // Serial.printf("type_of_value: %d\n", type_of_value);
        // Serial.printf("value: %f\n", value_f);

        return {value_f, status};
    }

    QueryResultMulti requestOnlineDataMulti(uint16_t* channels, uint8_t numchannels, uint8_t receiver_id = 1)
    {
        uint8_t ps = numchannels * 2 + 1;
        uint8_t* payload = (uint8_t*)malloc(ps);
        payload[0] = numchannels;

        for (auto i = 0; i < numchannels; i++)
        {
            payload[1 + 2 * i] = channels[i] % 256;
            payload[2 + 2 * i] = channels[i] >> 8;
        }

        auto response = sendRequest(receiver_id, 0x2F, 0x10, payload, ps);

        free(payload);

        if (response.size <= 1)
        {
            for (auto i = 0; i < numchannels; i++)
                multichannelvalues[i] = -1;

            return {multichannelvalues, response.size == 0 ? -1 : response.payload[0]};
        }

        int status = response.payload[0];
        numchannels = response.payload[1];
        uint8_t* ptr = &response.payload[2];

        for (auto i = 0; i < numchannels; i++)
        {
            uint8_t sublen = ptr[0] + 1;
            uint8_t type_of_value = ptr[4];
            uint8_t* valueptr = &ptr[5];
            float value_f = getValueAsFloat(valueptr, type_of_value);
            multichannelvalues[i] = value_f;

            // Serial.printf("type_of_value: %d\n", type_of_value);
            // Serial.printf("value: %f\n", value_f);
            // Serial.printf("sublen: %d\n", sublen);
            ptr += sublen;
        }

        return {multichannelvalues, status};
    }

    const char* checkStatus(int status)
    {
        if (status == 0)
            return ("Status: Kommando erfolgreich");
        else if (status == 16)
            return ("Status: unbekanntes Kommando; wird von diesen Gerät nicht unterstützt");
        else if (status == 17)
            return ("Status: ungültige Parameter");
        else if (status == 18)
            return ("Status: ungültige Header-Version");
        else if (status == 19)
            return ("Status: ungültige Version des Befehls");
        else if (status == 20)
            return ("Status: Passwort für Kommando falsch");
        else if (status == 32)
            return ("Status: Lesefehler");
        else if (status == 33)
            return ("Status: Schreibfehler");
        else if (status == 34)
            return ("Status: Länge zu groß; max. zulässige Länge wird in <maxlength> angegeben");
        else if (status == 35)
            return ("Status: ungültige Adresse / Speicherstelle");
        else if (status == 36)
            return ("Status: ungültiger Kanal");
        else if (status == 37)
            return ("Status: Kommando in diesem Modus nicht möglich");
        else if (status == 38)
            return ("Status: unbekanntes Test-/Abgleich-Kommando");
        else if (status == 39)
            return ("Status: Fehler bei der Kalibrierung");
        else if (status == 40)
            return ("Status: Gerät nicht bereit; z.B. Initialisierung / Kalibrierung läuft");
        else if (status == 41)
            return ("Status: Unterspannung");
        else if (status == 42)
            return ("Status: Hardwarefehler");
        else if (status == 43)
            return ("Status: Fehler in der Messung");
        else if (status == 44)
            return ("Status: Fehler bei der Geräteinitialisierung");
        else if (status == 45)
            return ("Status: Fehler im Betriebssystem");
        else if (status == 48)
            return ("Status: Fehler in der Konfiguration, Default-Konfiguration wurde geladen");
        else if (status == 49)
            return ("Status: Fehler im Abgleich / der Abgleich ist ungültig, Messung nicht möglich");
        else if (status == 50)
            return ("Status: CRC-Fehler beim Laden der Konfiguration; Default-Konfiguration wurde geladen");
        else if (status == 51)
            return ("Status: CRC-Fehler beim Laden der Abgleich-Daten; Messung nicht möglich");
        else if (status == 52)
            return ("Status: Abgleich Step 1");
        else if (status == 53)
            return ("Status: Abgleich OK");
        else if (status == 54)
            return ("Status: Kanal deaktiviert");
        else if (status == -1)
            return ("Status: Fehler beim Empfangen der Daten");
        else
            return ("Unbekannter Status");
    }
};