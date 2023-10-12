#include <Encoder.h>      //Library for encoder
#include "MIDIUSB.h"      //Library for Midi over USB

//Constants to be changed for Midi Communication
int MidiChannel = 0;      // 0-15, 0 is Midi Channel 1
int MidiCC = 7;           // CC Message to be used

//Pin definitions
const int CLK = 6;        //CLK Pin from Encoder 
const int DT = 5;         //Data Pin from Encoder
const int SW = 2;         //Switch Pin when encoder is pressed. Always high, low on press

//Helper signals for encoder
Encoder VolumeEncoder(DT,CLK);  //Definition of Encoder with Pins
long CurrentValue = 0;        //Last read value from encoder
long LastValue = -999;        //Storage of last position for comparison
int MidiLevel = 20;           //Measured Midi level, 20 used for starting
int LastMidiLevel = -99;      //Storage for last sended Midi Level, used for comparison

//Helper Signals for Switch
unsigned long lastDebounceTime = 0;   //Debounce time for Button
unsigned long debounceDelay = 50;     //Delay used for debouncing
int lastButtonState = LOW;            //Storage for last button state, used for comparison
int buttonState;                      //Measured Button state

//Helper SIgnals for Mute
bool Mute = false;                    //State of Mute
bool LastMute = false;                //Last state of Mute, used for comparison
int LevelBeforeMute = 0;              //Level before Mute, storing when Mute is activated




// Function for sending Midi Control Change commands
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
void controlChange(byte channel, byte control, byte value) 
{
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

//Setup Arduino controller
void setup()
{
  //Serial.begin(9600); //Only used for debugging
  pinMode(SW, INPUT);   //Define input of switch
  delay(1000);          //Small wait to make sure Midi is initialized
}

//Main Loop
void loop()
{
    //--------------------
    //Code handling Switch
    int reading = digitalRead(SW);      //Read switch value

    if (reading != lastButtonState)     //Check if different to last stored
    {
      lastDebounceTime = millis();      // reset the debouncing timer
    }

    if ((millis() - lastDebounceTime) > debounceDelay)    //Check if switch has debounced
    {
      if (reading != buttonState)       // if the button state has changed:
      {
        buttonState = reading;          //Store new values
        if (buttonState == LOW)         //If low, then switch Mute
        {
          //Serial.println("Switch betaetigt");
          Mute = !Mute;                 //Change Mute
        }
      }
    }
    lastButtonState = reading;          //Store hardware value

    //--------------------
    //Code handling Encoder
    CurrentValue = VolumeEncoder.read();   //Read Encoder value
    if (CurrentValue != LastValue)         //Has changed?
    {
      if(!Mute)                                 //Do not update Midi Level on Mute
      {
        if(CurrentValue > LastValue + 2)        //if changed more than 2, used for debouncing
        {
          MidiLevel++;                          //increment Midi Level
          LastValue = CurrentValue;          //Store new position of encoder 
        }
        else if(CurrentValue < LastValue - 2)
        {   
          MidiLevel--;                          //Decrement MIdi Level
          LastValue = CurrentValue;          //Store new position of encoder 
        }
        MidiLevel = constrain(MidiLevel, 0, 127);   //Make sure only 0-127 is used for Midi Level
      }
    }

    //--------------------
    //Code handling Mute
    if(Mute != LastMute)              //Mute changed
    {
      LastMute = Mute;                //Store new state
      if(Mute)                        //Mute is now active
      {
        LevelBeforeMute = MidiLevel;  //Store current level
        MidiLevel = 0;                //Set to 0 for quiet
      }
      else                            //Mute is inactive
      {
        MidiLevel = LevelBeforeMute;  //Restore level before Mute
      }
    }

    //--------------------
    //Code handling Midi
    if(MidiLevel!= LastMidiLevel)                       //Level has changed
    {
      LastMidiLevel = MidiLevel;                        //Store new level
      controlChange(MidiChannel, MidiCC, MidiLevel);    //Send CC Command
      MidiUSB.flush();                                  //Flush midi to send message
      //Serial.println(MidiLevel);
    }

}
