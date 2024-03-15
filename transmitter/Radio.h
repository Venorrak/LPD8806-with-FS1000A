#include "Print.h"
#ifndef __RADIO_H__
#define __RADIO_H__

#define RADIOHEAD

#include "HardwareSerial.h"
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef RADIOHEAD
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h>  // Not actually used but needed to compile
#endif
#else
#include <VirtualWire.h>
#endif

class Radio {
public:
  Radio(uint16_t speed = 2000, uint8_t pin = 11) : driver (speed, pin, pin) {

  }

  void sendMessage(uint8_t effet, uint32_t color, uint8_t speed, uint8_t part) {
    //add info to json document
    doc["e"] = effet;  // 0 - 4 (knight rider, onetwo, rainbow, flash, splash)
    doc["c"] = color;  // HEX use the rgbToHex function
    doc["s"] = speed;  // 0 - 9 more is slower
    doc["p"] = part;   // 0 - 2
    //register information
    _effet = effet;
    _color = color;
    _speed = speed;
    _part = part;
    serializeJson(doc, _msg);
    //send message on radio module

    driver.send((uint8_t *)_msg, strlen(_msg));
    driver.waitPacketSent();

    //print sent message
    Serial.print(_msg);
    Serial.println("sent");
  };

  void sendLastMessage() {
    //send message on radio module
    driver.send((uint8_t *)_msg, strlen(_msg));
    driver.waitPacketSent();
    //print sent message
    Serial.print(_msg);
    Serial.println("sent");
  };

  int getMessageLength(uint8_t effet, uint32_t color, uint8_t speed, uint8_t part) {
    //add info to json document
    doc["e"] = effet;  // 0 - 4 (knight rider, onetwo, rainbow, flash, splash)
    doc["c"] = color;  // HEX use the rgbToHex function
    doc["s"] = speed;  // 0 - 9 more is slower
    doc["p"] = part;   // 0 - 2
    //if message is longer than 32 it can't be transfered by the radio
    return measureJson(doc);
  };

  uint32_t rgbToHex(uint8_t red, uint8_t green, uint8_t blue) {
    //user sends a value between 0 and 255 for each value
    RGB |= ((uint32_t)red << 16);
    RGB |= ((uint32_t)green << 8);
    RGB |= blue;
    return RGB;
  };

  bool begin() {
    return driver.init();
  }

private:
  JsonDocument doc;
  uint8_t _effet;
  uint32_t _color;
  uint8_t _speed;
  uint8_t _part;
  char _msg[32];
  uint32_t RGB;
  RH_ASK driver;
};

#endif