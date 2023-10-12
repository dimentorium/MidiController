/**
  Midicontroller Trill 4
  Name: LED.h
  Purpose: Code for controlling LED strips

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/

#include "Arduino.h" 
#include <Adafruit_NeoPixel.h>

/**
 * @brief enum for colors to select
 * 
 */
enum LEDColors{
    black,
    blue,
    lightblue,
    red,
    lightred,
    green,
    lightgreen,
    yellow,
    orange,
    purple
};

/**
 * @brief array with color codes according to enum
 * 
 */
int Colors[10][3] = {
    {0, 0, 0},      //black
    {0, 0, 255},    //blue
    {0, 0, 20},     //lightblue
    {255, 0, 0},    //red
    {20, 0, 0},     //lightred
    {0, 255, 0},    //green
    {0, 20, 0},     //lightgreen
    {255, 255, 0},  //yellow
    {255, 127, 0},  //orange 
    {255, 0, 255}   //purple
};

/**
 * @brief structure for storing status of LEDbar
 * 
 */
struct LEDState{
   int value;
   LEDColors on;
   LEDColors off;
};

/**
 * @brief main class for controlling one LED strip
 * 
 */
class LedStrip {
    private:
    int NumPixels;                  //number of pixels in strip
    LEDState LastState;             //last signalled state to hardware
    Adafruit_NeoPixel ledpixel;     //access to hardware

    public:
    /**
     * @brief init function for starting communication
     * 
     * @param pin digital IO from which strip is controlled
     * @param numpixels number of pixels
     */
    void init(int pin, int numpixels){
        NumPixels = numpixels;
        ledpixel = Adafruit_NeoPixel(numpixels, pin, NEO_GRB + NEO_KHZ800);     //initialize neopixel communication
        ledpixel.begin();
        LastState = {-1, LEDColors::black, LEDColors::black};                   //reset internal state so it gets overwritten
    }

    /**
     * @brief Set state of the LED strip
     * 
     * @param state new state of strip
     * @param force update hardware even if same state
     */
    void SetBar(LEDState state, bool force = false)             
    {
        //Serial.println(state.value);
        if(CompareState(state) || force)                                                                            //update only if new state or forced
        {
            for(int j=0;j<NumPixels;j++)                                                                            //loop over all pixels
            {
                if(state.value >= ((NumPixels-j)* 9))                                                               //check if pixel should be on or off, 127/14 is 9
                {
                    ledpixel.setPixelColor(j, Colors[state.on][0], Colors[state.on][1], Colors[state.on][2]);
                }
                else
                {
                    ledpixel.setPixelColor(j, Colors[state.off][0], Colors[state.off][1], Colors[state.off][2]);
                }
                
            }
            LastState = state;                                                                                      //store new state
            ledpixel.show();                                                                                        //send the updated pixel color to the hardware.
        }
    }

    /**
     * @brief Update a single point on the strip
     * 
     * @param state state of the point
     */
    void SetPoint(LEDState state)
    {
        double point = (1 - state.value / 127.0) * 14.0;                                                //calculate which pixel to set
        int rounded = (int)round(point);
        ledpixel.setPixelColor(rounded, Colors[state.on][0], Colors[state.on][1], Colors[state.on][2]); //set color of pixel
        ledpixel.show();
    }

    /**
     * @brief Helper function to compare new and last state
     * 
     * @param newstate state to compare
     * @return true 
     * @return false 
     */
    bool CompareState(LEDState newstate)
    {
        return (newstate.value != LastState.value) || (newstate.on != LastState.on) || (newstate.off != LastState.off);
    }
};