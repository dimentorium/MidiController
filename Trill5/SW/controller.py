import colors

MODE_CC = 0
MODE_FADER = 1

FADERSTATE_NORMAL = 0
FADERSTATE_SOLO = 1
FADERSTATE_MUTE = 2

UPDATETYPE_PRIMARY = 0
UPDATETYPE_SECONDARY = 1
UPDATETYPE_SWITCHUP = 2
UPDATETYPE_SWITCHDOWN = 3

___faders = []
___mode = MODE_CC
___bank = 0

class simplefader:
    def __init__(self, cc) -> None:
        self.fadercc = cc
        self.value = 0
        self.forecolor = colors.BLUE
        self.backcolor = colors.LIGHTBLUE
        self.channel = 0
        self.fadermode = FADERSTATE_NORMAL
        self.newstateavailable = True
        self.secondaryavailable = False

    def update_cc(self, channel, cc, value):
        if self.channel == channel and self.fadercc == cc:
            self.value = value
            self.newstateavailable = True

    def update(self, value, updatetype):
        self.value = value
        self.newstateavailable = True

class extfader(simplefader):
    def __init__(self, cc1, cc2, ccup, ccdown) -> None:
        super().__init__(cc1)
        self.secvalue = 0
        self.secfadercc = cc2
        self.switchupcc = ccup
        self.switchdowncc = ccdown
        
        self.channel = 5

        self.forecolor = colors.BLUE
        self.backcolor = colors.GREEN
        self.secfadercolor = colors.PURPLE
        self.switchupcolor = colors.RED
        self.switchdowncolor = colors.YELLOW

    def update_cc(self, channel, cc, value):
        if self.channel == channel:
            self.newstateavailable = True
            if self.fadercc == cc:
                self.value = value
                self.secondaryavailable = False
            elif self.secfadercc == cc:
                self.secvalue = value
                self.secondaryavailable = True
            elif self.switchupcc == cc:
                if value != 0:
                    self.fadermode = FADERSTATE_SOLO
                else:
                    self.fadermode = FADERSTATE_NORMAL
            elif self.switchdowncc == cc:
                if value != 0:
                    self.fadermode = FADERSTATE_MUTE
                else:
                    self.fadermode = FADERSTATE_NORMAL
            else:
                pass

    def update(self, value, updatetype):
        self.newstateavailable = True
        if updatetype == UPDATETYPE_PRIMARY:
            self.value = value
            self.secondaryavailable = False
        elif updatetype == UPDATETYPE_SECONDARY:
            self.secvalue = value
            self.secondaryavailable = True
        elif updatetype == UPDATETYPE_SWITCHUP:
            if self.fadermode != FADERSTATE_SOLO:
                self.fadermode = FADERSTATE_SOLO
            else:
                self.fadermode = FADERSTATE_NORMAL
        elif updatetype == UPDATETYPE_SWITCHDOWN:
            if self.fadermode != FADERSTATE_MUTE:
                self.fadermode = FADERSTATE_MUTE
            else:
                self.fadermode = FADERSTATE_NORMAL
                

def init():
    global ___faders
    print("Initializing controller setup")
    ___faders.append(simplefader(11))
    ___faders.append(simplefader(1))
    ___faders.append(simplefader(21))
    ___faders.append(simplefader(102))

    for i in range(0, 32):
        ___faders.append(extfader(i, i+32, i+64, i+96))

def receive_cc(channel, ccnumber, value):
    global ___faders
    for fader in ___faders:
        fader.update_cc(channel, ccnumber, value)

def switch_mode(newmode):
    global ___mode
    ___mode = newmode
    for fader in ___faders:
        fader.newstateavailable = True
    print("Switch Mode:", ___mode)


def switch_bank(newbank):
    global ___bank, ___mode
    if ___mode == MODE_FADER:
        ___bank = newbank
        for fader in ___faders:
            fader.newstateavailable = True
        print("Switch Bank:", ___bank)

def get_fader(fadernumber):
    global ___faders, ___mode, ___bank
    return ___faders[(___mode * 4) + (___bank * 4) + fadernumber]

def set_fader(fadernumber, value, updatetype):
    global ___faders, ___mode, ___bank
    ___faders[(___mode * 4) + (___bank * 4) + fadernumber].update(value, updatetype)