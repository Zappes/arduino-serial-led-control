Control an LED with commands over a serial line
===============================================

What Is this?
-------------
This is a simple Arduino sketch that shows how the lighting level of an LED attached to a PWM pin can be controller using text commands that are issued over a serial line. You can attach a bluetooth module to that serial line in order to get wireless control over the LED lighting levels. This was my first Arduino project after the notorious "blink", so please excuse the triviality of the example. :)

Hardware
--------
For the example to work, you need two things:

* A serial line that's connected to some kind of terminal emulator. For debugging I used the built-in serial port over USB, but then I switched to a bluetooth module, which was really simple and a lot more satisfying to toy around with. For Linux users, I recommend using "minicom" as a terminal emulator. For Windows and Mac OS X, I recommend switching to Linux.
* An LED connected to a PWM-capable output pin of your arduino. Don't forget the 220 Ohm resistor.

The source code assumes that you have an Arduino Mega with a bluetooth module attached to seriall 1 (pins 18/19). If you habve an UNO or want to communicate using the USB conenction, locate this line:

    #define COMPORT Serial1

Change that line to use "Serial" instead of "Serial1" and you're set.

Supported Commands
------------------
After establishing a connection to the chip, e.g. using the serial monitor in the Arduino IDE, you can issue the following commands:

Command          | Result
-----------------|------------------------------------------------------------------------------------------
ON               | Turns the LED completely on, i.e. PWM level 255.
OFF              | Turns the LED off, i.e. PWM level 0.
LEVEL&nbsp;level | Sets the PWM level to the specified value between 0 and 255.
FADE&nbsp;level&nbsp;delay | Fades the LED from the current to the specified level with the given delay between steps.
STATUS           | Displays the current level of the LED.

All commands are case-insensitive. The arguments may be omitted, their default value is 0. 


Aknowledgements
---------------
Thanks to Pulsar256 (https://github.com/pulsar256) for infecting me with the Arduino bug. He also donated a bluetooth module to me and told me how to parse String commands in C, for both I'm very grateful. The idea of replacing spaces with null bytes in the input in order to make parsing a bit easier was mine, though. ;)

