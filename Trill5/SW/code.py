import time
import led
import bars
import colors
import controller
import midi

time.sleep(0.5)
print("Initializing Trill 5 V140823")
led.init()
bars.init()
controller.init()
midi.init()



while True:
    starttime = time.monotonic()
    for fader in bars.faders:
        state = fader.get_state()
        if state == bars.Primary:
            controller.set_fader(fader.fadernumber, fader.get_value(), controller.UPDATETYPE_PRIMARY)
        elif state == bars.Secondary:
            controller.set_fader(fader.fadernumber, fader.get_value(), controller.UPDATETYPE_SECONDARY)
        elif state == bars.DoubleTap:
            if fader.get_value() == 1:
                controller.set_fader(fader.fadernumber, fader.get_value(), controller.UPDATETYPE_SWITCHUP)
            else:
                controller.set_fader(fader.fadernumber, fader.get_value(), controller.UPDATETYPE_SWITCHDOWN)
        elif state == bars.BankSwitch:
            controller.switch_bank((fader.fadernumber * 2) + fader.get_value())
        elif state == bars.ModeSwitch:
            controller.switch_mode(fader.fadernumber)
        else:
            pass

    for i in range(0, 4):
        fader = controller.get_fader(i)
        if fader.newstateavailable:
            led.draw_fader(i, fader)
            midi.send_cc(fader.fadercc, fader.value, fader.channel)
            fader.newstateavailable = False

    stoptime = time.monotonic()
    waittime = 0.05 - (stoptime - starttime)
    #print(waittime)
    if waittime > 0:
        time.sleep(waittime)
