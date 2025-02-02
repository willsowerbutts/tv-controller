# TV IR Translator

This is a small bit of hardware which allows me to control my stereo system
from my LG Television remote control. The Television audio is played through
the stereo.

With this hardware, the volume/mute buttons on the TV remote control adjust the
amplifier volume instead of the TV's internal speakers, and the amplifier power
turns on and off automatically as the TV is turned on or off.

Using a single remote control makes the system significantly easier to control
for young children, guests, and IT consultants.

This repository contains a schematic and source code should you wish to build
something similar.

## My Setup

**Television**: The TV is an LG OLED (C4 model). The TV has a TOSLINK optical
output which is connected to my DAC. The TV also has an IR Blaster port which
is connected to my control hardware.

**DAC**: The DAC is a [Topping E70](https://www.toppingaudio.com/product-item/e70) 
which operates as a combined DAC and a pre-amplifier (volume control). The
balanced XLR audio outputs of the DAC are connected to my power amplifier. The
DAC has a 12V trigger output which is connected to my control hardware.

**Amplifier**: The amplifier is a dual mono power amplifier that I built using 
[Hypex UcD modules](https://www.hypex.nl/products/amplifier-families/ucd-family/).
This amplifies the output from the DAC/pre-amp and drives the speakers.

**Controller**: My controller consists of an Arduino nano clone (ATmega328P), a
quad-channel optocoupler (only 3 channels are used), a few resistors, and an
SPST relay with a 5V coil plus a diode and NPN transistor to drive it. It runs
the software contained in this repository, which is a simple C program (it does
not use the Arduino framework).

## The Problem

The TV can be configured to control the volume of various soundbars by sending
the IR codes they expect for "Volume Up", "Volume Down" and "Mute". However the
Topping E70 DAC/pre-amp I have is not a model the TV supports and there is no
way to program the TV with your own IR codes.

The TV can send power toggle commands over IR to a soundbar, however it only
does this when you select a menu option on the TV. It does not send the power
toggle command when the TV itself is powered on or off.

Both of these problems lead to me developing my own solution.

## The Solution

My hardware receives and decodes IR signals from the TV, then uses an IR LED to
transmit the equivalent volume/mute control codes to the Topping E70. When the
TV sends IR codes they are normally sent from the remote control itself, but if
a device is connected to the IR Blaster port (a 3.5mm socket on the TV) it uses
that instead. My hardware connects to the IR blaster port using an optocoupler.

The E70 DAC monitors the TOSLINK signal from the TV, turning itself on the
TOSLINK signal is present and off after it is absent for a few seconds. The E70
signals its power state using a 12V trigger output. My hardware monitors this
12V trigger output via an optocoupler and controls the amplifier power in
response.

My amplifier has a single momentary push button to turn it on or off, and an
LED which lights up when it is powered on. My hardware has a small relay wired
in parallel with the button, which it uses to synthesise button pushes, and it
monitors the LED state using an optocoupler to determine if the amplifier is on
or off. I modified the amplifier slightly to add some internal wiring and a
4-pin port making those signals availavble.

I bought an inexpensive IR blaster LED on Amazon. This has an IR LED and a
3.5mm plug on a length of two-core cable. I cut this in half, using the half
with the plug to connect to the IR Blaster output on the TV to my hardware, and
the LED half as the output side to signal to the E70.

## TV Configuration

The TV should be configured to disable the internal speakers and send all sound
to the TOSLINK optical output in PCM format. This can be done in the Sound
settings menu on the TV.

The TV should be configured to control the volume of a soundbar. This can be
done in the External Devices menu on the TV. From the list of supported devices
I chose "Marantz", but any brand which uses RC5 IR codes should work fine. 

## Building and testing

The schematic for the circuit can be [downloaded in PDF
format](/kicad/schematic.pdf) or viewed below. The relay used must have a 5V
coil. For the resistors calculate appropriate values, or just guess and use
about 1K. My IR LED is centimeters away from the IR receiver in the DAC, but if
you need more distance/power you could add another NPN transistor to drive it.

![Schematic](/kicad/schematic.png)

I soldered the components together on a bit of through-hole prototyping board. 
The wires to the various equipment connect to push-in terminal blocks. The
board is powered over USB from a spare phone charger.

![Board Photo](/board-photo.jpg)

Unless you are using exactly the same equipment you will likely need to modify
the software to suit your setup. Start with the code in `main.c`, in the
`main()` function you'll find the main loop where you can remove functions you
don't need. In the `check_infrared_input()` function you can change which RC5
codes the device listens for, and in the `vol_*()` functions you can change
which IR codes are transmitted.

If you are connecting a different model of DAC/pre-amplifier you will likely
have to modify the IR control codes that are sent, and maybe the transmission
protocol. I looked up the codes for the Topping device online and wrote my own
NEC protocol IR code transmitter routine, which turned out to be much simpler
than I had feared. The code is in `necir.c`.

The software uses the fantastic [AVR-LibC](https://github.com/avrdudes/avr-libc/) 
and avr-gcc compiler. On Debian (and similar Linux distributions like Ubuntu)
you can install the required software easily with `apt install avr-libc`. Build
the software with `make`. If your board is connected to `/dev/ttyUSB0` then you
can easily program it with `make program`, which will also run a terminal to talk
to the controller after programming it. 

The controller will report over serial when it receives an IR code on the
input. For testing you can also send characters over serial to test out the
various functions:

| Key | Function |
| --- | -------- |
| + | Transmit IR: Volume Up |
| - | Transmit IR: Volume Down |
| M | Transmit IR: Mute |
| N | Turn amplifier on |
| F | Turn amplifier off (immediately) |
| D | Turn amplifier off (delayed) |

## A note on Topping E70 firmware

I found the Topping E70 suffered from frequent audio drop-outs when connected
to the optical output of the LG TV. The solution was to upgrade the E70
firmware to 
[version 1.04](https://drive.google.com/file/d/1Iwp4hhrM1GvptI9PpsgWnCy6yguFDcb6/view)
and then, in the DAC settings (power on while pressing the volume knob)
increase the "PLL Bandwidth" parameter to 10 (setting "B-10").
