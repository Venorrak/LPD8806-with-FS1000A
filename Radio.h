#include "Print.h"
#ifndef __RADIO_H__
#define __RADIO_H__

#include "HardwareSerial.h"
#include <stdint.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <VirtualWire.h>


class Radio {
public:

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
    vw_send((uint8_t *)_msg, strlen(_msg));
    vw_wait_tx();
    //print sent message
    Serial.print(_msg);
    Serial.println("sent");
  };

  void sendLastMessage() {
    //send message on radio module
    vw_send((uint8_t *)_msg, strlen(_msg));
    vw_wait_tx();
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

  void begin(uint8_t radioPin, uint16_t dataTransferSpeed) {
    vw_set_tx_pin(radioPin);      // 12 by default
    vw_setup(dataTransferSpeed);  // speed of data transfer in bps, can max out at 10000 (4000)
  }

  void led(){
    //status led for when the radio is sending a message
    if (vw_tx_active()){
      digitalWrite(13, HIGH);
    }
    else {
      digitalWrite(13, LOW);
    }
  }
private:
  JsonDocument doc;
  uint8_t _effet;
  uint32_t _color;
  uint8_t _speed;
  uint8_t _part;
  char _msg[32];
  uint32_t RGB;
};

#endif