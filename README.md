AVRMIDI-TRIGGER
==========

This is an implementation of a simple MIDI to Trigger converter using a Atmel Attiny2313 microprocessor. Each Note-On Event in a given MIDI Note Range is converted into a Trigger Signal of 5 V.

The current Note Range is 8 Notes from middle C (MIDI-Channel 5 - see Makefile for more info).
I was able to reuse a lot of sourcecode from my AVR MIDI-CV converter here... which is nice.

It also supports multitrigger/flames via velocity: there are 16 pre-defined flame pattern and velocity of 127 gives you a simple trigger.

If you want to see the MIDI to Trigger converter in action (also hearing the flame pattern): [video of the SDSV clone i built](https://www.youtube.com/watch?v=ALU4drFba9A)

Schematic
=========

![schematic picture](https://github.com/oberling/AVRMIDI-TRIGGER/blob/master/pcb/midi_trigger_schem.png)

Teststatus
==========

complete.

The code has been tested with my SDS5 clone and works fine.
The code has been tested with an oscilloscope and seems to work fine.

