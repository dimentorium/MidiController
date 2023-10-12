#include <Trill.h>
#include <Adafruit_NeoPixel.h>
//#include <avr/power.h>
#include "MIDIUSB.h"


//Define trill objects
Trill trillbar0;
Trill trillbar1;
Trill trillbar2;
Trill trillsquare;
int button = 0;
int lastbutton = 0;
int debounce = 0;

//define led strips
#define NUMPIXELS      14
int color = 20;
Adafruit_NeoPixel fd1 = Adafruit_NeoPixel(NUMPIXELS, 11, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd2 = Adafruit_NeoPixel(NUMPIXELS, 10, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd3 = Adafruit_NeoPixel(NUMPIXELS, 9, NEO_GRB + NEO_KHZ800);

//structure for each fader data
struct Channel{
  Trill trill_bar;
  int ccchannel;
  int readval;
  int sendedval;
  Adafruit_NeoPixel ledpin;
};

//initialize array of values for faders
Channel controls[3];

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  Serial.println("Init");
  
  controls[0].trill_bar = trillbar0;
  controls[0].ccchannel = 11; //Expression
  controls[0].readval = 0;
  controls[0].sendedval = 0;
  fd1.begin();
  controls[0].ledpin = fd1;

  controls[1].trill_bar = trillbar1;
  controls[1].ccchannel = 1; //Expression
  controls[1].readval = 0;
  controls[1].sendedval = 0;
  fd2.begin();
  controls[1].ledpin = fd2;

  controls[2].trill_bar = trillbar2;
  controls[2].ccchannel = 21; //Expression
  controls[2].readval = 0;
  controls[2].sendedval = 0;
  fd3.begin();
  controls[2].ledpin = fd3;
  
  int ret = 0;
  controls[0].trill_bar.setup(Trill::TRILL_BAR, 0x20  );
  ret = controls[1].trill_bar.setup(Trill::TRILL_BAR, 0x21  );
  controls[2].trill_bar.setup(Trill::TRILL_BAR, 0x22  );
  trillsquare.setup(Trill::TRILL_SQUARE);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
  }
}

unsigned long starttime = 0;
unsigned long looptime = 0;

void loop() {
  
  // Read 10 times per second
  //delay(100);
  starttime = millis();

  //loop over all three faders
  for(int i=0; i<3; i++){
    //read value from fader
    controls[i].trill_bar.read();

    //if one or more touches found
    if(controls[i].trill_bar.getNumTouches() > 0) {
      //for(int j = 0; j < controls[i].trill_bar.getNumTouches(); j++) {
        
        int raw = controls[i].trill_bar.touchLocation(0);       //read touch value as raw data
        int constrained = (3100 - raw)/23;                      //calculate midi value
        controls[i].readval = constrain(constrained, 0, 127);
      //}
    }
    
    if(controls[i].readval != controls[i].sendedval) {
      for(int j=0;j<NUMPIXELS;j++)
      {
        if(controls[i].readval >= ((NUMPIXELS-j)* 9))
        {
          controls[i].ledpin.setPixelColor(j, 0, 0, 20);
        }
        else
        {
          controls[i].ledpin.setPixelColor(j, 0, 0, 0);
        }
        controls[i].ledpin.show(); // This sends the updated pixel color to the hardware.
      }
      controlChange(0, controls[i].ccchannel, controls[i].readval);
      controls[i].sendedval = controls[i].readval;
    }
    
  }
  /*
  Serial.print(controls[0].readval);
  Serial.print(" ");
  Serial.print(controls[1].readval);
  Serial.print(" ");
  Serial.print(controls[2].readval);
  Serial.println("");
  */

  trillsquare.read();

  if(trillsquare.getNumHorizontalTouches() > 0) {
    int horizontal = 2 - trillsquare.touchHorizontalLocation(0) / 580;
    int vertical = 2 - trillsquare.touchLocation(0) / 580;
    
    if(horizontal == 0 && vertical == 0){
      button = 1;
    }
    else if(horizontal == 1 && vertical == 0){
      button = 2;
    }
    else if(horizontal == 2 && vertical == 0){
      button = 3;
    }
    else if(horizontal == 0 && vertical == 1){
      button = 4;
    }
    else if(horizontal == 1 && vertical == 1){
      button = 5;
    }
    else if(horizontal == 2 && vertical == 1){
      button = 6;
    }
    else if(horizontal == 0 && vertical == 2){
      button = 7;
    }
    else if(horizontal == 1 && vertical == 2){
      button = 8;
    }
    else if(horizontal == 2 && vertical == 2){
      button = 9;
    }
    else {
      button = 0;
    }

    if(lastbutton != button){
      lastbutton = button;
      debounce = 0;
    }
    else{
      debounce++;
      if(debounce == 2){
        if(button == 2){
          byte ch = 1;
          controlChange(ch, 107, 127);
        }
        //Serial.print(button);
        //Serial.println("");
        lastbutton = 0;
        button = 0;
      }
    }
  }
  
  
  MidiUSB.flush();
  looptime = millis() - starttime;
  if(looptime < 100){
    delay(100-looptime);
  }
  else
  {
    delay(50);
  }
  //Serial.print(looptime);
  //Serial.println("");
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
