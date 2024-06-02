//#include <Meshtastic.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

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
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
  display_start();
  //ry_init();

  SPI.begin(SCK,MISO,MOSI,-1);
  LoRa.setPins(CS_LORA,RESET,IO0);

  Serial.println("LoRa Receiver");
  display.println("LoRa Receiver");
  display.display();

  while(!LoRa.begin(906875000)) {
    Serial.println("Starting LoRa failed!");
    //while (1);
  }

  LoRa.setSpreadingFactor(11);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(5);
  LoRa.setPreambleLength(16);
  LoRa.setSyncWord(0x2b);

  LoRa.receive(0);
  Serial.println("LoRa started");
  display.println("LoRa started");
  display.display();
}

int32_t last;
uint8_t outbound[256];
int outindex = 0;

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
  if(Serial.available())
  {
    char byte = Serial.read();
    if(outindex < 256)
    {
      outbound[outindex++] = byte;
      last = millis();
    }
  }
  else
  {
    if(last + 1000 <= millis() && outindex > 0)
    {
      outbound[outindex] = 0;
      display.printf("sending %s\n",outbound);
      display.display();
      LoRa.beginPacket(false);
      LoRa.write(outbound,outindex);
      LoRa.endPacket(false);
      outindex = 0;
      LoRa.receive(0);
    }
  }
}
