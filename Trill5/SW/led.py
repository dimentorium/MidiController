import time
import board
import neopixel
import colors
import controller


print("Loading LED library")
# GPIO-Pin für WS2812
pin_np = [0,1,2,3]

# Anzahl der LEDs
NUMLEDS = 14

# Helligkeit: 0 bis 255
brightness = 10

# Geschwindigkeit (Millisekunden)
speed = 0.05

# Initialisierung WS2812/NeoPixel
np0 = neopixel.NeoPixel(board.GP0, NUMLEDS, auto_write=False)
np1 = neopixel.NeoPixel(board.GP1, NUMLEDS, auto_write=False)
np2 = neopixel.NeoPixel(board.GP2, NUMLEDS, auto_write=False)
np3 = neopixel.NeoPixel(board.GP3, NUMLEDS, auto_write=False)
np4 = neopixel.NeoPixel(board.GP15, NUMLEDS, auto_write=False)
np = [np0, np1, np2, np3, np4]

def init():
    print("Testing LEDs")
    draw_off()
    time.sleep(0.5)
    for i in range (NUMLEDS):
        for strip in np:
            # Nächste LED einschalten
            strip[i] = colors.LIGHTBLUE
            strip.write()
        time.sleep(speed)
    draw_off()
    print("LED Test finished")

def draw_off():
    for strip in np:
        strip.fill((0,0,0))
        strip.write()

def draw_pixel(strip, lednumber, color):
    #np[strip].fill((0,0,0))
    np[strip][NUMLEDS - 1 - lednumber] = color
    #np[strip].write()

def draw_bar(strip, lednumber, color, colorback = colors.BLACK):
    #print(lednumber)
    for i in range (NUMLEDS):
        if(i <= lednumber):
            np[strip][NUMLEDS - 1 - i] = color
        else:
            np[strip][NUMLEDS - 1 - i] = colorback
    #np[strip].write()

def draw_buttons(strip):
    for i in range (NUMLEDS):
        if i < 2:
            np[strip][NUMLEDS - 1 - i] = colors.YELLOW
        elif i < 5:
            np[strip][NUMLEDS - 1 - i] = colors.RED
        elif i < 8:
            np[strip][NUMLEDS - 1 - i] = colors.GREEN
        elif i < 11:
            np[strip][NUMLEDS - 1 - i] = colors.LIGHTBLUE
        else:
            np[strip][NUMLEDS - 1 - i] = colors.YELLOW
    #np[strip].write()

def draw_fader(ledstrip, fader):
    if fader.fadermode == controller.FADERSTATE_NORMAL:
        draw_bar(ledstrip, (int)(fader.value/9), fader.forecolor, fader.backcolor)  
    elif fader.fadermode == controller.FADERSTATE_MUTE:
        draw_bar(ledstrip, (int)(fader.value/9), fader.forecolor, fader.switchdowncolor)
    elif fader.fadermode == controller.FADERSTATE_SOLO:
        draw_bar(ledstrip, (int)(fader.value/9), fader.forecolor, fader.switchupcolor)

    if fader.secondaryavailable:
            draw_pixel(ledstrip, (int)(fader.secvalue/9), fader.secfadercolor)
    np[ledstrip].write()
