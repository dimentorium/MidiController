/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io
This example allows you to create a slider from an arbitrary number of pads
which can be in any order. This example uses a Trill Flex sensor with a
custom flexible sensor with 3 sliders next to each other. See
learn.bela.io/flex-tutorial/ for more info on the sensor.
Trill Flex has 30 capactive pads in total and we are splitting this
into 3 groups of 10 pads. The order of the pads and their pin numbering is
defined in slider0Pads etc. We can also set the max number of centroid
which will define how many touches can be registered per slider. This
is currently set to 3 meaning that 3 individual touch points can be registered
per sensor.
Each touch has a location and a touch size which equates to how hard the finger
is pushing on the sensor. This example is particularly useful for working with
Trill Flex and Trill Craft. When working with these sensors it always important
to check that the Prescaler and Noisethreshold settings are optimum for your
application. Experiment with different values if you are not getting a reading
or seeing lots of cross talk between the sensors.
*/

#include <Trill.h>

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

const unsigned int maxNumCentroids = 4;
const unsigned int numSliders = 4;
CustomSlider sliders[numSliders];

void setup() {    
  sliders[0].setup(slider0Pads, slider0NumPads);
  sliders[1].setup(slider1Pads, slider1NumPads);
  sliders[2].setup(slider2Pads, slider2NumPads);
  sliders[3].setup(slider3Pads, slider3NumPads);
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
  // We recommend a prescaler value of 4
  trillSensor.setPrescaler(4);
  // Experiment with this value to avoid corss talk between sliders if they are position close together
  trillSensor.setNoiseThreshold(300);
}

void loop() {
  // Read 20 times per second
  delay(50);
  if(!trillSensor.requestRawData()) {
    Serial.println("Failed reading from device. Is it disconnected?");
    return setup();
  }
  unsigned n = 0;
  // read all the data from the device into a local buffer
  while(trillSensor.rawDataAvailable() > 0 && n < NUM_TOTAL_PADS) {
    rawData[n++] = trillSensor.rawDataRead();
  }
  for(uint8_t n = 0; n < numSliders; ++n) {
    // have each custom slider process the raw data into touches
    sliders[n].process(rawData);
    Serial.print("| s");
    Serial.print(n);
    Serial.print("[");
    Serial.print(sliders[n].getNumTouches());
    Serial.print("]: ");
    if(sliders[n].getNumTouches() > 0) {
      for(int i = 0; i < sliders[n].getNumTouches(); i++) {
          Serial.print(sliders[n].touchLocation(i));
          Serial.print(" ");
          Serial.print(sliders[n].touchSize(i));
          Serial.print(" ");
      }
    }
  }
  Serial.println("");
}
