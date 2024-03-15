#include <ArduinoJson.h>
#include "LPD8806.h"
#include <Watchdog.h>

#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h> // Not actually used but needed to compile
#endif

#define DEBUG 1

RH_ASK driver(4000, 4, 4);

//watchdog watches if the cpu is frozen
//Watchdog watchdog;
const unsigned long SETUP_LED_ON_IF_NOT_TRIPPED_DURATION = 4000;
const unsigned long SETUP_LED_ON_IF_TRIPPED_DURATION = 1500;
const unsigned long SETUP_LED_OFF_DURATION = 1000;

const int nLEDs = 116;
int nbLedInPart = 39;

//create LPD8806 instance
int dataPin = 2;
int clockPin = 3;
LPD8806 strip = LPD8806(nLEDs, dataPin, clockPin);

unsigned long lt1 = 0;
unsigned long lt2 = 0;
unsigned long lt3 = 0;
//sorry for the global variables, but it's the only way I make it work
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
//-----knight_rider variables-----------
int ledIndex1_Part1 = 0;
int ledIndex2_Part1 = nbLedInPart;
int ledIndex1_Part2 = nbLedInPart + 1;
int ledIndex2_Part2 = nbLedInPart * 2;
int ledIndex1_Part3 = (nbLedInPart * 2) + 1;
int ledIndex2_Part3 = 116;
//-------Flash variables----------------
int lastFlash_Part1 = 0;
int lastFlash_Part2 = 0;
int lastFlash_Part3 = 0;
//-------Splash variables---------------
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
//---------Rainbow variables--------------
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
  //watchdog setup
  pinMode(LED_BUILTIN, OUTPUT);
  // if (!watchdog.tripped()) {
  //   // blink once to indicate power cycle reset
  //   setLedOn(SETUP_LED_ON_IF_NOT_TRIPPED_DURATION);
  //   setLedOff();
  // } else {
  //   // blink twice to indicate watchdog tripped reset
  //   for (int i = 0; i < 10; i++) {
  //     setLedOn(500);
  //     digitalWrite(LED_BUILTIN, 500);
  //   }
  // }
  // watchdog.enable(Watchdog::TIMEOUT_1S);

  if (!driver.init()) {
    Serial.println("init failed");
  }

  //initialize the strip
  strip.begin();
  strip.show();
  //initialize the random seed
  randomSeed(analogRead(0));

  // //initialize the virtual wire
  // vw_set_rx_pin(4);  //connect the receiver data pin to pin 12
  // vw_setup(4000);    // speed of data transfer in bps, maxes out at 10000
  // vw_rx_start();     // Start the receiver PLL running
}

void loop() {
  //watchdog reset
  //watchdog.reset();
  unsigned long ct = millis();
  //get the message
  JsonDocument msg = getMessage();
  String msgTest = "";
  //deserialize the message
  serializeJson(msg, msgTest);
  //if the message is not null
  if (msgTest != "null") {
    //print the message
    serializeJson(msg, Serial);
    Serial.println();
    //get the values of the message(JSON format)
    effet = msg["e"];
    part = msg["p"];

    //set the animation
    switch (part) {
      case 0:
        //activate the animation to mesure the time since it's activated
        active1 = true;
        //change the animation
        AnimPart1 = effet;
        //change the color
        color1 = msg["c"];
        //change the speed and convert it to milliseconds
        speedPart1 = msg["s"];
        speedPart1 = speedPart1 * 100;
        //if the speed is 0, set it to 50ms
        if (speedPart1 == 0) { speedPart1 = 50; }
        //reset the values of all the animation of the part
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
    //turn off the led in the part where the animation is changed or activated
    changeStrip(off, part);

#if DEBUG
    Serial.print(F("Effect : "));
    Serial.print(effet);
    Serial.print(F("\tPart : "));
    Serial.print(part);
    Serial.print(F("\tmsg[c] : "));
    Serial.print((uint32_t)msg["c"]);
    Serial.print(F("\tmsg[s] : "));
    Serial.println((int)msg["s"]);
#endif
  }
  //count the time since the animation is activated
  if (active1) {
    activeTime1++;
  }
  if (active2) {
    activeTime2++;
  }
  if (active3) {
    activeTime3++;
  }
  //play the animations at the right speed
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
  //if the time since the animation is activated is greater than the animation time, turn off the animation
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
  
  blinkTask(2000);
}

//reset the values of the animation in part 1
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

//reset the values of the animation in part 2
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

//reset the values of the animation in part 3
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

// turn off the LED for watchdog
void setLedOff() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(SETUP_LED_OFF_DURATION);
}

// turn on the LED for watchdog
void setLedOn(unsigned long duration) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(duration);
}

