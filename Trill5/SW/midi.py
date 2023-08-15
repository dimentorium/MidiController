import usb_midi
import adafruit_midi  # MIDI protocol encoder/decoder library
from adafruit_midi.control_change import ControlChange

USB_MIDI_channel = 1  # pick your USB MIDI out channel here, 1-16
usb_midi = adafruit_midi.MIDI(
    midi_in=usb_midi.ports[0],
    midi_out=usb_midi.ports[1], 
    in_channel=(0, 5),
    out_channel=USB_MIDI_channel - 1
)

def init():
    global usb_midi
    print("Initializing USB MIDI")

def send_cc(ccnumber, value, midichannel):
    global usb_midi
    usb_midi.send(ControlChange(ccnumber, value), midichannel)

def receive_cc():
    global usb_midi
    received = usb_midi.receive()
    if isinstance(received, ControlChange):
        return received