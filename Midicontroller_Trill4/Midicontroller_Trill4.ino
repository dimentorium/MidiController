/**
  Midicontroller Trill 4
  Name: Midicontroller_Trill4.ino
  Purpose: Main file for running source code on controller with 4 trill bars and 6 buttons

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/


//Libraries that need to be installed
#include <Trill.h>
#include <Adafruit_NeoPixel.h>

//Project libraries
#include "Button.h"
#include "Midi.h"
#include "Looptimer.h"
#include "Trillbar.h"
#include "LED.h"
#include "State.h"

//Define system state
Controls controls;

//define looptimer object
LoopTimer LT;

//initialize array of values for trill faders
Trill_Fader faders[4];

//define Buttons
PushButton button0;
PushButton button1;
PushButton button2;
PushButton button3;
PushButton button4;
PushButton button5;

//define led strips
#define NUMPIXELS      14
LedStrip ledstrips[4];
bool update_leds;                      //flag to force update of LEDs

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(9600);
  delay(2000);                        //small delay, useful to make sure midi and serial is initialized
  Serial.println("Init");

  controls.init();

  //Initialize push buttons
  button0.init(6);
  button1.init(9);
  button2.init(10);
  button3.init(11);
  button4.init(12);
  button5.init(13);

  //Initialize Trill bars
  faders[0].init(0x20);
  faders[1].init(0x21);
  faders[2].init(0x22);
  faders[3].init(0x23);

  //Initialize LED Strips
  ledstrips[0].init(16, NUMPIXELS);
  ledstrips[1].init(17, NUMPIXELS);
  ledstrips[2].init(18, NUMPIXELS);
  ledstrips[3].init(19, NUMPIXELS);
  update_leds = true;
  Serial.println("Starting V0.30");
}

void loop() 
{  
  LT.start(25);   //init looptimer with 25ms looptime
  
  //receive midi packets
  midiEventPacket_t rx;
  do{
      rx = MidiUSB.read();
      if (rx.header != 0) {                                     //some header is available
        controls.receive_cc(rx.byte1-176, rx.byte2, rx.byte3);  //send message to main statemachine, -176 masks last four bits
      }
  }while (rx.header != 0);                                      //read buffer empty

  //Read all button values and send command for transport bar
  if(button0.get_state()) {controls.Transport(Stop);}
  if(button1.get_state()) {controls.Transport(Cycle);}
  if(button2.get_state()) {controls.Transport(Rewind);}
  if(button3.get_state()) {controls.Transport(Start);}
  if(button4.get_state()) {controls.Transport(Record);}
  if(button5.get_state()) {controls.Transport(Forward);}

  //loop over all four faders
  for(int i=0; i<4; i++){
    TouchState faderstate = faders[i].get_state();                  //read value from fader

    switch (faderstate)                                             //if one or more touches found
    {
      case Primary:
        controls.update_control(i, 0, faders[i].get_value());       //send value from bar to main state machine
        ledstrips[i].SetBar(controls.get_control(i, 0));            //update led bar with new values
        break;
      case DoubleTap:
        controls.button(i, faders[i].get_value());
        break;

      case Secondary:
        controls.update_control(i, 1, faders[i].get_value());
        ledstrips[i].SetBar(controls.get_control(i, 0), true);
        ledstrips[i].SetPoint(controls.get_control(i, 1));
        break;

      case BankSwitch:
        controls.switch_bank(i*2 + faders[i].get_value());
        update_leds = true;
        break;

      case ModeSwitch:
        controls.switch_mode(i);
        update_leds = true;
        break;

      case none:
      default:
        ledstrips[i].SetBar(controls.get_control(i, 0));
        break;
    }
  }

  //Update LEDs if needed
  if(update_leds){
      for(int i=0; i<4; i++){
          ledstrips[i].SetBar(controls.get_control(i, 0), true);
      }
      update_leds = false;
  }

  MidiUSB.flush();  //Send Midi commands
  LT.evaluate();    //Check looptime for delay
}
