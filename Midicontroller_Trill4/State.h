/**
  Midicontroller Trill 4
  Name: State.h
  Purpose: Code for keeping status of the DAW controller

  @author Philipp Noertersheuser
  @version 0.30 22/09/29
*/

#include "Arduino.h" 

/**
 * @brief Enum for signaling transport buttons
 * 
 */
enum TransportButtons{
    Start,
    Stop,
    Record,
    Cycle,
    Forward,
    Rewind
};

/**
 * @brief abstract class for a set of faders
 * 
 */
class Set{
    private:
    
    public:
    int Bank = 0;

    /**
     * @brief update the fader value
     * 
     * @param number number of fader in bank
     * @param function function of fader, primary or secondary
     * @param value value to update
     */
    virtual void update(int number, int function, int value) = 0;

    /**
     * @brief receive update via CC commands
     * 
     * @param channel received channel
     * @param ccnum received CC number
     * @param value received value
     */
    virtual void ReceiveCC(int channel, int ccnum, int value) = 0;

    /**
     * @brief read value of fader
     * 
     * @param number number of fader in bank
     * @param function number of function of fader
     * @return int stored value
     */
    virtual int Value(int number, int function) = 0;

    /**
     * @brief command to change fader bank of set
     * 
     * @param bank bank to switch to
     */
    void ChangeBank(int bank){ Bank = bank;}

    /**
     * @brief get On color for fader function
     * 
     * @param number number of fader in bank
     * @param function fader function to be used
     * @return LEDColors enum of the color
     */
    virtual LEDColors ColorOn(int number, int function) = 0;

    /**
     * @brief get Off color for fader function
     * 
     * @param number number of fader in bank
     * @param function fader function to be used
     * @return LEDColors enum of the color
     */
    virtual LEDColors ColorOff(int number, int function) = 0;

    /**
     * @brief Fader button, upper or lowe, pressed
     * 
     * @param number number of fader in bank
     * @param button upper or lower button
     */
    virtual void ButtonPress(int number, int button) = 0;
};

/**
 * @brief Set of controls for CC MIDI operation
 * 
 */
class Set_CC: public Set{
    private:
    int CCMessages[4] = {11, 1, 21, 102};                       //Expression, Volume, Vibrato, Custom
    int values[4];                                              //storage for four faders

    public:
    Set_CC()
    {
        for(int i=0; i<4; i++)                                     //reset all values
        {
            values[i] = 0;
        }
    }
    
    void update(int number, int function, int value)            //update CC value, no banks or secondary functions  
    {
        values[number] = value;
        controlChange(0, CCMessages[number], value);
    }

    virtual void ReceiveCC(int channel, int ccnum, int value)   //update CC value when received via MIDI
    {
        if(channel == 0)                                        //only update when channel 1 is received
        {
            switch (ccnum) {
                case 11:
                    values[0] = value;
                    break;
                case 1:
                    values[1] = value;
                    break;
                case 21:
                    values[2]  = value;
                    break;
                case 102:
                    values[3] = value;
                    break;
                default:
                    break;
            }
        }
    }

    int Value(int number, int function)                         //return value of CC fader
    {
        return values[number];
    }

    LEDColors ColorOn(int number, int function)                 //Faders have fixed On color
    {
        return LEDColors::blue;
    }

    LEDColors ColorOff(int number, int function)                 //Faders have fixed Off color
    {
        return LEDColors::lightblue;   
    }

    void ButtonPress(int number, int button)                    //no button press functions
    {}
};

/**
 * @brief Set of controls for DAW mixing, 32 channel strips with Volume, Pan, Solo and Mute
 * 
 */
class Set_Mix: public Set{
    private:
    int PrimaryValues[32];                                          //storage for 32 channel strips
    int SecondaryValues[32];
    bool Solo[32];
    bool Mute[32];
    
    
    public:
    Set_Mix()
    {
        for(int i=0; i<32; i++)                                     //reset all values
        {
            PrimaryValues[i] = 0;
            SecondaryValues[i] = 0;
            Solo[i] = false;
            Mute[i] = false;
        }
    }

    virtual void update(int number, int function, int value)        //update fader and storage
    {
        if(function == 0){                                          //update Volume
            PrimaryValues[(Bank * 4) + number] = value;
            controlChange(5, (Bank * 4) + number, value);           //send MIDI command
        }
        else{                                                       //Update Pan
            SecondaryValues[(Bank * 4) + number] = value;
            controlChange(5, (Bank * 4) + number + 32, value);      //Send Midi Command
        }
        
    }

    virtual void ReceiveCC(int channel, int ccnum, int value)       //Receive update from DAW
    {
        if(channel == 5)                                            //only receive on Channel 6
        {
            switch (ccnum) {
                case 0 ... 31:                                      //update volume
                    PrimaryValues[ccnum] = value;
                    break;
                case 32 ... 63:                                     //Update pan
                    SecondaryValues[ccnum - 32] = value;
                    break;
                case 64 ... 95:                                     //Update solo
                    Solo[ccnum - 64] = (value > 0);
                    break;
                case 96 ... 128:                                    //update Mute
                    Mute[ccnum - 96] = (value > 0);
                    break;
                default:
                    break;
            }
        }

    }

