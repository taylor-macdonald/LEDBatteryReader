#include "Particle.h"
#include "neopixel.h"
#include "inttypes.h"
#include "stdlib.h"
#include "SPI.h"
#include "ARD1939.h"

ARD1939 j1939;

int nCounter = 0;

SYSTEM_MODE(AUTOMATIC);


#define PIXEL_PIN A3
#define PIXEL_COUNT 24 //number of LEDs
#define BLOCK_COUNT 8 //number of sections
#define PIXEL_TYPE WS2812B
#define LIGHT_DELAY 20 //ms delay before lights change
#define FADE 1 //multiplier for fade (0-1). 1 is full intensity.
#define CHARGING_LED_QTY 1 //how many more LEDs to light up to indicate charging
#define PUBLISHING false //true: publish

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void setup() {
  Particle.publish("PTO1","Device initialized");
  strip.begin();
  strip.show();
}

void loop() {

  // boot lights
  bootLights();
  delay(1000);
  int8_t i;

  // test depletion
  for (i=0; i<=100; i=i+4){
    setBatteryBlock(100-i);
    delay(250);
  }

  //test error
  for (i=0; i<=5; i++){
    errorLights();
  }

  //test charging
  for (i=0; i<=100; i=i+4){
    charging(i);
  } 

  //test fully charged
  for (i=0; i<=5; i++){
    fullyCharged();
  }
}

void errorLights(){ // red light then no light
  uint16_t i;
  for (i=0;i<PIXEL_COUNT;i++){ // all pixels red
    strip.setPixelColor(i,255,0,0);
  }
  strip.show();//shows changes on LED strip
  delay(500);
  
  for (i=0;i<PIXEL_COUNT;i++){ // all pixels off
    strip.setPixelColor(i,0,0,0);
  }
  strip.show();//shows changes on LED strip
  delay(500);
}

void charging(uint8_t percentage){ //pass in a percentage (0-100) that the battery is currently at (when charging).
  if (PUBLISHING == true){
    Particle.publish("PTO1","Battery charged to " + String(percentage) + " percent");
  }
  setBatteryBlock(percentage);//calls setBatteryPercentage function
  delay(250);
  setBatteryBlock(percentage + CHARGING_LED_QTY * 100/BLOCK_COUNT);//calls setBatteryPercentage function with enough additional percent to activate more LEDs
  delay(250);
}

void setBatteryPercentage(uint8_t percentage){ //DEFUNCT: Superseded by Block function - pass in a percentage (0-100) that the battery is current at (not charging) 
  if (PUBLISHING == true){
    Particle.publish("PTO1","Battery set to " + String(percentage) + " percent");
  }
  uint16_t i;
  for (i=0; i<PIXEL_COUNT; i++){
    if (i<=PIXEL_COUNT * percentage/100){
      if (percentage <= 33){ // Low battery: set red
        strip.setPixelColor(i,255*FADE,0,0);
      }
      else if (percentage <= 66){ // set orange
        strip.setPixelColor(i,255*FADE,127*FADE,0);
      }
      else{ //set green if batter is above 66%
        strip.setPixelColor(i,0,255*FADE,0,0);
      }
    }
    else{ //sets the rest of the LEDs (if there are any) to off
      strip.setPixelColor(i,0,0,0);
    }
  }
  strip.show(); //shows changes on LED strip
} 

void bootLights(){
  /* ROTATE RING OF COLOURS*/
  if (PUBLISHING == true){
    Particle.publish("PTO1","Boot Sequence");
  }
  uint16_t i;
  for (i=0; i<PIXEL_COUNT;i++){ //red loop
    strip.setPixelColor(i, 255,0,0);
    delay(LIGHT_DELAY);
    strip.show();
  }  
  for (i=0; i<PIXEL_COUNT;i++){ //blue loop
    strip.setPixelColor(i, 0,0,255);
    delay(LIGHT_DELAY);
    strip.show();
  }
  for (i=0; i<PIXEL_COUNT;i++){ // green loop
    strip.setPixelColor(i, 0,255,0);
    delay(LIGHT_DELAY);
    strip.show();
  }
  for (i=0; i<PIXEL_COUNT;i++){ //yellow loop
    strip.setPixelColor(i,255,255,0);
    delay(LIGHT_DELAY);
    strip.show();
  }
  for (i=0; i<PIXEL_COUNT;i++){ //yellow loop
    strip.setPixelColor(i,0,0,0);
    delay(LIGHT_DELAY);
    strip.show();
  }
}

void setBatteryBlock(uint8_t percentage){ //pass in a percentage (0-100) that the battery is current at (not charging)
  if (PUBLISHING == true){ //publish event if enabled
    Particle.publish("PTO1","Battery set to " + String(percentage) + " percent");
  }
  uint16_t j, i;
  int LED_GROUPS = PIXEL_COUNT / BLOCK_COUNT; //LED GROUPS is how many LEDs are in each block. It is how many need to be activated each time
  for (j=0; j < BLOCK_COUNT; j++){ //repeat for each block
    if(j<=BLOCK_COUNT * percentage/100){ //converts percentage to block quantity
      if (percentage < 25){ // sets LEDs to Red if battery is below 25%
        for (i=0; i<LED_GROUPS; i++){
          strip.setPixelColor(j*LED_GROUPS+i,255*FADE,0,0); //j is the groupnumber, i is the LED in that group
        }
      }
      else if (percentage < 50){ // sets LEDs to Amber if battery is below 50%
        for (i=0; i<LED_GROUPS; i++){
          strip.setPixelColor(j*LED_GROUPS+i,255*FADE,127*FADE,0);
        }
      }
      else{ //set green if batter is above 50%
        for (i=0; i<LED_GROUPS; i++){ // sets groups of LEDs to Green if battery is above 50%
          strip.setPixelColor(j*LED_GROUPS+i,0,255*FADE,0,0);
        }
      }
    }
    else{
      for (i=0; i<LED_GROUPS; i++){ // turns the remaining groups of LEDs off
        strip.setPixelColor(j*LED_GROUPS+i,0,0,0); 
      }
    }
  }
  strip.show();//shows changes on LED strip
} 

void fullyCharged(){ // Green light then dim green light
  uint16_t i;
  for (i=0;i<PIXEL_COUNT;i++){ //set all pixels off
    strip.setPixelColor(i,0,100,0);
  }
  strip.show();//shows changes on LED strip
  delay(500);

  for (i=0;i<PIXEL_COUNT;i++){ //set all pixels green
    strip.setPixelColor(i,0,250,0);
  }
  strip.show();//shows changes on LED strip
  delay(500);
}
