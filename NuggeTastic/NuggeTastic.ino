#include <esp_task_wdt.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
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

#define LED_COUNT 4
#define LED_PIN 12

#define i2c_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

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
  esp_task_wdt_config_t wdtconfig;
  wdtconfig.timeout_ms = 200;
  wdtconfig.idle_core_mask = -1;
  wdtconfig.trigger_panic = 1;
  esp_task_wdt_init(&wdtconfig);  // enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); 

  uint8_t aeskey[16] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59, 0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
  Serial.begin(9600);
  //while(!Serial);
  display_start();

  mt.init(0x11223344);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP

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

uint8_t rxdata[256];
size_t rxsize = 0;

uint8_t txdata[256];
size_t txsize = 0;

void loop() {
  esp_task_wdt_reset(); 
  mt.update();
  if((rxsize = mt.packet_available())>0)
  {
    if(rxsize == 1)
    {
      strip.setPixelColor(0,strip.Color(0,20,0));
      display.println("good");
      display.display();
    }
    if(rxsize == 2)
    {
      strip.setPixelColor(0,strip.Color(20,0,0));
      display.println("bad");
      display.display();
    }
    strip.show();
    mt.read_packet(rxdata);
    
  }
  if(Serial.available() >= 1)
  {
    if(Serial.peek() == '\n')
    {
      strip.setPixelColor(0,strip.Color(0,0,20));
      strip.show();
      Serial.printf("%s(%d)\n",__FILE__,__LINE__);
      Serial.read();
      mt.write_packet(txdata,txsize);
      txsize = 0;
    }
  }
  if(Serial.available()>1)
  {
    uint8_t rxbyte;
    uint8_t rx[3];
    rx[0] = Serial.read();
    rx[1] = Serial.read();
    rx[2] = 0;
    rxbyte = strtol((char *)rx,0,16);
    if(txsize<256)
    {
      txdata[txsize++] = rxbyte;
      Serial.printf("%s(%d)\n",__FILE__,__LINE__);
    }
  }
}
