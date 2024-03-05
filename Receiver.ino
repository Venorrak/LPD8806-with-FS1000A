#include <VirtualWire.h>
#include <ArduinoJson.h>
#include "LPD8806.h"
#include <Watchdog.h>

Watchdog watchdog;
const unsigned long SETUP_LED_ON_IF_NOT_TRIPPED_DURATION = 4000;
const unsigned long SETUP_LED_ON_IF_TRIPPED_DURATION = 1500;
const unsigned long SETUP_LED_OFF_DURATION = 1000;

const int nLEDs = 116;
int nbLedInPart = 39;
int dataPin = 2;
int clockPin = 3;
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);
unsigned long lt1 = 0;
unsigned long lt2 = 0;
unsigned long lt3 = 0;
enum animations { KNIGHT_RIDER,
                  ONETWO,
                  RAINBOW,
                  FLASH,
                  SPLASH,
                  NONE };
animations AnimPart1 = NONE;
animations AnimPart2 = NONE;
animations AnimPart3 = NONE;
int effet = 0;
uint32_t color1 = 0x000000;
uint32_t color2 = 0x000000;
uint32_t color3 = 0x000000;
int speedPart1 = 0;
int speedPart2 = 0;
int speedPart3 = 0;
int part = 0;
int activeTime1 = 0;
int activeTime2 = 0;
int activeTime3 = 0;
bool active1 = false;
bool active2 = false;
bool active3 = false;
//--------------------------------------
int ledIndex1_Part1 = 0;
int ledIndex2_Part1 = nbLedInPart;
int ledIndex1_Part2 = nbLedInPart + 1;
int ledIndex2_Part2 = nbLedInPart * 2;
int ledIndex1_Part3 = (nbLedInPart * 2) + 1;
int ledIndex2_Part3 = 116;
//--------------------------------------
int lastFlash_Part1 = 0;
int lastFlash_Part2 = 0;
int lastFlash_Part3 = 0;
//--------------------------------------
enum splashState { RANDOM,
                   STROB,
                   CLEAR };
splashState SState_Part1 = RANDOM;
splashState SState_Part2 = RANDOM;
splashState SState_Part3 = RANDOM;
int nbDone_Part1 = 0;
int nbDone_Part2 = 0;
int nbDone_Part3 = 0;
bool ledState_Part1 = false;
bool ledState_Part2 = false;
bool ledState_Part3 = false;
int timeStrob_Part1 = 0;
int timeStrob_Part2 = 0;
int timeStrob_Part3 = 0;
//--------------------------------------
int offset = 0;
const int multiple = 4;
const int offsetDelta = 2;

//---------------------MODIFIABLE--------------------

//time of the animations
int animationTime = 3000;
//default color (r, g, b)
uint32_t off = strip.Color(0, 0, 0);
//length of the moving light(knight rider)
int ledLength = 3;
//minimum segment length(splash)
int minLength = 2;
//maximum segment length(splash)
int maxLength = 3;
//nuber of times the strob effects is done(splash)
int noStrob = 20;

//----------------------------------------------------

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  if (!watchdog.tripped()) {
    // blink once to indicate power cycle reset
    setLedOn(SETUP_LED_ON_IF_NOT_TRIPPED_DURATION);
  } else {
    // blink twice to indicate watchdog tripped reset
    setLedOn(SETUP_LED_ON_IF_TRIPPED_DURATION);
    setLedOff();
    setLedOn(SETUP_LED_ON_IF_TRIPPED_DURATION);
  }
  watchdog.enable(Watchdog::TIMEOUT_1S);

  strip.begin();
  strip.show();
  randomSeed(analogRead(0));

  vw_set_rx_pin(4);  //connect the receiver data pin to pin 12
  vw_setup(4000);    // speed of data transfer in bps, maxes out at 10000
  vw_rx_start();     // Start the receiver PLL running
}