    int Value(int number, int function)                             //return current value of fader
    {
        if(function == 0){
            return PrimaryValues[(Bank * 4) + number];
        }
        else
        {
            return SecondaryValues[(Bank * 4) + number];
        }
    }

    LEDColors ColorOn(int number, int function)                     //determine color of fader
    {
        LEDColors ret = LEDColors::blue;                            //base color is blue
        if(Solo[(Bank * 4) + number]) {ret = LEDColors::lightred;}  //if in solo, show with red
        if(Mute[(Bank * 4) + number]) {ret = LEDColors::orange;}    //if in mute show as orange
        if(function == 1) {ret = LEDColors::purple;}                //for secondary function use purple
        
        return ret;
    }

    LEDColors ColorOff(int number, int function)                    //off color is fixed to green
    {
        LEDColors ret = LEDColors::lightgreen;
        return ret;   
    }
    
    void ButtonPress(int number, int button)                                            //button press on fader
    {
        if(button == 1){                                                                //if upper button, this is solo
            Solo[(Bank * 4) + number] = !Solo[(Bank * 4) + number];
            controlChange(5, (Bank * 4) + number + 64, Solo[(Bank * 4) + number]*127);
        }
        else {                                                                          //if upper button, this is mute
            Mute[(Bank * 4) + number] = !Mute[(Bank * 4) + number];
            controlChange(5, (Bank * 4) + number + 96, Mute[(Bank * 4) + number]*127);
        }
    }

    int numtochannel(int number){
        return (Bank * 4) + number;
    }

    
};

/**
 * @brief Class holding all sets for faders
 * 
 */
class Controls{
    private:
    Set *sets[2];               //sets stored in controller
    int Mode;                   //storage for current mode
    int Bank;                   //storage for current bank
    bool TransportState[6];     //state of transport buttons

    public:
    /**
     * @brief initialize class
     * 
     */
    void init(){
        sets[0] = new Set_CC;
        sets[1] = new Set_Mix;
        switch_mode(0);
    }

    /**
     * @brief update fader according to bank and mode
     * 
     * @param fadernumber number of fader in bank
     * @param function used function
     * @param value value for update
     */
    void update_control(int fadernumber, int function, int value)
    {
        sets[Mode]->update(fadernumber, function, value);               //call fader from current selected set
    }

    /**
     * @brief update control from CC command
     * 
     * @param channel received channel
     * @param ccnum received cc
     * @param value received value
     */
    void receive_cc(int channel, int ccnum, int value)
    {
        if(channel == 0){sets[0]->ReceiveCC(channel, ccnum, value);}        //CC0 is CC mode update
        else if(channel == 5){sets[1]->ReceiveCC(channel, ccnum, value);}   //CC5 is for the DAW mixing
    }

    /**
     * @brief Get the state of the fader for the LED
     * 
     * @param number number of fader
     * @param function function to be used
     * @return LEDState current state for updating LEDstrip
     */
    LEDState get_control(int number, int function)
    {
        return {sets[Mode]->Value(number, function), sets[Mode]->ColorOn(number, function), sets[Mode]->ColorOff(number, function)};
    }

    /**
     * @brief signal button press to faders
     * 
     * @param fadernumber number of fader in bank
     * @param button upper or lower button pressed
     */
    void button(int fadernumber, int button)
    {
        sets[Mode]->ButtonPress(fadernumber, button);
    }

    /**
     * @brief Switch bank to new value
     * 
     * @param newbank new bank to select
     */
    void switch_bank(int newbank){
        sets[Mode]->ChangeBank(newbank);
    }

    /**
     * @brief Switch mode of controller
     * 
     * @param newmode new selected mode
     */
    void switch_mode(int newmode){
        switch(newmode)
        {
            case 0:
            case 1:
                Mode = newmode;
                sets[Mode]->ChangeBank(0);             //make sure to reset bank
                break;
            defaul:
                break;
        }
    }

    /**
     * @brief Send messages when transport buttons are pressed
     * 
     * @param tp enum which button is pressed
     */
    void Transport(TransportButtons tp){
        TransportState[tp] = !TransportState[tp];                   //Store value in array
        switch (tp)
        {
            case Start:
                controlChange(15, 115, 127);                        //buttons should send 1 and 0 so it is interpreted correctly
                controlChange(15, 115, 0);
                break;
            case Stop:
                controlChange(15, 114, 127);
                controlChange(15, 114, 0);
                break;
            case Record:
                controlChange(15, 117, 127);
                controlChange(15, 117, 0);
                break;
            case Cycle:
                controlChange(15, 116, 127);
                controlChange(15, 116, 0);
                break;
            case Forward:
                controlChange(15, 113, TransportState[tp]*127);     //Forward rewind should send continuously
                break;
            case Rewind:
                controlChange(15, 112, TransportState[tp]*127);
                break;
            default:
                break;
        }
    }
};