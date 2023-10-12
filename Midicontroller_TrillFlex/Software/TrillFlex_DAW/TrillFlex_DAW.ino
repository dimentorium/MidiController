#include <Adafruit_NeoPixel.h>
#include <Trill.h>
#include "MIDIUSB.h"

Trill trillSensor;

const unsigned int NUM_TOTAL_PADS = 30;
CustomSlider::WORD rawData[NUM_TOTAL_PADS];

const uint8_t slider0NumPads = 6;
const uint8_t slider1NumPads = 6;
const uint8_t slider2NumPads = 6;
const uint8_t slider3NumPads = 6;

// Order of the pads used by each slider
uint8_t slider0Pads[slider0NumPads] = {5, 4, 3, 2, 1, 0};
uint8_t slider1Pads[slider1NumPads] = {11, 10, 9, 8, 7, 6};
uint8_t slider2Pads[slider2NumPads] = {17, 16, 15, 14, 13, 12};
uint8_t slider3Pads[slider3NumPads] = {23, 22, 21, 20, 19, 18};

int buttonStates[6];

const unsigned int maxNumCentroids = 4;
const unsigned int numSliders = 4;
CustomSlider sliders[numSliders];

//define led strips
#define NUMPIXELS      6
int color = 80;
Adafruit_NeoPixel fd1 = Adafruit_NeoPixel(NUMPIXELS, 7, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd2 = Adafruit_NeoPixel(NUMPIXELS, 8, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd3 = Adafruit_NeoPixel(NUMPIXELS, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel fd4 = Adafruit_NeoPixel(NUMPIXELS, 10, NEO_GRB + NEO_KHZ800);

//structure for each fader data
struct Channel{
  int ccchannel;
  int readval;
  int sendedval;
  Adafruit_NeoPixel ledpin;
};

//initialize array of values for faders
Channel controls[4];

void setup() {
  sliders[0].setup(slider0Pads, slider0NumPads);
  sliders[1].setup(slider1Pads, slider1NumPads);
  sliders[2].setup(slider2Pads, slider2NumPads);
  sliders[3].setup(slider3Pads, slider3NumPads);
  
  controls[0].ccchannel = 11; //Expression
  controls[0].readval = 0;
  controls[0].sendedval = 0;
  fd1.begin();
  controls[0].ledpin = fd1;

  controls[1].ccchannel = 1; //Expression
  controls[1].readval = 0;
  controls[1].sendedval = 0;
  fd2.begin();
  controls[1].ledpin = fd2;

  controls[2].ccchannel = 21; //Expression
  controls[2].readval = 0;
  controls[2].sendedval = 0;
  fd3.begin();
  controls[2].ledpin = fd3;

  controls[3].ccchannel = 102; //??
  controls[3].readval = 0;
  controls[3].sendedval = 0;
  fd4.begin();
  controls[3].ledpin = fd4;

  
  // Initialise serial and touch sensor
  Serial.begin(115200);
  Serial.println(ARDUINO);
  int ret;
  while(trillSensor.setup(Trill::TRILL_FLEX)) {
    Serial.println("failed to initialise trillSensor");
    Serial.println("Retrying...");
    delay(100);
  }
  Serial.println("Success initialising trillSensor");
  
  trillSensor.setMode(Trill::DIFF);
  trillSensor.setScanSettings(0, 16);
  // We recommend a prescaler value of 4
  trillSensor.setPrescaler(5);
  // Experiment with this value to avoid corss talk between sliders if they are position close together
  trillSensor.setNoiseThreshold(150);

}

unsigned long starttime = 0;
unsigned long looptime = 0;

void loop() {
  starttime = millis();

  //receive midi packets
  midiEventPacket_t rx;
  do{
      rx = MidiUSB.read();
  }while (rx.header != 0);                                      //read buffer empty

  updateFaders();
  updateLEDs();  
  sendMidi();

  looptime = millis() - starttime;
  //Serial.println(looptime);
  if(looptime < 100){
    delay(50-looptime);
  }
  else
  {
    delay(50);
  }
}

void updateFaders(){
  unsigned n = 0;
  if(!trillSensor.requestRawData()) {
    Serial.println("Failed reading from device. Is it disconnected?");
    return setup();
  }
  while(trillSensor.rawDataAvailable() > 0 && n < NUM_TOTAL_PADS) {
    rawData[n++] = trillSensor.rawDataRead();
  }
  
  for(uint8_t n = 0; n < numSliders; ++n) 
  {
    // have each custom slider process the raw data into touches
    sliders[n].process(rawData);
    if(sliders[n].getNumTouches() > 0) 
    {
      if(sliders[n].touchSize(0) > 50)
      {
        float rawvalue = (sliders[n].touchLocation(0) / 4.4) - 10;
        controls[n].readval = constrain(rawvalue, 0, 127);
        //Serial.print(controls[n].readval);
        //Serial.print("");
      }
    }
  }
  //Serial.println("");

  for(uint8_t n = 0; n < 6; ++n) {
    //Button 0, top left, mid left, bottom left, top right, mid right, bottom right
    if(rawData[24 + n] > 300 && buttonStates[n] == 0){
      buttonStates[n] = 1;
      byte ch = 1;
      controlChange(ch, 107 + n, 127);
    }
    else if(rawData[24 + n] > 300 && buttonStates[n] == 1){
      //do nothing
    }
    else
    {
      buttonStates[n] = 0;
    }
  }
}

void updateLEDs(){
  for(uint8_t i = 0; i < numSliders; ++i) 
  {
    if(controls[i].readval != controls[i].sendedval) {
      
      for(int j=0;j<NUMPIXELS;j++)
      {
        if(controls[i].readval >= ((j+1) * 18))
        {
          controls[i].ledpin.setPixelColor(j, color, 0, 0);
        }
        else
        {
          controls[i].ledpin.setPixelColor(j, 0, 0, 0);
        }
        controls[i].ledpin.show(); // This sends the updated pixel color to the hardware.
      }
      
    }
  }
}

void sendMidi(){
  for(uint8_t i = 0; i < numSliders; ++i) 
  {
    if(controls[i].readval != controls[i].sendedval)
    {
      controlChange(0, controls[i].ccchannel, controls[i].readval);
      controls[i].sendedval = controls[i].readval;
    }
  }
  MidiUSB.flush();
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
