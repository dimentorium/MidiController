#include "Arduino.h" 
/**
  Midicontroller Trill 4
  Name: Looptimer.h
  Purpose: Helper for delaying loop to a fixed frequency

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/


/**
 * @brief helper class for keeping track of looptime and delaying accordingly, keep processor not too busy
 * 
 */
class LoopTimer{
    private:
    unsigned long starttime = 0;            //when did loop start
    unsigned long looptime = 0;             //end of loop
    unsigned long delaytimems = 0;          //max number of loop delay

    public:
    /**
     * @brief initialize looptimer
     * 
     * @param waittime time in ms to set as loopdelay
     */
    void start(unsigned long waittime)      
    {
        starttime = millis();
        delaytimems = waittime;
    }

    /**
     * @brief check looptime and wait according to desired delay
     * 
     */
    void evaluate()
    {
        looptime = millis() - starttime;        //check how long loop took
        //Serial.println(looptime);
        if(looptime < delaytimems){             //looptime under desired delay
            delay(delaytimems-looptime);        //wait until delay is over
        }
        else
        {
            delay(delaytimems);                 //wait full delay
        }
    }
};