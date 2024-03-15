#include <ArduinoJson.h>
#include "radio.h"

unsigned long lastMessage = 0;

Radio radio(4000, 12);
uint32_t color;
void setup() {
  Serial.begin(9600);
  //status led
  pinMode(13, OUTPUT);

  radio.begin();
  //create a color with rgb values between 0 and 255
  color = radio.rgbToHex(196, 0, 196);
  //get message length, if more than 32 it won't work
  radio.getMessageLength(0, color, 1, 0);
  //sends the message on the radio
  radio.sendMessage(1, color, 1, 1);
}

int currentEffect = 0;

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastMessage > 10000){
    radio.sendMessage(currentEffect, color, 1, 1);
    lastMessage = currentMillis;
    currentEffect = (currentEffect + 1) % 5;
  }
}