void loop() {
  watchdog.reset();
  unsigned long ct = millis();
  JsonDocument msg = getMessage();
  String msgTest = "";
  serializeJson(msg, msgTest);
  if (msgTest != "null") {
    serializeJson(msg, Serial);
    Serial.println();

    effet = msg["e"];
    part = msg["p"];

    switch (part) {
      case 0:
        active1 = true;
        AnimPart1 = effet;
        color1 = msg["c"];
        speedPart1 = msg["s"];
        speedPart1 = speedPart1 * 100;
        if (speedPart1 == 0) { speedPart1 = 50; }
        resetAnimValues1();
        break;
      case 1:
        active2 = true;
        AnimPart2 = effet;
        color2 = msg["c"];
        speedPart2 = msg["s"];
        speedPart2 = speedPart2 * 100;
        if (speedPart2 == 0) { speedPart2 = 50; }
        resetAnimValues2();
        break;
      case 2:
        active3 = true;
        AnimPart3 = effet;
        color3 = msg["c"];
        speedPart3 = msg["s"];
        speedPart3 = speedPart3 * 100;
        if (speedPart3 == 0) { speedPart3 = 50; }
        resetAnimValues3();
        break;
    }

    changeStrip(off, part);
  }
  if (active1) {
    activeTime1++;
  }
  if (active2) {
    activeTime2++;
  }
  if (active3) {
    activeTime3++;
  }
  if (ct - lt1 > speedPart1) {
    switch (AnimPart1) {
      case KNIGHT_RIDER:
        knightRider(color1, 0);
        break;
      case ONETWO:
        oneTwo(color1, 0);
        break;
      case FLASH:
        flash(color1, 0);
        break;
      case SPLASH:
        splash(color1, 0);
        break;
      case RAINBOW:
        rainbow(0);
        break;
    }
    lt1 = ct;
  }
  if (ct - lt2 > speedPart2) {
    switch (AnimPart2) {
      case KNIGHT_RIDER:
        knightRider(color2, 1);
        break;
      case ONETWO:
        oneTwo(color2, 1);
        break;
      case FLASH:
        flash(color2, 1);
        break;
      case SPLASH:
        splash(color2, 1);
        break;
      case RAINBOW:
        rainbow(1);
        break;
    }
    lt2 = ct;
  }
  if (ct - lt3 > speedPart3) {
    switch (AnimPart3) {
      case KNIGHT_RIDER:
        knightRider(color3, 2);
        break;
      case ONETWO:
        oneTwo(color3, 2);
        break;
      case FLASH:
        flash(color3, 2);
        break;
      case SPLASH:
        splash(color3, 2);
        break;
      case RAINBOW:
        rainbow(2);
        break;
    }
    lt3 = ct;
  }
  if (activeTime1 >= animationTime) {
    AnimPart1 = NONE;
    active1 = false;
    changeStrip(off, 0);
  }
  if (activeTime2 >= animationTime) {
    AnimPart2 = NONE;
    active2 = false;
    changeStrip(off, 1);
  }
  if (activeTime3 >= animationTime) {
    AnimPart3 = NONE;
    active3 = false;
    changeStrip(off, 2);
  }
}

JsonDocument getMessage() {
  JsonDocument msg;
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen))  // if we get a message that we recognize on this buffer...
  {
    String Json = "";
    for (int i = 0; i < buflen; i++) {
      Json += (char)buf[i];  // fill the string with the data received
    }
    DeserializationError error = deserializeJson(msg, Json);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
  }
  return msg;
}

int getSegmentLength() {
  return random(minLength, maxLength);
}

int getSegmentLength(int modMaxLength) {
  Serial.println("eille");
  return random(minLength, modMaxLength);
}

int getSegmentStart(int currentPart) {
  int start = currentPart * nbLedInPart;
  return random(start, start + nbLedInPart);
}

