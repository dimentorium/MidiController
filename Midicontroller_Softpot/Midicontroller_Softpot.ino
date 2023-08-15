#include "MIDIUSB.h"
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

//Constants that can be changed
int MIDICHANNEL = 15;   //Midi channel used, this is Midichannel 16, as it starts with 0
int CC1 = 20;           //CC Message for Fader 1
int CC2 = 21;           //CC Message for Fader 2
int CC3 = 22;           //CC Message for Fader 3

#define PIN            11
#define NUMPIXELS      15
int color = 60;
Adafruit_NeoPixel fd1 = Adafruit_NeoPixel(NUMPIXELS, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd2 = Adafruit_NeoPixel(NUMPIXELS, 10, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd3 = Adafruit_NeoPixel(NUMPIXELS, 11, NEO_GRB + NEO_KHZ800);

struct Fader{
  int pin;
  int ccchannel;
  int readval;
  int lastval;
  int lastsended;
  int debounce;
  unsigned long updatetime; 
  Adafruit_NeoPixel ledpin;
};

Fader softpots[3];
int led = 0;

void setup(){
  Serial.begin(9600);
  delay(2000);
  Serial.println("Init");

  softpots[0].pin = A0;
  softpots[0].ccchannel = CC1;
  softpots[0].readval = 0;
  softpots[0].lastval = 0;
  softpots[0].lastsended = 0;
  softpots[0].debounce = 0;
  softpots[0].updatetime = 0;

  softpots[1].pin = A1;
  softpots[1].ccchannel = CC2;
  softpots[1].readval = 0;
  softpots[1].lastval = 0;
  softpots[1].lastsended = 0;
  softpots[1].debounce = 0;
  softpots[1].updatetime = 0;

  softpots[2].pin = A2;
  softpots[2].ccchannel = CC3;
  softpots[2].readval = 0;
  softpots[2].lastval = 0;
  softpots[2].lastsended = 0;
  softpots[2].debounce = 0;
  softpots[2].updatetime = 0;

   //enable pullup resistor for softpot pins
  digitalWrite(softpots[0].pin, HIGH);
  digitalWrite(softpots[1].pin, HIGH);
  digitalWrite(softpots[2].pin, HIGH);
  
  pinMode(LED_BUILTIN, OUTPUT);
  
  fd1.begin();
  fd2.begin();
  fd3.begin();

  softpots[0].ledpin = fd1;
  softpots[1].ledpin = fd2;
  softpots[2].ledpin = fd3;
  
  Serial.println("Ready");
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}


void loop(){
  led = 0;

  //receive midi packets
  midiEventPacket_t rx;
  do{                                             //read buffer empty
    rx = MidiUSB.read();
    if (rx.header != 0) {                         //some header is available
      int channel = rx.byte1-176;                 //midi channel starting with 0, -176 for masking last four bits
      int ccnum = rx.byte2;
      int value = rx.byte3;
      if(channel == MIDICHANNEL)
      {
        if(ccnum == softpots[0].ccchannel) {
          softpots[0].lastval = value;
          Serial.println(value);
        }
        if(ccnum == softpots[1].ccchannel) {
          softpots[1].lastval = value;
        }
        if(ccnum == softpots[2].ccchannel) {
          softpots[2].lastval = value;
        }
      }
    }
  }while (rx.header != 0); 

  //read analog inputs
  for(int i=0; i<3; i++){
    int softpotReading = analogRead(softpots[i].pin);
    if( softpotReading < 889)
    {
      //Serial.println(i);
      int constrained = (889 - softpotReading)/7; 
      softpots[i].readval = constrain(constrained, 0, 127);
      int difference = abs(softpots[i].readval-softpots[i].lastval);
      if(difference == 0)
      {
        softpots[i].debounce = softpots[i].debounce + 1;
        if(softpots[i].debounce > 4)
        {
          softpots[i].debounce = 0;
          if(softpots[i].lastval != softpots[i].lastsended){
            led = 1;
            controlChange(MIDICHANNEL, softpots[i].ccchannel, softpots[i].lastval);
            softpots[i].lastsended = softpots[i].lastval;
          }
        }
      }
      else
      {
        softpots[i].lastval = softpots[i].readval;
        softpots[i].debounce = 0;
      }
    }
    else
    {
      softpots[i].debounce = 0;
    }

    //update LEDs every 20 ms
    if((millis() - softpots[i].updatetime) > 40){
      for(int j=0;j<NUMPIXELS;j++)
      {
        if(softpots[i].lastval >= ((NUMPIXELS-j-1)* 9))
        {
          softpots[i].ledpin.setPixelColor(j, 0, 0, color);
        }
        else
        {
          softpots[i].ledpin.setPixelColor(j, 0, 0, 0);
        }
      }
      softpots[i].ledpin.show(); // This sends the updated pixel color to the hardware.
      softpots[i].updatetime = millis();
    }
  }
  MidiUSB.flush();  //Send Midi commands
  digitalWrite(LED_BUILTIN, led);
  //delay(1); //just here to slow down the output for easier reading
}
