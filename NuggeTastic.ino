//#include <Meshtastic.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

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

void handle_lora(int size)
{
  uint8_t data[size];
  int i;
  // received a packet
  Serial.print("Received packet '");

  for(i=0;i<size;i++)
  {
    data[i] = LoRa.read();
    Serial.printf("%02hhX ",data[i]);
  }
  if(data[0] == 0x94 && data[1] == 0xc3)
  {
    display.println("probably meshtastic");
    display.display();
  }
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  display_start();

  SPI.begin(SCK,MISO,MOSI,-1);
  LoRa.setPins(CS_LORA,RESET,IO0);
  LoRa.setPreambleLength(16);
  Serial.println("LoRa Receiver");
  display.println("LoRa Receiver");
  display.display();

  while(!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }
  Serial.println("LoRa started");
  display.println("LoRa started");
  display.display();
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if(packetSize > 0)
  {
    handle_lora(packetSize);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("packet size %d\n",packetSize);
    display.display();
  }
}
