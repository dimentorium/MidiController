import time
import board
import busio
import trill

print("Starting Trill Bars")

faders = []
transport = None

def millis():
    return int(time.monotonic() * 1000)

TOUCHLENGTH = 150   #length when a touch is recognized
TAPLENGTH = 40      #minimum length for a tap

NoTouch = 0
Primary = 1
SingleTap = 2
Secondary = 3
BankSwitch = 4
ModeSwitch = 5
DoubleTap = 6

class Taps:    
    def __init__(self):
        self.___Taps = []      #millis time when typ has to be removed
        self.reset()
    
    def add(self):
        self.___Taps.append(millis() + 500);      #store new tap with millis plus 500

    def check(self):
        if len(self.___Taps) > 0:                           #is a tap registered
            if(millis() > self.___Taps[0]):                 #has time run out
                self.___Taps.pop(0)                         #remove first element

    def number(self):
        return len(self.___Taps)

    def reset(self):                            #reset class
        self.___Taps = []


class Trill_FaderXT:
    def __init__(self, i2c, address, fadernumber):
        #print("Initializing:", address)
        self.trillbar = trill.Bar(i2c, address)
        #self.trillbar.set_noise_threshold(250)
        #self.trillbar.set_scan_settings(0, 16)

        self.raw = 0            #raw value read from bar
        self.constrained = 0    #value normalized to midi
        self.readval = 0        #value read from trill bar
        self.touchesread = 0    #touches read from trillbar
        self.toucheslastread = 0    #stored touches from last reading     
        self.tapvalue = 0           #value of last tap

        self.tstate = NoTouch                                       #state of fader to be signalled
        self.touch_start = 0                                        #when did touch start   
        self.touch_length = 0                                       #length of current touch
        self.taps = Taps()
        self.touch_wait = False                                     #flag to wait before new touch
        self.fadernumber = fadernumber

    def get_state(self):
        self.taps.check();                        #check for taps
        self.trillbar.read();                     #read hw value
        self.touchesread = self.trillbar.number_of_vertical_touches()

        if(self.touch_wait):                                        #has there been a touch, then ignore everything until wait is over
            if(self.touchesread == 0):
                self.touch_wait = False
            return None

        if(self.touchesread == 0):                                              #no touch has been found
            self.tstate = NoTouch;                                          #reset state
            if(self.touch_start > 0):                                       #in last call there has been a touch
                #print("Touch End detected")
                self.touch_length = millis() - self.touch_start             #calculate touch length
                #print(self.touch_length)
                if(self.touch_length > TAPLENGTH and self.touch_length < TOUCHLENGTH):      #long enough for a tap
                    #print("New Tap detected") 
                    self.taps.add();                                               #register tap
                    if(self.taps.number() >= 2):                                   #double tap found
                        self.tstate = DoubleTap
                        self.taps.reset()
                        #print("Double Tap detected") 
                    else:                                                          #single tap found
                        self.tstate = SingleTap
            self.touch_start = 0
            
        elif(self.touchesread > 0 and self.touch_start == 0):                          #might be a new touch
            self.touch_start = millis()                                            #store current time
            self.tapvalue = self.trillbar.vertical_touches[0].location                      #store value for potential tap
            self.toucheslastread = self.touchesread                                        #store number of touches
        elif(self.touchesread > 0 and self.touchesread != self.toucheslastread):               #number of touches changed during initial wait
            self.touch_start = 0
        elif(self.touchesread > 0 and (millis() - self.touch_start) >= TOUCHLENGTH):   #touch longer than delay, so signal it to caller
            #print("New Slide detected")
            if(self.tstate == NoTouch):                                            #no touch has been signalled before
                if self.touchesread == 1:
                    self.tstate = Primary
                elif self.touchesread == 2:
                    if((millis() - self.touch_start) >= 2*TOUCHLENGTH):            #2 finger slide should wait longer before signalling
                        self.tstate = Secondary
                        #print("New Sec Slide detected")
                elif self.touchesread == 3:
                    if((millis() - self.touch_start) >= 2*TOUCHLENGTH):            #3 finger slide should wait longer before signalling
                        self.tstate = BankSwitch
                        self.touch_wait = True
                elif self.touchesread == 4:
                    if((millis() - self.touch_start) >= 2*TOUCHLENGTH):            #4 finger slide should wait longer before signalling
                        self.tstate = ModeSwitch
                        self.touch_wait = True
                else:
                    pass
        return self.tstate


    def get_value(self):
        if self.tstate == Primary:
            self.raw = self.trillbar.vertical_touches[0].location                                                                 #read touch value as raw data
            self.constrained = (3100 - self.raw)/23                                                                           #calculate midi value
            self.readval = (int)(min(max(self.constrained, 0), 127))
        elif self.tstate == Secondary:
                if self.trillbar.number_of_vertical_touches() == 2:
                    self.raw = (self.trillbar.vertical_touches[0].location  + self.trillbar.vertical_touches[1].location ) / 2                   #calculate average of two fingers
                    self.constrained = (3100 - self.raw)/23
                    self.readval = (int)(min(max(self.constrained, 0), 127))
        elif self.tstate == BankSwitch:
                self.raw = (self.trillbar.vertical_touches[0].location  + self.trillbar.vertical_touches[1].location  + self.trillbar.vertical_touches[2].location ) / 3      #calculate average of all three fingers
                if(self.raw > 1600):
                    self.readval = 0                                                                                                             #only output 0 or 1 if upper or lower
                else:
                    self.readval = 1
        elif self.tstate == SingleTap or self.tstate == DoubleTap:
            if(self.tapvalue > 1600):
                self.readval = 0                                                                     #For tap only output 0 or 127 
            else:
                self.readval = 1
        elif self.tstate == ModeSwitch:                                                              #no value for modeswitch
            self.readval = 0
        else:
            pass
        return self.readval


def init():
    global transport, faders
    print("Initializing Trill bars")
    i2c = busio.I2C(board.GP13, board.GP12, frequency=400000)
    transport = Trill_FaderXT(i2c, 0x20, 0) #Transport
    faders.append(Trill_FaderXT(i2c, 0x28, 0)) #Faders
    faders.append(Trill_FaderXT(i2c, 0x21, 1))
    faders.append(Trill_FaderXT(i2c, 0x22, 2))
    faders.append(Trill_FaderXT(i2c, 0x23, 3))
    print("Trill Bars initialized")