/* This example shows basic usage of the NeoTrellis.
  The buttons will light up various colors when pressed.
  The interrupt pin is not used in this example.
*/

#include "Adafruit_NeoTrellis.h"
#include "MIDIUSB.h"

Adafruit_NeoTrellis trellis;

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  // Check is the pad pressed?
  int converted = (evt.bit.NUM / 4) * 4 + (3 - (evt.bit.NUM % 4));
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    trellis.pixels.setPixelColor(evt.bit.NUM, Wheel(map(evt.bit.NUM, 0, trellis.pixels.numPixels(), 0, 255))); //on rising
    Serial.println(evt.bit.NUM);
    
    Serial.println(converted);
    controlChange(0, 36 + converted, 127);
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
  // or is the pad released?
    trellis.pixels.setPixelColor(evt.bit.NUM, Wheel_off(map(evt.bit.NUM, 0, trellis.pixels.numPixels(), 0, 255))); //off falling
    controlChange(0, 36 + converted, 0);
  }

  // Turn on/off the neopixels!
  trellis.pixels.show();

  return 0;
}

void controlChange(byte channel, byte control, byte value) {
  //midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  midiEventPacket_t event = {0x09, 0x90 | channel, control, value};     //Note on
  if(value == 0) {
    event = {0x08, 0x80 | channel, control, 127};     //Note on
  }
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void setup() {
  Serial.begin(9600);
  // while(!Serial) delay(1);
  
  if (!trellis.begin()) {
    Serial.println("Could not start trellis, check wiring?");
    while(1) delay(1);
  } else {
    Serial.println("NeoPixel Trellis started");
  }

  //activate all keys and set callbacks
  for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
  }

  //do a little animation to show we're on
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, Wheel(map(i, 0, trellis.pixels.numPixels(), 0, 255)));
    trellis.pixels.show();
    delay(50);
  }
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i, Wheel_off(map(i, 0, trellis.pixels.numPixels(), 0, 255)));
    trellis.pixels.show();
    delay(50);
  }
}

void loop() {
  trellis.read();  // interrupt management does all the work! :)
  
  delay(20); //the trellis has a resolution of around 60hz
}


/******************************************/

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return trellis.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return trellis.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return trellis.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  return 0;
}

uint32_t Wheel_off(byte WheelPos) {
  if(WheelPos < 85) {
   return trellis.pixels.Color(WheelPos * 3/10, (255 - WheelPos * 3)/10, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return trellis.pixels.Color((255 - WheelPos * 3)/10, 0, WheelPos * 3/10);
  } else {
   WheelPos -= 170;
   return trellis.pixels.Color(0, WheelPos * 3/10, (255 - WheelPos * 3)/10);
  }
  return 0;
}