//get the message from the receiver
JsonDocument getMessage() {
  JsonDocument msg;
  uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);

  if (driver.recv(buf, &buflen))  // if we get a message that we recognize on this buffer...
  {
    driver.printBuffer("Got:", buf, buflen);
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

//get segment length for the splash animation
int getSegmentLength() {
  return random(minLength, maxLength);
}

//get the start of the segment for the splash animation
int getSegmentLength(int modMaxLength) {
  return random(minLength, modMaxLength);
}

//get the start of the segment for the splash animation if the segment is too long
int getSegmentStart(int currentPart) {
  int start = currentPart * nbLedInPart;
  return random(start, start + nbLedInPart);
}

//change the color of the strip in a part
void changeStrip(uint32_t color, int currentPart) {
  int start = currentPart * nbLedInPart;
  for (int i = start; i < start + nbLedInPart; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}
//----------------------------------------------
//-----------------ANIMATIONS-------------------
//----------------------------------------------

void knightRider(uint32_t currentColor, int currentPart) {
  //switch the part to animate
  switch (currentPart) {
    case 0:
      //set the color of the leds and the leds behind
      strip.setPixelColor(ledIndex2_Part1, currentColor);
      strip.setPixelColor(ledIndex2_Part1 + ledLength, off);
      strip.setPixelColor(ledIndex1_Part1, currentColor);
      strip.setPixelColor(ledIndex1_Part1 - ledLength, off);
      //if the leds are at the end of the part, reset the values
      if (ledIndex1_Part1 >= 1 * nbLedInPart) {
        ledIndex1_Part1 = 0;
        ledIndex2_Part1 = nbLedInPart;
      }
      //increment the leds position
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
        ledIndex1_Part3 = nbLedInPart * 2 + 1;
        ledIndex2_Part3 = 116;
      }
      ledIndex1_Part3++;
      ledIndex2_Part3--;
      break;
  }
  strip.show();
}

void oneTwo(uint32_t currentColor, int currentPart) {
  //decal is used to switch between odd and even leds
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
  //get a random led in the part
  int n = random(currentPart * nbLedInPart, (currentPart * nbLedInPart) + nbLedInPart);
  strip.setPixelColorRGB(n, currentColor);
  //turn off the last led that was on depending on the part
  switch (currentPart) {
    case 0:
      strip.setPixelColorRGB(lastFlash_Part1, off);
      lastFlash_Part1 = n;
      break;
    case 1:
      strip.setPixelColorRGB(lastFlash_Part2, off);
      lastFlash_Part2 = n;
      break;
    case 2:
      strip.setPixelColorRGB(lastFlash_Part3, off);
      lastFlash_Part3 = n;
      break;
  }
  strip.show();
}

void splash(uint32_t currentColor, int currentPart) {
  //switch the part to animate
  switch (currentPart) {
    case 0:
      //switch the state of the animation
      switch (SState_Part1) {
        case RANDOM:
          //get the start of the segment and the length of the segment
          static int start1;
          start1 = getSegmentStart(currentPart);
          static int segmentLength;
          //if the segment is too long, change the maximum length of the segment
          if (((currentPart + 1) * nbLedInPart) - start1 < maxLength) {
            segmentLength = getSegmentLength(((currentPart + 1) * nbLedInPart) - start1);
          } else {
            segmentLength = getSegmentLength();
          }
          //set the color of the leds in the segment
          for (int i = start1; i < start1 + segmentLength; i++) {
            strip.setPixelColor(i, currentColor);
          }
          strip.show();
          //count the number of leds that are on
          for (int i = currentPart * nbLedInPart; i < (currentPart * nbLedInPart) + nbLedInPart; i++) {
            if (strip.getPixelColor(i) != off && strip.getPixelColor(i) != 0) {
              nbDone_Part1++;
            }
          }
          //if all the leds are on, switch to the strob state
          if (nbDone_Part1 >= nbLedInPart - 1) {
            SState_Part1 = STROB;
          }
          nbDone_Part1 = 0;
          break;
        case STROB:

          //turn on and off the leds
          if (ledState_Part1) {
            changeStrip(currentColor, currentPart);
            ledState_Part1 = !ledState_Part1;
          } else {
            changeStrip(off, currentPart);
            ledState_Part1 = !ledState_Part1;
          }
          //count the times the strob is done
          timeStrob_Part1++;
          //if the strob is done the number of times set, switch to the clear state
          if (timeStrob_Part1 >= noStrob) {
            SState_Part1 = CLEAR;
            timeStrob_Part1 = 0;
          }
          break;
        case CLEAR:
          //turn off the leds
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
  // Cycle through the LEDs in the strip, setting the colour of each
  for (n = currentPart * nbLedInPart; n < (currentPart * nbLedInPart) + nbLedInPart; n++)
    strip.setPixelColor(n, hsvToColour(n * multiple + offset, s, v));
  strip.show();
}

void heartBeatTask() {
  static unsigned long lastTime = 0;
  static int rate = 5;
  static int fade = 0;

  if (millis() - lastTime > rate) {

    lastTime = millis();

    analogWrite(LED_BUILTIN, fade);

    if (fade < 0 || fade > 250 ) {
      rate = -rate;
    }
    fade += rate;

#if DEBUG
    Serial.print(millis());
    Serial.print(" : Fade values : ");
    Serial.print(fade);
    Serial.print(" -> ");
    Serial.println(rate);
    delay(10);
#endif
  }
}

void blinkTask(int rate) {
  static unsigned long lastTime = 0;
  static int ledState = 0;

  if (millis() - lastTime < rate) return;

  lastTime = millis();

  digitalWrite(LED_BUILTIN, ledState);
  ledState = !ledState;

  #if DEBUG
    Serial.print(millis());
    Serial.print(" : Blinking : ");
    Serial.println(ledState);
    delay(10);
#endif
}
