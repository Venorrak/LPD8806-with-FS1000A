#include <ArduinoJson.h>
#include "radio.h"

#define RADIO_PIN 12
#define IR_PIN 4

unsigned long lastMessage = 0;
unsigned long currentTime;

Radio radio(4000, RADIO_PIN);
uint32_t color;


void setup() {
  Serial.begin(9600);
  //status led
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IR_PIN, INPUT);


  radio.begin();
  //create a color with rgb values between 0 and 255
  color = radio.rgbToHex(196, 0, 196);
  //get message length, if more than 32 it won't work
  radio.getMessageLength(0, color, 1, 0);
  //sends the message on the radio
  radio.sendMessage(1, color, 1, 1);

  Serial.println("Setup completed!");
}

int currentEffect = 0;

void loop() {
  currentTime = millis();
  if (currentTime - lastMessage > 10000){
    radio.sendMessage(currentEffect, color, 1, 1);
    lastMessage = currentTime;
    currentEffect = (currentEffect + 1) % 5;
  }

  objectTask();
  heartBeatTask();
}

void heartBeatTask(){
  static unsigned long lastTime = 0;
  static byte ledState = 0;
  const int rate = 2000;

  if (currentTime - lastTime > rate) {
    lastTime = currentTime;
    ledState = !ledState;

    digitalWrite(LED_BUILTIN, ledState);
  }
}

void objectTask() {
  static int irPrevious = 1;
  static int irCurrent = 1;

  irCurrent = digitalRead(IR_PIN);

  if (!irCurrent && irCurrent != irPrevious) {
    radio.sendMessage(3, 0xFF << 16 | 0x00 << 8 | 0x00, 1, 3);

    Serial.print(currentTime);
    Serial.println(" - Object detected");
  }

  irPrevious = irCurrent;


}


