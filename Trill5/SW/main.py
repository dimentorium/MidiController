import time
import led
import bars
import controller
import midi
import updatetype

time.sleep(0.5)
print("Initializing Trill 5 V140823")
led.init()
bars.init()
controller.init()
midi.init()



while True:
    starttime = time.monotonic()        #startime for calculating final wait

    #read hardware values from trill bars and update controller
    for fader in bars.faders:
        state = fader.get_state()
        if state != updatetype.NOUPDATE:
            controller.set_fader(fader.fadernumber, fader.get_value(), state)

    #update LEDs with values from controller set
    for i in range(0, 4):
        fader = controller.get_fader(i)
        if fader.newstateavailable:
            led.draw_fader(i, fader)
            midi.send_cc(fader.fadercc, fader.value, fader.channel)
            fader.newstateavailable = False

    stoptime = time.monotonic()         #read stoptime, compare with start and calculate wait
    waittime = 0.05 - (stoptime - starttime)
    #print(waittime)
    if waittime > 0:
        time.sleep(waittime)
