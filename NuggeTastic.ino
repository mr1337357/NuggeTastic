//#include <Meshtastic.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <EEPROM.h>

#include "mt-lite.h"
#include "picopb.h"

#define RXD 38
#define TXD 40
#define RY_RESET 3
#define LED 15

#define CS_LORA 13
#define MOSI 10
#define MISO 8
#define SCK 6
#define RESET  5
#define IO0 16

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

mt_lite mt;

void display_start()
{
  delay(250);
  display.begin(i2c_Address,true);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.display();
}

void setup() {
  uint8_t aeskey[16] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59, 0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
  Serial.begin(9600);
  //while(!Serial);
  display_start();

  mt.init(0x11223344);

  mt.set_aeskey(aeskey,sizeof(aeskey));

  SPI.begin(SCK,MISO,MOSI,-1);
  LoRa.setPins(CS_LORA,RESET,IO0);

  Serial.println("LoRa Receiver");
  display.println("LoRa Receiver");
  display.display();

  while(!LoRa.begin(906875000)) {
    Serial.println("Starting LoRa failed!");
  }

  LoRa.setSpreadingFactor(11);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(5);
  LoRa.setPreambleLength(16);
  LoRa.setSyncWord(0x2b);

  Serial.println("LoRa started");
  display.println("LoRa started");
  display.display();
}

int32_t last;
uint8_t outbound[256];
int outindex = 0;

void loop() {
  mt.update();
}
