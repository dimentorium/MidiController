/**
  Midicontroller Trill 4
  Name: Button.h
  Purpose: Code fopr debouncing buttons

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/

#include "Arduino.h" 

/**
 * @brief                   Class for pushbuttons
 * 
 */
class PushButton {
    private:
    unsigned long lastDebounceTime = 0;     //Debounce time for Button
    unsigned long debounceDelay = 50;       //Delay used for debouncing
    int lastButtonState = LOW;              //Storage for last button state, used for comparison
    int buttonState;                        //Measured Button state
    int button_pin;                         //Hardware pin for button

    public:
    /**
     * @brief               Init function for Pushbuttons, as it is hardware, the constructor should not be used
     * 
     * @param BUTTON_PIN    pin number for digital IO
     */
    void init(int BUTTON_PIN){
        pinMode(BUTTON_PIN, INPUT_PULLDOWN);        //enable pulldown for button
        button_pin = BUTTON_PIN;                    //store pin inside class
    }

    /**
     * @brief               Get the state of digital pin
     * 
     * @return true         if pin has debounced, is returned only once
     * @return false        if pin has not debounced
     */
    bool get_state(){
        int reading = digitalRead(button_pin);      //Read switch value
        bool output = false;

        if (reading != lastButtonState)                         //Check if different to last stored
        {
            lastDebounceTime = millis();                        // reset the debouncing timer
        }

        if ((millis() - lastDebounceTime) > debounceDelay)      //Check if switch has debounced
        {
            if (reading > buttonState)                          // was button pressed?
            {
                output = true;   
                //Serial.println(button_pin);
            }
            buttonState = reading;                              //Store new values
        }
        lastButtonState = reading;                              //Store hardware value
        return output;
    }
};