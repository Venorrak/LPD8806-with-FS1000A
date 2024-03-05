#include <VirtualWire.h>
#include <ArduinoJson.h>
#include "radio.h"

unsigned long lastMessage = 0;

Radio radio;

void setup() {
  Serial.begin(9600);
  //status led
  pinMode(13, OUTPUT);
  // create radio object
  radio.begin(12, 4000);
  //create a color with rgb values between 0 and 255
  uint32_t color = radio.rgbToHex(255, 255, 255);
  //get message length, if more than 32 it won't work
  radio.getMessageLength(0, color, 1, 0);
  //sends the message on the radio
  radio.sendMessage(4, color, 1, 1);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastMessage > 20000){
    radio.sendLastMessage();
    lastMessage = currentMillis;
  }
}