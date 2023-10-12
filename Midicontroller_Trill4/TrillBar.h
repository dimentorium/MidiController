/**
  Midicontroller Trill 4
  Name: Trillbar.h
  Purpose: Code for reading Trill bars with extended functions

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/

#include "Arduino.h" 
#include <Trill.h>

const unsigned long TOUCHLENGTH = 150;          //length when a touch is recognized
const unsigned long TAPLENGTH = 40;             //minimum length for a tap

/**
 * @brief enum for signalling current state of the trillbar
 * 
 */
enum TouchState {
  none,
  Primary,
  SingleTap,
  Secondary,
  BankSwitch,
  ModeSwitch,
  DoubleTap
};


/**
 * @brief enum for storing current touchmode
 * 
 */
enum TouchMode{
    NoTouch,
    Tap,
    Touch
};

/**
 * @brief class for counting number of taps
 * 
 */
class Taps{
    private:
    int count;              //count number of taps
    unsigned long tp[5];    //millis time when typ has to be removed

    public:
    //reset the counter
    Taps(){
        reset();
    }

    
    void add(){
        tp[count] = millis() + 500;     //store new tap with millis plus 500
        count++;                        //increase tap counter
    }

    void check(){
        if(tp[0] > 0){                          //is a tap registered
            if(millis() > tp[0]){               //has time run out
                for (size_t i = 0; i < 4; i++)  //move all taps up
                {
                    tp[i] = tp[i+1];
                }
                count--;                        //rdecrease tap count
            }
        }
    }

    int number(){                               //return number of counts
        return count;
    }

    void reset(){                               //reset class
        for (size_t i = 0; i < 5; i++)          //set all elements to zero
        {
            tp[i] = 0;
        }
        count = 0;                              //set count to zero
    }
};

/**
 * @brief main class for extended trill bas
 * 
 */
class Trill_Fader {
    private:
    Trill trill_bar;                //access to hardware
    int readval;                    //value read from trill bar
    u_int8_t touches;               //touches read from trillbar
    u_int8_t touchnumber;           //stored touches from last reading         
    unsigned long touch_start;      //when did touch start     
    unsigned long touch_length;     //length of current touch
    bool touch_wait;                //flag to wait before new touch
    TouchState tstate;              //state of fader to be signalled
    int raw;                        //raw value read from bar
    int constrained;                //value normalized to midi
    int tapvalue;                   //value of last tap
    Taps tp;                        //storage for taps
    

    public:
    /**
     * @brief initialize trill bar
     * 
     * @param address address of trill bar from x20-x27
     */
    void init(u_int8_t address){
        int ret = trill_bar.setup(Trill::TRILL_BAR, address);   //initialize hardware       
        trill_bar.setNoiseThreshold(250);                       //configure noise level high (max 255)
        //trill_bar.setScanSettings(0, 16);                     //configure scan settings, not needed
        tstate = TouchState::none;                              //no touch
        touch_start == 0;                                       //reset start flag
        tp.reset();                                             
    }

    /**
     * @brief Get the state object, main function to be called in loop
     * 
     * @return TouchState current state of the trill bar
     */
    TouchState get_state(){
        tp.check();                             //check for taps
        trill_bar.read();                       //read hw value
        touches = trill_bar.getNumTouches();    //get number of touches

        if(touch_wait){                         //has there been a touch, then ignore everything until wait is over
            if(touches == 0){
                touch_wait = false;
            }
            return none;
        }        

        if(touches == 0){                                                   //no touch has been found
            tstate = none;                                                  //reset state
            if(touch_start > 0){                                            //in last call there has been a touch
                //Serial.println("Touch End detected");
                touch_length = millis() - touch_start;                      //calculate touch length
                //Serial.println(touch_length);
                if(touch_length > TAPLENGTH && touch_length < TOUCHLENGTH){ //long enough for a tap
                    //Serial.println("New Tap detected"); 
                    tp.add();                                               //register tap
                    if(tp.number() >= 2){                                   //double tap found
                        tstate = DoubleTap;
                        tp.reset();
                        //Serial.println("Double Tap detected"); 
                    }else{                                                  //single tap found
                        tstate = SingleTap;
                    }
                }
            }
            touch_start = 0;
            
        }else if(touches > 0 && touch_start == 0){                          //might be a new touch
            touch_start = millis();                                         //store current time
            tapvalue = trill_bar.touchLocation(0);                          //store value for potential tap
            touchnumber = touches;                                          //store number of touches
        }else if(touches > 0 && touches != touchnumber){                    //number of touches changed during initial wait
            touch_start = 0;
        }else if(touches > 0 && (millis() - touch_start) >= TOUCHLENGTH){   //touch longer than delay, so signal it to caller
            //Serial.println("New Slide detected");
            if(tstate == none){                                             //no touch has been signalled before
                switch (touches)
                {
                case 1:
                    tstate = Primary;
                    break;
                case 2:
                    if((millis() - touch_start) >= 2*TOUCHLENGTH){          //2 finger slide should wait longer before signalling
                        tstate = Secondary;
                    }
                    break;
                case 3:
                    if((millis() - touch_start) >= 2*TOUCHLENGTH){          //3 finger slide should wait longer before signalling
                        tstate = BankSwitch;
                        touch_wait = true;
                    }
                    break;
                case 4:
                    if((millis() - touch_start) >= 2*TOUCHLENGTH){          //4 finger slide should wait longer before signalling
                        tstate = ModeSwitch;
                        touch_wait = true;
                    }
                    break;
                default:
                    break;
                }
            }
        }

        return tstate;
    }

    /**
     * @brief Get the value of the bar
     * 
     * @return int MIDI value from fader 
     */
    int get_value()
    {
        switch (tstate) {
            case Primary:
                raw = trill_bar.touchLocation(0);                                                                   //read touch value as raw data
                constrained = (3100 - raw)/23;                                                                      //calculate midi value
                readval = constrain(constrained, 0, 127);
                break;
            case Secondary:
                raw = (trill_bar.touchLocation(0) + trill_bar.touchLocation(1)) / 2;                                //calculate average of two fingers
                constrained = (3100 - raw)/23;                                                                      //calculate midi value
                readval = constrain(constrained, 0, 127);
                break;
            case BankSwitch:
                raw = (trill_bar.touchLocation(0) + trill_bar.touchLocation(1) + trill_bar.touchLocation(2)) / 3;   //calculate average of all three fingers
                if(raw > 1600) { readval = 0;}                                                                      //only output 0 or 1 if upper or lower
                else {readval = 1;}
                break;
            case SingleTap:
            case DoubleTap:
                if(tapvalue > 1600) { readval = 0;}                                                                 //For tap only output 0 or 127 
                else {readval = 1;}
            case ModeSwitch:                                                                                        //no value for modeswitch
            default:
                break;
        }
        return readval;
    }
};