void changeStrip(uint32_t color, int currentPart) {
  int start = currentPart * nbLedInPart;
  for (int i = start; i < start + nbLedInPart; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void knightRider(uint32_t currentColor, int currentPart) {
  switch (currentPart) {
    case 0:
      strip.setPixelColor(ledIndex2_Part1, currentColor);
      strip.setPixelColor(ledIndex2_Part1 + ledLength, off);
      strip.setPixelColor(ledIndex1_Part1, currentColor);
      strip.setPixelColor(ledIndex1_Part1 - ledLength, off);
      if (ledIndex1_Part1 >= 1 * nbLedInPart) {
        ledIndex1_Part1 = 0;
        ledIndex2_Part1 = nbLedInPart;
      }
      ledIndex1_Part1++;
      ledIndex2_Part1--;
      break;
    case 1:
      strip.setPixelColor(ledIndex2_Part2, currentColor);
      strip.setPixelColor(ledIndex2_Part2 + ledLength, off);
      strip.setPixelColor(ledIndex1_Part2, currentColor);
      strip.setPixelColor(ledIndex1_Part2 - ledLength, off);
      if (ledIndex1_Part2 >= 2 * nbLedInPart) {
        ledIndex1_Part2 = nbLedInPart + 1;
        ledIndex2_Part2 = nbLedInPart * 2;
      }
      ledIndex1_Part2++;
      ledIndex2_Part2--;
      break;
    case 2:
      strip.setPixelColor(ledIndex2_Part3, currentColor);
      strip.setPixelColor(ledIndex2_Part3 + ledLength, off);
      strip.setPixelColor(ledIndex1_Part3, currentColor);
      strip.setPixelColor(ledIndex1_Part3 - ledLength, off);
      if (ledIndex1_Part3 >= 116) {
        ledIndex1_Part3 = nbLedInPart + 1;
        ledIndex2_Part3 = 116;
      }
      ledIndex1_Part3++;
      ledIndex2_Part3--;
      break;
  }
  strip.show();
}

void oneTwo(uint32_t currentColor, int currentPart) {
  static bool decal = false;
  for (int i = decal + (currentPart * nbLedInPart); i < (currentPart * nbLedInPart) + nbLedInPart; i++) {
    if (decal == true) {
      if (i % 2 != 0) {
        strip.setPixelColor(i, currentColor);
      } else {
        strip.setPixelColor(i, off);
      }
    } else {
      if (i % 2 == 0) {
        strip.setPixelColor(i, currentColor);
      } else {
        strip.setPixelColor(i, off);
      }
    }
  }
  strip.show();
  decal = !decal;
}

void flash(uint32_t currentColor, int currentPart) {
  int n = random(currentPart * nbLedInPart, (currentPart * nbLedInPart) + nbLedInPart);
  strip.setPixelColor(n, currentColor);
  switch (currentPart) {
    case 0:
      strip.setPixelColor(lastFlash_Part1, off);
      lastFlash_Part1 = n;
      break;
    case 1:
      strip.setPixelColor(lastFlash_Part2, off);
      lastFlash_Part2 = n;
      break;
    case 2:
      strip.setPixelColor(lastFlash_Part3, off);
      lastFlash_Part3 = n;
      break;
  }
  strip.show();
}

void splash(uint32_t currentColor, int currentPart) {
  switch (currentPart) {
    case 0:
      switch (SState_Part1) {
        case RANDOM:
          static int start1;
          start1 = getSegmentStart(currentPart);
          static int segmentLength;
          if (((currentPart + 1) * nbLedInPart) - start1 < maxLength) {
            segmentLength = getSegmentLength(((currentPart + 1) * nbLedInPart) - start1);
          } else {
            segmentLength = getSegmentLength();
          }
          for (int i = start1; i < start1 + segmentLength; i++) {
            strip.setPixelColor(i, currentColor);
          }
          strip.show();
          for (int i = currentPart * nbLedInPart; i < (currentPart * nbLedInPart) + nbLedInPart; i++) {
            if (strip.getPixelColor(i) != off && strip.getPixelColor(i) != 0) {
              nbDone_Part1++;
            }
          }
          if (nbDone_Part1 >= nbLedInPart - 1) {
            SState_Part1 = STROB;
          }
          nbDone_Part1 = 0;
          break;
        case STROB:
          if (ledState_Part1) {
            changeStrip(currentColor, currentPart);
            ledState_Part1 = !ledState_Part1;
          } else {
            changeStrip(off, currentPart);
            ledState_Part1 = !ledState_Part1;
          }
          timeStrob_Part1++;
          if (timeStrob_Part1 >= noStrob) {
            SState_Part1 = CLEAR;
            timeStrob_Part1 = 0;
          }
          break;
        case CLEAR:
          changeStrip(off, currentPart);
          //changeStrip(off, currentPart + 1);
          SState_Part1 = RANDOM;
          break;
      }
      break;
    case 1:
      switch (SState_Part2) {
        case RANDOM:
          static int start2;
          start2 = getSegmentStart(currentPart);
          for (int i = start2; i < start2 + getSegmentLength(); i++) {
            strip.setPixelColor(i, currentColor);
          }
          strip.show();
          for (int i = currentPart * nbLedInPart; i < (currentPart * nbLedInPart) + nbLedInPart; i++) {
            if (strip.getPixelColor(i) != off && strip.getPixelColor(i) != 0) {
              nbDone_Part2++;
            }
          }
          if (nbDone_Part2 >= nbLedInPart - 1) {
            SState_Part2 = STROB;
          }
          nbDone_Part2 = 0;
          break;
        case STROB:
          if (ledState_Part2) {
            changeStrip(currentColor, currentPart);
            ledState_Part2 = !ledState_Part2;
          } else {
            changeStrip(off, currentPart);
            ledState_Part2 = !ledState_Part2;
          }
          timeStrob_Part2++;
          if (timeStrob_Part2 >= noStrob) {
            SState_Part2 = CLEAR;
            timeStrob_Part2 = 0;
          }
          break;
        case CLEAR:
          changeStrip(off, currentPart);
          SState_Part2 = RANDOM;
          break;
      }
      break;
    case 2:
      switch (SState_Part3) {
        case RANDOM:
          static int start3;
          start3 = getSegmentStart(currentPart);
          for (int i = start3; i < start3 + getSegmentLength(); i++) {
            strip.setPixelColor(i, currentColor);
          }
          strip.show();
          for (int i = currentPart * nbLedInPart; i < (currentPart * nbLedInPart) + nbLedInPart; i++) {
            if (strip.getPixelColor(i) != off && strip.getPixelColor(i) != 0) {
              nbDone_Part3++;
            }
          }
          if (nbDone_Part3 >= nbLedInPart - 1) {
            SState_Part3 = STROB;
          }
          nbDone_Part3 = 0;
          break;
        case STROB:
          if (ledState_Part3) {
            changeStrip(currentColor, currentPart);
            ledState_Part3 = !ledState_Part3;
          } else {
            changeStrip(off, currentPart);
            ledState_Part3 = !ledState_Part3;
          }
          timeStrob_Part3++;
          if (timeStrob_Part3 >= noStrob) {
            SState_Part3 = CLEAR;
            timeStrob_Part3 = 0;
          }
          break;
        case CLEAR:
          changeStrip(off, currentPart);
          SState_Part3 = RANDOM;
          break;
      }
      break;
  }
}

void rainbow(int currentPart) {
  cycle(offset, 255, 255, currentPart);
  // Increment the offset to animate the colour pattern
  offset = (offset + offsetDelta) % strip.numPixels();
}

uint32_t hsvToColour(unsigned int h, unsigned int s, unsigned int v) {

  unsigned char region, remainder, p, q, t;

  // Sanity check ranges and check for no saturation
  h = h % 256;
  if (s > 255) s = 255;
  if (v > 255) v = 255;
  else v = (v * v) >> 8;
  if (s == 0) return strip.Color(v >> 1, v >> 1, v >> 1);

  // Map HSV to RGB, use to build a colour value for the strip library
  region = h / 43;
  remainder = (h - (region * 43)) * 6;
  p = (v * (255 - s)) >> 9;
  q = (v * (255 - ((s * remainder) >> 8))) >> 9;
  t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 9;
  v = v >> 1;
  switch (region) {
    case 0:
      return strip.Color(v, p, t);
    case 1:
      return strip.Color(q, p, v);
    case 2:
      return strip.Color(p, t, v);
    case 3:
      return strip.Color(p, v, q);
    case 4:
      return strip.Color(t, v, p);
  }
  return strip.Color(v, q, p);
}

void cycle(unsigned int offset, unsigned int s, unsigned int v, int currentPart) {
  unsigned int n;
  for (n = currentPart * nbLedInPart; n < (currentPart * nbLedInPart) + nbLedInPart; n++)
    strip.setPixelColor(n, hsvToColour(n * multiple + offset, s, v));
  strip.show();
}

void resetAnimValues1() {
  ledIndex1_Part1 = 0;
  ledIndex2_Part1 = nbLedInPart;
  lastFlash_Part1 = 0;
  SState_Part1 = RANDOM;
  nbDone_Part1 = 0;
  ledState_Part1 = false;
  timeStrob_Part1 = 0;
  activeTime1 = 0;
}

void resetAnimValues2() {
  ledIndex1_Part2 = nbLedInPart + 1;
  ledIndex2_Part2 = nbLedInPart * 2;
  lastFlash_Part2 = 0;
  SState_Part2 = RANDOM;
  nbDone_Part2 = 0;
  ledState_Part2 = false;
  timeStrob_Part2 = 0;
  activeTime2 = 0;
}

void resetAnimValues3() {
  ledIndex1_Part3 = (nbLedInPart * 2) + 1;
  ledIndex2_Part3 = 116;
  lastFlash_Part3 = 0;
  SState_Part3 = RANDOM;
  nbDone_Part3 = 0;
  ledState_Part3 = false;
  timeStrob_Part3 = 0;
  activeTime3 = 0;
}

void setLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(SETUP_LED_OFF_DURATION);
}

void setLedOn(unsigned long duration) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(duration);
}