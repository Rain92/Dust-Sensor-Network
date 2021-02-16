#pragma once

#define OLED_RESET -1
#define PIN_SDA 21
#define PIN_SCL 22

#define SDCARD_CS (13)
#define SDCARD_MOSI (15)
#define SDCARD_MISO (2)
#define SDCARD_SCLK (14)

#define SDS011_TX 23  // 25  // 11, 13, 17, 21, 22, 23
#define SDS011_RX 36  // + 33 - 39

// for windsensor
#define WS200_TX 23
#define WS200_RX 36

#define PIN_LED 25

#define PIN_BATTERY_VOLTAGE 35  // battery probe GPIO pin -> ADC1_CHANNEL_7

#define PIN_TOUCH_1 12

// #define LORA_SCK (5)
// #define LORA_CS (18)
// #define LORA_MISO (19)
// #define LORA_MOSI (27)
// #define LORA_RST (23)
// #define LORA_IRQ (26)
// #define LORA_IO1 (33)
// #define LORA_IO2 (32)